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

#define ENABLE_TTY_SERIAL
#define ENABLE_TTY_VGA
#define EARLY_BOOT_PRINTF


#define PANIC_AUTO_REBOOT

//#define IO_FUZZING
#define PCI_FUZZING
//#define EXTENDED_PCI_FUZZING
//#define APIC_FUZZING
//#define HPET_FUZZING

//#define SEED 0xf3cafc34

//#define FILTER "PCI/PCI-Express Bridge"
//#define FILTER "VGA"
//#define FILTER "Ethernet"