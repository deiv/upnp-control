/*
 * @file main.c
 *
 * @brief Main program entry point
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

#define LOG_MODULE_NAME main
#include <logging/log.h>
LOG_MODULE_REGISTER(LOG_MODULE_NAME, LOG_LEVEL_DBG);

#define CONFIG_NET_DHCPV4 // XXX: remove
#define  CONFIG_NET_IPV4

#include <net/net_if.h>

#include "network.h"
#include "httpd.h"
#include "ssdp.h"


void panic(const char *msg)
{
    if (msg) {
        LOG_ERR("%s", msg);
    }

    for (;;) {
        k_sleep(K_FOREVER);
    }
}



void main(void)
{
    int ret;
    struct net_if *iface = NULL;

    iface = net_if_get_first_by_type(&NET_L2_GET_NAME(ETHERNET));

    if (iface == NULL) {
        panic("No existe ninguna interfaz de red");
    }

    ret = wait_for_net_interface_up(iface);

    if (ret != 0) {
        panic(NULL);
    }

    ret = httpd_init();

    if (ret != 0) {
        panic(NULL);
    }

    ret = ssdp_server_init(iface);

    if (ret != 0) {
        panic(NULL);
    }

    LOG_INF("exiting");
}
