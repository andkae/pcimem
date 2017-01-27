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

#define PRINT_ERROR \
	do { \
		fprintf(stderr, "Error at line %d, file %s (%d) [%s]\n", \
		__LINE__, __FILE__, errno, strerror(errno)); exit(1); \
	} while(0)

#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)




/** 
 *  pcimemFind
 * 	----------
 */
int pcimemFind(char vendorID[], char deviceID[], char bar[], char devicePath[], uint32_t devicePathMax)
{
	/** used variables **/
	char 		cmd[256]; 					// command buffer
	char		devPath[256];				// path buffer
	char 		line[1024], line2[1024];	// read buffer
	uint32_t	uint32DevPathIdx;			// match with buf
	uint8_t		uint8FoundDevice;			// device found
	FILE 		*foundVendor;				// system call answer
	FILE		*foundDevice;

	
	/* search in system path for pci devices*/
	strcpy(cmd, "grep -irnw /sys/bus/pci/devices/*/vendor -e ");
	strcat(cmd, vendorID);
	foundVendor = popen(cmd, "r");
	
	/* process found device list for device ID */
	uint8FoundDevice  = 0;
	while(fscanf(foundVendor, "%s", line) != EOF) {	
		/* 	extract device path for given vendor id
		 * 	before: /sys/bus/pci/devices/0000:03:0d.0/vendor:1:0x110a
		 * 	after:	/sys/bus/pci/devices/0000:03:0d.0
		 */
		if (strstr(line, "/vendor")-line < (int)(sizeof(devPath)/sizeof(devPath[0]))) {
			uint32DevPathIdx = strstr(line, "/vendor")-line;
		} else {
			uint32DevPathIdx = 0;
		}
		strncpy(devPath, line, uint32DevPathIdx);
		devPath[uint32DevPathIdx] = '\0';		// termination character
		
		/*	build command for device id look up
		 * 	cat /sys/bus/pci/devices/0000:03:0d.0/device
		*/
		strcpy(cmd, "cat ");
		strcat(cmd, devPath);
		strcat(cmd, "/device");
		foundDevice = popen(cmd, "r");
		
		/* process system call response */
		fscanf(foundDevice, "%s", line2);	// read only first line
		if (strcmp(line2, deviceID) == 0) {
			++uint8FoundDevice;				// increment match counter
		}
		
		/* close opened pipe */
		pclose(foundDevice);
	}

	/* 	build path to bar
	 * 	before: /sys/bus/pci/devices/0000:03:0d.0/
	 * 	after:	/sys/bus/pci/devices/0000:03:0d.0/resource<bar>
	 */
	strcat(devPath, "/resource");
	strcat(devPath, bar);
	
	/* release path */
	if (uint8FoundDevice == 1) {
		strncpy(devicePath, devPath, devicePathMax);
	} else {
		devicePath[0] = '\0';
	}
	
	/* close opened pipe */
	pclose(foundVendor);

	/* finish function */
	return uint8FoundDevice-1;
}



/** 
 *  main
 * 	----
 */
int main(int argc, char **argv) {
	int fd;
	void *map_base, *virt_addr;
	uint32_t read_result, writeval;
	char *vendorID, *deviceID, *bar;
	char filename[1024];
	off_t target;
	int access_type = 'w';


	/* check for root rights */
	if (getuid()) {
		printf("ERROR: root rights required! Try sudo.\n");
		exit(EXIT_FAILURE);
	}

	/* to few arguments */
	if(argc < 6) {
		// pcimem	0x110A	0x4080	0 		0x100	w 		0x00
		// argv[0]  [1]   	[2]     [3] 	[4]   	[5]		[6]
		fprintf(stderr, "\nUsage:\t%s { vendorID } { deviceID } { bar } { offset } [ type [ data ] ]\n"
			"\tvendorID: vendor identification of PCI device f.e. 0x110A\n"
			"\tdeviceID: device identification of PCI device f.e. 0x4080\n"
			"\tbar     : bar of targeted PCI device\n"
			"\toffset  : offset into pci memory region to act upon\n"
			"\ttype    : access operation type : [b]yte, [h]alfword, [w]ord\n"
			"\tdata    : data to be written\n\n",
			argv[0]);
		exit(1);
	}
	vendorID 	= argv[1];
	deviceID 	= argv[2];
	bar			= argv[3];
	target 		= strtoul(argv[4], 0, 0);

	/* find pci device */
	if (pcimemFind(vendorID, deviceID, bar, filename, sizeof(filename)/sizeof(filename[0])) != 0) PRINT_ERROR;

	/* check more arguments */
	if(argc > 6)
		access_type = tolower(argv[5][0]);
		
	/* open bar handle */
    if((fd = open(filename, O_RDWR | O_SYNC)) == -1) PRINT_ERROR;
    printf("%s opened.\n", filename);
    printf("Target offset is 0x%x, page size is %ld\n", (int) target, sysconf(_SC_PAGE_SIZE));
    fflush(stdout);

    /* Map one page */
    printf("mmap(%d, %ld, 0x%x, 0x%x, %d, 0x%x)\n", 0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, (int) target);
    map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, target & ~MAP_MASK);
    if(map_base == (void *) -1) PRINT_ERROR;
    printf("PCI Memory mapped to address 0x%08lx.\n", (unsigned long) map_base);
    fflush(stdout);

    virt_addr = map_base + (target & MAP_MASK);
    switch(access_type) {
		case 'b':
			read_result = *((uint8_t *) virt_addr);
			break;
		case 'h':
			read_result = *((uint16_t *) virt_addr);
			break;
		case 'w':
			read_result = *((uint32_t *) virt_addr);
			break;
		default:
			fprintf(stderr, "Illegal data type '%c'.\n", access_type);
			exit(2);
	}
    printf("Value at offset 0x%X (%p): 0x%X\n", (int) target, virt_addr, read_result);
    fflush(stdout);

	if(argc > 6) {
		writeval = strtoul(argv[6], 0, 0);
		switch(access_type) {
			case 'b':
				*((uint8_t *) virt_addr) = writeval;
				read_result = *((uint8_t *) virt_addr);
				break;
			case 'h':
				*((uint16_t *) virt_addr) = writeval;
				read_result = *((uint16_t *) virt_addr);
				break;
			case 'w':
				*((uint32_t *) virt_addr) = writeval;
				read_result = *((uint32_t *) virt_addr);
				break;
		}
		printf("Written 0x%X; readback 0x%X\n", writeval, read_result);
		fflush(stdout);
	}

	if(munmap(map_base, MAP_SIZE) == -1) PRINT_ERROR;
    close(fd);
    return 0;
}
