/*
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

#include <zephyr.h>
#include <net/net_core.h>
#include <net/socket.h>
#include <net/net_pkt.h>
#include <net/net_core.h>
#include <net/net_context.h>
#include <net/udp.h>
#include <net/net_app.h>

#include "network.h"

#include "ssdp.h"


#define UPNP_MCAST_GRP  ("239.255.255.250")
#define UPNP_MCAST_PORT (1900)

//static int igmp_join(void);
static void udp_received(struct net_app_ctx *ctx, struct net_pkt *pkt, int status, void *user_data);
static void pkt_sent(struct net_app_ctx *ctx, int status, void *user_data_send, void *user_data);
static inline void set_dst_addr(sa_family_t family,
                                struct net_pkt *pkt,
                                struct sockaddr *dst_addr,struct net_app_ctx *ctx);
static int init_udp_server(void);


static struct net_app_ctx udp;
static const char *location_ip = NULL;

/*
int igmp_join(void)
{
    int ret;

    struct sockaddr_in addr4 = {
            .sin_family = AF_INET,
            .sin_port = htons(UPNP_MCAST_PORT),
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

#define MAX_DBG_PRINT 64
#define BUF_TIMEOUT K_MSEC(100)

struct net_pkt *build_reply_pkt(struct net_app_ctx *ctx, struct net_pkt *pkt)
{
    struct net_pkt *reply_pkt;
    struct pbuf *p;
    char msg[500];
    size_t msg_len;

    reply_pkt = net_app_get_net_pkt(ctx, net_pkt_family(pkt), BUF_TIMEOUT);

    if (!reply_pkt) {
        return NULL;
    }

    NET_ASSERT(net_pkt_family(reply_pkt) == net_pkt_family(pkt));

    snprintf(msg, sizeof(msg),
             "HTTP/1.1 200 OK\r\n"
             "CACHE-CONTROL: max-age=86400\r\n"
             "DATE: Fri, 15 Apr 2016 04:56:29 GMT\r\n"
             "EXT:\r\n"
             "LOCATION: http://%s:80/root.xml\r\n"
             "OPT: \"http://schemas.upnp.org/upnp/1/0/\"; ns=01\r\n"
             "01-NLS: b9200ebb-736d-4b93-bf03-835149d13983\r\n"
             "SERVER: Unspecified, UPnP/1.0, Unspecified\r\n"
             "ST: upnp:rootdevice\r\n"
             "USN: uuid:0622707c-da97-4286-deiv-000000000001::upnp:rootdevice\r\n"
             "X-User-Agent: nipe\r\n\r\n", location_ip);

    msg_len = strlen(msg) + 1;

    net_pkt_set_appdatalen(reply_pkt, msg_len);
    net_pkt_append(reply_pkt, msg_len, msg, BUF_TIMEOUT);

    return reply_pkt;
}

static inline void set_dst_addr(sa_family_t family,
                                struct net_pkt *pkt,
                                struct sockaddr *dst_addr,struct net_app_ctx *ctx)
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

static void udp_received(struct net_app_ctx *ctx, struct net_pkt *pkt, int status, void *user_data)
{
    static char dbg[MAX_DBG_PRINT + 1];
    struct net_pkt *reply_pkt;
    struct sockaddr dst_addr;
    sa_family_t family = net_pkt_family(pkt);
    socklen_t dst_len;
    u32_t pkt_len;
    int ret;

    snprintk(dbg, MAX_DBG_PRINT, "UDP IPv%c", family == AF_INET6 ? '6' : '4');
    LOG_INF("%s received %d bytes", dbg, net_pkt_appdatalen(pkt));

    if (family == AF_INET6) {
        dst_len = sizeof(struct sockaddr_in6);

    } else {
        dst_len = sizeof(struct sockaddr_in);
    }

    set_dst_addr(family, pkt, &dst_addr, ctx);

    reply_pkt = build_reply_pkt(ctx, pkt);

    net_pkt_unref(pkt);

    if (!reply_pkt) {
        return;
    }

    pkt_len = net_pkt_appdatalen(reply_pkt);

    ret = net_app_send_pkt(ctx, reply_pkt, &dst_addr, dst_len, K_NO_WAIT, UINT_TO_POINTER(pkt_len));

    if (ret < 0) {
        LOG_ERR("cannot send data to peer (%d)", ret);
        net_pkt_unref(reply_pkt);
    }
}

static void pkt_sent(struct net_app_ctx *ctx, int status, void *user_data_send, void *user_data)
{
    if (!status) {
        LOG_INF("Sent %d bytes", POINTER_TO_UINT(user_data_send));
    }
}

static int init_udp_server(void)
{
    int ret;

    ret = net_app_init_udp_server(&udp, NULL, UPNP_MCAST_PORT, NULL);

    if (ret < 0) {
        LOG_ERR("Cannot init SSDP service at addr: '%s', port '%d'", location_ip, UPNP_MCAST_PORT);
        return -1;
    }

#if defined(CONFIG_NET_CONTEXT_NET_PKT_POOL)
    net_app_set_net_pkt_pool(&udp, tx_udp_slab, data_udp_pool);
#endif

    ret = net_app_set_cb(&udp, NULL, udp_received, pkt_sent, NULL);

    if (ret < 0) {
        LOG_ERR("Cannot set callbacks (%d)", ret);
        net_app_release(&udp);
        return -1;
    }

    net_app_server_enable(&udp);
    ret = net_app_listen(&udp);

    if (ret < 0) {
        LOG_ERR("Cannot wait for connections (%d)", ret);
        net_app_release(&udp);
        return -1;
    }

    return 0;
}

int ssdp_server_init(struct net_if *iface)
{
    /*
     * get the first unicast ip address configured in the interface as the ssdp location
     *
     * NOTE: this thing should be k_free'd, at some point ...
     */
    location_ip = format_ip_address(iface->config.ip.ipv4->unicast[0].address);

    if (location_ip == NULL) {
        return -1;
    }

    init_udp_server();

    //igmp_join();

    return 0;
}
