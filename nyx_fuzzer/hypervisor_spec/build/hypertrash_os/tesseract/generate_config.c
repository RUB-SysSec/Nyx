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

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "state.h"
#include <unistd.h>
#include <sys/mman.h>
#include <sys/fcntl.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>

#define round_up(x, y) (((x) + (y) - 1) & ~((y) - 1))

static int pagesize;

static void __attribute__((constructor)) init_ps(void)
{
        pagesize = sysconf(_SC_PAGESIZE);
}

void *mapfile(char *fn, uint32_t *size)
{
        int fd = open(fn, O_RDONLY);
        if (fd < 0){
            return NULL;
        }
        struct stat st;
        void *map = (void *)-1L;
        if (fstat(fd, &st) >= 0) {
                *size = (uint32_t)st.st_size;
                map = mmap(NULL, round_up(st.st_size, pagesize),
                           PROT_READ,
                           MAP_PRIVATE, fd, 0);
        }
        close(fd);
        return map != (void *)-1L ? map : NULL;
}


void unmapfile(void *map, uint32_t size)
{
    munmap(map, round_up((uint32_t)size, pagesize));
}

int main(){

	void* saved_data = NULL;
	uint32_t saved_len = 0;

	void* config_data = 0;
	uint32_t config_len = 0;

	config_t config;
	area_t_export areas[3];
	memset(&config, 0x0, sizeof(config_t));
	memset(&areas, 0x0, sizeof(area_t_export) * 3);

	config.magic = MAGIC_NUMBER; 
	config.num_mmio_areas = 1;
	config.num_io_areas = 2;
	config.num_alloc_areas = 4;

	areas[0].base = 0xbad000;
	areas[0].size = 0x400;
	strncpy(areas[0].desc, "PCI: GPU 0034:3230", sizeof("PCI: GPU 0034:3230"));

	areas[1].base = 0xd000;
	areas[1].size = 0x50;
	strncpy(areas[1].desc, "PCI: NETWORK 1234:4444", sizeof("PCI: NETWORK 1234:4444"));

	areas[2].base = 0x1500;
	areas[2].size = 0x70;
	strncpy(areas[2].desc, "PCI: USB 0412:4212", sizeof("PCI: USB 0412:4212"));

	FILE* f = fopen ("/tmp/config.hypertrash", "w");
	fwrite(&config, sizeof(config_t), 1, f);
	fwrite(&areas, sizeof(area_t_export), 3, f);
	fclose(f); 

	config_data = mapfile("/tmp/config.hypertrash", &config_len);
	if (!config_data){
		printf("ERROR: File not found\n");
		return 1;
	}

	state_t* state_data = load_state(config_data, config_len);

	save_state(state_data, &saved_data, &saved_len);

	assert(!memcmp(config_data, saved_data, saved_len));
	munmap(config_data, config_len);

	return 0;
}