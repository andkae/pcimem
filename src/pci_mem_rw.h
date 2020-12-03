 /**********************************************************************
 *  @copyright  Siemens AG, 2020
 *  @license    GPLv3
 *  Address     Clemens-Winkler-Strasse 3, 09116 Chemnitz
 *  Telephone   +49 371 4750
 *
 *  @file       pci_mem_rw.h
 *  @author     Andreas Kaeberlein <andreas.kaeberlein@siemens.com>
 *  @date       Nov 26, 2020
 * 
 *  @brief      PCI MM access
 * 
 *  provides  read/write access to memory mapped PCI address space
 *
 **********************************************************************/



// Define Guard
#ifndef __PCI_MEM_RW_H
#define __PCI_MEM_RW_H



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
typedef struct t_pci_mem_rw {
    uint8_t             uint8DbgMsgLevel;           /**<  mesage level      			*/
    uint8_t             uint8IsOpen;              	/**<  handle is open           		*/
    int                 intBarFh;                   /**<  Bar File handle               */
    void*               voidPtrMem;                 /**<  void pointer to mmap handle   */
    uint32_t			uint32PageOffset;			/**<  offset in mempage         	*/
    
} t_pci_mem_rw;



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
int pci_mem_rw_init( t_pci_mem_rw *this );


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
int pci_mem_rw_set_verbose( t_pci_mem_rw *this, uint8_t level );


/** @brief open memory-mapped handle
 *
 *  open memory window to hardware
 *
 *  @param[in,out]  this                storage element @ref t_pci_mem_rw
 *  @param[in]      path                linux system path to file which represents PCI device bar
 *  @param[in]      bar                 pci device bar
 *  @param[in]      ofs                 bar offset
 *  @return         int                 state
 *  @retval         0                   OK
 *  @retval         -1                  FAIL
 *  @since          Dec 03, 2020
 *  @author         Andreas Kaeberlein
 */
int pci_mem_rw_open( t_pci_mem_rw *this, char path[], int8_t bar, uint32_t ofs );


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
int pci_mem_rw_close( t_pci_mem_rw *this );


/** @brief read32
 *
 *  read 32bit value from register
 *
 *  @param[in,out]  this                storage element @ref t_pci_mem_rw
 *  @param[in,out]  val                 register read value
 *  @return         int                 state
 *  @retval         0                   OK
 *  @retval         -1                  FAIL
 *  @since          Dec 03, 2020
 *  @author         Andreas Kaeberlein
 */
int pci_mem_rw_read32( t_pci_mem_rw *this, uint32_t *val );


/** @brief write32
 *
 *  write 32bit value
 *
 *  @param[in,out]  this                storage element @ref t_pci_mem_rw
 *  @param[in]      val                 register write
 *  @return         int                 state
 *  @retval         0                   OK
 *  @retval         -1                  FAIL
 *  @since          Dec 03, 2020
 *  @author         Andreas Kaeberlein
 */
int pci_mem_rw_write32( t_pci_mem_rw *this, uint32_t val );





#ifdef __cplusplus
}
#endif // __cplusplus

#endif  // __PCI_MEM_RW_H

