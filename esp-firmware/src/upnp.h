/**
 * @file upnp.h
 *
 * @brief UPnP handling
 * @author David Suárez
 * @date Mon, 24 Dec 2018 17:46:33 +0100
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

#ifndef ESP_FIRMWARE_UPNP_H
#define ESP_FIRMWARE_UPNP_H

#include "net/net_ip.h"

/**
 * URI type
 */
typedef const char* uri_t;

/**
 * UPnP service info
 */
typedef struct upnp_service {

    /** URI containing the uPnP URN (Uniform Resource Name) */
    uri_t uri;

    /** URL for the service XML descriptor */
    uri_t description_uri;

    /** URL for the service control endpoint */
    uri_t control_uri;

    /** URL for the service event endpoint */
    uri_t event_uri;

} t_upnp_service;

/**
 * UPnP device info
 */
typedef struct upnp_device {

    /** URI containing the uPnP URN (Uniform Resource Name) */
    uri_t uri;

    /** Universally unique ID for this device */
    char* uuid;

    /** URL for the device XML descriptor */
    uri_t description_uri;

    /** List of services that this device publish */
    t_upnp_service *services;

    /* upnp_device *embedded_devices; */

    /** ip address configured for the device */
    const char *ip_address;

    /** boot counter: incremented each time the device (re)joins the network, or is updated */
    long boot_id;

} t_upnp_device;

/**
 * Initialize the UPnP system:
 *
 *   - Discovery: SSDP discovery protocol (HTTPU - multicast and unicast over UDP message(s))
 *   - Description/Control/Eventing/Presentation: HTTP/SOAP for device(s) message(s) transport
 *
 * @param net_addr network address on which the uPnP will publish their services.
 *   The uPnP arquitecture requires that the ip address of the network has to be assigned from
 *   a DHCP server in the network (network is managed) or from automatic ip addressing (Auto-IP).
 *
 * @return 0 on success; !0 on error
 */
int init_upnp_system(struct net_addr* net_addr);

#endif //ESP_FIRMWARE_UPNP_H
