/*
	$Id$

	Module code for controlling capture and saving captured data to disc.


	Copyright (C) 2007 Alex Waugh
	
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	
	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <ctype.h>
#include <swis.h>
#include <kernel.h>
#include <time.h>


#include "wiresalmonmod.h"

void semiprint(char *msg);

static _kernel_oserror error_nomem = {0x58C80, "Out of memory"};
static _kernel_oserror error_overflow = {0x58C81, "Buffer overflowed while capturing, or error when writing file, some packets may have been dropped"};

_kernel_oserror *claimswi(void *wkspc);
_kernel_oserror *releaseswi(void);


typedef struct pcap_hsr_s {
	unsigned magic_number;
	unsigned version_major:16;
	unsigned version_minor:16;
	int thiszone;
	unsigned sigfigs;
	unsigned snaplen;
	unsigned network;
} pcap_hdr_t;

typedef struct pcaprec_hdr_s {
	unsigned ts_sec;
	unsigned ts_usec;
	unsigned incl_len;
	unsigned orig_len;
} pcaprec_hdr_t;

struct dib {
	unsigned dib_swibase;
	char *dib_name;
	unsigned dib_unit;
	unsigned char *dib_address;
	char *dib_module;
	char *dib_location;
	unsigned dib_slot;
	unsigned dib_inquire;
};

struct dibchain {
	struct dibchain *next;
	struct dib *dib;
};

struct module {
	int start;
	int init;
	int final;
	int service;
	int title;
	int help;
	int command;
	int swibase;
};

#define MAX_DRIVERS 10
#define MAX_CLAIMS 20

struct driver {
	int swibase;
	char mac[6];
	char pad[2];
};

static struct {
	char *volatile writeptr;
	char *volatile writeend;
	volatile int overflow;
	pcaprec_hdr_t rechdr;
	void *oldswihandler;
	FILE *capturing;
	struct driver drivers[MAX_DRIVERS];
	int numrxclaims;
	void *rxclaims[2*MAX_CLAIMS];
} workspace;

static char *databuffer1 = NULL;
static char *databuffer2;
static int databuffersize;
static int writebuffer;


/* Get the swi bases of all loaded DCI4 driver modules */
static _kernel_oserror *get_drivers(void)
{
	int numswis = 0;
	_kernel_oserror *err;
	struct dibchain *chain;

	err = _swix(OS_ServiceCall, _INR(0,1) | _OUT(0), 0, 0x9B, &chain);
	if (err) return err;
	while (chain) {
		struct dibchain *next = chain->next;
		/* Leave the last entry empty, as a list terminator */
		if (numswis < MAX_DRIVERS - 1) {
			int i;
			/* Only add if not already in the list */
			for (i = 0; i < numswis; i++) {
				if (chain->dib->dib_swibase == workspace.drivers[i].swibase) break;
			}
			if (i == numswis) {
				memcpy(workspace.drivers[numswis].mac, chain->dib->dib_address, 6);
				workspace.drivers[numswis++].swibase = chain->dib->dib_swibase;
			}
		}
		free(chain);
		chain = next;
	}
	return NULL;
}

/* rmreinit all DCI4 driver modules, to ensure that we can insert or remove our hook into the rx routines */
static void reinit_drivers(void)
{
	int i;

	/* Search through module list to find modules with matching SWI bases */
	for (i = 0; i < MAX_DRIVERS; i++) {
		int j = 0;
		struct module *module;

		/* Check for end of list */
		if (workspace.drivers[i].swibase == 0) break;

		while (_swix(OS_Module, _INR(0,2) | _OUT(3), 12, j, 0, &module) == NULL) {
			if (module->swibase == workspace.drivers[i].swibase) {
				/* Found match, so reinitialise it */
				char *title = (char *)module + module->title;
				/* Ignore errors, so we don't terminate before doing all modules needed */
				_swix(OS_Module, _INR(0,1), 3, title);
				break;
			}
			j++;
		}
	}
}

_kernel_oserror *callevery_handler(_kernel_swi_regs *r, void *pw)
{
	(void)r;
	(void)pw;

	/* Increment timestamp */
	workspace.rechdr.ts_usec += 20000;
	if (workspace.rechdr.ts_usec > 1000000) {
		workspace.rechdr.ts_usec -= 1000000;
		workspace.rechdr.ts_sec += 1;
	}

	if (workspace.writeptr == ((writebuffer == 1) ? databuffer1 : databuffer2)) {
		// No new data
		return NULL;
	}

	_swix(OS_AddCallBack, _INR(0, 1), callback, pw);
	return NULL;
}

_kernel_oserror *callback_handler(_kernel_swi_regs *r, void *pw)
{
	char *end;
	char *start;
	static volatile int sema = 0;

	(void)r;
	(void)pw;

	/* Turn inturrupts off while switching buffers, to ensure the
	   SWI handler sees a consistent view */
	_swix(OS_IntOff,0);
	if (sema) {
		/* Prevent reentrancy */
		_swix(OS_IntOn,0);
		return NULL;
	}
	sema = 1;

	end = workspace.writeptr;
	if (writebuffer == 1) {
		start = databuffer1;
		workspace.writeptr = databuffer2;
		workspace.writeend = databuffer2 + databuffersize/2;
		writebuffer = 2;
	} else {
		start = databuffer2;
		workspace.writeptr = databuffer1;
		workspace.writeend = databuffer1 + databuffersize/2;
		writebuffer = 1;
	}
	if (workspace.overflow) {
		/* Drop whole packets */
		end = start; 
	}
	/* Reenable interrupts before doing I/O */
	_swix(OS_IntOn,0);

	/* Write data to file */
	if (workspace.capturing) {
		size_t written;
		written = fwrite(start, 1, end - start, workspace.capturing);
		if (written < (end - start)) {
			workspace.overflow = 1;
		}
	}
	sema = 0;

	return NULL;
}

static _kernel_oserror *stop_capture(void)
{
	FILE *file = workspace.capturing;

	/* Ensure capturing has stopped before closing the file */
	workspace.capturing = NULL;
	if (file) fclose(file);

	if (databuffer1) free(databuffer1);
	databuffer1 = NULL;

	if (workspace.overflow) {
		return &error_overflow;
	}
	return NULL;
}

static _kernel_oserror *start_capture(char *filename, int bufsize)
{
	FILE *file;
	pcap_hdr_t hdr;

	stop_capture();

	if (bufsize == 0) bufsize = 512*1024;
	databuffersize = bufsize;
	databuffer1 = malloc(databuffersize);
	if (databuffer1 == NULL) return &error_nomem;
	databuffer2 = databuffer1 + databuffersize/2;
	workspace.writeptr = databuffer1;
	workspace.writeend = databuffer2;
	writebuffer = 1;

	workspace.overflow = 0;

	file = fopen(filename,"wb");
	if (file == NULL) {
		return _kernel_last_oserror();
	}

	/* Write the pcap file header */
	hdr.magic_number = 0xa1b2c3d4;
	hdr.version_major = 2;
	hdr.version_minor = 4;
	hdr.thiszone = 0;
	hdr.sigfigs = 0;
	hdr.snaplen = databuffersize/2;
	hdr.network = 1;
	fwrite(&hdr, sizeof(hdr), 1, file);

	/* Enable capturing to start */
	workspace.capturing = file;

	return NULL;
}

_kernel_oserror *finalise(int fatal, int podule, void *private_word)
{
	_kernel_oserror *err;

	(void)fatal;
	(void)podule;

	stop_capture();

	_swix(OS_RemoveTickerEvent, _INR(0,1), callevery, private_word);
	_swix(OS_RemoveTickerEvent, _INR(0,1), callback, private_word);

	err = releaseswi();
	if (err) return err;

	/* Reinitialise the drivers so the rx hooks don't get called anymore */
	reinit_drivers();

	return NULL;
}

_kernel_oserror *initialise(const char *cmd_tail, int podule_base, void *private_word)
{
	_kernel_oserror *err;

	(void)cmd_tail;
	(void)podule_base;

	workspace.rechdr.ts_sec = time(NULL);
	workspace.rechdr.ts_usec = 0;
	memset(&workspace.drivers, 0, sizeof(struct driver) * MAX_DRIVERS);
	workspace.numrxclaims = 0;
	workspace.capturing = NULL;

	err = get_drivers();
	if (err) return err;

	_swix(OS_IntOff,0);
	err = claimswi(&workspace);
	_swix(OS_IntOn,0);

	if (err) return err;

	err = _swix(OS_CallEvery, _INR(0,2), 1, callevery, private_word);
	if (err) {
		releaseswi();
		return err;
	}

	/* Reinitialise all drivers, so we can insert our hook into the rx call */
	reinit_drivers();

	return 0;
}

_kernel_oserror *swi(int swi_no, _kernel_swi_regs *r, void *private_word)
{
	(void)private_word;

	switch (swi_no) {
	case Wiresalmon_Start - Wiresalmon_00:
		return start_capture((char *)r->r[0], r->r[1]);
	case Wiresalmon_Stop - Wiresalmon_00:
		return stop_capture();
	default:
		return error_BAD_SWI;
	}
	return NULL;
}

