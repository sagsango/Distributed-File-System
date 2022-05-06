/* riscosify_ee.c
   $Id: riscosify_ee.c,v 1.4 2004/03/20 20:23:35 joty Exp $
   Main code to call external_edit()

   Copyright (c) 2003-2004 John Tytgat / BASS <John.Tytgat@aaug.net>

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
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* ANSI C headers :
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* UnixLib specific headers :
 */
#include <unixlib/local.h>
/* Project headers :
 */
#include "eecode.h"

int __riscosify_control = __RISCOSIFY_NO_PROCESS;

        int main(int argc, char *argv[])
/*	--------------------------------
 */
{
  char buf[1024];
  const char *roSpecP;
  const char *fileNameArgP;
  int fileTypeArg;
  int isUnixSpecArg;
  EE_ReturnCode_e eeRtrn;
  const char *progNameP;
  int i;
  const char *errorStrP;

fileNameArgP = NULL;
fileTypeArg = -1;
isUnixSpecArg = 0;
errorStrP = NULL;
for (i = 1; i < argc && errorStrP == NULL; ++i)
  {
  if (argv[i][0] == '-')
    {
    if (strcmp(&argv[i][1], "unix") == 0)
      isUnixSpecArg = 1;
    else if (strcmp(&argv[i][1], "t") == 0)
      {
      if (++i < argc)
        errorStrP = filetype_atoi(argv[i], &fileTypeArg);
      else
        errorStrP = "Filetype missing";
      }
    else
      errorStrP = "Unrecognized argument";
    }
  else
    {
    if (fileNameArgP == NULL)
      fileNameArgP = argv[i];
    else
      errorStrP = "More than one filename";
    }
  }
if (errorStrP == NULL && fileNameArgP == NULL)
  errorStrP = "Filename missing";
if (errorStrP != NULL)
  {
  fprintf(stderr, "%s\n"
                  "Syntax: ee [-unix] [-t <filetype>] <filename>\n"
                  "  -unix : <filename> is filename in Unix notation, e.g. /RAM::RamDisc0/$/FileToEdit\n"
                  "  -t <filetype> : filetype to edit as\n",
                  errorStrP);
  return EXIT_FAILURE;
  }

if (isUnixSpecArg)
  {
    int fileType;

  if (__riscosify(fileNameArgP, 0, 0, buf, sizeof(buf), &fileType) == NULL)
    {
    fprintf(stderr, "__riscosify() failed\n");
    return EXIT_FAILURE;
    }
  if (fileTypeArg == -1)
    fileTypeArg = fileType;
  roSpecP = buf;
  }
else
  roSpecP = fileNameArgP;

  {
    const char *strP;

  for (progNameP = strP = argv[0]; *strP != '\0'; ++strP)
    {
    if ((*strP == '/' || *strP == '.') && strP[1] != '\0')
      progNameP = strP + 1;
    }
  }

eeRtrn = external_edit(roSpecP, fileTypeArg, "CVS", progNameP);

return (eeRtrn == EE_eUPDATED) ? EXIT_SUCCESS : EXIT_FAILURE;
}
