/*
 * HyperCube OS
 * (c) Sergej Schumilo, 2019 <sergej@schumilo.de> 
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/fcntl.h>
#include <assert.h>
#include "state.h"
#include "core.h"

#define USAGE() 	printf("Usage %s <config> file\n", argv[0])

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

int main(int argc, char** argv){
	state_t* state_obj = NULL;
	uint32_t len = 0;
	hexa_op* input_base = NULL;
	void* config_data = 0;
	uint32_t config_len = 0;
	
	assert((uint32_t)sizeof(hexa_op) == 6);

	if (argc != 2 && argc != 3){
		USAGE();
		return 0;
	}

	if(argc == 2){
		input_base = (hexa_op*)mapfile(argv[1], &len);
		if (!input_base){
			printf("ERROR: File not found %d\n", len);
			USAGE();
			return 1;
		}
		state_obj = new_state();
	}
	else{
		input_base = (hexa_op*)mapfile(argv[2], &len);
		if (!input_base){
			printf("ERROR: File not found %d\n", len);
			USAGE();
			return 1;
		}

		config_data = mapfile(argv[1], &config_len);
		if (!config_data){
			printf("ERROR: File not found %d\n", len);
			USAGE();
			return 1;
		}

		state_obj = load_state(config_data, config_len);
	}

	
	run(input_base, len, state_obj);
	destroy_state(state_obj);

	munmap(input_base, len);
	if(argc == 3){
		munmap(config_data, config_len);
	}


	return 0;
}
