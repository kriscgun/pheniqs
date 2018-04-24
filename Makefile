# Pheniqs : PHilology ENcoder wIth Quality Statistics
# Copyright (C) 2018  Lior Galanti
# NYU Center for Genetics and System Biology

# Author: Lior Galanti <lior.galanti@nyu.edu>

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.

# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# The PHENIQS_VERSION defaults to $(MAJOR_REVISON).$(MINOR_REVISON) if not provided by the environment.
# git revision checksum is appended if available
# If available in the environment, dependency version are also included when generating version.h
# 	PHENIQS_VERSION
#	ZLIB_VERSION
#	BZIP2_VERSION
#	XZ_VERSION
#	RAPIDJSON_VERSION
#	HTSLIB_VERSION

MAJOR_REVISON := 1
MINOR_REVISON := 1

CC              = clang++
PREFIX          = /usr/local
BIN_PREFIX      = $(PREFIX)/bin
INCLUDE_PREFIX  = $(PREFIX)/include
LIB_PREFIX      = $(PREFIX)/lib

CFLAGS          = -std=c++11 -O3 -Wall -Wsign-compare -pedantic
LDFLAGS         =
LIBS            = -lhts -lz -lbz2 -llzma
STATIC_LIBS     = $(LIB_PREFIX)/libhts.a $(LIB_PREFIX)/libz.a $(LIB_PREFIX)/libbz2.a $(LIB_PREFIX)/liblzma.a

PHENIQS_SOURCES = \
	json.cpp \
	url.cpp \
	interface.cpp \
	atom.cpp \
	transform.cpp \
	auxiliary.cpp \
	sequence.cpp \
	segment.cpp \
	specification.cpp \
	feed.cpp \
	fastq.cpp \
	hts.cpp \
	environment.cpp \
	accumulate.cpp \
	pipeline.cpp \
	pheniqs.cpp

PHENIQS_OBJECTS = \
	json.o \
	url.o \
	interface.o \
	atom.o \
	transform.o \
	auxiliary.o \
	sequence.o \
	segment.o \
	specification.o \
	feed.o \
	fastq.o \
	hts.o \
	environment.o \
	accumulate.o \
	pipeline.o \
	pheniqs.o

PHENIQS_EXECUTABLE = pheniqs

PHENIQS_GIT_REVISION := $(shell git describe --abbrev=40 --always 2> /dev/null)

PLATFORM := $(shell uname -s)

ifneq ('$(wildcard $(LIB_PREFIX)/libdeflate.a)','')
    ifeq ($(PLATFORM), Darwin)
        with-libdeflate = 1
    else ifeq ($(PLATFORM), Linux)
        ifneq ('$(wildcard $(LIB_PREFIX)/libdeflate.so)','')
            with-libdeflate = 1
        endif
    else ifeq ($(PLATFORM), Darwin)
        with-libdeflate = 0
    endif
else
    with-libdeflate = 0
endif

ifndef PHENIQS_VERSION
    PHENIQS_VERSION := $(MAJOR_REVISON).$(MINOR_REVISON)
endif

ifdef PHENIQS_GIT_REVISION
    override PHENIQS_VERSION := '$(PHENIQS_VERSION).$(PHENIQS_GIT_REVISION)'
endif

ifdef PREFIX
    CFLAGS += -I$(INCLUDE_PREFIX)
    LDFLAGS += -L$(LIB_PREFIX)
endif

ifeq ($(with-libdeflate), 1)
    LIBS += -ldeflate
    STATIC_LIBS += $(LIB_PREFIX)/libdeflate.a
endif

all: $(PHENIQS_SOURCES) configuration.h version.h $(PHENIQS_OBJECTS)
	$(CC) $(PHENIQS_OBJECTS) $(LDFLAGS) -pthread $(LIBS) -o $(PHENIQS_EXECUTABLE)

static: $(PHENIQS_SOURCES) configuration.h version.h $(PHENIQS_OBJECTS)
	$(CC) $(PHENIQS_OBJECTS) $(LDFLAGS) -pthread $(STATIC_LIBS) -o $(PHENIQS_EXECUTABLE)

config:
	@echo 'PHENIQS_VERSION  :  $(PHENIQS_VERSION)'
	@echo 'CC               :  $(CC)'
	@echo 'PREFIX           :  $(PREFIX)'
	@echo 'BIN_PREFIX       :  $(BIN_PREFIX)'
	@echo 'INCLUDE_PREFIX   :  $(INCLUDE_PREFIX)'
	@echo 'LIB_PREFIX       :  $(LIB_PREFIX)'
	@echo 'CFLAGS           :  $(CFLAGS)'
	@echo 'LDFLAGS          :  $(LDFLAGS)'
	@echo 'CFLAGS           :  $(CFLAGS)'
	@echo 'LIBS             :  $(LIBS)'
	@echo 'STATIC_LIBS      :  $(STATIC_LIBS)'
	@echo 'with-libdeflate  :  $(with-libdeflate)'

.cpp.o:
	$(CC) $(CFLAGS) -c -o $@ $<

# Regenerate version.h when PHENIQS_VERSION changes
version.h: $(if $(wildcard version.h),$(if $(findstring "$(PHENIQS_VERSION)",$(shell cat version.h)),,clean-version))
	@echo Generate version.h with $(PHENIQS_VERSION)
	$(if $(PHENIQS_VERSION),    @echo '#define PHENIQS_VERSION "$(PHENIQS_VERSION)"'        >> $@)
	$(if $(ZLIB_VERSION),       @echo '#define ZLIB_VERSION "$(ZLIB_VERSION)"'              >> $@)
	$(if $(BZIP2_VERSION),      @echo '#define BZIP2_VERSION "$(BZIP2_VERSION)"'            >> $@)
	$(if $(XZ_VERSION),         @echo '#define XZ_VERSION "$(XZ_VERSION)"'                  >> $@)
	$(if $(LIBDEFLATE_VERSION), @echo '#define LIBDEFLATE_VERSION "$(LIBDEFLATE_VERSION)"'  >> $@)
	$(if $(RAPIDJSON_VERSION),  @echo '#define RAPIDJSON_VERSION "$(RAPIDJSON_VERSION)"'    >> $@)
	$(if $(HTSLIB_VERSION),     @echo '#define HTSLIB_VERSION "$(HTSLIB_VERSION)"'          >> $@)

# Regenerate configuration.h when configuration.json file changes
configuration.h: configuration.json
	@echo Generate command line interface configuration
	$(shell ./configuration.sh)

clean-version:
	-@rm -f version.h

clean-configuration:
	-@rm -f configuration.h

clean: clean-version clean-configuration
	-@rm -f $(PHENIQS_EXECUTABLE) $(PHENIQS_OBJECTS)

install: pheniqs
	if( test ! -d $(PREFIX)/bin ) ; then mkdir -p $(PREFIX)/bin ; fi
	cp -f pheniqs $(PREFIX)/bin/pheniqs
	chmod a+x $(PREFIX)/bin/pheniqs

# Dependencies
# Regenerate modules when header files they import change
json.o: \
	error.h \
	json.h

url.o: \
	error.h \
	json.h \
	url.h

interface.o: \
	error.h \
	json.h \
	interface.h

atom.o: \
	error.h \
	json.h \
	atom.h

transform.o: \
	error.h \
	json.h \
	transform.h

sequence.o: \
	error.h \
	json.h \
	nucleotide.h \
	phred.h \
	transform.h \
	sequence.h

auxiliary.o: \
	error.h \
	json.h \
	sequence.h \
	auxiliary.h

segment.o: \
	error.h \
	json.h \
	nucleotide.h \
	sequence.h \
	auxiliary.h \
	segment.h

specification.o: \
	error.h \
	json.h \
	url.h \
	atom.h \
	sequence.h \
	specification.h

accumulate.o: \
	error.h \
	json.h \
	nucleotide.h \
	phred.h \
	sequence.h \
	segment.h \
	specification.h \
	accumulate.h

environment.o: \
	version.h \
	configuration.h \
	interface.h \
	error.h \
	json.h \
	url.h \
	nucleotide.h \
	phred.h \
	atom.h \
	specification.h \
	environment.h

feed.o: \
	error.h \
	json.h \
	url.h \
	sequence.h \
	specification.h \
	feed.h

fastq.o: \
	error.h \
	json.h \
	nucleotide.h \
	phred.h \
	sequence.h \
	segment.h \
	specification.h \
	feed.h \
	fastq.h

hts.o: \
	error.h \
	json.h \
	nucleotide.h \
	phred.h \
	atom.h \
	sequence.h \
	segment.h \
	specification.h \
	feed.h \
	hts.h

pipeline.o: \
	error.h \
	json.h \
	url.h \
	nucleotide.h \
	phred.h \
	atom.h \
	accumulate.h \
	environment.h \
	feed.h \
	fastq.h \
	hts.h \
	pipeline.h

pheniqs.o: \
	error.h \
	json.h \
	environment.h \
	pipeline.h
