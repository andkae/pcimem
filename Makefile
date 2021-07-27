# ***********************************************************************
# @copyright    : Siemens AG, 2020
# @license      : GPLv3
# Address       : Clemens-Winkler-Strasse 3, 09116 Chemnitz
# Telephone     : +49 371 4750
#
# @author       : Andreas Kaeberlein
# Email         : andreas.kaeberlein@siemens.com
# Telephone     : +49 371 4810 2108
#
# @file         : Makefile
#
# @brief        : build project
#
# @date         : 2016-12-06
# *********************************************************************/



# select compiler
CC = gcc

# set linker
LINKER = gcc

# set compiler flags
CFLAGS = -c -O -Wall -Wextra -Wimplicit -Wconversion

# linking flags here
ifeq ($(origin LFLAGS), undefined)
  LFLAGS = -Wall -Wextra -Wimplicit -I. -lm
endif



all: ./bin/pcimem


./bin/pcimem: ./obj/pcimem.o ./obj/pciinfo.o ./obj/pci_mem_rw.o
	$(LINKER) ./obj/pcimem.o ./obj/pciinfo.o ./obj/pci_mem_rw.o $(LFLAGS) -o ./bin/pcimem


./obj/pcimem.o: ./src/pcimem.c
	$(CC) $(CFLAGS) ./src/pcimem.c -o ./obj/pcimem.o

./obj/pciinfo.o: ./inc/pciinfo/src/pciinfo.c
	$(CC) $(CFLAGS) ./inc/pciinfo/src/pciinfo.c -o ./obj/pciinfo.o

./obj/pci_mem_rw.o: ./src/pci_mem_rw.c
	$(CC) $(CFLAGS) ./src/pci_mem_rw.c -o ./obj/pci_mem_rw.o


clean:
	rm -f ./obj/*o ./bin/pcimem
