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
 * @file        pcimem.h
 * @date        Nov 26, 2020
 *
 * @brief       PCI MM access
 *
 * provides  read/write access to memory mapped PCI address space
 *
 **********************************************************************/



// Define Guard
#ifndef __PCIMEM_H
#define __PCIMEM_H



/**
 *  @typedef t_pci_mem_rw
 *
 *  @brief  common handles
 *
 *  handles all related information for mm pci access
 *
 *  @since  Dec 3, 2020
 *  @author Andreas Kaeberlein
 */
typedef struct t_pcimem {
    uint8_t             uint8DbgMsgLevel;       /**<  mesage level                  */
    uint8_t             uint8IsOpen;            /**<  handle is open                */
    int                 intBarFh;               /**<  Bar File handle               */
    void*               voidPtrMem;             /**<  void pointer to mmap handle   */
    uint32_t            uint32MemPageOffset;    /**<  offset in mempage             */

} t_pcimem;



/* C++ compatibility */
#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus


/** @brief init
 *
 *  initializes common data structure
 *
 *  @param[in,out]  this                storage element @ref t_pci_mem_rw
 *  @return         int                 state
 *  @retval         0                   OK
 *  @since          Dec 03, 2020
 *  @author         Andreas Kaeberlein
 */
int pcimem_init( t_pcimem *this );


/** @brief set verbose
 *
 *  configures verbose level
 *
 *  @param[in,out]  this                storage element @ref t_pci_mem_rw
 *  @param[in]      level               new verbose level
 *  @return         int                 state
 *  @retval         0                   OK
 *  @since          Dec 03, 2020
 *  @author         Andreas Kaeberlein
 */
int pcimem_verbose( t_pcimem *this, uint8_t level );


/** @brief open memory-mapped handle
 *
 *  open memory window to hardware
 *
 *  @param[in,out]  this                storage element @ref t_pci_mem_rw
 *  @param[in]      linuxPciBarPath     linux system path to file which represents PCI device bar
 *  @param[in]      barOffset           bar offset
 *  @return         int                 state
 *  @retval         0                   OK
 *  @retval         -1                  FAIL
 *  @since          Dec 03, 2020
 *  @author         Andreas Kaeberlein
 */
int pcimem_open( t_pcimem *this, char linuxPciBarPath[], uint32_t barOffset );


/** @brief close memory-mapped handle
 *
 *  close open memory window to hardware
 *
 *  @param[in,out]  this                storage element @ref t_pci_mem_rw
 *  @return         int                 state
 *  @retval         0                   OK
 *  @retval         -1                  FAIL
 *  @since          Dec 03, 2020
 *  @author         Andreas Kaeberlein
 */
int pcimem_close( t_pcimem *this );


/** @brief memory pointer
 *
 *  returns memory pointer starting at offset given on open
 *
 *  @param[in,out]  this                storage element @ref t_pci_mem_rw
 *  @return         void*               state
 *  @retval         NULL                memory handle not valid
 *  @retval         VAL                 memory window is open
 *  @since          Jul 28, 2021
 *  @author         Andreas Kaeberlein
 */
void* pcimem_ptr( t_pcimem *this );


#ifdef __cplusplus
}
#endif // __cplusplus

#endif  // __PCI_MEM_RW_H

