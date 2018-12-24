/*
 * @file ssdp.h
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

#ifndef HTTP_SERVER_SSDP_H
#define HTTP_SERVER_SSDP_H

/**
 * Init ssdp server on the interface provided
 *
 * @param iface network interface
 * @return 0 on success; !0 on error
 */
int ssdp_server_init(struct net_if *iface);

#endif //HTTP_SERVER_SSDP_H
