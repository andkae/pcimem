[![Build](https://code.siemens.com/linuxHWtools/pciMemRW/badges/master/pipeline.svg "Build")](https://code.siemens.com/linuxHWtools/pciMemRW/pipelines)

# pciMemRW
CLI tool and library to interact with PCI interfaced registers.


## [CLI](./src/pcimem_main.c)
```bash
Usage:  sudo ./bin/pcimem { vendorID } { deviceID } { bar } { offset } { type } [ data ]
  vendorID : PCI device vendor ID
  deviceID : PCI device device ID
  bar      : PCI Bar of device to accessed
  offset   : offset into pci memory region to act upon
  type     : access operation type : [b]yte, [h]alfword, [w]ord
  data     : data to be written, single value only
```


## [Library](./src/pcimem.h)

### pcimem_init
Initializes common PCI handle data structure.
```c
int pcimem_init( t_pcimem *this );
```

### pcimem_verbose
Sets output level.
```c
int pcimem_verbose( t_pcimem *this, uint8_t level );
```

### pcimem_open
Open memory window to PCI mapped memory.
```c
pcimem_open( t_pcimem *this, char linuxPciBarPath[], uint32_t barOffset=0, uint32_t mapLen=<OSpage> );
```

### pcimem_close
Close memory handle.
```c
int pcimem_close( t_pcimem *this );
```

### pcimem_ptr
Returns memory void pointer to opened PCI memory base.
```c
void* pcimem_ptr( t_pcimem *this );
```


## References
* [Github: devmem2.c](https://github.com/hackndev/tools/blob/master/devmem2.c)
