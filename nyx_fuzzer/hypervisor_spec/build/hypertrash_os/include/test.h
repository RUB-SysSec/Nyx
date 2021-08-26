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

#ifndef TEST_H
#define TEST_H

enum test_type { 
	TEST_DIV_BY_ZERO,
	TEST_PAGE_FAULT,
	TEST_PCI_MMIO,
	TEST_PCI_IO,
	TEST_MSR,
	TEST_SMEP,
	TEST_CVE_2015_3456,
	TEST_CVE_2011_1751,
	TEST_IO
};

void test(enum test_type id);

#endif 