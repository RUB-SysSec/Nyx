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

#define outb(__value,__port)   asm volatile ("outb %%al,  %%dx"::"a"(__value),"d"(__port))
#define outw(__value,__port)   asm volatile ("outw %%ax,  %%dx"::"a"(__value),"d"(__port))
#define outl(__value,__port)   asm volatile ("outl %%eax, %%dx"::"a"(__value),"d"(__port))

#define inb(__port)												\
   ({															\
      uint8_t __data;											\
      asm volatile ("inb %%dx,%%al":"=a"(__data):"d"(__port));	\
      __data;													\
   })

#define inw(__port)												\
   ({															\
      uint16_t __data;											\
      asm volatile ("inw %%dx,%%ax":"=a"(__data):"d"(__port));	\
      __data;													\
   })

#define inl(__port)                                   \
   ({                                           \
      uint32_t __data;											\
      asm volatile ("inl %%dx,%%eax":"=a"(__data):"d"(__port));	\
      __data;													\
   })
