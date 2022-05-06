#
#    $Id$
#

CC = gcc
CFLAGS = -mlibscl -mthrowback -mmodule -Wall -O2 -mpoke-function-name

CMHG = cmunge -32bit -tgcc
CMHGFLAGS = -d wiresalmonmod.h

DISTCC = distcc

CXX = $(DISTCC) g++

CXXFLAGS = -Irtk: -Wall -mthrowback -mlibscl -O1 -mpoke-function-name


all: Wiresalmon !RunImage

Wiresalmon: wiresalmon.o wiresalmonmod.o wiresalmonasm.o
	$(CC) $(CFLAGS) -o $@ wiresalmon.o wiresalmonmod.o wiresalmonasm.o

!RunImage: frontend.o
	$(CXX) $(CXXFLAGS) frontend.o -o $@ -lrtk:a.rtk

%.o: %.s
	objasm -throwback -c -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c -o $@ $<

wiresalmon.o: wiresalmonmod.o

wiresalmonmod.o:   cmhg.wiresalmonmod
	cdir o
	$(CMHG) $(CMHGFLAGS)  cmhg.wiresalmonmod -o wiresalmonmod.o

