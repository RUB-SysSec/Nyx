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
#include "libk.h"
#include "tty.h"

void microdelay(uint32_t us){
	for(uint32_t i = 0;i < us;i++)
		;
}

static void print_dec(unsigned int value, unsigned int width, char * buf, int * ptr ) {
	unsigned int n_width = 1;
	unsigned int i = 9;
	while (value > i && i < UINT32_MAX) {
		n_width += 1;
		i *= 10;
		i += 9;
	}

	int printed = 0;
	while (n_width + printed < width) {
		buf[*ptr] = '0';
		*ptr += 1;
		printed += 1;
	}

	i = n_width;
	while (i > 0) {
		unsigned int n = value / 10;
		int r = value % 10;
		buf[*ptr + i - 1] = r + '0';
		i--;
		value = n;
	}
	*ptr += n_width;
}

static void print_hex(unsigned int value, unsigned int width, char * buf, int * ptr) {
	int i = width;

	if (i == 0) i = 8;

	unsigned int n_width = 1;
	unsigned int j = 0x0F;
	while (value > j && j < UINT32_MAX) {
		n_width += 1;
		j *= 0x10;
		j += 0x0F;
	}

	while (i > (int)n_width) {
		buf[*ptr] = '0';
		*ptr += 1;
		i--;
	}

	i = (int)n_width;
	while (i-- > 0) {
		buf[*ptr] = "0123456789abcdef"[(value>>(i*4))&0xF];
		*ptr += + 1;
	}
}


void hexdump (char *desc, void *addr, int len) {
    int i;
    unsigned char buff[17];
    unsigned char *pc = (unsigned char*)addr;

    if (desc != NULL)
        printf ("%s:\n", desc);

    if (len == 0) {
        printf("  ZERO LENGTH\n");
        return;
    }
    if (len < 0) {
        printf("  NEGATIVE LENGTH: %i\n",len);
        return;
    }

    for (i = 0; i < len; i++) {
        if ((i % 16) == 0) {
            if (i != 0)
                printf ("  %s\n", buff);

            printf ("  %04x ", i);
        }

        printf (" %02x", pc[i]);

        if ((pc[i] < 0x20) || (pc[i] > 0x7e))
            buff[i % 16] = '.';
        else
            buff[i % 16] = pc[i];
        buff[(i % 16) + 1] = '\0';
    }

    while ((i % 16) != 0) {
        printf ("   ");
        i++;
    }

    printf ("  %s\n", buff);
}

size_t vasprintf(char * buf, const char *fmt, va_list args) {
	int i = 0;
	char *s;
	int ptr = 0;
	int len = strlen(fmt);
	for ( ; i < len && fmt[i]; ++i) {
		if (fmt[i] != '%') {
			buf[ptr++] = fmt[i];
			continue;
		}
		++i;
		unsigned int arg_width = 0;
		while (fmt[i] >= '0' && fmt[i] <= '9') {
			arg_width *= 10;
			arg_width += fmt[i] - '0';
			++i;
		}
		/* fmt[i] == '%' */
		switch (fmt[i]) {
			case 's': /* String pointer -> String */
				s = (char *)va_arg(args, char *);
				while (*s) {
					buf[ptr++] = *s++;
				}
				break;
			case 'c': /* Single character */
				buf[ptr++] = (char)va_arg(args, int);
				break;
			case 'x': /* Hexadecimal number */
				print_hex((unsigned long)va_arg(args, unsigned long), arg_width, buf, &ptr);
				break;
			case 'd': /* Decimal number */
				print_dec((unsigned long)va_arg(args, unsigned long), arg_width, buf, &ptr);
				break;
			case '%': /* Escape */
				buf[ptr++] = '%';
				break;
			default: /* Nothing at all, just dump it */
				buf[ptr++] = fmt[i];
				break;
		}
	}
	buf[ptr] = '\0';
	return ptr;
}

size_t vsnprintf(char * buf, size_t n, const char *fmt, va_list args) {
	uint32_t i = 0;
	char *s;
	int ptr = 0;
	uint32_t len = strlen(fmt);
	if(!n){
		return 0;
	}
	for ( ; i < len && fmt[i] && i < (n-1); ++i) {
		if (fmt[i] != '%') {
			buf[ptr++] = fmt[i];
			continue;
		}
		++i;
		unsigned int arg_width = 0;
		while (fmt[i] >= '0' && fmt[i] <= '9') {
			arg_width *= 10;
			arg_width += fmt[i] - '0';
			++i;
		}
		/* fmt[i] == '%' */
		switch (fmt[i]) {
			case 's': /* String pointer -> String */
				s = (char *)va_arg(args, char *);
				while (*s) {
					buf[ptr++] = *s++;
				}
				break;
			case 'c': /* Single character */
				buf[ptr++] = (char)va_arg(args, int);
				break;
			case 'x': /* Hexadecimal number */
				print_hex((unsigned long)va_arg(args, unsigned long), arg_width, buf, &ptr);
				break;
			case 'd': /* Decimal number */
				print_dec((unsigned long)va_arg(args, unsigned long), arg_width, buf, &ptr);
				break;
			case '%': /* Escape */
				buf[ptr++] = '%';
				break;
			default: /* Nothing at all, just dump it */
				buf[ptr++] = fmt[i];
				break;
		}
	}
	buf[ptr] = '\0';
	return ptr;
}

int sprintf(char * buf, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	int out = vasprintf(buf, fmt, args);
	va_end(args);
	return out;
}

size_t strlen(const char* str) {
	size_t len = 0;
	while (str[len])
		len++;
	return len;
}

//#define MEMCPY_DEBUG

void memcpy(void* dst, void* src, size_t len) {
#ifdef MEMCPY_DEBUG
	printf("%s(%x, %x, %x)\n\r", __func__, dst, src, len);
#endif
	for(size_t i = 0; i < len; i++) {
		((uint8_t*)dst)[i] = ((uint8_t*)src)[i];
	}
}

void* memset(void* dst, int c, size_t len) {
	for(size_t i = 0; i < len; i++) {
		((char*)dst)[i] = (char)c;
	}
	return dst;
}

int raise_gpf(){
	asm("int3");
	return 0;
}

void abort(void){
	raise_gpf();
}

void assert(bool cond){
	if(!cond){
		abort();
	}
}

void assert2(bool cond, const char* reason){
	if(!cond){
		printf("< %s >\n", reason);
		abort();
	}
}

int strcmp(const char *str1, const char *str2){
    const unsigned char *p1 = ( const unsigned char * )str1;
    const unsigned char *p2 = ( const unsigned char * )str2;

    while ( *p1 && *p1 == *p2 ) ++p1, ++p2;
    return ( *p1 > *p2 ) - ( *p2  > *p1 );
}

char* strncpy(char* dst, const char* src, size_t num){
	uint32_t i = 0;
	for(i = 0; i < num && ((uint8_t*)src)[i] != 0; i++) {
		((uint8_t*)dst)[i] = ((uint8_t*)src)[i];
	}
	((uint8_t*)dst)[i] = '\x00';
	return 0; // :-(
}