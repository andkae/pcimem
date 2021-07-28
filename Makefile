# **********************************************************************
#  @copyright	: Siemens AG
#  @license		: GPLv3
#  @author		: Andreas Kaeberlein
#  @address		: Clemens-Winkler-Strasse 3, 09116 Chemnitz
#
#  @maintainer	: Andreas Kaeberlein
#  @telephone	: +49 371 4810-2108
#  @email		: andreas.kaeberlein@siemens.com
#
#  @file		: Makefile
#  @date		: 2016-12-06
#
#  @brief		: Build
#				  builds sources with all dependencies
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


all: pcimem_cli

pcimem_cli: pcimem_cli.o pciinfo.o pcimem.o
	$(LINKER) ./obj/pcimem_cli.o ./obj/pciinfo.o ./obj/pcimem.o $(LFLAGS) -o ./bin/pcimem_cli

pcimem_cli.o: ./src/pcimem_cli.c
	$(CC) $(CFLAGS) ./src/pcimem_cli.c -o ./obj/pcimem_cli.o

pciinfo.o: ./inc/pciinfo/src/pciinfo.c
	$(CC) $(CFLAGS) ./inc/pciinfo/src/pciinfo.c -o ./obj/pciinfo.o

pcimem.o: ./src/pcimem.c
	$(CC) $(CFLAGS) ./src/pcimem.c -o ./obj/pcimem.o

ci: ./src/pcimem.c
	$(CC) $(CFLAGS) -Werror ./src/pcimem.c -o ./obj/pcimem.o

clean:
	rm -f ./obj/*o ./bin/pcimem_cli
