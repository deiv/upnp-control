/*
 * @file httpd.c
 *
 * @brief HTTP server
 * @author David Suárez
 * @date Mon, 17 Dec 2018 13:36:48 +0100
 *
 * Copyright (c) 2018 David Suárez.
 * Email: david.sephirot@gmail.com
 *
 * This file is part of upnp-control.
 *
 * upnp-control is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * upnp-control is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with upnp-control.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <stdio.h>

#define LOG_MODULE_NAME httpd
#include <logging/log.h>
LOG_MODULE_REGISTER(LOG_MODULE_NAME, LOG_LEVEL_DBG);

#include <zephyr.h>
#include <net/net_context.h>
#include <net/http.h>

#include "httpd.h"

/*
 * NOTE: the http_server_ctx and http_server_urls are quite large (avoid allocate from stack).
 */
static struct http_ctx http_ctx;
static struct http_server_urls http_urls;

#define RESULT_BUF_SIZE 1500
static u8_t http_result[RESULT_BUF_SIZE];

#define HTML_HEADER		"<html><head>" \
				"<title>Zephyr HTTP Server</title>" \
				"</head><body><h1>" \
				"<center>Zephyr HTTP server</center></h1>\r\n"

#define HTTP_STATUS_200_OK	"HTTP/1.1 200 OK\r\n" \
				"Content-Type: text/html\r\n" \
				"Transfer-Encoding: chunked\r\n"

#define HTTP_STATUS_404	"HTTP/1.1 404 Not Found\r\n" \
				"Content-Type: text/html\r\n" \
				"Transfer-Encoding: chunked\r\n"


static const char *get_string(int str_len, const char *str)
{
    static char buf[64];
    int len = min(str_len, sizeof(buf) - 1);

    memcpy(buf, str, len);
    buf[len] = '\0';

    return buf;
}

static int http_response(struct http_ctx *ctx, const char *header, const char *payload, size_t payload_len,
                         char *str, const struct sockaddr *dst)
{
    int ret;

    ret = http_add_header(ctx, header, dst, str);

    if (ret < 0) {
        LOG_ERR("Cannot add HTTP header (%d)", ret);
        return ret;
    }

    ret = http_add_header(ctx, HTTP_CRLF, dst, str);

    if (ret < 0) {
        return ret;
    }

    ret = http_send_chunk(ctx, payload, payload_len, dst, str);

    if (ret < 0) {
        LOG_ERR("Cannot send data to peer (%d)", ret);
        return ret;
    }

    ret = http_send_chunk(ctx, NULL, 0, dst, NULL);

    if (ret < 0) {
        LOG_ERR("Cannot send data to peer (%d)", ret);
        return ret;
    }

    return http_send_flush(ctx, str);
}

static int http_serve_index_html(struct http_ctx *ctx, const struct sockaddr *dst)
{
    static const char index_html[] = {
#include "index.html.inc"
    };

    LOG_DBG("Sending index.html (%zd bytes) to client", sizeof(index_html));

    return http_response(ctx, HTTP_STATUS_200_OK, index_html, sizeof(index_html), "Index", dst);
}

static int http_response_soft_404(struct http_ctx *ctx, const struct sockaddr *dst)
{
    static const char not_found[] = {
#include "404.html.inc"
    };

    LOG_DBG("Sending 404.html (%zd bytes) to client", sizeof(not_found));

    return http_response(ctx, HTTP_STATUS_404, not_found, sizeof(not_found), "404", dst);
}


static enum http_verdict default_handler(struct http_ctx *ctx, enum http_connection_type type,
                                                               const struct sockaddr *dst)
{
    LOG_DBG("No handler for %s URL %s",
            type == HTTP_CONNECTION ? "HTTP" : "WS",
            get_string(ctx->http.url_len, ctx->http.url));

    if (type == HTTP_CONNECTION) {
        http_response_soft_404(ctx, dst);
    }

    return HTTP_VERDICT_DROP;
}

static void http_connected(struct http_ctx *ctx, enum http_connection_type type,
                           const struct sockaddr *dst, void *user_data)
{
    char url[32];
    int len = min(sizeof(url) - 1, ctx->http.url_len);

    memcpy(url, ctx->http.url, len);
    url[len] = '\0';

    LOG_DBG("%s connect attempt URL %s",
            type == HTTP_CONNECTION ? "HTTP" : "WS", url);

    if (type == HTTP_CONNECTION) {
        if (strncmp(ctx->http.url, "/",
                    ctx->http.url_len) == 0) {
            http_serve_index_html(ctx, dst);
            http_close(ctx);
            return;
        }

        if (strncmp(ctx->http.url, "/index.html",
                    ctx->http.url_len) == 0) {
            http_serve_index_html(ctx, dst);
            http_close(ctx);
            return;
        }
    }

    /* Give 404 error for all the other URLs we do not want to handle
     * right now.
     */
    http_response_soft_404(ctx, dst);
    http_close(ctx);
}

static void http_received(struct http_ctx *ctx, struct net_pkt *pkt, int status, u32_t flags,
                          const struct sockaddr *dst, void *user_data)
{
    if (!status) {
        if (pkt) {
            LOG_DBG("Received %d bytes data",
                    net_pkt_appdatalen(pkt));
            net_pkt_unref(pkt);
        }
    } else {
        LOG_ERR("Receive error (%d)", status);

        if (pkt) {
            net_pkt_unref(pkt);
        }
    }
}

static void http_sent(struct http_ctx *ctx, int status, void *user_data_send, void *user_data)
{
    LOG_DBG("%s sent", (char *)user_data_send);
}

static void http_closed(struct http_ctx *ctx, int status, void *user_data)
{
    LOG_DBG("Connection %p closed", ctx);
}

int httpd_init()
{
    struct sockaddr addr, *server_addr;
    int ret;

    LOG_INF("initializating HTTP server");

    (void) memset(&addr, 0, sizeof(addr));

    net_sin(&addr)->sin_port = htons(80); //htons(ZEPHYR_PORT);

    /* listen only IPv4 */
    addr.sa_family = AF_INET; //family = AF_INET;

    server_addr = &addr;

    http_server_add_default(&http_urls, default_handler);
    http_server_add_url(&http_urls, "/index.html", HTTP_URL_STANDARD);
    http_server_add_url(&http_urls, "/", HTTP_URL_STANDARD);

    ret = http_server_init(&http_ctx, &http_urls, server_addr, http_result,
                           sizeof(http_result), "Zephyr HTTP Server",
                           NULL);
    if (ret < 0) {
        LOG_ERR("Cannot initialize HTTP server (%d)", ret);

        return 1;
    }

#if defined(CONFIG_NET_CONTEXT_NET_PKT_POOL)
    net_app_set_net_pkt_pool(&http_ctx.app_ctx, tx_slab, data_pool);
#endif

    http_set_cb(&http_ctx, http_connected, http_received, http_sent, http_closed);

    /*
     * If needed, the HTTP parser callbacks can be set according to
     * applications own needs before enabling the server. In this example
     * we use the default callbacks defined in HTTP server API.
     */
    http_server_enable(&http_ctx);

    return 0;
}
