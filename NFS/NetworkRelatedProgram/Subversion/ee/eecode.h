/*******************************************************************
 * File:     eecode.h
 * Purpose:  External edits from outside the desktop
 * Author:   Justin Fletcher
 * Date:     06 Sep 1997
 *
 * $Id: eecode.h,v 1.3 2003/08/22 00:01:07 joty Exp $
 ******************************************************************/

#ifndef EECODE_HEADER_INCLUDED
#define EECODE_HEADER_INCLUDED

typedef enum {
  EE_eUPDATED   = 0,
  EE_eUNCHANGED = 1,
  EE_eFAILED    = 2,
  EE_eKILLED    = 3
  } EE_ReturnCode_e;

/*********************************************** <c> Gerph *********
 Function:     external_edit
 Description:  External edits a file if possible
 Parameters:   filename-> filename to edit
               type = filetype to edit as, or -1 for type of file
               editname-> a name to give the file in the window
               program-> the name of this program
 Returns:      code; EE_eUPDATED if the file has changed
                     EE_eUNCHANGED if the file was not changed
                     EE_eFAILED if the edit failed (variety of reasons)
                               (file is unchanged)
                     EE_eKILLED if the task was killed from task manager
                               (file is unchanged)
 ******************************************************************/
EE_ReturnCode_e external_edit(const char *filename, int type, const char *editname, const char *program);


/*******************************************************************
 Function:     filetype_atoi
 Description:  Converts an ASCII filetype into its integer equivalent
 Parameters:   filetype_a-> CTRL terminated filetype in ASCII
 Returns:      <> NULL, ptr to error string, don't use *filetype_i
               == NULL, *filetype_i is the integer filetype
 ******************************************************************/
const char *filetype_atoi(const char *filetype_a, int *filetype_i);

#endif
