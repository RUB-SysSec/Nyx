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

#include <stdint.h>

void hc_set_scratch_area(uintptr_t addr, uint16_t offset, uint32_t seed, uint16_t len);
void hc_set_scratch_area_2(uintptr_t addr, uint32_t seed, uint16_t len);

void hc_mmio_write_32(uint32_t region_base, uint32_t region_size, uint32_t offset, uint32_t data);
void hc_mmio_write_16(uint32_t region_base, uint32_t region_size, uint32_t offset, uint16_t data);
void hc_mmio_write_8(uint32_t region_base, uint32_t region_size, uint32_t offset, uint8_t data);

void hc_mmio_read_32(uint32_t region_base, uint32_t region_size, uint32_t offset);
void hc_mmio_read_16(uint32_t region_base, uint32_t region_size, uint32_t offset);
void hc_mmio_read_8(uint32_t region_base, uint32_t region_size, uint32_t offset);

void hc_mmio_xor_32(uint32_t region_base, uint32_t region_size, uint32_t offset, uint32_t mask);
void hc_mmio_xor_16(uint32_t region_base, uint32_t region_size, uint32_t offset, uint16_t mask);
void hc_mmio_xor_8(uint32_t region_base, uint32_t region_size, uint32_t offset, uint8_t mask);

void hc_mmio_write_bruteforce_32(uint32_t region_base, uint32_t region_size, uint32_t offset, uint32_t data, uint8_t num);
void hc_mmio_write_bruteforce_16(uint32_t region_base, uint32_t region_size, uint32_t offset, uint16_t data, uint8_t num);
void hc_mmio_write_bruteforce_8(uint32_t region_base, uint32_t region_size, uint32_t offset, uint8_t data, uint8_t num);

void hc_mmio_memset_32(uint32_t region_base, uint32_t region_size, uint32_t offset, uint32_t data, uint16_t num);
void hc_mmio_memset_16(uint32_t region_base, uint32_t region_size, uint32_t offset, uint32_t data, uint16_t num);
void hc_mmio_memset_8(uint32_t region_base, uint32_t region_size, uint32_t offset, uint32_t data, uint16_t num);

void hc_mmio_write_scratch_ptr(uint32_t region_base, uint32_t region_size, uint32_t offset);


void hc_io_write_32(uint32_t region_base, uint32_t region_size, uint32_t offset, uint32_t data);
void hc_io_write_16(uint32_t region_base, uint32_t region_size, uint32_t offset, uint32_t data);
void hc_io_write_8(uint32_t region_base, uint32_t region_size, uint32_t offset, uint32_t data);

void hc_io_read_32(uint32_t region_base, uint32_t region_size, uint32_t offset);
void hc_io_read_16(uint32_t region_base, uint32_t region_size, uint32_t offset);
void hc_io_read_8(uint32_t region_base, uint32_t region_size, uint32_t offset);

void hc_io_xor_32(uint32_t region_base, uint32_t region_size, uint32_t offset, uint32_t mask);
void hc_io_xor_16(uint32_t region_base, uint32_t region_size, uint32_t offset, uint32_t mask);
void hc_io_xor_8(uint32_t region_base, uint32_t region_size, uint32_t offset, uint32_t mask);

void hc_io_write_bruteforce_32(uint32_t region_base, uint32_t region_size, uint32_t offset, uint32_t data, uint8_t num);
void hc_io_write_bruteforce_16(uint32_t region_base, uint32_t region_size, uint32_t offset, uint32_t data, uint8_t num);
void hc_io_write_bruteforce_8(uint32_t region_base, uint32_t region_size, uint32_t offset, uint32_t data, uint8_t num);

void hc_io_write_scratch_ptr(uint32_t region_base, uint32_t region_size, uint32_t offset);


void hc_io_memset_32(uint32_t region_base, uint32_t region_size, uint32_t offset, uint32_t data, uint16_t num);
void hc_io_memset_16(uint32_t region_base, uint32_t region_size, uint32_t offset, uint16_t data, uint16_t num);
void hc_io_memset_8(uint32_t region_base, uint32_t region_size, uint32_t offset, uint8_t data, uint16_t num);

void hc_io_writes_32(uint32_t region_base, uint32_t region_size, uint32_t offset, uint32_t data, uint16_t num);
void hc_io_writes_16(uint32_t region_base, uint32_t region_size, uint32_t offset, uint32_t data, uint16_t num);
void hc_io_writes_8(uint32_t region_base, uint32_t region_size, uint32_t offset, uint32_t data, uint16_t num);

void hc_io_reads_32(uint32_t region_base, uint32_t region_size, uint32_t offset, uint16_t num);
void hc_io_reads_16(uint32_t region_base, uint32_t region_size, uint32_t offset, uint16_t num);
void hc_io_reads_8(uint32_t region_base, uint32_t region_size, uint32_t offset, uint16_t num);
