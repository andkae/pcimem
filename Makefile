# **********************************************************************
#  @copyright   : Siemens AG
#  @license     : BSDv3
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



# set compiler
ifeq ($(origin CC), undefined)
	CC := gcc
endif

# set linker
ifeq ($(origin LINKER), undefined)
	LINKER := $(CC)
endif

# Get Absoulute path of this file
#   https://stackoverflow.com/questions/18136918/how-to-get-current-relative-directory-of-your-makefile
FILE_PATH_VARS := $(patsubst %/,%,$(dir $(realpath $(lastword $(MAKEFILE_LIST)))))

# set compiler flags
ifeq ($(origin CFLAGS), undefined)
  CFLAGS = 	-c -O -Wall -Wextra -Wimplicit -Wpadded -Wconversion \
			-I ${FILE_PATH_VARS} \
			-I ${FILE_PATH_VARS}/inc/pciinfo
endif

# linking flags here
ifeq ($(origin LFLAGS), undefined)
  LFLAGS = -Wall -Wextra -Wimplicit -I. -lm
endif


# The Directories Source, Includes, Objects, Binary and Resources
OBJDIR = obj
BINDIR = bin
INSTALL ?= install
PREFIX ?= /usr

# executable file name
_TARGET = pcimem

# obj files
_OBJS = pcimem.o pciinfo.o pcimem_main.o

# build paths to object and executabel
OBJS = $(addprefix $(OBJDIR)/, $(_OBJS))
TARGET = $(addprefix $(BINDIR)/, $(_TARGET))


# compile
all: $(TARGET)


# Link
$(TARGET):  $(OBJS)
	@ mkdir -p $(BINDIR)
	$(LINKER) $(OBJS) $(LFLAGS) -o $(TARGET)


# Compile sources
#   '.' 
$(OBJDIR)/%.o: %.c %.h
	@ mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

#   'inc/*/src' 
$(OBJDIR)/%.o: inc/*/src/%.c inc/*/src/*.h
	@ mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

#   'inc/*/' 
$(OBJDIR)/%.o: inc/*/%.c inc/*/%.h
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

#   './src/*'
$(OBJDIR)/%.o: src/%.c src/*.h
	@ mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@


install: $(TARGET)
	@echo "Installing $(TARGET) to $(DESTDIR)$(PREFIX)/bin/"
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/bin/
	$(INSTALL) -m 0755 $< $(DESTDIR)$(PREFIX)/bin/


clean:
	rm -f $(OBJDIR)/*o $(BINDIR)/$(TARGET)
