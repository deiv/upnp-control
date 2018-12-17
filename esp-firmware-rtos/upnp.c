/*
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

#include <string.h>

#include <espressif/esp_common.h>

#include <lwip/udp.h>
#include <lwip/igmp.h>
#include <lwip/ip_addr.h>

#include "upnp.h"

#define UPNP_MCAST_GRP  ("239.255.255.250")
#define UPNP_MCAST_PORT (1900)

static const char* get_my_ip(void)
{
    static char ip[16] = "0.0.0.0";
    struct ip_info ipinfo;

    ip[0] = 0;
    sdk_wifi_get_ip_info(STATION_IF, &ipinfo);
    snprintf(ip, sizeof(ip), IPSTR, IP2STR(&ipinfo.ip));

    return (char*) ip;
}

/**
  * @brief Joins a multicast group with the specified ip/port
  *
  * @param group_ip the specified multicast group ip
  * @param group_port the specified multicast port number
  * @param recv the lwip UDP callback
  *
  * @retval udp_pcb* or NULL if joining failed
  */
static struct udp_pcb* mcast_join_group(char *group_ip, uint16_t group_port,
                                        void (* recv)(void *arg, struct udp_pcb *upcb, struct pbuf *p,
                                                      const ip_addr_t *addr, u16_t port))
{
    bool status = false;
    struct udp_pcb *upcb;

    printf("upnpd: joining mcast group %s:%d\n", group_ip, group_port);

    do {
        upcb = udp_new();
        if (!upcb) {
            printf("upnpd: eror, udp_new failed");
            break;
        }
        udp_bind(upcb, IP4_ADDR_ANY, group_port);
        struct netif* netif = sdk_system_get_netif(STATION_IF);
        if (!netif) {
            printf("upnpd: error, netif is null");
            break;
        }
        if (!(netif->flags & NETIF_FLAG_IGMP)) {
            netif->flags |= NETIF_FLAG_IGMP;
            igmp_start(netif);
        }
        ip4_addr_t ipgroup;
        ip4addr_aton(group_ip, &ipgroup);
        err_t err = igmp_joingroup_netif(netif, &ipgroup);
        if (ERR_OK != err) {
            printf("upnpd: failed to join multicast group: %d", err);
            break;
        }
        status = true;
    } while(0);

    if (status) {
        printf("upnpd: join success\n");
        udp_recv(upcb, recv, upcb);

    } else {
        if (upcb) {
            udp_remove(upcb);
        }
        upcb = NULL;
    }

    return upcb;
}

/**
  * @brief send upnp discovery payload
  *
  * @param upcb the udp_pcb which received data
  * @param addr the remote IP address
  * @param port the remote port
  *
  * @retval None
  */
static void send_udp(struct udp_pcb *upcb, const ip_addr_t *addr, u16_t port)
{
    struct pbuf *p;
    char msg[500];
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
        "X-User-Agent: nipe\r\n\r\n", get_my_ip());

    p = pbuf_alloc(PBUF_TRANSPORT, strlen(msg)+1, PBUF_RAM);

    if (!p) {
        printf("upnpd: failed to allocate transport buffer\n");

    } else {
        memcpy(p->payload, msg, strlen(msg)+1);    
        err_t err = udp_sendto(upcb, p, addr, port);

        if (err < 0) {
            printf("upnpd: error sending message: %s (%d)\n", lwip_strerr(err), err);
        } else {
            printf("upnpd: sent message '%s - addr: %s'\n", msg, ipaddr_ntoa(addr));
        }

        pbuf_free(p);
    }
}

/**
  * @brief This function is called when an UDP datagram has been received on the port UDP_PORT.
  *
  * @param arg user supplied argument (udp_pcb.recv_arg)
  * @param upcb the udp_pcb which received data
  * @param p the packet buffer that was received
  * @param addr the remote IP address from which the packet was received
  * @param port the remote port from which the packet was received
  *
  * @retval None
  */
static void receive_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
    if (p) {
        printf("upnpd: msg received port:%d len:%d\n", port, p->len);
        uint8_t *buf = (uint8_t*) p->payload;
        printf("upnpd: msg received port:%d len:%d\nbuf: %s\n", port, p->len, buf);

        send_udp(upcb, addr, port);

        pbuf_free(p);
    }
}

/**
  * @brief Initialize the upnp server
  *
  * @retval true, if init was succcessful
  */
bool upnp_server_init(void)
{
    struct udp_pcb *upcb = mcast_join_group(UPNP_MCAST_GRP, UPNP_MCAST_PORT, receive_callback);

    return (upcb != NULL);
}
