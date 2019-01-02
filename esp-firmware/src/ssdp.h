/**
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

#include "net/net_ip.h"

/**
 * URI type
 */
typedef const char* uri_t;

/**
 * List of advertisement notifications subtypes
 */
typedef enum  {
    /** device available */
    SSDP_NOTIFY_NOTIFICATION_SUBTYPE_AVAILABLE,

    /** device unavailable */
    SSDP_NOTIFY_NOTIFICATION_SUBTYPE_UNAVAILABLE,

    /** device updated */
    SSDP_NOTIFY_NOTIFICATION_SUBTYPE_UPDATE
} ssdp_notification_subtype_t;

struct ssdp_config_t;

/**
 * Callback called each time a search request is received.
 *
 * @param config SSDP configuration struct
 * @param src_addr source address
 * @param st search target field received on the request
 */
typedef void (* ssdp_search_callback_t)(struct ssdp_config_t* config, struct sockaddr* src_addr, uri_t st);

/**
 * Configuration data for an instance of a SSDP service
 */
typedef struct ssdp_config_t {

    /** search requests callback (@see ssdp_search_callback_t) */
    ssdp_search_callback_t ssdp_search_callback;

} ssdp_config_t;

/**
 * Init the ssdp server on the network address provided
 *
 * @param net_addr network address to listen
 *
 * @return a ssdp_config_t on success; NULL on error
 */
ssdp_config_t* ssdp_server_init(struct net_addr* net_addr);

/**
 * Closes the ssdp server
 *
 * @param config ssdp_config_t pointer
 */
void ssdp_server_close(ssdp_config_t* config);

/**
 * Sends an advertisement message: let interested SSDP clients know of own presence.
 *
 * @param config SSDP configuration struct
 * @param type notification type header: here goes the service type URI
 * @param subtype notification subtype header (@see ssdp_notification_subtype_t): ssdp:alive, ssdp:byebye, ssdp:update
 * @param usn URN (unique service name) for the service
 * @param location service's location URI
 * @param extra_headers extra http headers
 *
 * @return 0 on success; !0 on error
 */
int ssdp_send_advertisement(ssdp_config_t* config, uri_t type, ssdp_notification_subtype_t subtype,
        uri_t usn, uri_t location, const char* extra_headers);

/**
 * Sends a search response message.
 *
 * This should be called, as a reaction on the search target field of the search request,
 * when a SSDP search message is received (@see ssdp_config @see ssdp_search_callback_t).
 *
 * @param config SSDP configuration struct
 * @param dst_addr destination addr
 * @param st search target field: here goes the service type URI
 * @param usn URN (unique service name) for the service
 * @param location service's location URI
 * @param extra_headers extra http headers
 *
 * @return 0 on success; !0 on error
 */
int ssdp_send_search_response(ssdp_config_t* config, struct sockaddr* dst_addr,
        uri_t st, uri_t usn, uri_t location, const char* extra_headers);

#endif //HTTP_SERVER_SSDP_H
