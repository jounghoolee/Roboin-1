/*
 *######################################################################
 *                                RAppIDJDP
 *           Rapid Application Initialization and Documentation Tool
 *                         Freescale Semiconductor Inc.
 *
 *######################################################################
 *
 * Project Name           : NGV_DMU
 *
 * Project File           : NGV_DMU.rsp
 *
 * Revision Number        : 1.0
 *
 * Tool Version           : 1.2.1.5
 *
 * file                   : rchw_init.c
 *
 * Target Compiler        : Codewarrior
 *
 * Target Part            : MPC5606B
 *
 * Part Errata Fixes      : none
 *
 * Project Last Save Date : 28-Dec-2014 21:53:21
 *
 * Created on Date        : 28-Dec-2014 21:53:27
 *
 * Brief Description      : File contains RCHW initialization
 *
 ******************************************************************************** 
 *
 * Detail Description     : File contains the Bootup Reset Config Half Word(RCHW)
 *                         initialization. This files pushed control on reset to
 *                         crt0.s start function.
 *
 ******************************************************************************** 
 *
 *######################################################################
*/

#include "ngv_dmu.h"


#if NGV_DMU_PROJECT == 0
 
#pragma section code_type ".init"

extern __asm void __start();

typedef void (*resetfuncptr)(void);

#pragma push /* Save the current state */
#pragma section sconst_type ".__bam_bootarea"
extern const long int bam_rchw;
extern const resetfuncptr bam_resetvector;

const long int bam_rchw =  0x005A0000;             
    /* BOOTID: Boot Identifier is: 005A0000*/


const resetfuncptr bam_resetvector = __start;

#pragma pop

#endif
 
/*
 *######################################################################
 *                           End of File
 *######################################################################
*/

