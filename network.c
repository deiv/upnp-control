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
#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>
#include <ssid_config.h>
#include <espressif/esp_wifi.h>
#include <ssid_config.h>
#include <espressif/esp_sta.h>

#include "network.h"

/** User friendly FreeRTOS delay macro */
#define delay_ms(ms) vTaskDelay(ms / portTICK_PERIOD_MS)

/**
  * @brief Task to obtain a network connection (wifi one)
  *
  * @retval None
  */
void network_init(TaskHandle_t *services_task)
{
    uint8_t status = 0;
    uint8_t retries = 30;

    struct sdk_station_config config = {
        .ssid = WIFI_SSID,
        .password = WIFI_PASS,
    };

    printf("WiFi: connecting to WiFi\n");
    sdk_wifi_set_opmode(STATION_MODE);
    sdk_wifi_station_set_config(&config);
    sdk_wifi_station_connect();

    while(1) {
        while (status != STATION_GOT_IP && retries) {
            status = sdk_wifi_station_get_connect_status();
            if(status == STATION_WRONG_PASSWORD) {
                printf("WiFi: wrong password\n");
                break;
            } else if(status == STATION_NO_AP_FOUND) {
                printf("WiFi: AP not found\n");
                break;
            } else if(status == STATION_CONNECT_FAIL) {
                printf("WiFi: connection failed\n");
                break;
            }
            delay_ms(1000);
            retries--;
        }

        if (status == STATION_GOT_IP) {
            printf("WiFi: connected\n");

            /*
             * Signal the services task, that we have a valid network connection
             */
            xTaskNotifyGive( services_task ); //xSemaphoreGive(wifi_alive);
            taskYIELD();
        }

        while ((status = sdk_wifi_station_get_connect_status()) == STATION_GOT_IP) {
            taskYIELD();
        }

        printf("WiFi: disconnected\n");
        sdk_wifi_station_disconnect();
        delay_ms(1000);
    }
}
