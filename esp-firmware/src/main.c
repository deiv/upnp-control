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

    ret = httpd_init();

    if (ret != 0) {
        panic(NULL);
    }

    /*ret = ssdp_server_init();

    if (ret != 0) {
        panic(NULL);
    }*/

    LOG_INF("exiting");
}
