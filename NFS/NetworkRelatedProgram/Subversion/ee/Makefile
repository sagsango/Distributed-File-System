# Project:	CVS ee
# Author:	John Tytgat / BASS
# $Id: Makefile,fe1,v 1.4 2004/03/20 20:26:14 joty Exp $

# Project defines :
# ProjectDefines = -g
ProjectDefines = 

CC = gcc
CCFLAGS = -c  $(ProjectDefines)
LINK = gcc
LINKFLAGS = -o $@
OBJASM = objasm
OBJASMFLAGS =
#MKDIR = CDir

# Object files
OBJS	= riscosify_ee.o eecode.o

# System libraries :
LDLIBS	=

# User libraries :
USRLIBS = 

# Final target:

All:	../!SVN/ee,ff8

../!SVN/ee,ff8: $(OBJS) $(LDLIBS)
	$(LINK) $(OBJS) $(LDLIBS) $(LINKFLAGS)

.SUFFIXES: .o .c

.c.o:;	$(CC) $(CCFLAGS) -o $@ $<
.s.o:; 	$(OBJASM) $(OBJASMFLAGS) -c $< -o $@

# Dynamic dependencies:
