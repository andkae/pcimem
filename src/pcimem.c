/***********************************************************************
 * @copyright   Siemens AG, 2021
 * @license     GPLv3
 * @author      Andreas Kaeberlein
 * @address     Clemens-Winkler-Strasse 3, 09116 Chemnitz
 *
 * @maintainer  Andreas Kaeberlein
 * @telephone   +49 371 4810-2108
 * @email       andreas.kaeberlein@siemens.com
 *
 * @file        pcimem.c
 * @date        Nov 26, 2020
 *
 * @brief       PCI MM access
 *
 * provides  read/write access to memory mapped PCI address space
 *
 **********************************************************************/



/** Includes **/
/* Standard libs */
#include <stdio.h>      // f.e. printf
#include <stdlib.h>     // defines four variables, several macros,
                        // and various functions for performing
                        // general functions
#include <stdint.h>     // defines fixed data types, like int8_t...
#include <unistd.h>     // system call wrapper functions such as fork, pipe and I/O primitives (read, write, close, etc.).
#include <string.h>     // string handling functions
#include <fcntl.h>      // manipulate file descriptor
#include <stdarg.h>     // variable parameter list
/* System specific libs */
#include <sys/types.h>  //
#include <sys/mman.h>   // maps virtual to physical space
/* User Libs */
#include "pcimem.h"     // self



/**
 *  pcimem_init
 *    initializes common data structure
 */
int pcimem_init( t_pcimem *this )
{
    /* init variable */
    this->uint8DbgMsgLevel = 0;     // mesage level, disabled
    this->uint8IsOpen = 0;          // handle is open, closed
    this->intBarFh = -1;            // Bar File handle, invalid
    this->voidPtrMem = NULL;        // void pointer to mmap handle
    this->uint32MemPageOffset = 0;  // offset in mempage
    this->uint32MapLen = 0;         // mapping length

    /* finish function */
    return 0;
}



/**
 *  pcimem_verbose
 *    set verbose level
 */
int pcimem_verbose( t_pcimem *this, uint8_t level )
{
    this->uint8DbgMsgLevel = level; // set message level
    return 0;                       // return function
}



/**
 *  pcimem_open
 *    open memory window to hardware
 */
int pcimem_open_( t_pcimem *this, char linuxPciBarPath[], uint32_t barOffset, uint32_t mapLen)
{
    /** variables **/
    const uint32_t  uint32PageSize = (uint32_t) getpagesize();  // get OS page size in Bytes
    uint32_t        uint32BarOffsetInMemPages;                  // PCI Bar offset in multiples of mempages


    /* Function Call Message */
    if ( 0 != this->uint8DbgMsgLevel ) { printf("__FUNCTION__ = %s\n", __FUNCTION__); };

    /* check for empty path */
    if ( 0 == strlen(linuxPciBarPath) ) {
        if ( this->uint8DbgMsgLevel != 0 ) {
            printf("  ERROR:%s: Empty PCI path provided\n", __FUNCTION__);
        }
        return -1;
    }

    /* calculate Page of PCI space, and offset in Page */
    uint32BarOffsetInMemPages   = barOffset / uint32PageSize;                           // 32Bit address, get in multiple of page sizes
    this->uint32MemPageOffset   = barOffset - uint32BarOffsetInMemPages*uint32PageSize; // get offset in mempage
    if ( 0 != this->uint8DbgMsgLevel ) {                                                // debug output
        printf("  INFO:%s:LINUX:   BARPATH       = %s\n",       __FUNCTION__, linuxPciBarPath);
        printf("  INFO:%s:LINUX:   PAGESIZE      = %i\n",       __FUNCTION__, uint32PageSize);
        printf("  INFO:%s:PCIMEM:  PCIBAROFFSET  = 0x%08zx\n",  __FUNCTION__, (size_t) barOffset);
        printf("  INFO:%s:PCIMEM:  MEMPAGE       = %i\n",       __FUNCTION__, uint32BarOffsetInMemPages);
        printf("  INFO:%s:PCIMEM:  MEMPAGEOFFSET = 0x%08zx\n",  __FUNCTION__, (size_t) this->uint32MemPageOffset);
        printf("  INFO:%s:PCIMEM:  MAPLENGTH     = %i\n",       __FUNCTION__, mapLen);
    }

    /* Open file handle to bar */
    this->intBarFh = open(linuxPciBarPath, O_RDWR | O_SYNC);
    if ( this->intBarFh == -1 ) {
        if ( this->uint8DbgMsgLevel != 0 ) { printf("  ERROR:%s: Failed open BAR as file handle\n", __FUNCTION__); };
        return -1;
    }

    /*  Open Memory Window to hardware
     *  SRC: https://www.safaribooksonline.com/library/view/linux-system-programming/0596009585/ch04s03.html
     *  map according length
     *    void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
     */
    this->uint32MapLen = mapLen;
    this->voidPtrMem = mmap(0, this->uint32MapLen, PROT_READ | PROT_WRITE, MAP_SHARED, this->intBarFh, uint32BarOffsetInMemPages*uint32PageSize);
    if ( this->voidPtrMem == MAP_FAILED ) {
        if ( 0 != this->uint8DbgMsgLevel ) {
            printf("  ERROR:%s: Registers mapping failed\n", __FUNCTION__);
            printf("    mmap(0, %i, PROT_READ | PROT_WRITE, MAP_SHARED, %i, %i)\n", this->uint32MapLen, this->intBarFh, uint32BarOffsetInMemPages*uint32PageSize);
        }
        return -1;
    }
    if ( 0 != this->uint8DbgMsgLevel ) {
        printf("  INFO:%s:LINUX:   PHYSICAL      = %p\n", __FUNCTION__, (void*) this->voidPtrMem);
    }

    /* succesful opened */
    this->uint8IsOpen = 1;

    /* normal end */
    return 0;
}



/**
 *  pcimem_close
 *    closes handle
 */
int pcimem_close( t_pcimem *this )
{
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
    if ( 0 != munmap( (void*) this->voidPtrMem, this->uint32MapLen) ) {
        if ( 0 != this->uint8DbgMsgLevel ) {
            printf("  ERROR:%s: Handle unmapping failed\n", __FUNCTION__);
            printf("    munmap(0x%zx, %i)\n", (size_t) this->voidPtrMem, this->uint32MapLen);
        }
        return -1;
    }
    this->voidPtrMem = NULL;
    this->uint32MapLen = 0;

    /* close file handle */
    if ( 0 != close(this->intBarFh) ) {
        if ( 0 != this->uint8DbgMsgLevel ) {
            printf("  ERROR:%s: close file handle\n", __FUNCTION__);
        }
        return -1;
    }
    this->intBarFh = -1;

    /* normal end */
    return 0;
}



/**
 *  pcimem_ptr
 *    returns memory pointer starting at offset given on open
 */
volatile void* pcimem_ptr( t_pcimem *this )
{
    /* Function Call Message */
    if ( 0 != this->uint8DbgMsgLevel ) { printf("__FUNCTION__ = %s\n", __FUNCTION__); };

    /* handle open */
    if ( 0 == this->uint8IsOpen ) {
        if ( 0 != this->uint8DbgMsgLevel ) {
            printf("  ERROR:%s: handle not open\n", __FUNCTION__);
        }
        return NULL;
    }

    /* calc pointer */
    return ((volatile void*) (this->voidPtrMem + this->uint32MemPageOffset));
}
