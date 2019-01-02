/**
 * @file upnp.c
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

#define LOG_MODULE_NAME upnp
#include <logging/log.h>
LOG_MODULE_REGISTER(LOG_MODULE_NAME, LOG_LEVEL_DBG);

#include "network.h"
#include "httpd.h"
#include "ssdp.h"

#include "upnp.h"

/**
 * The music service
 */
t_upnp_service music_service = {
        "urn:schemas-upnp-org:service:RenderingControl:1",  /* URI */
        "music.xml",                                        /* description URI */
        "MusicControl1",                                    /* control URI */
        "MusicRenderingControl1",                           /* event URI */
};

/**
 * Our uPnP root device
 */
t_upnp_device upnp_device = {
        "urn:ojete-org:service:homecontroller:1",           /* URI */
        "uuid:0622707c-da97-4286-deiv-000000000001",        /* UUID */
        "root.xml",                                         /* description URI */
        { &music_service, NULL },                           /* services list */
        NULL,                                               /* Ip address */
        0,                                                  /* boot id */
};

/*
 * On the UPnP protocol, the search requests aren't mapped 1:1 to responses. We should switch ( st ) as follows:
 *
 *   - ssdp:all
 *       Respond 3+2d+k times for a root device with d embedded devices and s embedded services but only
 *       k distinct service types (see clause 1.1.2, “SSDP message header fields” for a definition of each
 *       message to be sent). Field value for ST header field shall be the same as for the NT header field in
 *       NOTIFY messages with ssdp:alive. (See above.)
 *
 *   - upnp:rootdevice
 *       Respond once for root device. Shall be upnp:rootdevice.
 *
 *   - uuid:device-UUID
 *       Respond once for each matching device, root or embedded. Shall be uuid:device-UUID where device-
 *       UUID is specified by the UPnP vendor. See clause 1.1.4, “UUID format and recommended generation
 *       algorithms” for the MANDATORY UUID format.
 *
 *   - urn:schemas-upnp-org:device:deviceType:ver
 *       Respond once for each matching device, root or embedded.Shall be
 *       urn:schemas-upnp-org:device:deviceType:ver where deviceType and ver are defined by UPnP Forum
 *       working committee and ver shall contain the version of the device type contained in the M-SEARCH
 *       request.
 *
 *   - urn:schemas-upnp-org:service:serviceType:ver
 *       Respond once for each matching service type. shall be urn:schemas-upnp-org:service:serviceType:ver
 *       where serviceType and ver are defined by the UPnP
 *       Forum working committee and ver shall contain the version of the service type contained in the M -
 *       SEARCH request.
 *
 *   - urn:domain-name:device:deviceType:ver
 *       Respond once for each matching device, root or embedded. shall be urn:domain-
 *       name:device:deviceType:ver where domain-name (a Vendor Domain Name), deviceType and ver are
 *       defined by the UPnP vendor and ver shall contain the version of the device type from the M-SEARCH
 *       request. Period characters in the Vendor Domain Name shall be replaced with hyphens in accordance
 *       with RFC 2141.
 *
 *   - urn:domain-name:service:serviceType:ver
 *       Respond once for each matching service type. shall be urn: domain-name:service:serviceType:ver
 *       where domain-name (a Vendor Domain Name), serviceType and ver are defined by the UPnP vendor
 *       and ver shall contain the version of the service type from the M-SEARCH request. Period characters
 *       in the Vendor Domain Name shall be replaced with hyphens in accordance with RFC 2141.
 */
void ssdp_search_callback(struct ssdp_config_t* config, struct sockaddr* src_addr, uri_t st)
{

    char* ip = format_sock_address_ip(src_addr);

    LOG_INF("ssdp_search_callback: ip='%s' st='%s'", ip, st);

    k_free(ip);

    ssdp_send_search_response(config, src_addr, st, "usn", "location", "extra_headers");
}

int init_upnp_system(struct net_addr* net_addr)
{
    int ret;
    ssdp_config_t* ssdp_config;

    /* NOTE: this thing should be k_free'd, at some point ... */
    const char* iface_ip = format_net_address_ip(net_addr);

    if (iface_ip == NULL) {
        return -1;
    }

    upnp_device.ip_address = iface_ip;

    /* increment boot counter */
    upnp_device.boot_id++;

    /*
     * Initialize the http and soap transports.
     *
     * This goes prior SSDP initilization, SSDP should knows about the http endpoint for the root device description.
     */
    ret = httpd_init();

    if (ret != 0) {
        return -1;
    }

    /*
     * Initialize the SSDP discovery protocol
     */
    ssdp_config = ssdp_server_init(net_addr);

    if (ssdp_config == NULL) {
        return -1;
    }

    ssdp_config->ssdp_search_callback = ssdp_search_callback;

    return 0;
}