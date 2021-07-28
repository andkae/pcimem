[![Build](https://code.siemens.com/linuxHWtools/pciMemRW/badges/master/pipeline.svg "Build")](https://code.siemens.com/linuxHWtools/pciMemRW/pipelines)

# pciMemRW
CLI tool and library to interact with PCI interfaced registers.


## [CLI](./src/pcimem_cli.c)
```bash
Usage:  sudo ./pcimem_cli { vendorID } { deviceID } { bar } { offset } { type } [ data ]
  vendorID : PCI device vendor ID
  deviceID : PCI device device ID
  bar      : PCI Bar of device to accessed
  offset   : offset into pci memory region to act upon
  type     : access operation type : [b]yte, [h]alfword, [w]ord
  data     : data to be written, single value only
```


## [Library](./src/pcimem.c)


## References
* [Github: devmem2.c](https://github.com/hackndev/tools/blob/master/devmem2.c)
