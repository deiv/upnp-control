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

#include <task.h>

#ifndef UPNP_CONTROL_NETWORK_H
#define UPNP_CONTROL_NETWORK_H

void network_init(TaskHandle_t *services_task);

#endif //UPNP_CONTROL_NETWORK_H
