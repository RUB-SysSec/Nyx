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

#pragma pack(push, 1)

typedef struct tagBITMAPFILEHEADER
{
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1; 
    uint16_t bfReserved2; 
    uint32_t bOffBits;
}BITMAPFILEHEADER;

#pragma pack(pop)

#pragma pack(push, 1)

typedef struct tagBITMAPINFOHEADER
{
    uint32_t biSize;
    int32_t biWidth;
    int32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t biXPelsPerMeter;
    int32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
}BITMAPINFOHEADER;

#pragma pack(pop)