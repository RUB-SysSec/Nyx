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

#pragma once

#include "system.h"
#include "vga.h"

#ifdef SHARKOS
#define DEFAULT_COLOR vga_entry_color(VGA_COLOR_BLACK, VGA_COLOR_WHITE)
#else
#define DEFAULT_COLOR vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_LIGHT_BLUE)
#endif

#define SUCCESS_COLOR vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_LIGHT_BLUE)
#define FAIL_COLOR vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_LIGHT_BLUE)
#define CRASH_COLOR vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_RED)

#define printf(...) tty_printf(__VA_ARGS__);

void tty_initialize(uint8_t default_terminal_color);

void tty_setcolor(uint8_t color);
void tty_setpos(size_t column, size_t row);

void tty_writestring(const char* data);
int tty_printf(const char* fmt, ...);

void tty_drawbmp(uint8_t* data);
void enable_printf(void);
void disable_printf(void);