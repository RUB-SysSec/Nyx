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


#include "smp.h"
#include "apic.h"
#include "tty.h"
#include "mmio.h"
#include "libk.h"
#include "mem.h"
#include "io.h"
#include "fuzz.h"


#define STR_PREFIX	" [ISA]  "
#define STR_PIO_PREFIX	" [PIO]  "


#define PNP_ADDRESS		0x279
#define PNP_WRITE_DATA	0xa79

#define PNP_MAX_CARDS 8

#define PNP_SET_RD_DATA		0x00
#define PNP_SERIAL_ISOLATION	0x01
#define	PNP_CONFIG_CONTROL	0x02
#define PNP_CONFIG_CONTROL_RESET_CSN	0x04
#define PNP_CONFIG_CONTROL_WAIT_FOR_KEY	0x02
#define PNP_CONFIG_CONTROL_RESET	0x01
#define PNP_WAKE		0x03

typedef struct _pnp_id {
	uint32_t vendor_id;
	uint32_t serial;
	uint8_t checksum;
} pnp_id;

static void isa_pnp_init(){

	/* credit: freebsd */
	int cur, i;

	outb(0, PNP_ADDRESS);
	outb(0, PNP_ADDRESS);

	cur = 0x6a;
	outb(cur, PNP_ADDRESS);

	for (i = 1; i < 32; i++) {
		cur = (cur >> 1) | (((cur ^ (cur >> 1)) << 7) & 0xff);
		outb(cur, PNP_ADDRESS);
	}
}

static void pnp_write(uint8_t d, uint8_t r){
	outb(d, PNP_ADDRESS);
	outb(r, PNP_WRITE_DATA);
}

static uint8_t pnp_get_serial(pnp_id *p, uint16_t read_port){
	int i, bit, valid = 0, sum = 0x6a;
	uint8_t *data = (uint8_t *)p;

	memset(data, 0, sizeof(char) * 9);
	outb(PNP_SERIAL_ISOLATION, PNP_ADDRESS);
	for (i = 0; i < 72; i++) {
		bit = inb(read_port) == 0x55;

		microdelay(2500000);

		bit = (inb(read_port) == 0xaa) && bit;
		microdelay(2500000);

		valid = valid || bit;
		if (i < 64)
			sum = (sum >> 1) |
			  (((sum ^ (sum >> 1) ^ bit) << 7) & 0xff);
		data[i / 8] = (data[i / 8] >> 1) | (bit ? 0x80 : 0);
	}

	valid = valid && (data[8] == sum);

	return (valid);
}


uint32_t isa_pnp_isolation(uint16_t read_port){
	pnp_id id;
	isa_pnp_init();

	pnp_write(PNP_CONFIG_CONTROL, PNP_CONFIG_CONTROL_RESET_CSN);
	pnp_write(PNP_WAKE, 0);
	pnp_write(PNP_SET_RD_DATA, read_port);

	for (uint8_t csn = 1; csn < PNP_MAX_CARDS; csn++) {
		outb(PNP_SERIAL_ISOLATION, PNP_ADDRESS);
		microdelay(1000000);

		/* more magic */
		if(pnp_get_serial(&id, read_port)){
			printf(STR_PREFIX"FOUND SOMETHING\n");
		}

		pnp_write(PNP_WAKE, 0);
	}

	return 0;
}


void isa_state_enum(fuzzer_state_t* fuzzer){
	uint32_t num_devs = 0; 

	uint32_t last_io = 0;
	bool last_pending = false;
	uint32_t prev_io = 0xfff;
	for(uint32_t i = 0xA0; i < 0x5000; i++){

		if(i == 0x8900) /* BOCHS SHUTDOWN */
			continue;

		if(i == 0x604) /* QEMU SHUTDOWN */
			continue;

		if(i == 0x455)
			continue; // VMWARE ?

		uint8_t value = inb(i);
			if(value != 0xff){			
				if(last_pending && prev_io+1 != i){
					printf(STR_PIO_PREFIX"found ports at %x - %x\n\r", last_io, prev_io);

					register_area(fuzzer, last_io, (prev_io-last_io)+1, 0, "PIO");
					last_pending = false;
				}
				
				if(!last_pending){
					last_io = i;
					last_pending = true;
				}

				prev_io = i;
				
			}
			else{
				for(uint8_t j = 0; j < 0xf; j++){
					outb(j, i);
					if(value != inb(i)){
						printf("=>>>>> PORT FOUND %x\n", i);
						outb(i, value);
						break;
					}
					outb(value, i);
				}
				if(value && value != 0xff){
					printf("=>>>>> PORT FOUND %x %x\n", i, value);
				}
				outb(value+1, i);
				if(value != inb(i)){
					printf("=>>>>> PORT FOUND %x\n", i);
				}
				outb(value, i);
				
			}
	}
	return;

	for(uint16_t read_port = 0x203; read_port <= 0x3ff; read_port++){
		num_devs = isa_pnp_isolation(read_port);
		if(num_devs){
			break;
		}
	}
	printf(STR_PREFIX"%d pnp devices found!\n", num_devs);
}


