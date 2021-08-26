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

#include "system.h"
#include "serial.h"
#include "libk.h"
#include "io.h"
#include "config.h"

bool serial_enabled = false;

void serial_enable(int device) {
	outb(0x00, device + 1);
	outb(0x80, device + 3); /* Enable divisor mode */
	outb(0x03, device + 0); /* Div Low:  03 Set the port to 38400 baud */
	outb(0x00, device + 1); /* Div High: 00 */
	outb(0x03, device + 3); /* 8 bits, no parity, one stop bit */
	outb(0xC7, device + 2);
	outb(0x0B, device + 4);
	serial_enabled = true;
}

int serial_rcvd(int device) {
	return inb(device + 5) & 1;
}

char serial_recv(int device) {
	while (serial_rcvd(device) == 0) ;
	return inb(device);
}

int serial_recv_string(int device, char* buf, int size) {
	if(size == 0)
		return 0;

	int pos = 0;
	while(1) {
		char c = serial_recv(device);
		buf[pos] = c;
		pos++;

		if(c == 0 || c == '\n' || pos >= size)
			return pos;

	}
}

char serial_recv_async(int device) {
	return inb(device);
}

int serial_transmit_empty(int device) {
	return inb(device + 5) & 0x20;
}

void serial_send(int device, char out) {
	if(serial_enabled){
		while (serial_transmit_empty(device) == 0);
		outb(out, device);
	}
}

void serial_send_string(int device, char * out) {
#ifdef ENABLE_TTY_SERIAL
	for (uint32_t i = 0; i < strlen(out); ++i) {
		serial_send(device, out[i]);
	}
#endif
}

int serial_printf(int device, const char* fmt, ...) {
	char buf[1024];
	va_list args;
	va_start(args, fmt);
	int out = vasprintf(buf, fmt, args);
	va_end(args);
	serial_send_string(device, buf);
	return out;
}
