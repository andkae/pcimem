# **********************************************************************
# Copyright		: (c) All Rights Reserved
# Company		: Siemens AG
# Address		: Clemens-Winkler-Strasse 3, 09116 Chemnitz
# Telephone		: +49 371 4750
# 
# @author		: Andreas Kaeberlein
#			
# eMail			: andreas.kaeberlein.ext@siemens.com
#
# @file			: Makefile
#
# @brief		: build project
#
# @date			: 2016-12-06
# **********************************************************************



# select compiler
CC=gcc

# set compiler flags
CFLAGS=-c -Wall -Wextra

# set linker
LINKER   = gcc

# linking flags here
LFLAGS   = -Wall -Wextra -I. -lm



all: ./bin/pcimem


./bin/pcimem: ./obj/pcimem.o ./obj/pciinfo.o
	$(LINKER) ./obj/pcimem.o ./obj/pciinfo.o $(LFLAGS) -o ./bin/pcimem


./obj/pcimem.o: ./src/pcimem.c
	$(CC) $(CFLAGS) ./src/pcimem.c -o ./obj/pcimem.o
	
	
./obj/pciinfo.o: ./inc/pciinfo/src/pciinfo.c
	$(CC) $(CFLAGS) ./inc/pciinfo/src/pciinfo.c -o ./obj/pciinfo.o

	
clean:
	rm ./obj/*o ./bin/pcimem
