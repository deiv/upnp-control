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

#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <esp8266.h>
#include <espressif/esp_common.h>
#include <esp/uart.h>
#include <FreeRTOS.h>
#include <task.h>
#include <ssid_config.h>
#include <espressif/esp_wifi.h>

#include "network.h"
#include "upnp.h"
#include "httpd.h"


/**
  * @brief Network services task
  *
  * @param arg user supplied argument from xTaskCreate
  * @retval None
  */
static void services_task(void *arg)
{
    // XXX: loop ???
    for( ;; ) {


        printf("services_task\n");
        /*
         * Wait for the network up
         */
        ulTaskNotifyTake( pdTRUE, portMAX_DELAY );

        /*
         * Init the httpd task prior the upnp one (upnp needs http serving)
         */
        printf("initializating httpd_task\n");
        (void) httpd_server_init();

        printf("initializating mcast_task\n");
        (void) upnp_server_init();
    }

    vTaskDelete(NULL);
}

/**
  * @brief Network configuration task
  *
  * @param arg user supplied argument from xTaskCreate
  * @retval None
  */
static void network_task(void *arg)
{
    network_init(arg);

    vTaskDelete(NULL);
}

/**
  * @brief Entry point for open rtos kernel
  *
  * @retval None
  */
void user_init(void)
{
    printf("SDK version:%s\n", sdk_system_get_sdk_version());

    uart_set_baud(0, 115200);

    TaskHandle_t services_task_handle = NULL;

    xTaskCreate(&services_task, "services_task", 1024, NULL, 4, &services_task_handle);
    xTaskCreate(&network_task, "network_task",  256, services_task_handle, 2, NULL);
}
