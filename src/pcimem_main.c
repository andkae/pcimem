/***********************************************************************
 * @copyright   Siemens AG, 2025
 * @license     BSDv3
 * @author      Andreas Kaeberlein
 * @address     Clemens-Winkler-Strasse 3, 09116 Chemnitz
 *
 * @maintainer  Andreas Kaeberlein
 * @telephone   +49 371 4810-2108
 * @email       andreas.kaeberlein@siemens.com
 *
 * @file        pcimem_main.c
 * @date        Jul 28, 2021
 * @see         https://github.com/hackndev/tools/blob/master/devmem2.c
 *
 * @brief       PCI MM access
 *
 * provides  read/write access to memory mapped PCI address space
 *
 **********************************************************************/



/** Include **/
/* Standard libs */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
/* User Libs */
#include "pciinfo.h"	// PCI device search
#include "pcimem.h"     // provides acces functions



/**
 *  main
 *  ----
 */
int main(int argc, char **argv)
{
    /** Variables **/
    char        charPciBarPath[1024];   // Path to PCI device
    t_pcimem    pciHandle;              // manages interaction with pci devices
    char        charWriteVal[12];
    char        charReadVal[12];
    uint32_t    uint32BarOfs;
    uint32_t    uint32WriteVal;
    uint32_t    uint32MskVal;
    uint32_t    uint32Temp;



    /* check for root rights */
    if (getuid()) {
        printf("ERROR: root rights required! Try 'sudo %s'\n", argv[0]);
        exit(EXIT_FAILURE);
    }


    /* to few arguments */
    if(argc < 6) {
        // pcimem   0x110A  0x4080  0       0x100   w       0x00
        // argv[0]  [1]     [2]     [3]     [4]     [5]     [6]
        printf("\n");
        printf("Usage:\n");
        printf("  %s { vendorID } { deviceID } { bar } { offset } { type } [ data[:mask] ]\n", argv[0]);
        printf("\n");
        printf("Arguments:\n");
        printf("  vendorID     vendor identification of PCI device f.e. 0x110A\n");
        printf("  deviceID     device identification of PCI device f.e. 0x4080\n");
        printf("  bar          bar of targeted PCI device\n");
        printf("  offset       byte offset in BAR\n");
        printf("  type         access data width\n");
        printf("                [b]yte,      8Bit\n");
        printf("                [h]alfword, 16Bit\n");
        printf("                [w]ord,     32Bit\n");
        printf("  data[:mask]  write data\n");
        printf("                if mask is used, then read-modify-write\n");
        printf("\n");
        printf("Contribute:\n");
        printf("  https://github.com/andkae/pcimem\n");
        printf("\n");
        printf("\n");
        exit(EXIT_FAILURE);
    }


    /* search for PCI device */
    if ( 0 != pciinfoBarPath ( argv[1],                                         // vendorID
                               argv[2],                                         // deviceID
                               (uint8_t) atol(argv[3]),                         // bar
                               charPciBarPath,                                  // devicePath
                               sizeof(charPciBarPath)/sizeof(charPciBarPath[0]) // devicePathMax
                             )
    ) {
        printf("ERROR: Find PCI Device with VendorID=%s, DeviceID=%s and Bar=%s\n", argv[1], argv[2], argv[3]);
        exit(EXIT_FAILURE);
    }


    /* init pci handle */
    if ( 0 != pcimem_init(&pciHandle) ) {
        printf("ERROR: Failed to initiliaze PCI memory handle\n");
        exit(EXIT_FAILURE);
    }


    /* set verbose level */
    pcimem_verbose(&pciHandle, 0);


    /* align BAR offset to acces type */
    uint32BarOfs = (uint32_t) strtoul(argv[4], 0, 0);
    switch(tolower(argv[5][0])) {
        case 'h':
            /* halfword access */
            uint32BarOfs &= ~((uint32_t) 1);    // mask byte address
            break;
        case 'w':
            /* word access */
            uint32BarOfs &= ~((uint32_t) 3);    // mask byte/halfword address
            break;
    }


    /* open handle */
    if ( 0 != pcimem_open ( &pciHandle,     // common handle
                            charPciBarPath, // linux system pci bar path
                            uint32BarOfs    // bar offset
                          )
    ) {
        printf("ERROR: Failed to open PCI memory handle\n");
        exit(EXIT_FAILURE);
    }


    /* read access, only if maxima 6 command line arguments given */
    if (argc == 6) {
        switch(tolower(argv[5][0])) {   // access type
            case 'b':
                printf("OFFSET=0x%08zX; DATA=0x%02X;\n", (size_t) uint32BarOfs, *((volatile uint8_t *) pcimem_ptr(&pciHandle))); fflush(stdout);
                break;
            case 'h':
                printf("OFFSET=0x%08zX; DATA=0x%04X;\n", (size_t) uint32BarOfs, *((volatile uint16_t *) pcimem_ptr(&pciHandle))); fflush(stdout);
                break;
            case 'w':
                printf("OFFSET=0x%08zX; DATA=0x%08X;\n", (size_t) uint32BarOfs, *((volatile uint32_t *) pcimem_ptr(&pciHandle))); fflush(stdout);
                break;
            default:
                printf("ERROR: Illegal data type '%c'.\n", tolower(argv[5][0]));
                exit(EXIT_FAILURE);
        }
    /* write access, only one written value supported */
    } else if (argc == 7) {
        /* write or read-modify-write */
        uint32MskVal = (uint32_t) ~0;
        if ( NULL != strstr(argv[6], ":") ) {
            if ( 2 != sscanf(argv[6], "%i:%i", &uint32WriteVal, &uint32MskVal) ) {
                printf("ERROR: Illegal write data '%s'.\n", argv[6]);
                exit(EXIT_FAILURE);
            }
        } else {
            if ( 1 != sscanf(argv[6], "%i", &uint32WriteVal) ) {
                printf("ERROR: Illegal write data '%s'.\n", argv[6]);
                exit(EXIT_FAILURE);
            }
        }
        /* dispatch based on acces type */
        switch(tolower(argv[5][0])) {   // access type
            case 'b':
                /* write */
                uint32Temp = uint32WriteVal;
                /* read modify write */
                if ( (uint32_t) ~0 != uint32MskVal ) {
                    uint32Temp = (uint32_t) *((volatile uint8_t *) pcimem_ptr(&pciHandle)); // read
                    uint32Temp = uint32Temp & ((uint32_t) ~uint32MskVal);                   // clear all masked bits
                    uint32Temp = uint32Temp | (uint32WriteVal & uint32MskVal);              // or set bits into again
                }
                *((volatile uint8_t *) pcimem_ptr(&pciHandle)) = (uint8_t) uint32Temp;  // write value
                /* check */
                sprintf(charReadVal, "0x%02X", *((volatile uint8_t *) pcimem_ptr(&pciHandle))); // read back
                sprintf(charWriteVal, "0x%02X", (uint8_t) uint32Temp);                          // convert write value into hexadecimal string
                break;
            case 'h':
                /* write */
                uint32Temp = uint32WriteVal;
                /* read modify write */
                if ( (uint32_t) ~0 != uint32MskVal ) {
                    uint32Temp = (uint32_t) *((volatile uint16_t *) pcimem_ptr(&pciHandle));    // read
                    uint32Temp = uint32Temp & ((uint32_t) ~uint32MskVal);                       // clear all masked bits
                    uint32Temp = uint32Temp | (uint32WriteVal & uint32MskVal);                  // or set bits into again
                }
                *((volatile uint16_t *) pcimem_ptr(&pciHandle)) = (uint16_t) uint32Temp;        // write value
                /* check */
                sprintf(charReadVal, "0x%04X", *((volatile uint16_t *) pcimem_ptr(&pciHandle)));    // read
                sprintf(charWriteVal, "0x%04X", (uint16_t) uint32Temp);                             // convert write value into hexadecimal string
                break;
            case 'w':
                /* write */
                uint32Temp = uint32WriteVal;
                /* read modify write */
                if ( (uint32_t) ~0 != uint32MskVal ) {
                    uint32Temp = (uint32_t) *((volatile uint32_t *) pcimem_ptr(&pciHandle));    // read
                    uint32Temp = uint32Temp & ((uint32_t) ~uint32MskVal);                       // clear all masked bits
                    uint32Temp = uint32Temp | (uint32WriteVal & uint32MskVal);                  // or set bits into again
                }
                *((volatile uint32_t *) pcimem_ptr(&pciHandle)) = (uint32_t) uint32Temp;        // write value
                /* check */
                sprintf(charReadVal, "0x%08X", *((volatile uint32_t *) pcimem_ptr(&pciHandle)));    // read back
                sprintf(charWriteVal, "0x%08X", (uint32_t) uint32Temp);                             // convert write value into hexadecimal string
                break;
            default:
                printf("ERROR: Illegal data type '%c'.\n", tolower(argv[5][0]));
                exit(EXIT_FAILURE);
        }
        /* perform Write/Read Compare */
        if (strcmp(charReadVal, charWriteVal) != 0) {
            printf("WARNING: WRITE=%s; READ=%s;\n", charWriteVal, charReadVal); fflush(stdout);
        }
    /* write access, multiple data forbidden */
    } else {
        printf("WARNING: Single data value write only supported.\n");
    }


    /* close handle */
    if ( 0 != pcimem_close(&pciHandle) ) {
        printf("ERROR: Failed to close PCI memory handle\n");
        exit(EXIT_FAILURE);
    }


    /* graceful end */
    exit(EXIT_SUCCESS);
}
