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
#include "tty.h"
#include "libk.h"
#include "vga.h"
#include "bmp.h"
#include "serial.h"
#include "config.h"
#include "fuzz.h"


size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;


static void disable_vga_blinking(void){
    __asm__ __volatile__(
	"mov $0x03DA, %dx\n\r"
	"in %dx, %al\n\r"

	"mov $0x03C0, %dx\n\r"
	"mov $0x30, %al\n\r"
	"out %al, %dx\n\r"

	"inc %dx\n\r"
	"in %dx, %al\n\r"


	"and $0xF7, %al\n\r"

	"dec %dx\n\r"
	"out %al, %dx\n\r"
    );  
}

void tty_initialize(uint8_t default_terminal_color) {
	disable_vga_blinking();
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = default_terminal_color;
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			VGA_MEMORY[index] = vga_entry(' ', terminal_color);
		}
	}
}

void tty_setcolor(uint8_t color) {
	terminal_color = color;
}

void tty_setpos(size_t column, size_t row) {
	terminal_column = column;
	terminal_row = row;
}

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) {
	const size_t index = y * VGA_WIDTH + x;
	VGA_MEMORY[index] = vga_entry(c, color);
}

void terminal_scroll() {
	for(size_t y = 1; y < VGA_HEIGHT; y++) {
		for(size_t x = 0; x < VGA_WIDTH; x++) {
			VGA_MEMORY[(y - 1) * VGA_WIDTH + x] = VGA_MEMORY[y * VGA_WIDTH + x];
		}
	}

	uint16_t entry = vga_entry(' ', DEFAULT_COLOR);
	for(size_t x = 0; x < VGA_WIDTH; x++) {
		VGA_MEMORY[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = entry;
	}

	terminal_row = VGA_HEIGHT - 1;
}

static void tty_putchar(char c) {
	if(c == '\n') {
		terminal_column = 0;
		if(++terminal_row == VGA_HEIGHT)
			terminal_scroll();
	}
	else {
		terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
		if (++terminal_column == VGA_WIDTH) {
			terminal_column = 0;
			if (++terminal_row == VGA_HEIGHT)
				terminal_scroll();
		}
	}
}

static void tty_write(const char* data, size_t size) {
	for (size_t i = 0; i < size; i++){
		if(data[i] == '\t'){
			tty_putchar(' ');
			tty_putchar(' ');
			tty_putchar(' ');
		}
		else{
			tty_putchar(data[i]);
		}
	}
}

void tty_writestring(const char* data) {
#ifdef ENABLE_TTY_VGA
	tty_write(data, strlen(data));
#endif
}

#ifdef EARLY_BOOT_PRINTF
int enabled = 1;
#else
int enabled = 0;
#endif

void enable_printf(void){
	enabled = 1;
}

void disable_printf(void){
	enabled = 0;
}

int tty_printf(const char* fmt, ...) {
	char buf[1024];
	va_list args;
	va_start(args, fmt);
	int out = vasprintf(buf, fmt, args);
	va_end(args);

#ifndef PAYLOAD
	exec_hprintf_str(buf);
#else
	//tty_writestring(buf);
	//serial_send_string(SERIAL_PORT_A, buf);
#endif

	return out;
}

#define BMP_PRINT_RIGHT_SHIFT 38

void tty_drawbmp(uint8_t* data) {
	BITMAPFILEHEADER* file_header = (BITMAPFILEHEADER*)data;
	BITMAPINFOHEADER* info_header = (BITMAPINFOHEADER*)(data + sizeof(BITMAPFILEHEADER));

	if(file_header->bfType != 0x4D42) {
		tty_setcolor(FAIL_COLOR);
		tty_writestring("Invalid Bitmap!\n");
		return;
	}

	if((uint32_t)info_header->biWidth > VGA_WIDTH) {
		tty_setcolor(FAIL_COLOR);
		tty_writestring("Bitmap too large!\n");
		return;
	}

	if(info_header->biClrUsed != 16) {
		tty_setcolor(FAIL_COLOR);
		tty_printf("Unsupported color profile (%x)!\n", info_header->biClrUsed);
		return;
	}

	uint8_t* pixels = (uint8_t*)(data + file_header->bOffBits);

	int row_bits = info_header->biWidth * info_header->biBitCount;
	row_bits = row_bits % 32 == 0 ? row_bits : row_bits + 32 - row_bits % 32;
	int row_bytes = row_bits / 8;

	int x, y;
	for(x = 0; x < info_header->biWidth; x += 2) {
		for(y = 0; y < info_header->biHeight / 2; y++) {
			int index = (info_header->biHeight - 2 - y * 2) * row_bytes + (x >> 1);
			uint8_t upper_pixel = pixels[index];
			index = (info_header->biHeight - 1 - y * 2) * row_bytes + (x >> 1);
			uint8_t lower_pixel = pixels[index];

			if ((x + 1U + BMP_PRINT_RIGHT_SHIFT) >= VGA_WIDTH || (x + 2U + BMP_PRINT_RIGHT_SHIFT) >= VGA_WIDTH){
				continue;
			}
			terminal_putentryat('\xdc', vga_entry_color(upper_pixel >> 4, lower_pixel >> 4), x + 1 + BMP_PRINT_RIGHT_SHIFT, y);
			terminal_putentryat('\xdc', vga_entry_color(upper_pixel & 0x0f, lower_pixel & 0x0f), x + 2 + BMP_PRINT_RIGHT_SHIFT, y);
		}
	}

}
