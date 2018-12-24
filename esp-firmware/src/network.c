/*
 * @file network.c
 *
 * @brief Network helpers
 * @author David Suárez
 * @date Mon, 24 Dec 2018 00:43:29 +0100
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

#define LOG_MODULE_NAME network
#include <logging/log.h>
LOG_MODULE_REGISTER(LOG_MODULE_NAME, LOG_LEVEL_DBG);

#include <stdlib.h>
#include <net/net_if.h>
#include <net/dhcpv4.h>

#include "network.h"

/**
 * Max wait time to get dhcp ack
 */
#define MAX_NET_CONFIGURATION_WAIT_MILISECONDS (1000 * 60 * 5)

#define WAIT_MILISECONDS_FOR_DHCP_ACK (1000)

const char* format_ip_address(struct net_addr addr)
{
    char *ip_as_string;
    char *ret;

    ip_as_string = malloc(NET_IPV4_ADDR_LEN + 1);

    if (ip_as_string == NULL) {
        return NULL;
    }

    ret = net_addr_ntop(addr.family, (const void*) &addr.in_addr, ip_as_string, NET_IPV4_ADDR_LEN + 1);

    if (ret == NULL) {
        k_free(ip_as_string);
    }

    return ret;
}

int wait_for_net_interface_up(struct net_if *iface)
{
    int wait_miliseconds = 0;

    /*
     * wait for dhcp to provide us an ip address
     */
    while (iface->config.dhcpv4.state != NET_DHCPV4_BOUND) {

        LOG_INF("waiting interface '%s', to get dhcp lease", iface->if_dev->dev->config->name);

        wait_miliseconds += WAIT_MILISECONDS_FOR_DHCP_ACK;
        k_sleep(WAIT_MILISECONDS_FOR_DHCP_ACK);

        if (wait_miliseconds > MAX_NET_CONFIGURATION_WAIT_MILISECONDS) {
            LOG_ERR("wait time for network configuration has exceed the maximum allowed");

            return -1;
        }
    }
}
