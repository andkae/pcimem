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


all: pcimem


sinu828FPGAprog: pcimem.o


pcimem.o: ./src/pcimem.c
	$(CC) $(CFLAGS) ./src/pcimem.c

clean:
	rm *o pcimem
