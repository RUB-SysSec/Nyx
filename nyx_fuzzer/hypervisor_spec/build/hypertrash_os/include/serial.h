/*
 * HyperCube OS
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */ 

 /* credits: wiki.osdev.org, syssec.rub.de (angry_os) */

#pragma once

#include "system.h"

#define SERIAL_PORT_A 0x3F8
#define SERIAL_PORT_B 0x2F8
#define SERIAL_PORT_C 0x3E8
#define SERIAL_PORT_D 0x2E8

void serial_enable(int device);

int serial_rcvd(int device);
char serial_recv(int device);
char serial_recv_async(int device);
int serial_recv_string(int device, char* buf, int size);

int serial_transmit_empty(int device);
void serial_send(int device, char out);
void serial_send_string(int device, char * out);
int serial_printf(int device, const char* fmt, ...);