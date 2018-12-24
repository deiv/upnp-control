/*
 * @file network.h
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
#ifndef ESP_FIRMWARE_NETWORK_H
#define ESP_FIRMWARE_NETWORK_H

/**
 * Format a net_addr to string
 *
 * @param addr ip address
 * @return pointer to null terminated string or NULL on fail
 */
const char* format_ip_address(struct net_addr addr);

/**
 * Wait fot the network interface to get configured (we wait for dhcp4 ack)
 *
 * @param iface the interface
 * @return 0 if success; !0 if error
 */
int wait_for_net_interface_up(struct net_if *iface);

#endif //ESP_FIRMWARE_NETWORK_H
