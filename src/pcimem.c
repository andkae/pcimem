/**
 *	@file		pcimem.c
 *  @author		Andreas Kaeberlein <andreas.kaeberlein.ext@siemens.com>, Heitec AG
 *  @date		Feb 3, 2017
 *  @version	1.0
 * 
 *  @brief		simple pci bus access tool
 * 
 *  simple pci bus access tool; Original file header below
 *
 *  @copyright	(c) All Rights Reserved
 *  Company		Siemens AG
 *  Address		Clemens-Winkler-Strasse 3, 09116 Chemnitz
 *  Telephone	+49 371 4851
 * 
 */



/*
 * pcimem.c: Simple program to read/write from/to a pci device from userspace.
 *
 *  Copyright (C) 2010, Bill Farrow (bfarrow@beyondelectronics.us)
 *
 *  Based on the devmem2.c code
 *  Copyright (C) 2000, Jan-Derk Bakker (J.D.Bakker@its.tudelft.nl)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */



/** Standard libs **/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/mman.h>


/** User Libs **/
#include "../inc/pciinfo/src/pciinfo.h"


/** 
 *  main
 * 	----
 */
int main(int argc, char **argv) 
{
	/** Variables **/
	const uint32_t	uint32PageSize = getpagesize();		// OS page size in bytes
	int 			fd;
	void			*map_base;
	char 			*vendorID, *deviceID, *bar;
	char 			filename[1024];
	char 			charWriteVal[12];
	char 			charReadVal[12];
	int 			access_type = 'w';
	uint32_t		uint32ActualPage;					// page to mount
	uint32_t		uint32PageOffset;					// offset inside memory page
	uint32_t		uint32BarOfs;						// Bar offset


	/* check for root rights */
	if (getuid()) {
		printf("ERROR: root rights required! Try 'sudo %s'\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	/* to few arguments */
	if(argc < 6) {
		// pcimem	0x110A	0x4080	0 		0x100	w 		0x00
		// argv[0]  [1]   	[2]     [3] 	[4]   	[5]		[6]
		printf("\n");
		printf("Usage:\n");
		printf("  %s vendorID deviceID bar offset type [ data ]\n", argv[0]);
		printf("\n");
		printf("Arguments:\n");
		printf("  vendorID   vendor identification of PCI device f.e. 0x110A\n");
		printf("  deviceID   device identification of PCI device f.e. 0x4080\n");
		printf("  bar        bar of targeted PCI device\n");
		printf("  offset     byte offset in BAR\n");
		printf("  type       access data width\n");
		printf("              [b]yte,      8Bit\n");
		printf("              [h]alfword, 16Bit\n");
		printf("              [w]ord,     32Bit\n");
		printf("  data       write data\n");
		printf("\n");
		printf("\n");
		exit(EXIT_FAILURE);
	}
	
	/* assign command line arguments to variables */
	vendorID 		= argv[1];
	deviceID 		= argv[2];
	bar				= argv[3];
	uint32BarOfs	= (uint32_t) strtoul(argv[4], 0, 0);
	access_type		= tolower(argv[5][0]);

	/* find pci device */
	if (pciinfoFind(vendorID, deviceID, filename, sizeof(filename)/sizeof(filename[0])) != 0) {
		printf("ERROR: Find PCI Device with VendorID=%s and DeviceID=%s\n", vendorID, deviceID);
		exit(EXIT_FAILURE);
	}
	
	/* 	build path to bar
	 * 	before: /sys/bus/pci/devices/0000:03:0d.0/
	 * 	after:	/sys/bus/pci/devices/0000:03:0d.0/resource<bar>
	 */
	strcat(filename, "/resource");
	strcat(filename, bar);	
	
	/* open bar handle */
    if((fd = open(filename, O_RDWR | O_SYNC)) == -1) {
		printf("ERROR: open '%s' failed\n", filename);
		exit(EXIT_FAILURE);
    }
	

    /* calculate page and page offset */
	uint32ActualPage 	= uint32BarOfs / uint32PageSize;					// if offset larger then system page, calc needed page
	uint32PageOffset 	= uint32BarOfs - uint32ActualPage*uint32PageSize;	// calculate offset in actual page
    
    /* Map one page */
    map_base = mmap(0, uint32PageSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, uint32ActualPage);
    if(map_base == (void *) -1) {
		printf("ERROR: mmap(%d, %d, 0x%x, 0x%x, %d, 0x%zx)\n", 0, uint32PageSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, (size_t) uint32ActualPage);
		exit(EXIT_FAILURE);
	}

	/* read access, only if maxima 6 command line arguments given */
    if (argc == 6) {
		switch(access_type) {
			case 'b':
				printf("OFFSET=0x%08x; DATA=0x%02X;\n", (size_t) (uint32BarOfs & ~0), *((uint8_t *) (map_base + (uint32PageOffset & ~0)))); fflush(stdout);
				break;
			case 'h':
				printf("OFFSET=0x%08x; DATA=0x%04X;\n", (size_t) (uint32BarOfs & ~1), *((uint16_t *) (map_base + (uint32PageOffset & ~1)))); fflush(stdout);
				break;
			case 'w':
				printf("OFFSET=0x%08x; DATA=0x%08X;\n", (size_t) (uint32BarOfs & ~3), *((uint32_t *) (map_base + (uint32PageOffset & ~3)))); fflush(stdout);
				break;
			default:
				printf("ERROR: Illegal data type '%c'.\n", access_type);
				exit(EXIT_FAILURE);
		}
	
	/* write access, only one written value supported */
	} else if (argc == 7) {
		switch(access_type) {
			case 'b':
				*((uint8_t *) (map_base + (uint32PageOffset & ~0))) = (uint8_t) strtoul(argv[6], 0, 0);		// write value
				sprintf(charReadVal, "0x%02X", *((uint8_t *) (map_base + (uint32PageOffset & ~0))));		// read back
				sprintf(charWriteVal, "0x%02X", (uint8_t) strtoul(argv[6], 0, 0));							// convert write value into hexadecimal string
				break;
			case 'h':
				*((uint16_t *) (map_base + (uint32PageOffset & ~1))) = (uint16_t) strtoul(argv[6], 0, 0);	// write
				sprintf(charReadVal, "0x%04X", *((uint16_t *) (map_base + (uint32PageOffset & ~1))));		// read
				sprintf(charWriteVal, "0x%04X", (uint16_t) strtoul(argv[6], 0, 0));							// convert write value into hexadecimal string
				break;
			case 'w':
				*((uint32_t *) (map_base + (uint32PageOffset & ~3))) = (uint32_t) strtoul(argv[6], 0, 0);	// write
				sprintf(charReadVal, "0x%08X", *((uint32_t *) (map_base + (uint32PageOffset & ~3))));		// read back
				sprintf(charWriteVal, "0x%08X", (uint32_t) strtoul(argv[6], 0, 0));							// convert write value into hexadecimal string
				break;
		}
		/* perform Write/Read Compare */
		if (strcmp(charReadVal, charWriteVal) != 0) {
			printf("WARNING: WRITE=%s; READ=%s;\n", charWriteVal, charReadVal); fflush(stdout);
		}
	
	/* write access, multiple data forbidden */
	} else {
		printf("WARNING: Single data value write only supported.\n");
	}


	/* unmap PCI handle */
	if(munmap(map_base, uint32PageSize) == -1) {
		printf("ERROR: Unmapping PCI device\n");
		exit(EXIT_FAILURE);
	}
	
	/* close PCI bar file handle */
    close(fd);
    
	/* graceful end */
	exit(EXIT_SUCCESS);
}
