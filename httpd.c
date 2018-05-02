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
#include <stdio.h>

#include <espressif/esp_common.h>
#include <esp8266.h>
#include <esp/uart.h>
#include <FreeRTOS.h>
#include <task.h>
#include <ssid_config.h>

#include <httpd/httpd.h>

#include "httpd.h"

char *setup_cgi_handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);

void httpd_server_init()
{
    /*tCGI pCGIs[] = {
        {"/root.xml", (tCGIHandler) setup_cgi_handler},
    };*/
   
    httpd_init();

    //for (;;);
}
/*
char *setup_cgi_handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    return "/root.xml";
}*/

