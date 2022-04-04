# **********************************************************************
#  @copyright   : Siemens AG
#  @license     : GPLv3
#  @author      : Andreas Kaeberlein
#  @address     : Clemens-Winkler-Strasse 3, 09116 Chemnitz
#
#  @maintainer  : Andreas Kaeberlein
#  @telephone   : +49 371 4810-2108
#  @email       : andreas.kaeberlein@siemens.com
#
#  @file        : Makefile
#  @date        : 2016-12-06
#
#  @brief       : Build
#                   builds sources with all dependencies
# **********************************************************************



# select compiler
CC = gcc

# set linker
LINKER = gcc

# set compiler flags
ifeq ($(origin CFLAGS), undefined)
  CFLAGS = -c -O -Wall -Wextra -Wimplicit -Wconversion
endif

# linking flags here
ifeq ($(origin LFLAGS), undefined)
  LFLAGS = -Wall -Wextra -Wimplicit -I. -lm
endif

INSTALL ?= install
PREFIX ?= /usr


all: pcimem

pcimem: pcimem.o pciinfo.o pcimem_lib.o
	$(LINKER) ./obj/pcimem.o ./obj/pciinfo.o ./obj/pcimem_lib.o $(LFLAGS) -o ./bin/pcimem

pcimem.o: ./src/pcimem_main.c
	$(CC) $(CFLAGS) ./src/pcimem_main.c -o ./obj/pcimem.o

pciinfo.o: ./inc/pciinfo/src/pciinfo.c
	$(CC) $(CFLAGS) ./inc/pciinfo/src/pciinfo.c -o ./obj/pciinfo.o

pcimem_lib.o: ./src/pcimem.c
	$(CC) $(CFLAGS) ./src/pcimem.c -o ./obj/pcimem_lib.o

ci: ./src/pcimem.c
	$(CC) $(CFLAGS) -Werror ./src/pcimem.c -o ./obj/pcimem_lib.o


install: ./bin/pcimem
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/bin/
	$(INSTALL) -m 0755 $< $(DESTDIR)$(PREFIX)/bin/


clean:
	rm -f ./obj/*o ./bin/pcimem
