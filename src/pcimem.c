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
 * @file        pcimem.c
 * @date        Nov 26, 2020
 * @see         https://github.com/andkae/pcimem
 *
 * @brief       PCI MM access
 *
 * provides  read/write access to memory mapped PCI address space
 *
 **********************************************************************/



/** Includes **/
/* Standard libs */
#include <stdio.h>      // f.e. printf
#include <stdint.h>     // defines fixed data types, like int8_t...
#include <unistd.h>     // system call wrapper functions: fork, pipe, read, write, close
#include <fcntl.h>      // open()
#include <string.h>     // string handling functions
/* System specific libs */
#include <sys/mman.h>   // maps virtual to physical space, mmap(), munmap()
/* User Libs */
#include "pcimem.h"     // self



/**
 *  @defgroup off_t_limit
 *
 *  @brief defines off_t datatype limits
 *
 *  max/min value for off_t
 *
 *  @since  2025-10-12
 *  @see https://stackoverflow.com/questions/32278611/maximum-and-minimum-off-t
 */
#ifndef OFF_MAX
    #define OFF_MAX ((((off_t)1 << (sizeof(off_t)*8 - 2)) - 1) * 2 + 1)
#endif

#ifndef OFF_MIN
    #define OFF_MIN (-OFF_MAX - 1)
#endif
/** @} */   // off_t_limit



/**
 *  pcimem_init
 *    initializes common data structure
 */
int pcimem_init (t_pcimem *self)
{
    /* init variable */
    self->intMsgLevel = 0;          // mesage level, disabled
    self->uint8IsOpen = 0;          // handle is open, closed
    self->intBarFh = -1;            // Bar File handle, invalid
    self->voidPtrMem = NULL;        // void pointer to mmap handle
    self->sizeMemPageOffset = 0;    // offset in mempage
    self->sizeMapLen = 0;           // mapping length

    /* finish function */
    return 0;
}



/**
 *  pcimem_verbose
 *    set verbose level
 */
void pcimem_verbose (t_pcimem *self, int level)
{
    self->intMsgLevel = level;  // set message level
}



/**
 *  pcimem_open
 *    open memory window to hardware
 */
int pcimem_open_ (t_pcimem *self, char linuxPciBarPath[], size_t barOffset, size_t mapLen)
{
    /** variables **/
    const int   intPageSize = getpagesize();    // get OS page size in Bytes
    size_t      barOffsetInMemPages;            // PCI Bar offset in multiples of mempages
    off_t       offset;                         // mmap offset


    /* Function Call Message */
    if ( 0 != self->intMsgLevel ) { printf("__FUNCTION__ = %s\n", __FUNCTION__); };

    /* check for empty path */
    if ( 0 == strlen(linuxPciBarPath) ) {
        if ( 0 != self->intMsgLevel ) {
            printf("  ERROR:%s: Empty PCI path provided\n", __FUNCTION__);
        }
        return -1;
    }

    /* get os native page size */
    if ( 0 > intPageSize ) {
        if ( 0 != self->intMsgLevel ) {
            printf("  ERROR:%s: Failed to acquire OS page size\n", __FUNCTION__);
        }
        return -1;
    }
    const size_t pageSize = (size_t) intPageSize;

    /* calculate Page of PCI space, and offset in Page */
    self->sizeMapLen = mapLen;
    barOffsetInMemPages = barOffset / pageSize; // get baroffset in multiple of os page sizes

    if ( OFF_MAX < (barOffsetInMemPages * pageSize) ) {
        if ( self->intMsgLevel != 0 ) { printf("  ERROR:%s: offset exceeded 'void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)'\n", __FUNCTION__); };
        return -1;
    }
    offset = (off_t) (barOffsetInMemPages * pageSize);  // calc offset for mmap

    if ( (size_t) offset > barOffset ) {
        if ( self->intMsgLevel != 0 ) { printf("  ERROR:%s: overflow in bar offset calculation\n", __FUNCTION__); };
        return -1;
    }
    self->sizeMemPageOffset = barOffset - (size_t) offset; // get offset in mempage

    /* user message */
    if ( 0 != self->intMsgLevel ) { // debug output
        printf("  INFO:%s:LINUX:   BARPATH       = %s\n",       __FUNCTION__, linuxPciBarPath);
        printf("  INFO:%s:LINUX:   PAGESIZE      = %zu\n",      __FUNCTION__, pageSize);
        printf("  INFO:%s:LINUX:   MAPLENGTH     = %zu\n",      __FUNCTION__, mapLen);
        printf("  INFO:%s:PCIMEM:  PCIBAROFFSET  = 0x%zx\n",    __FUNCTION__, barOffset);
        printf("  INFO:%s:PCIMEM:  MEMPAGE       = %zu\n",      __FUNCTION__, barOffsetInMemPages);
        printf("  INFO:%s:PCIMEM:  MEMPAGEOFFSET = 0x%08zx\n",  __FUNCTION__, self->sizeMemPageOffset);
    }

    /* Open file handle to bar */
    self->intBarFh = open(linuxPciBarPath, O_RDWR | O_SYNC);
    if ( self->intBarFh == -1 ) {
        if ( self->intMsgLevel != 0 ) { printf("  ERROR:%s: Failed open BAR as file handle\n", __FUNCTION__); };
        return -1;
    }

    /*  Open Memory Window to hardware
     *  SRC: https://www.safaribooksonline.com/library/view/linux-system-programming/0596009585/ch04s03.html
     *  map according length
     *    void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
     */
    self->voidPtrMem = mmap(0, self->sizeMapLen, PROT_READ | PROT_WRITE, MAP_SHARED, self->intBarFh, offset);
    if ( self->voidPtrMem == MAP_FAILED ) {
        if ( 0 != self->intMsgLevel ) {
            printf("  ERROR:%s: Registers mapping failed\n", __FUNCTION__);
            printf("    mmap(0, %zu, PROT_READ | PROT_WRITE, MAP_SHARED, %i, %zu)\n", self->sizeMapLen, self->intBarFh, offset);
        }
        return -1;
    }
    if ( 0 != self->intMsgLevel ) {
        printf("  INFO:%s:LINUX:   PHYSICAL      = %p\n", __FUNCTION__, (void*) self->voidPtrMem);
    }

    /* succesful opened */
    self->uint8IsOpen = 1;

    /* normal end */
    return 0;
}



/**
 *  pcimem_close
 *    closes handle
 */
int pcimem_close (t_pcimem *self)
{
    /* Function Call Message */
    if ( 0 != self->intMsgLevel ) { printf("__FUNCTION__ = %s\n", __FUNCTION__); };

    /* check if handle is open */
    if ( 0 == self->uint8IsOpen ) {
        if ( 0 != self->intMsgLevel ) {
            printf("  ERROR:%s: handle not open\n", __FUNCTION__);
        }
        return -1;
    }

    /* clear flags, access pointer invalid */
    self->uint8IsOpen = 0;

    /*  unmap handle
     *  src: https://linux.die.net/man/3/munmap
     */
    if ( 0 != munmap( (void*) self->voidPtrMem, self->sizeMapLen) ) {
        if ( 0 != self->intMsgLevel ) {
            printf("  ERROR:%s: Handle unmapping failed\n", __FUNCTION__);
            printf("    munmap(0x%zx, %zu)\n", (size_t) self->voidPtrMem, self->sizeMapLen);
        }
        return -1;
    }
    self->voidPtrMem = NULL;
    self->sizeMapLen = 0;

    /* close file handle */
    if ( 0 != close(self->intBarFh) ) {
        if ( 0 != self->intMsgLevel ) {
            printf("  ERROR:%s: close file handle\n", __FUNCTION__);
        }
        return -1;
    }
    self->intBarFh = -1;

    /* normal end */
    return 0;
}



/**
 *  pcimem_ptr
 *    returns memory pointer starting at offset given on open
 */
volatile void* pcimem_ptr (t_pcimem *self)
{
    /* Function Call Message */
    if ( 0 != self->intMsgLevel ) { printf("__FUNCTION__ = %s\n", __FUNCTION__); };

    /* handle open */
    if ( 0 == self->uint8IsOpen ) {
        if ( 0 != self->intMsgLevel ) {
            printf("  ERROR:%s: handle not open\n", __FUNCTION__);
        }
        return NULL;
    }

    /* calc pointer */
    return ((volatile void*) (self->voidPtrMem + self->sizeMemPageOffset));
}
