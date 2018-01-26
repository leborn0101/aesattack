# See LICENSE file for license and copyright information

# project
PROJECT = cache_attack_spy
VERSION = 0.0.1

# arch
ARCH = armv7

# version checks
# If you want to disable any of the checks, set *_VERSION_CHECK to 0.

LIBFLUSH_VERSION_CHECK ?= 1
LIBFLUSH_MIN_VERSION = 0.0.1
LIBFLUSH_PKG_CONFIG_NAME = libflush

# pkg-config binary
PKG_CONFIG ?= pkg-config

# paths
PREFIX ?= /usr
DEPENDDIR ?= .depend
BUILDDIR ?= build/${ARCH}
BUILDDIR_RELEASE ?= ${BUILDDIR}/release
BUILDDIR_DEBUG ?= ${BUILDDIR}/debug
BINDIR ?= bin

# libs
LIBFLUSH_INC ?= $(shell ${PKG_CONFIG} --cflags libflush)
LIBFLUSH_LIB ?= $(shell ${PKG_CONFIG} --libs libflush)

INCS = ${LIBFLUSH_INC}
LIBS = ${LIBFLUSH_LIB}

# compiler flags
CFLAGS += -std=c11 -pedantic -Wall -Wno-format-zero-length -Wextra -O3 $(INCS)

# debug
DFLAGS ?= -g

# linker flags
LDFLAGS += -rdynamic

# compiler
CC ?= gcc

# strip
SFLAGS ?= -s

# valgrind
VALGRIND = valgrind
VALGRIND_ARGUMENTS = --tool=memcheck --leak-check=yes --leak-resolution=high \
	--show-reachable=yes --log-file=${PROJECT}-valgrind.log
VALGRIND_SUPPRESSION_FILE = ${PROJECT}.suppression

# set to something != 0 if you want verbose build output
VERBOSE ?= 0

# colors
COLOR ?= 1

# dist
TARFILE = ${PROJECT}-${VERSION}.tar.gz
TARDIR = ${PROJECT}-${VERSION}

# android
ANDROID_PLATFORM ?= android-23
ADB = adb
ADB_SHELL = ${ADB} shell
ADB_PUSH = ${ADB} push

# remote setting
ADDRESS_START = b6ba6000
ADDRESS_END = b6bc0000
REMOTE_EXECUTE_FILE_DIR = /data/local/tmp
ATTACKED_LIB_PATH = /system/lib/libinput.so

# setting for myphone

THRESHOLD = 960

# android device
WITH_ANDROID ?= 1

# thread support
WITH_THREADS ?= 0
