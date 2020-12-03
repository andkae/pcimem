 /**********************************************************************
 *  @copyright  Siemens AG, 2020
 *  @license    GPLv3
 *  Address     Clemens-Winkler-Strasse 3, 09116 Chemnitz
 *  Telephone   +49 371 4750
 *
 *  @file       pci_mem_rw.c
 *  @author     Andreas Kaeberlein <andreas.kaeberlein@siemens.com>
 *  @date       Nov 26, 2020
 *
 *  @brief      PCI MM access
 *
 *  provides  read/write access to memory mapped PCI address space
 *
 **********************************************************************/



/** Includes **/
/* Standard libs */
#include <stdio.h>          // f.e. printf
#include <stdlib.h>         // defines four variables, several macros,
                            // and various functions for performing
                            // general functions
#include <stdint.h>         // defines fixed data types, like int8_t...
#include <unistd.h>         // system call wrapper functions such as fork, pipe and I/O primitives (read, write, close, etc.).
#include <string.h>         // string handling functions
#include <fcntl.h>          // manipulate file descriptor
/* System specific libs */
#include <sys/types.h>      //
#include <sys/mman.h>       // maps virtual to physical space
/* User Libs */
#include "pci_mem_rw.h"     // self



/**
 *  pci_mem_rw_init
 *    initializes common data structure
 */
int pci_mem_rw_init( t_pci_mem_rw *this )
{
    /* init variable */
    this->uint8DbgMsgLevel = 0;             // mesage level, disabled
    this->uint8IsOpen = 0;                  // handle is open, closed
    this->intBarFh = -1;                    // Bar File handle, invalid
    this->voidPtrMem = NULL;                // void pointer to mmap handle
    this->uint32PageOffset = (uint32_t) -1; // offset in mempage

    /* finish function */
    return 0;
}



/**
 *  x86_mmio_uart_set_verbose
 *    set verbose level
 */
int pci_mem_rw_set_verbose( t_pci_mem_rw *this, uint8_t level )
{
    this->uint8DbgMsgLevel = level; // set message level
    return 0;                       // return function
}



/**
 *  pci_mem_rw_open
 *    open memory window to hardware
 */
int pci_mem_rw_open( t_pci_mem_rw *this, char path[], int8_t bar, uint32_t ofs )
{
    /** variables **/
    const uint32_t  uint32PageSize = (uint32_t) getpagesize();  // get OS page size in Bytes
    char            charPciPath[1024];                          // path to file handle of correponsing bar
    uint32_t        uint32BarOffsetInMemPages;                  // bar offset aligned to mem pages


    /* Function Call Message */
    if ( 0 != this->uint8DbgMsgLevel ) { printf("__FUNCTION__ = %s\n", __FUNCTION__); };

    /* check for empty path */
    if ( 0 == strlen(path) ) {
        if ( this->uint8DbgMsgLevel != 0 ) {
            printf("  ERROR:%s: Empty PCI path provided\n", __FUNCTION__);
        }
        return -1;
    }

    /* check for enough memory
     *   +12: '/resourceX'
     */
    if ( strlen(path)+12 > sizeof(charPciPath)/sizeof(charPciPath[0]) ) {
        if ( this->uint8DbgMsgLevel != 0 ) {
            printf("  ERROR:%s: not enough memory statically allocated\n", __FUNCTION__);
        };
        return -1;
    }

    /* Build Path to Bar, "/resourceX" */
    if ( -1 == bar ) {
        if ( 0 != this->uint8DbgMsgLevel ) {
            printf("  ERROR:%s: BAR=%i is undefined\n", __FUNCTION__, bar);
        };
        return -1;
    }
    charPciPath[0] = '\0';  // make empty
    sprintf(charPciPath, "%s/resource%i", path, bar);

    /* Open bar as file handle */
    this->intBarFh = open(charPciPath, O_RDWR | O_SYNC);
    if ( this->intBarFh == -1 ) {
        if ( 0 != this->uint8DbgMsgLevel ) {
            printf("  ERROR:%s: Failed to open bar as file handle\n", __FUNCTION__);
        };
        return -1;
    }
    if ( 0 != this->uint8DbgMsgLevel ) {
        printf("  INFO:%s: FH            = %s\n", __FUNCTION__, charPciPath);
    }

    /* calculate Page of PCI space, and offset in Page */
    uint32BarOffsetInMemPages   = ofs / uint32PageSize;                                         // bar offset in multiples of pages
    this->uint32PageOffset      = (uint32_t) (ofs - uint32BarOffsetInMemPages*uint32PageSize);  // get offset in mempage
    if ( 0 != this->uint8DbgMsgLevel ) {                                                               // debug output
        printf("  INFO:%s: PAGESIZE      = %i\n",       __FUNCTION__, uint32PageSize);
        printf("  INFO:%s: PCIBAROFFSET  = 0x%08zx\n",  __FUNCTION__, (size_t) ofs);
        printf("  INFO:%s: MEMPAGE       = %i\n",       __FUNCTION__, uint32BarOffsetInMemPages);
        printf("  INFO:%s: MEMPAGEOFFSET = 0x%08zx\n",  __FUNCTION__, (size_t) this->uint32PageOffset);
    }

    /*  Open Memory Window to hardware
     *  SRC: https://www.safaribooksonline.com/library/view/linux-system-programming/0596009585/ch04s03.html
     *  map one complete page
     */
    this->voidPtrMem = mmap(0, uint32PageSize, PROT_READ | PROT_WRITE, MAP_SHARED, this->intBarFh, uint32BarOffsetInMemPages*uint32PageSize);
    if ( MAP_FAILED == this->voidPtrMem ) {
        if ( 0 != this->uint8DbgMsgLevel ) {
            printf("  ERROR:%s: Registers mapping failed\n", __FUNCTION__);
            printf("    mmap(0, %i, PROT_READ | PROT_WRITE, MAP_SHARED, %i, %i)\n", uint32PageSize, this->intBarFh, uint32BarOffsetInMemPages*uint32PageSize);
        }
        return -1;
    }
    if ( 0 != this->uint8DbgMsgLevel ) {
        printf("  INFO:%s: PHYSICAL      = %p\n", __FUNCTION__, (void*) this->voidPtrMem);
    }

    /* succesful opened */
    if ( 0 != this->uint8DbgMsgLevel ) { // debug output
        printf("  INFO:%s: BAR succesful mapped\n", __FUNCTION__);
    }
    this->uint8IsOpen = 1;

    /* normal end */
    return 0;
}



/**
 *  pci_mem_rw_close
 *    closes handle
 */
int pci_mem_rw_close( t_pci_mem_rw *this )
{
    /** variables **/
    const uint32_t  uint32PageSize = (uint32_t) getpagesize();  // get OS page size in Bytes


    /* Function Call Message */
    if ( 0 != this->uint8DbgMsgLevel ) { printf("__FUNCTION__ = %s\n", __FUNCTION__); };

    /* check if handle is open */
    if ( 0 == this->uint8IsOpen ) {
        if ( 0 != this->uint8DbgMsgLevel ) {
            printf("  ERROR:%s: handle not open\n", __FUNCTION__);
        }
        return -1;
    }

    /* clear flags, access pointer invalid */
    this->uint8IsOpen = 0;

    /*  unmap handle
     *  src: https://linux.die.net/man/3/munmap
     */
    if ( 0 != munmap(this->voidPtrMem, uint32PageSize) ) {
        if ( 0 != this->uint8DbgMsgLevel ) {
            printf("  ERROR:%s: Handle unmapping failed\n", __FUNCTION__);
            printf("    munmap(0x%zx, %i)\n", (size_t) this->voidPtrMem, uint32PageSize);
        }
        return -1;
    }
    this->voidPtrMem = NULL;

    /* close file handle */
    if ( 0 != close(this->intBarFh) ) {
        if ( 0 != this->uint8DbgMsgLevel ) {
            printf("  ERROR:%s: close file handle\n", __FUNCTION__);
        }
        return -1;
    }
    this->intBarFh = -1;

    /* normal end */
    if ( 0 != this->uint8DbgMsgLevel ) { // debug output
        printf("  INFO:%s: BAR succesful closed\n", __FUNCTION__);
    }
    return 0;
}



/**
 *  pci_mem_rw_read32
 *    read 32bit value from register
 */
int pci_mem_rw_read32( t_pci_mem_rw *this, uint32_t *val )
{
    /** Variables **/
    volatile uint32_t* uint32PtrMem;    // memory pointer

    /* Function Call Message */
    if ( 0 != this->uint8DbgMsgLevel ) { printf("__FUNCTION__ = %s\n", __FUNCTION__); };

    /* handle open */
    if ( 0 == this->uint8IsOpen ) {
        if ( 0 != this->uint8DbgMsgLevel ) {
            printf("  ERROR:%s: handle not open\n", __FUNCTION__);
        }
        return -1;
    }

    /* build mem pointer */
    uint32PtrMem = ((volatile uint32_t*) (this->voidPtrMem + this->uint32PageOffset));

    /* read value */
    *val = uint32PtrMem[0];

    /* normal end */
    return 0;
}



/**
 *  pci_mem_rw_write32
 *    read 32bit value from register
 */
int pci_mem_rw_write32( t_pci_mem_rw *this, uint32_t val )
{
    /** Variables **/
    volatile uint32_t* uint32PtrMem;    // memory pointer

    /* Function Call Message */
    if ( 0 != this->uint8DbgMsgLevel ) { printf("__FUNCTION__ = %s\n", __FUNCTION__); };

    /* handle open */
    if ( 0 == this->uint8IsOpen ) {
        if ( 0 != this->uint8DbgMsgLevel ) {
            printf("  ERROR:%s: handle not open\n", __FUNCTION__);
        }
        return -1;
    }

    /* build mem pointer */
    uint32PtrMem = ((volatile uint32_t*) (this->voidPtrMem + this->uint32PageOffset));

    /* read value */
    uint32PtrMem[0] = val;

    /* normal end */
    return 0;
}



