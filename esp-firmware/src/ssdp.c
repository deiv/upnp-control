/**
 * @file ssdp.c
 *
 * @brief Simple Service Discovery Protocol (ssdp)
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

#define LOG_MODULE_NAME ssdp
#include <logging/log.h>
LOG_MODULE_REGISTER(LOG_MODULE_NAME, LOG_LEVEL_DBG);

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include <zephyr.h>
//#include <net/socket.h>
#include <net/net_pkt.h>
#include <net/net_core.h>
#include <net/net_context.h>
#include <net/udp.h>
#include <net/net_app.h>

#include "network.h"

#include "ssdp.h"



#define SSDP_NOTIFY_NOTIFICATION_SUBTYPE_AVAILABLE_STRING   "ssdp:alive"
#define SSDP_NOTIFY_NOTIFICATION_SUBTYPE_UNAVAILABLE_STRING "ssdp:byebye"
#define SSDP_NOTIFY_NOTIFICATION_SUBTYPE_UPDATE_STRING      "ssdp:update"

/**
 * Timeout for the packets methods
 */
#define PKT_BUF_TIMEOUT K_MSEC(100)

/**
 * UPnP multicast address
 */
#define UPNP_MCAST_GRP  ("239.255.255.250")

/**
 * UPnP multicast port
 */
#define SSDP_MCAST_PORT (1900)

//static int igmp_join(void);

static void udp_received(struct net_app_ctx *ctx, struct net_pkt *pkt, int status, void *user_data);
static void pkt_sent(struct net_app_ctx *ctx, int status, void *user_data_send, void *user_data);
static inline void set_src_addr(sa_family_t family, struct net_pkt *pkt, struct sockaddr *dst_addr);
static int init_udp_server(ssdp_config_t* config, struct sockaddr *server_addr);

/*
 * NOTE: the net_app_ctx is quite large (avoid allocate from stack).
 */
static struct net_app_ctx udp_ctx;






/** Server User Agent */
#define SSDP_SERVER_USER_AGENT "Unspecified, UPnP/2.0, Unspecified"

/** SSDP SP for all devices/services */
#define SSDP_SEARCH_TARGET_ALL         "ssdp:all"

/** SSDP SP for root device only */
#define SSDP_SEARCH_TARGET_ROOT_DEVICE "upnp:rootdevice"



#define SSDP_NOTIFY  \
    "NOTIFY * HTTP/1.1\r\n" \
    "HOST: 239.255.255.250:1900\r\n" \
    "CACHE-CONTROL: max-age=86400\r\n" \
    "LOCATION: http://%s:80/%s\r\n" \
    "NT: %s\r\n" \
    "NTS: %s\r\n" \
    "SERVER: %s\r\n" \
    "USN: %s" \
    "BOOTID.UPNP.ORG: %s\r\n\r\n"

/*
#define SSDP_SEARCH_RESPONSE \
    "HTTP/1.1 200 OK\r\n" \
    "CACHE-CONTROL: max-age=86400\r\n" \
    "DATE: Fri, 15 Apr 2016 04:56:29 GMT\r\n" \
    "EXT:\r\n" \
    "LOCATION: http://%s:80/%s\r\n" \
    "SERVER: %s\r\n" \
    "ST: %s\r\n" \
    "USN: %s\r\n" \
    "BOOTID.UPNP.ORG: %s" \
    "X-User-Agent: nipe\r\n\r\n"
*/
//  "OPT: \"http://schemas.upnp.org/upnp/1/0/\"; ns=01\r\n"
//  01-NLS: b9200ebb-736d-4b93-bf03-835149d13983\r\n"

#define SSDP_SEARCH_RESPONSE \
    "HTTP/1.1 200 OK\r\n" \
    "EXT:\r\n" \
    "CACHE-CONTROL: max-age=86400\r\n" \
    "ST: %s\r\n" \
    "USN: %s\r\n" \
    "LOCATION: %s\r\n" \
    "%s" \
    "\r\n\r\n"

/*    "DATE: Fri, 15 Apr 2016 04:56:29 GMT\r\n" \
    "SERVER: %s\r\n" \
    "BOOTID.UPNP.ORG: %s" \
    "X-User-Agent: nipe\r\n\r\n"*/



/*
int igmp_join(void)
{
    int ret;

    struct sockaddr_in addr4 = {
            .sin_family = AF_INET,
            .sin_port = htons(SSDP_MCAST_PORT),
            .sin_addr = { { { 239, 255, 255, 250 } } },
    };

    struct sockaddr_in addrdd = {
            .sin_family = AF_INET,
            .sin_port = htons(22222)
    };

    struct sockaddr *bind_addr = &addr4;
    struct sockaddr *bind_addrdd = &addrdd;

    char* dd = "M-SEARCH * HTTP/1.1\n"
               "HOST: 239.255.255.250:1900\n"
               "MAN: \"ssdp:discover\"\n"
               "MX: 1\n"
               "ST: ssdp:all\n"
               "";

    int sock = zsock_socket(bind_addrdd->sa_family, SOCK_DGRAM, IPPROTO_UDP);

    if (sock < 0) {
        LOG_ERR("Failed to create UDP socket : %d", errno);
        return -errno;
    }

    if (ret < 0) {
        LOG_ERR("Failed to bind UDP socket: %d", errno);
        ret = -errno;
    }

    ret = zsock_sendto(sock, dd, strlen(dd), 0,
                       &bind_addr, sizeof(addr4));
    if (ret < 0) {
        LOG_ERR("UDP: Failed to send %d", errno);
        ret = -errno;
    }

    LOG_INF("igmp_joinigmp_joinigmp_join");

    return ret;
}*/


static inline void set_src_addr(sa_family_t family, struct net_pkt *pkt, struct sockaddr *dst_addr)
{
    struct net_udp_hdr hdr, *udp_hdr;

    udp_hdr = net_udp_get_hdr(pkt, &hdr);

    if (!udp_hdr) {
        return;
    }

#if defined(CONFIG_NET_IPV6)
    if (family == AF_INET6) {
		net_ipaddr_copy(&net_sin6(dst_addr)->sin6_addr,
				&NET_IPV6_HDR(pkt)->src);
		net_sin6(dst_addr)->sin6_family = AF_INET6;
		net_sin6(dst_addr)->sin6_port = udp_hdr->src_port;
	}
#endif /* CONFIG_NET_IPV6) */

#if defined(CONFIG_NET_IPV4)
    if (family == AF_INET) {
		net_ipaddr_copy(&net_sin(dst_addr)->sin_addr, &NET_IPV4_HDR(pkt)->src);
		net_sin(dst_addr)->sin_family = AF_INET;
		net_sin(dst_addr)->sin_port = udp_hdr->src_port;
	}
#endif /* CONFIG_NET_IPV6) */
}

#define CR                  '\r'
#define LF                  '\n'
#define SPACE               ' '

typedef enum {
    SSDP_HTTP_NOTIFY,
    SSDP_HTTP_SEARCH,
    SSDP_HTTP_UNKNOW
} ssdp_http_method_t;

/**
 * Get the HTTP method from request data.
 *
 * The method is included in the http request line (first line), which has the next format:
 *
 *  - [method-token] [Request-URI] [protocol version] CRLF
 *
 *  Ex: NOTIFY * HTTP/1.1
 *
 * @param data pointer to http data
 * @param len length of data
 *
 * @return ssdp_http_method_t type
 */
static ssdp_http_method_t get_http_method(const char *data, u16_t len)
{
    const char *p = data;
    char ch;
    int ret;

    for (p = data; p != data + len; p++) {
        ch = *p;

        if (ch == SPACE) {
            break;
        }
    }

    /* found something ? */
    if (p == (data + len)) {
        return SSDP_HTTP_UNKNOW;
    }

    ret = memcmp(data, "NOTIFY", p - data);

    if (ret == 0) {
        return SSDP_HTTP_NOTIFY;
    }

    ret = memcmp(data, "M-SEARCH", p - data);

    if (ret == 0) {
        return SSDP_HTTP_SEARCH;
    }

    return SSDP_HTTP_UNKNOW;
}

int skip_after_str(const char *data, u16_t len, int current, const char *str)
{
    const char *p;
    int str_len = strlen(str);
    int found;

    for (p = data + current; p < data + len - str_len; p++) {

        found = memcmp(p, str, str_len);

        if (found == 0) {
            return p + str_len - data;
        }
    }

    return -1;
}

int found_str(const char *data, u16_t len, int current, const char *str)
{
    const char *p = data + current;
    int str_len = strlen(str);
    int found;

    if ((data + str_len) > data + len) {
        return -1;
    }

    found = memcmp(p, str, str_len);

    if (found == 0) {
        return p + str_len - data;
    }

    return 0;
}

char* get_http_header_value(const char *data, u16_t len, const char* header_name)
{
    const char *p = data;
    char ch;
    int ret;
    int is_last_cr = 0;

    int found_idx;

    found_idx = skip_after_str(data, len, 0, "\r\n");

    /* final reached ? */
    if (found_idx < 0) {
        return NULL;
    }

    found_idx = skip_after_str(data, len, found_idx, "\r\n");

    /* final reached ? */
    if (found_idx < 0) {
        return NULL;
    }

    int header_found;

    while (true) {

        header_found = found_str(data, len, found_idx, header_name);

        /* found something ? */
        if (header_found > 0 ) {
            break;

        /* final reached ? */
        } else if (header_found < 0) {
            return NULL;
        }

        found_idx = skip_after_str(data, len, found_idx, "\r\n");

        /* final reached ? */
        if (found_idx < 0) {
            return NULL;
        }
    }

    found_idx = skip_after_str(data, len, header_found, "\r\n");

    /* final reached ? */
    if (found_idx < 0 ) {
        return NULL;
    }

    int header_value_len = found_idx - 2 - header_found;
    char* value;

    value = k_malloc(header_value_len + 1);

    if (value == NULL) {
        return NULL;
    }

    memcpy(value, data + header_found, header_value_len);

    value[header_value_len] = 0;

    return value;
}

static void udp_received(struct net_app_ctx *ctx, struct net_pkt *pkt, int status, void *user_data)
{
    ssdp_config_t* config = (ssdp_config_t*) user_data;
    struct sockaddr src_addr;

    LOG_INF("received %d bytes", net_pkt_appdatalen(pkt));

    if (config->ssdp_search_callback) {

        ssdp_http_method_t ssdp_method = get_http_method(pkt->appdata, pkt->appdatalen);

        if (ssdp_method == SSDP_HTTP_SEARCH) {

            char* st_value = get_http_header_value(pkt->appdata, pkt->appdatalen, "ST:");

            if (st_value == NULL) {
                return;
            }

            set_src_addr(net_pkt_family(pkt), pkt, &src_addr);

            config->ssdp_search_callback(config, &src_addr, st_value);

            k_free(st_value);
        }
    }

    net_pkt_unref(pkt);
}

static void pkt_sent(struct net_app_ctx *ctx, int status, void *user_data_send, void *user_data)
{
    if (!status) {
        LOG_INF("Sent %d bytes", POINTER_TO_UINT(user_data_send));
    }
}

static int init_udp_server(ssdp_config_t* config, struct sockaddr *server_addr)
{
    int ret;

   // ret = net_app_init_udp_server(&udp_ctx, server_addr, 0, config);
    ret = net_app_init_udp_server(&udp_ctx, NULL, SSDP_MCAST_PORT, config);

    if (ret < 0) {
        // TODO: add ip address
        LOG_ERR("Cannot init SSDP service at port '%d'", SSDP_MCAST_PORT);
        return -1;
    }

#if defined(CONFIG_NET_CONTEXT_NET_PKT_POOL)
    net_app_set_net_pkt_pool(&udp, tx_udp_slab, data_udp_pool);
#endif

    ret = net_app_set_cb(&udp_ctx, NULL, udp_received, pkt_sent, NULL);

    if (ret < 0) {
        LOG_ERR("Cannot set callbacks (%d)", ret);
        net_app_release(&udp_ctx);
        return -1;
    }

    net_app_server_enable(&udp_ctx);
    ret = net_app_listen(&udp_ctx);

    if (ret < 0) {
        LOG_ERR("Cannot wait for connections (%d)", ret);
        net_app_release(&udp_ctx);
        return -1;
    }

    return 0;
}

struct net_pkt *build_reply_pkt(struct net_app_ctx *ctx, sa_family_t family /*struct net_pkt *pkt*/, u8_t* data, size_t data_len)
{
    struct net_pkt *reply_pkt;

    reply_pkt = net_app_get_net_pkt(ctx, family, PKT_BUF_TIMEOUT);

    if (!reply_pkt) {
        return NULL;
    }

    NET_ASSERT(net_pkt_family(reply_pkt) == net_pkt_family(pkt));

    net_pkt_set_appdatalen(reply_pkt, data_len);
    net_pkt_append(reply_pkt, data_len, data, PKT_BUF_TIMEOUT);

    return reply_pkt;
}

/*struct net_pkt *build_notify_packet(struct net_app_ctx *ctx, sa_family_t family,
                                    const char* notification_type, const char* notification_subtype, const char* uuid)
{
    char msg[500];
    size_t msg_len;

    snprintf(
            msg,
            sizeof(msg),
            SSDP_NOTIFY,
            location_ip,
            notification_type,
            notification_subtype,
            SSDP_SERVER_USER_AGENT,
            uuid,
            boot_id);

    msg_len = strlen(msg) + 1;


    return build_reply_pkt(ctx, family, msg, msg_len);
}*/



struct net_pkt *build_search_response_packet(struct net_app_ctx *ctx, sa_family_t family,
                                             uri_t st, uri_t usn, uri_t location, const char* extra_headers)
{
    char msg[500];
    size_t msg_len;

    if (extra_headers == NULL) {
        extra_headers = "";
    }

    snprintf(
            msg,
            sizeof(msg),
    SSDP_SEARCH_RESPONSE,
            st,
            usn,
            location,
            extra_headers);

    msg_len = strlen(msg) + 1;


    return build_reply_pkt(ctx, family, msg, msg_len);
}

ssdp_config_t* ssdp_server_init(struct net_addr* net_addr)
{
    ssdp_config_t* config;
    struct sockaddr server_addr;
    int ret;

    config = k_malloc(sizeof(ssdp_config_t));

    if (config == NULL) {
        return NULL;
    }

    config->ssdp_search_callback = NULL;

#if defined(CONFIG_NET_IPV6)
    if (net_addr->family == AF_INET6) {
        net_ipaddr_copy(&net_sin6(&server_addr)->sin6_addr, &net_addr->in6_addr);
		net_sin6(&server_addr)->sin6_family = AF_INET6;
		net_sin6(&server_addr)->sin6_port = SSDP_MCAST_PORT;
	}
#endif /* CONFIG_NET_IPV6) */

#if defined(CONFIG_NET_IPV4)
    if (net_addr->family == AF_INET) {
        net_ipaddr_copy(&net_sin(&server_addr)->sin_addr, &net_addr->in_addr);
        net_sin(&server_addr)->sin_family = AF_INET;
		net_sin(&server_addr)->sin_port = SSDP_MCAST_PORT;
	}
#endif /* CONFIG_NET_IPV4) */

    ret = init_udp_server(config, &server_addr);

    if (ret < 0) {
        free(config);
        return NULL;
    }

    //igmp_join();

    return config;
}

void ssdp_server_close(ssdp_config_t* config)
{
    net_app_release(&udp_ctx);

    free(config);
}

int ssdp_send_search_response(ssdp_config_t* config, struct sockaddr* dst_addr,
                              uri_t st, uri_t usn, uri_t location, const char* extra_headers)
{
    struct net_pkt *reply_pkt;
    socklen_t dst_len;
    u32_t pkt_len;
    int ret;

    if (dst_addr->sa_family == AF_INET6) {
        dst_len = sizeof(struct sockaddr_in6);

    } else {
        dst_len = sizeof(struct sockaddr_in);
    }

    reply_pkt = build_search_response_packet(&udp_ctx, dst_addr->sa_family, st, usn, location, extra_headers);

    if (!reply_pkt) {
        return -1;
    }

    pkt_len = net_pkt_appdatalen(reply_pkt);

    ret = net_app_send_pkt(&udp_ctx, reply_pkt, &dst_addr, dst_len, K_NO_WAIT, UINT_TO_POINTER(pkt_len));

    if (ret < 0) {
        LOG_ERR("cannot send data to peer (%d)", ret);
        net_pkt_unref(reply_pkt);
    }

    return 0;
}

int ssdp_send_advertisement(ssdp_config_t* config, uri_t type, ssdp_notification_subtype_t subtype,
                            uri_t usn, uri_t location, const char* extra_headers)
{

    return 0;
}
