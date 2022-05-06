; The EE tool and library is licensed under the same license as ZLib.
;
;   Copyright (C) 2003 Justin Fletcher
;
;   This software is provided 'as-is', without any express or implied
;   warranty.  In no event will the authors be held liable for any damages
;   arising from the use of this software.
;
;   Permission is granted to anyone to use this software for any purpose,
;   including commercial applications, and to alter it and redistribute it
;   freely, subject to the following restrictions:
;
;   1. The origin of this software must not be misrepresented; you must not
;      claim that you wrote the original software. If you use this software
;      in a product, an acknowledgment in the product documentation would be
;      appreciated but is not required.
;   2. Altered source versions must be plainly marked as such, and must not
;      be misrepresented as being the original software.
;   3. This notice may not be removed or altered from any source
;      distribution.
;
; Original AOF version in JPatch was labeled 1.00z
; The following changes were made by John Tytgat / BASS :
;  - Converted to ObjAsm format
;  - Made 32-bit compliant
;  - Bug fixing
;  - Added filetype_atoi()
;
; $Id: eecode.s,v 1.4 2003/08/22 00:01:35 joty Exp $

S_None			EQU 0	; We're outside the states !
S_AwaitAck		EQU 1	; Awaiting acknowledgement of EE
S_AwaitSaveAck		EQU 2	; Awaiting a save ack
S_AwaitLoadAck		EQU 3	; Awaiting a load ack (also 'middle' state)
S_AwaitESave		EQU 4	; Awaiting an EditDataSave
S_AwaitLoad		EQU 5	; Awaiting a DataLoad
S_SendReturn		EQU 8	; Request from application to return data
S_SendAbort		EQU 9	; Request from application to abort

TW_Morite		EQU &808C4	; Sent to task to kill it
TW_Input		EQU &808C0	; Sent to give keypresses to it
DataSaveAck		EQU 2		; Save ok
DataLoad		EQU 3		; Load this
DataLoadAck		EQU 4		; Loaded ok

Ret_None		EQU &1000	; Result = Not returning yet (if returned is an error)
Ret_Updated		EQU 0	; Result = 'updated'	 (ok)
Ret_Unchanged		EQU 1	; Result = 'unchanged' (ok)
Ret_Failed		EQU 2	; Result = 'failed' ;-(	  (error)
Ret_Killed		EQU 3	; Result = 'killed' ;-(	  (error)

OS_Byte			EQU &6
OS_File			EQU &8
OS_FSControl		EQU &29
OS_WriteS		EQU &1
OS_Module		EQU &1E
OS_NewLine		EQU &3
OS_ReadMonotonicTime	EQU &42
OS_ReadUnsigned		EQU &21

TaskWindow_TaskInfo	EQU &43380

Wimp_AddMessages	EQU &400F6
Wimp_CloseDown		EQU &400DD
Wimp_Initialise		EQU &400C0
Wimp_Poll		EQU &400C7
Wimp_ReadSysInfo	EQU &400F2
Wimp_SendMessage	EQU &400E7

Filter_RegisterPostFilter	EQU &42641
Filter_DeRegisterPostFilter	EQU &42643

XOS_Byte		EQU OS_Byte + (1<<17)
XOS_File		EQU OS_File + (1<<17)
XOS_FSControl		EQU OS_FSControl + (1<<17)
XOS_Module		EQU OS_Module + (1<<17)
XOS_ReadUnsigned	EQU OS_ReadUnsigned + (1<<17)

XWimp_AddMessages	EQU Wimp_AddMessages + (1<<17)
XWimp_Initialise	EQU Wimp_Initialise + (1<<17)
XWimp_CloseDown		EQU Wimp_CloseDown + (1<<17)

		^	0, ip
|parentprogname|	#	4		; Ptr to program name
|editname|	#	4		; Ptr to edit name
|filename|	#	4		; Ptr to filename of file to edit
|filetype|	#	4		; filetype specified -or- original filetype
|orgfiletype|	#	4		; its original filetype
|oldescapestate|	#	4	; old escape state
|taskhandle|	#	4		; task handle, or 0 if 'inside' task
|intaskwindow|	#	4		; 1 if we're in a taskwindow
|filtername_ptr|	#	4	; name pointer (for release)
|filterstate|	#	4		; the state of the filter manager (see S_* definitions)
|reason|	#	4		; reason for it all
|ee_jobhandle|	#	4		; the job handle
|ee_taskhandle|	#	4		; their task handle
|returnstate|	#	4		; the state we're returning
|blk|		#	256		; just a few bytes
		#       (((:INDEX:@)+3):AND::NOT:3)-(:INDEX:@)
WSSize		*       :INDEX: @

	; Macro for embedding function names in code, just before
	; the function prologue.
	MACRO
	NAME	$name
	DCB	"$name",0
	ALIGN
	DCD	&FF000000 + (:LEN: "$name"+3+1) :AND: &FFFFFFFC
	MEND

	IMPORT	|__rt_stkovf_split_big|
	EXPORT	|external_edit|
	EXPORT	|filetype_atoi|

	AREA	|ASMEE$DATA|, DATA

	%	WSSize

	AREA	|ASMEE$CODE|, CODE, READONLY

	; *******************************************************************
	; Subroutine;	filetype_atoi
	; Description;  Converts an ASCII filetype into its integer equivalent
	; Parameters;	r0-> ASCII filetype, CTRL terminated
	;		r1-> integer
	; Returns;	r0 = NULL, no error, r1(on entry) points to integer
	;	             result.
	;		   -> error string
	; *******************************************************************
	NAME	"filetype_atoi"
|filetype_atoi|
	MOV	r3, r1			; r3 => where result needs to come
	MOV	r1, r0
	MOV	r0, #16
	ADD	r0, r0, #(1<<29) + (1<<31)
	MOV	r2, #&1000
	SUB	r2, r2, #1
	SWI	XOS_ReadUnsigned
	ADDVS	r0, r0, #4
	MOVVC	r0, #0
	STRVC	r2, [r3, #0]
	MOV	pc, lr

	; *******************************************************************
	; Subroutine;	external_edit
	; Description;  Edit a file
	; Parameters;	r0-> filename to edit
	;		r1 = filetype (or -1 for none)
	;		r2-> edit name
	;		r3-> program name
	; Returns;	r0 = return code
	;		     0 = updated
	;		     1 = unchanged
	;		     2 = failed \_ these can be treated as
	;		     3 = killed /  pretty much identical
	; *******************************************************************
	NAME	"external_edit"
|external_edit|
	MOV	ip, sp			; APCS
	STMFD	sp!, {a1-a4, v1-v6, fp, ip, lr, pc} ; Stack registers
	SUB	fp, ip, #4		; points as saved pc
	SUB	ip, sp, #1024*3		; we're going to need a big stack
	CMP	ip, sl			; is it below stack limit
	BLLT	|__rt_stkovf_split_big|

	LDR	ip, =|ASMEE$DATA|

	MOV	lr, #Ret_None
	STR	lr, |returnstate|

	STR	r3, |parentprogname|	; store progname as 'parent'
	STR	r2, |editname|		; store editname
	MOV	r7, r1			; r7 = type to edit as
	STR	r0, |filename|		; store the filename used
	MOV	r1, r0			; r1-> filename
	MOV	r0, #20
	SWI	XOS_File
	BVS	exit_failed
	  ; get the info for it
	CMP	r0, #0			; was it 'not found' ?
	BEQ	exit_failed		; if so, return as 'failed'

	  ; now the type stuff
	CMN	r7, #1			; if -1, then we use the type given
	MOVEQ	r7, r6			; edit filetype = real filetype
	STR	r7, |filetype|		; store filetype
	STR	r6, |orgfiletype|	; the original type (to restore)

	  ; disable escape
	MOV	r0, #229
	MOV	r1, #1
	MOV	r2, #0
	SWI	XOS_Byte
	BVS	exit_failed
	STR	r1, |oldescapestate|	; store the escape state

|__z0_restart|
	LDR	r1, |TASKWord|		; r1 = 'TASK' word
	LDR	r2, |parentprogname|	; taskname = parent
	  ; "Taskname = %$2"
	MOV	r0, #200
	SWI	XWimp_Initialise
	  ; initialise us as a task
	MOVVS	r0, #0			; if error, task handle = 0
	STR	r0, |taskhandle|	; store handle
	MOV	r1, r0			; hang on to the handley thing
; check taskwindow
	SWI	TaskWindow_TaskInfo	; are we in a task ?
	CMP	r0, #0			; well?
	MOVNE	r0, #1			; marks us as a taskwindow
	CMP	r0, #0			; are we outside taskwindow ?
	CMPEQ	r1, #0			; and did the init fail ?
	BNE	|__z0_notatshellcli|
	SWI	XWimp_CloseDown		; close us down
	B	|__z0_restart|
|TASKWord|
	DCB	"TASK"			; word 'task'

|__z0_notatshellcli|
	STR	r0, |intaskwindow|	; mark it
	  ; "Taskwindow= %r0"
	BL	|addfilter|		; add our filter
	  ; "Filter added"
	BL	ee_initiate		; start the first ee

	  ; "Initiated EE"
	LDR	r0, |intaskwindow|
	TEQ	r0, #0			; are we in taskwindow ?
	BEQ	__z0_notintw		; nope, so jump out
	SWI	OS_WriteS		; write string
	DCB	"Press R to return data, A to abort", 0
	ALIGN
	SWI	OS_NewLine
|__z0_waitloop|
	  ; check state
	LDR	r0, |returnstate|	; read return state
	TEQ	r0, #Ret_None		; is it 'none' ?
	BNE	taskend			; if not, exit

	  ; now check keys
	MOV	r0, #&81
	MOV	r1, #25
	MOV	r2, #0
	SWI	XOS_Byte
	  ; read a character (25cs)
	CMP	r2, #255		; was it valid ?
	BEQ	__z0_waitloop		; nope, so try again

	TEQ	r1, #"R"		; was it 'r' to return ?
	TEQNE	r1, #"r"
	MOVEQ	r0, #S_SendReturn	; let's return
	STREQ	r0, |filterstate|	; store it as the state

	TEQ	r1, #"A"		; was it 'a' to abort ?
	TEQNE	r1, #"a"
	TEQNE	r1, #27			; or escape?
	MOVEQ	r0, #S_SendAbort	; let's abort
	STREQ	r0, |filterstate|	; store it as the state

	B	__z0_waitloop		; go again

	; not in a taskwindow
|__z0_notintw|
	  ; "Non-taskwindow poll"
|__z0_pollloop|
	  ; check state
	LDR	r0, |returnstate|	; read return state
	TEQ	r0, #Ret_None		; is it 'none' ?
	BNE	taskend			; if not, exit
	  ; now poll
	ADR	r1, blk			; address of our block
	MOV	r0, #0
	SWI	Wimp_Poll
	  ; poll the wimp
	ADR	lr, __z0_pollloop	; where to return to
	TEQ	r0, #17			; is it usermessage?
	TEQNE	r0, #18			; or usermessagerecorded?
	LDREQ	r0, [r1, #16]		; read message type
	TEQEQ	r0, #0			; is it 'quit' ?
	MOVEQ	r0, #Ret_Killed		; mark us as killed
	STREQ	r0, |returnstate|	; store as returnstate
	BEQ	taskend			; if so, end nicely
	B	__z0_pollloop		; jump back to poll again

	NAME	"taskend"
|taskend|
	BL	removefilter		; remove the filter

	LDR	r0, |taskhandle|	; read the taskhandle
	TEQ	r0, #0			; was it valid ?
	SWINE	Wimp_CloseDown		; yep, so shut us down

	  ; Restore Escape state
	LDR	r1, |oldescapestate|	; read old escape state
	MOV	r0, #229
	MOV	r2, #0
	SWI	OS_Byte
	LDR	r0, |returnstate|	; read the return code
	LDMDB	fp, {v1-v6, fp, sp, pc}	; return

|exit_failed|
	  ; "Returning= failed"
	MOV	r0, #Ret_Failed		; return 'failed' value
	LDMDB	fp, {v1-v6, fp, sp, pc}	; return

	NAME	"addfilter"
|addfilter|
	STMFD	sp!, {r0-r5, lr}	; Stack registers
	MOV	r0, #Ret_None		; make sure we can do multiple ee's
	STR	r0, |returnstate|	; store as return state

	  ; add the messages we need
	ADR	r0, ee_msgs
	SWI	XWimp_AddMessages

	  ; read task handle
	MOV	r0, #5
	SWI	Wimp_ReadSysInfo
	MOV	r3, r0			; r3 = task handle

	ADR	R0, |filtername|
	  ; filter name
	BL	|ee_strdup|		; copy it to the module area
	STR	r0, |filtername_ptr|	; store it for later
	ADR	r1, |filter|
	  ; filter code
	MOV	r2, ip
	MOV	r4, #0
	SWI	Filter_RegisterPostFilter
	  ; install it
	LDMFD	sp!, {r0-r5, pc}	; Return from call


	NAME	"removefilter"
|removefilter|
	STMFD	sp!, {r0-r5, lr}	; Stack registers
	  ; read task handle
	MOV	r0, #5
	SWI	Wimp_ReadSysInfo
	MOV	r3, r0			; r3 = task handle

	LDR	r0, |filtername_ptr|	; filter name pointer
	TEQ	r0, #0			; is it 0 ?
	BEQ	__z1_exit		; if so, we've already released
	ADR	r1, |filter|
	MOV	r2, ip
	MOV	r4, #0
	SWI	Filter_DeRegisterPostFilter
	  ; remove
	BL	release			; release the filtername block
	MOV	r0, #0
	STR	r0, |filtername_ptr|	; zero the name pointer
|__z1_exit|
	LDMFD	sp!, {r0-r5, pc}	; Return from call


	NAME	"ee_initiate"
|ee_initiate|
	STMFD	sp!,{r0-r5, lr}		; Stack registers
	ADR	r5, blk			; the block base
	LDR	r0, |editname|		; the name for the editor
	BL	|ee_strlen|		; find len to r1
	ADD	r1, r1, #52+3+1		; 52 for base, 3 to align, 1 term
	BIC	r1, r1, #3		; align
	STR	r1, [r5, #0]		; store as block length
	  ; "Block size=%r1"
	LDR	r3, |const_ee_editrq|
	STR	r3, [r5, #16]		; store as message
	ADD	r1, r5, #52		; r1-> block + 52
	BL	|ee_strcpy|		; copy edit name there
	  ; "Initiating EE for %$1"
	LDR	r0, |parentprogname|	; read -> parent name
	ADD	r1, r5, #32		; r1-> block + 32
	BL	|ee_strcpy|		; copy parent name there
	  ; "Parent=%$1"
	LDR	r0, |filetype|		; filetype
	STR	r0, [r5, #20]		; store as datatype
	SWI	OS_ReadMonotonicTime
	BIC	r0, r0, #&FF000000	; clear top bits
	BIC	r0, r0, #&00FF0000	; clear top-mid bits
	STR	r0, [r5, #24]		; store as job handle
	MOV	r0, #0			; just edit and return on save
	STR	r0, [r5, #28]		; store as flags
	STR	r0, [r5, #12]		; store as ourref
	MOV	r0, #18
	ADR	r1, blk
	MOV	r2, #0
	SWI	Wimp_SendMessage
	  ; broadcast recorded
; set new state
	MOV	r0, #S_AwaitAck		; awaiting 'ack' message
	STR	r0, |filterstate|	; store it
	LDMFD	sp!, {r0-r5, pc}	; Return from call

|tempname|
	DCB	"<Wimp$Scrap>", 0
	ALIGN

	; Messages we /need/ to receive to work properly (read-only)
|ee_msgs|
	DCD	2			; datasaveack (for returns)
	DCD	3			; dataload (for sends)
	DCD	4			; dataloadack (for returns)
|const_ee_editrq|
	DCD	&45d80			; Message_EditRq
|const_ee_editack|
	DCD	&45d81			; Message_EditAck
|const_ee_return|
	DCD	&45d82			; Message_EditReturn
|const_ee_abort|
	DCD	&45d83			; Message_EditAbort
|const_ee_datasave|
	DCD	&45d84			; Message_EditDataSave
|const_tw_morite|
	DCD	TW_Morite		; TaskWindow_Morite
	; end of list
	DCD	0

|filtername|				; Read-only data
	DCB	"ExternalEdit filter", 0
	ALIGN

; abort the edit
	NAME	"ee_abort"
|ee_abort|
	STMFD	sp!, {r0-r5, lr}	; Stack registers
	  ; "Sending abort message"
	ADR	r5, blk			; our workspace
	MOV	r0, #28			; length of message
	STR	r0, [r5, #0]		; store len
	MOV	r0, #0
	STR	r0, [r5, #20]		; store 0 value
	LDR	r0, |ee_jobhandle|
	STR	r0, [r5, #24]		; store job handle
	LDR	r0, |const_ee_abort|	; edit abort
	STR	r0, [r5, #16]		; store it
	LDR	r2, |ee_taskhandle|	; their handle
	MOV	r0, #17
	MOV	r1, r5
	SWI	Wimp_SendMessage
	  ; send it
; set new state
	MOV	r0, #S_None		; we're not in any state
	STR	r0, |filterstate|	; store it
	LDMFD	sp!, {r0-r5, pc}	; Return from call

; try to return the file to us
	NAME	"ee_return"
|ee_return|
	STMFD	sp!, {r0-r5, lr}	; Stack registers
	  ; "Attempting get data back from Zap"
	MOV	r5, r1			; r5-> block
	LDR	r0, |filetype|		; filetype
	STR	r0, [r5, #20]		; store that
	MOV	r0, #0			; not reply
	STR	r0, [r5, #12]		; store as ourref
	LDR	r0, |ee_jobhandle|
	STR	r0, [r5, #24]		; store job handle
	LDR	r0, |const_ee_return|	; return request
	STR	r0, [r5, #16]		; store it
	LDR	r2, |ee_taskhandle|	; their handle
	MOV	r0, #17
	MOV	r1, r5
	SWI	Wimp_SendMessage
	  ; send it
; set new state
	MOV	r0, #S_AwaitESave	; awaiting 'editdatasave' message
	STR	r0, |filterstate|	; store it
	LDMFD	sp!, {r0-r5, pc}	; Return from call

; try to send the file
	NAME	"ee_startsendfile"
|ee_startsendfile|
	STMFD	sp!,{r0-r5, lr}		; Stack registers
	  ; "Attempting to send a file at Zap"
	MOV	r5, r1			; r5-> block
	LDR	r0, |editname|		; the name to use
	BL	|ee_strlen|		; find it's len (to r1)
	ADD	r2, r1, #44+3+1		; len+ base + align + term
	BIC	r2, r2, #3		; align now!
	STR	r2, [r5, #0]		; store as blk len
	ADD	r1, r5, #44		; base
	BL	|ee_strcpy|		; copy leafname
	LDR	r0, |filetype|		; filetype
	STR	r0, [r5, #40]		; store that
	MOV	r0, #0			; unknown size
	STR	r0, [r5, #36]		; store as size
	STR	r0, [r5, #12]		; store as ourref
	LDR	r0, |ee_jobhandle|
	STR	r0, [r5, #20]		; store job handle
	LDR	r0, |const_ee_datasave|	; datasave request
	STR	r0, [r5, #16]		; store it
	LDR	r2, |ee_taskhandle|	; their handle
	MOV	r0, #18
	MOV	r1, r5
	SWI	Wimp_SendMessage
	  ; send it
; set new state
	MOV	r0, #S_AwaitSaveAck	; awaiting 'ack' message
	STR	r0, |filterstate|	; store it
	LDMFD	sp!, {r0-r5, pc}	; Return from call

; try to give them a file to save to (for return)
	NAME	"ee_sendsaveack"
|ee_sendsaveack|
	STMFD	sp!,{r0-r5, lr}		; Stack registers
	ADR	r1, tempname		; the temporary name to use
	MOV	r0, #6
	SWI	XOS_File
	  ; delete it
	LDR	r5, [sp,#4*1]		; re-read r1
	MOV	r0, r1			; r1
	BL	|ee_strlen|		; find it's len (to r1)
	ADD	r2, r1, #44+3+1		; len+ base + align + term
	BIC	r2, r2, #3		; align now!
	STR	r2, [r5, #0]		; store as blk len
	ADD	r1, r5, #44		; base
	BL	|ee_strcpy|		; copy leafname
	  ; "Attempting to send save to Zap"
	MOV	r0, #-1			; not safe
	STR	r0, [r5, #36]		; store as size
	LDR	r0, |filetype|		; filetype
	STR	r0, [r5, #40]		; store as size
	LDR	r0, [r5, #8]		; their ref
	STR	r0, [r5, #12]		; store as ourref
	MOV	r0, #DataSaveAck	; datasave request
	STR	r0, [r5, #16]		; store it
	LDR	r2, |ee_taskhandle|	; their handle
	MOV	r0, #17
	MOV	r1, r5
	SWI	Wimp_SendMessage
	  ; send it
; set new state
	MOV	r0, #S_AwaitLoad	; awaiting 'ack' message
	STR	r0, |filterstate|	; store it
	LDMFD	sp!, {r0-r5, pc}	; Return from call

	NAME	"ee_sendloadack"
|ee_sendloadack|
	STMFD	sp!,{r0-r5, lr}		; Stack registers
	MOV	r5, r1			; r1-> workspace
	ADD	r1, r5, #44		; pointer to filename
	  ; "Attempting to copy file to original location"
	LDR	r2, |filename|		; -> filename
	MOV	r0, #26
	MOV	r3, #2_10000011
	SWI	XOS_FSControl
	BVS	__z2_failed		; argh.
	MOV	r1, r2			; r1-> filename
	LDR	r2, |orgfiletype|	; read the original type
	MOV	r0, #18
	SWI	XOS_File
	LDR	r0, [r5, #8]		; their ref
	STR	r0, [r5, #12]		; store as ourref
	MOV	r0, #DataLoadAck	; dataload request
	STR	r0, [r5, #16]		; store it
	LDR	r2, |ee_taskhandle|	; their handle
	MOV	r0, #17
	MOV	r1, r5
	SWI	Wimp_SendMessage
	  ; send it
	LDMFD	sp!,{r0-r5, pc}		; Return from call

|__z2_failed|
	  ; "LoadAck copy failed"
	MOV	r0, #Ret_Failed
	STR	r0, |returnstate|	; store the return state
	       ; we failed to launch edit
	BL     |ee_abort|
	       ; and send the abort
	LDMFD	sp!,{r0-r5, pc}		; Return from call

; try to give them the file to load
	NAME	"ee_sendload"
|ee_sendload|
	STMFD	sp!,{r0-r5, lr}		; Stack registers
	MOV	r5, r1			; r1-> workspace
	ADD	r2, r5, #44		; pointer to filename
	  ; "Attempting to send load to Zap"
	LDR	r1, |filename|		; read -> filename
	MOV	r0, #26
	MOV	r3, #2_11
	SWI	XOS_FSControl
	BVS	__z3_failed		; argh.
	MOV	r0, #0			; unknown size
	STR	r0, [r5, #36]		; store as size
	LDR	r0, [r5, #8]		; their ref
	STR	r0, [r5, #12]		; store as ourref
	MOV	r0, #DataLoad		; dataload request
	STR	r0, [r5, #16]		; store it
	LDR	r2, |ee_taskhandle|	; their handle
	MOV	r0, #17
	MOV	r1, r5
	SWI	Wimp_SendMessage
	  ; send it
; set new state
	MOV	r0, #S_AwaitLoadAck	; awaiting 'ack' message
	STR	r0, |filterstate|	; store it
	LDMFD	sp!, {r0-r5, pc}	; Return from call

|__z3_failed|
	  ; "Load copy failed"
	MOV	r0, #Ret_Failed
	STR	r0, returnstate		; store the return state
	  ; we failed to launch edit
	BL     |ee_abort|
	  ; and send the abort
	LDMFD	sp!,{r0-r5, pc}		; Return from call


	; The filter to handle things
	NAME	"filter"
|filter|
	STMFD	sp!, {r1-r5, lr}	; Stack registers
	STR	r0, reason		; hang on to reason
	  ; "%c04%c30Filter= reason= %r0"
	ADR	lr, __z4_return		; address to return to
	TEQ	r0, #0			; is it null ?
	BEQ	null			; handle it

	TEQ	r0, #17			; is it usermessage ?
	TEQNE	r0, #18			; or usermessagerecorded ?
	BEQ	usermessage		; it's a usermessage
	TEQ	r0, #19			; is it usermessageack ?
	BEQ	usermessageack
|__z4_return|
	LDR	r0, reason		; re-read reason
	  ; "Returning reason %r0"
	LDMFD	sp!, {r1-r5, pc}	; Return from call

	NAME	"null"
|null|
	STMFD	sp!, {r0, lr}		; Stack registers
	LDR	r0, |filterstate|	; read state
	TEQ	r0, #S_SendReturn	; we need to return
	BLEQ	|ee_return|
	  ; send the 'editreturn'
	TEQ	r0, #S_SendAbort	; we need to abort and return
	MOVEQ	r0, #Ret_Unchanged
	STREQ	r0, returnstate		; store the return state
	  ; the file wasn't changed
	BLEQ	|ee_abort|
	  ; and send the abort
	LDMFD	sp!, {r0, pc}		; Return from call

	NAME	"usermessageack"
|usermessageack|
	LDR	r2, [r1, #16]		; read message type

	MOV	r4, lr
	  ; "ReceivedAck message %&2"
	MOV	lr, r4

	LDR	r3, |const_ee_editrq|	; EditRq bounced ?
	CMP	r2, r3			; was that it ?
	BEQ	um_editrq_bounced	; ok, so deal with it
	LDR	r3, |const_ee_datasave|	; EditDataSave bounced ?
	CMP	r2, r3			; was that it ?
	BEQ	um_editds_bounced	; ok, so deal with it
	MOV	pc,lr

	NAME	"um_editrq_bounced"
|um_editrq_bounced|
	STMFD	sp!,{r0-r5, lr}		; Stack registers
	  ; "EditRq bounced"
	LDR	r0, |filterstate|	; read the state
	TEQ	r0, #S_AwaitAck		; are we waiting for ack ?
	MOVEQ	r0, #Ret_Failed
	STREQ	r0, returnstate		; store the return state

	  ; with the code 'failed'
	  ; "Back to Editrq_bounced"
	MOV	r0, #-1			; don't pass on
	STR	r0, reason		; store as reason
	LDMFD	sp!,{r0-r5, pc}		; Return from call

	NAME	"um_editds_bounced"
|um_editds_bounced|
	STMFD	sp!,{r0-r5, lr}		; Stack registers
	  ; "EDS bounced"
	LDR	r0, |filterstate|	; read the state
	TEQ	r0, #S_AwaitSaveAck	; are we waiting for saveack ?
	MOVEQ	r0, #Ret_Failed
	STREQ	r0, returnstate		; store the return state
	  ; we've failed
	BLEQ   |ee_abort|
	  ; and send the abort
	MOV	r0, #-1			; don't pass on
	STR	r0, reason		; store as reason
	LDMFD	sp!,{r0-r5, pc}		; Return from call

	NAME	"usermessage"
|usermessage|
	LDR	r2, [r1, #16]		; read message type

	TEQ	r2, #0			; is it 'quit' ?
	LDRNE	r3, |const_tw_morite|	; have to read as a word
	TEQNE	r2, r3			; or tw_morite ?
	BEQ	um_quit			; we've been told to quit

	LDR	r3, |const_ee_editack|	; EditAck
	TEQ	r2, r3			; was that it ?
	BEQ	um_editack		; yeah, we got it !

	LDR	r3, |const_ee_abort|	; EditAbort
	TEQ	r2, r3			; was that it ?
	BEQ	um_editabort		; yeah, we got it !

	LDR	r3, |const_ee_datasave|	; EditDataSave
	TEQ	r2, r3			; was that it ?
	BEQ	um_editds		; yeah, we got it !

	TEQ	r2, #DataSaveAck	; is it DataSaveAck ?
	BEQ	um_datasaveack		; ooh, ok !

	TEQ	r2, #DataLoad		; is it DataLoad ?
	BEQ	um_dataload		; hey, things returing !
	MOV	pc, lr

	NAME	"um_editds"
|um_editds|
	STMFD	sp!,{lr}		; Stack registers
	ADD	r3, r1, #44
	LDR	r4, [r1, #40]
	       ; "Their filename was %$3, type=%&4"
	LDR	r0, |filterstate|	; read filterstate
	       ; "Received EditDataSave, state=%r0"
	TEQ	r0, #S_AwaitLoadAck	; are we awaiting a save ? (middle)
	TEQNE	r0, #S_AwaitESave	; or explicitly awaiting one ?
	LDMNEFD sp!, {pc}		; if not, return

	BL	ee_sendsaveack		; send a datasaveack at the task
	MOV	r0, #-1			; don't pass on
	STR	r0, reason		; store as reason
	LDMFD	sp!, {pc}		; Return from call


	NAME	"um_editabort"
|um_editabort|
	STMFD	sp!, {r0, lr}		; Stack registers
	MOV	r0, #Ret_Unchanged
	STR	r0, returnstate		; store the return state

	  ; the file wasn't changed
	MOV	r0, #-1			; don't pass on
	STR	r0, reason		; store as reason
	LDMFD	sp!, {r0, pc}		; Stack registers

	NAME	"um_datasaveack"
|um_datasaveack|
	STMFD	sp!,{lr}		; Stack registers
	LDR	r0, |filterstate|	; read filterstate
	  ; "Received SaveAck, state=%r0"
	TEQ	r0, #S_AwaitSaveAck	; are we awaiting a save ?
	LDMNEFD sp!, {pc}		; if not, return

	BL	ee_sendload		; send a dataload at the task
	MOV	r0, #-1			; don't pass on
	STR	r0, reason		; store as reason
	LDMFD	sp!,{pc}		; Return from call

	NAME	"um_dataload"
|um_dataload|
	STMFD	sp!,{lr}		; Stack registers
	LDR	r0, |filterstate|	; read filterstate
	  ; "Received Load, state=%r0"
	TEQ	r0, #S_AwaitLoad	; are we awaiting a save ?
	LDMNEFD sp!, {pc}		; if not, return

	BL	ee_sendloadack		; send a dataloadack at the task
	MOV	r0, #Ret_Updated
	STR	r0, returnstate		; store the return state

	  ; the file WAS changed
	MOV	r0, #-1			; don't pass on
	STR	r0, reason		; store as reason
	LDMFD	sp!,{pc}		; Return from call


	NAME	"um_editack"
|um_editack|
	STMFD	sp!, {r0-r5, lr}	; Stack registers
	LDR	r0, |filterstate|	; read filterstate
	  ; "Received Ack, state=%r0"
	TEQ	r0, #S_AwaitAck		; are we awaiting an ack ?
	LDMNEFD sp!, {r0-r5, pc}	; if not, return

	  ; "Storing handles and things"
	LDR	r0, [r1, #24]		; read jobhandle
	STR	r0, |ee_jobhandle|	; store it
	LDR	r0, [r1, #4]		; read taskhandle
	STR	r0, |ee_taskhandle|	; store it
	BL	ee_startsendfile	; send the file
	MOV	r0, #-1			; don't pass on
	STR	r0, reason		; store as reason
	LDMFD	sp!,{r0-r5, pc}		; Return from call


	NAME	"um_quit"
|um_quit|
	STMFD	sp!, {lr}		; Stack registers
	BL	|ee_abort|		; send an abort
	BL	removefilter		; remove the filter
	MOV	r0, #-1			; don't pass on
	STR	r0, reason		; store as reason
	LDMFD	sp!, {pc}		; Return from call


	; > r0=input string
	;   r1=ouput buffer
	NAME	"ee_strcpy"
|ee_strcpy|
	STMFD	sp!, {r0-r1, lr}
|ee_strcpy_loop|
	LDRB	r14, [r0], #1
	STRB	r14, [r1], #1
	CMP	r14, #31
	BGT	|ee_strcpy_loop|
	LDMFD	sp!, {r0-r1, pc}


	; > r0=input string
	; < r1=LEN(r0)
	NAME	"ee_strlen"
|ee_strlen|
	STMFD	sp!, {lr}
	MOV	r1, #0
|ee_strlen_loop|
	LDRB	r14, [r0, r1]
	ADD	r1, r1, #1
	CMP	r14, #31
	BGT	|ee_strlen_loop|
	SUB	r1, r1, #1
	LDMFD	sp!, {pc}


; *******************************************************************
; Subroutine;	claim
; Description;  claim some RMA to r0 (size=r0)
; Parameters;	r0 = size
; Returns;	r0 = address, or 0 if failed
; *******************************************************************
	NAME	"claim"
|claim|
	STMFD	sp!, {r1-r3, lr}	; Stack registers
	MOV	r3, r0			; right register
	MOV	r0, #6
	SWI	XOS_Module
	MOVVS	r0, #0			; if error, return 0
	MOVVC	r0, r2			; return address
	LDMFD	sp!, {r1-r3, pc}	; Return from call

; *******************************************************************
; Subroutine;	release
; Description;  release RMA at r0
; Parameters;	r0 = address
; Returns;	none
; *******************************************************************
	NAME	"release"
|release|
	STMFD	sp!, {r0-r2, lr}	; Stack registers
	MOV	r2, r0			; right register
	MOV	r0, #7
	SWI	XOS_Module
	LDMFD	sp!, {r0-r2, pc}	; Return from call


; *******************************************************************
; Subroutine;	ee_strdup
; Description;  Copy a string
; Parameters;	r0-> string
; Returns;	r0-> new copy of string, = 0 if error
; *******************************************************************
	NAME	"ee_strdup"
|ee_strdup|
	STMFD	sp!, {r0-r2, lr}	; Stack registers
	BL	|ee_strlen|		; find length of string
	MOV	r2, r1			; hang on to length of string
	ADD	r0, r1, #1		; including terminator
	BL	claim			; claim space for it
	TEQ	r0, #0			; did claim fail ?
	BEQ	|ee_strdup_fail|	; if so, give error

	MOV	r1, r0			; r1=destination
	LDR	r0, [sp]		; re-read source
	BL	|ee_strcpy|		; copy string
	STR	r1, [sp]		; return destination
	MOV	r0, #0			; null terminator
	STRB	r0, [r1, r2]		; zero terminated
	LDMFD	sp!, {r0-r2, pc}	; Return from call
|ee_strdup_fail|
	ADD	sp, sp, #4		; skip r0
	LDMFD	sp!, {r1-r2, pc}	; restore registers

	END
