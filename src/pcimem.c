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
#include "../inc/bus/pci/pciinfo/pciinfo.h"


/** Precompiler directives **/
#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)


/** 
 *  main
 * 	----
 */
int main(int argc, char **argv) {
	int fd;
	void *map_base, *virt_addr;
	char *vendorID, *deviceID, *bar;
	char filename[1024];
	char charWriteVal[12];
	char charReadVal[12];
	
	off_t target;
	int access_type = 'w';


	/* check for root rights */
	if (getuid()) {
		printf("ERROR: root rights required! Try 'sudo ./%s'\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	/* to few arguments */
	if(argc < 6) {
		// pcimem	0x110A	0x4080	0 		0x100	w 		0x00
		// argv[0]  [1]   	[2]     [3] 	[4]   	[5]		[6]
		fprintf(stderr, "\nUsage:\t%s { vendorID } { deviceID } { bar } { offset } { type } [ data ] ]\n"
			"\tvendorID: vendor identification of PCI device f.e. 0x110A\n"
			"\tdeviceID: device identification of PCI device f.e. 0x4080\n"
			"\tbar     : bar of targeted PCI device\n"
			"\toffset  : offset into pci memory region to act upon\n"
			"\ttype    : access operation type : [b]yte, [h]alfword, [w]ord\n"
			"\tdata    : data to be written\n\n",
			argv[0]);
		exit(1);
	}
	
	/* assign command line arguments to variables */
	vendorID 	= argv[1];
	deviceID 	= argv[2];
	bar			= argv[3];
	target 		= strtoul(argv[4], 0, 0);

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
	
    /* Map one page */
    map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, target & ~MAP_MASK);
    if(map_base == (void *) -1) {
		printf("ERROR: mmap(%d, %ld, 0x%x, 0x%x, %d, 0x%x)\n", 0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, (int) target);
		exit(EXIT_FAILURE);
	}

	/* prepare for PCI memory access */
	virt_addr	= map_base + (target & MAP_MASK);
	access_type	= tolower(argv[5][0]);

	/* read access, only if maxima 6 command line arguments given */
    if (argc == 6) {
		switch(access_type) {
			case 'b':
				printf("OFFSET=0x%X; DATA=0x%02X;\n", (int) target, *((uint8_t *) virt_addr)); fflush(stdout);
				break;
			case 'h':
				printf("OFFSET=0x%X; DATA=0x%04X;\n", (int) target, *((uint16_t *) virt_addr)); fflush(stdout);
				break;
			case 'w':
				printf("OFFSET=0x%X; DATA=0x%08X;\n", (int) target, *((uint32_t *) virt_addr)); fflush(stdout);
				break;
			default:
				printf("ERROR: Illegal data type '%c'.\n", access_type);
				exit(EXIT_FAILURE);
		}
	
	/* write access, only one written value supported */
	} else if (argc == 7) {
		switch(access_type) {
			case 'b':
				*((uint8_t *) virt_addr) = strtoul(argv[6], 0, 0);
				sprintf(charReadVal, "0x%08X", *((uint8_t *) virt_addr));
				break;
			case 'h':
				*((uint16_t *) virt_addr)	= strtoul(argv[6], 0, 0);
				sprintf(charReadVal, "0x%08X", *((uint16_t *) virt_addr));
				break;
			case 'w':
				*((uint32_t *) virt_addr)	= strtoul(argv[6], 0, 0);
				sprintf(charReadVal, "0x%08X", *((uint32_t *) virt_addr));
				break;
		}
		/* perform Write/Read Compare */
		sprintf(charWriteVal, "0x%08X", (uint32_t) strtoul(argv[6], 0, 0));	// convert write value into hexadecimal string
		if (strcmp(charReadVal, charWriteVal) != 0) {
			printf("WARNING: WRITE=%s; READ=%s;\n", charWriteVal, charReadVal); fflush(stdout);
		}
	
	/* write access, multiple data forbidden */
	} else {
		printf("WARNING: Single data value write only supported.\n");
	}


	/* unmap PCI handle */
	if(munmap(map_base, MAP_SIZE) == -1) {
		printf("ERROR: Unmapping PCI device\n");
		exit(EXIT_FAILURE);
	}
	
	/* close PCI bar file handle */
    close(fd);
    
	/* graceful end */
	exit(EXIT_SUCCESS);
}
