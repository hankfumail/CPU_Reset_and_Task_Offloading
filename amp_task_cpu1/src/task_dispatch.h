/*
 * Copyright (c) 2009 Xilinx, Inc.  All rights reserved.
 *
 * Xilinx, Inc.
 * XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS" AS A
 * COURTESY TO YOU.  BY PROVIDING THIS DESIGN, CODE, OR INFORMATION AS
 * ONE POSSIBLE   IMPLEMENTATION OF THIS FEATURE, APPLICATION OR
 * STANDARD, XILINX IS MAKING NO REPRESENTATION THAT THIS IMPLEMENTATION
 * IS FREE FROM ANY CLAIMS OF INFRINGEMENT, AND YOU ARE RESPONSIBLE
 * FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE FOR YOUR IMPLEMENTATION.
 * XILINX EXPRESSLY DISCLAIMS ANY WARRANTY WHATSOEVER WITH RESPECT TO
 * THE ADEQUACY OF THE IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO
 * ANY WARRANTIES OR REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE
 * FROM CLAIMS OF INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

	 
#ifndef __TASK_DISPATCH_H_
#define __TASK_DISPATCH_H_


#include <stdio.h>

#include "sleep.h"

#include "platform.h"

#include "xil_types.h"
#include "xil_io.h"
#include "xil_mmu.h"
#include "xil_cache.h"
#include "xil_exception.h"

#include "Xpseudo_asm.h"




/***************** Macros (Inline Functions) Definitions *********************/

/*
 *
 * CPU 0 Application:  1M-------(16M-1), base: 0x100000,  long: 0x0f00000;
 * CPU 1 Application: 64M-------(80M-1), base: 0x4000000, long: 0x1000000;
 *
 * Task function address and flag area: 128M, base: 0x8000000, long: 0x200;
 *    Offset 0:  valid flag 1, 0x34567812
 *    Offset 4:  valid flag 2, 0xaa5599bb
 *    Offset 8:  Task function address
 *    Offset c:  Task function address
 *
 *  CPU1 program backup memory region:   768M----(768M+16M-1)  : 0x30000000 ----- 0x30000000 + 0x1000000 *
 *  CPU1 program backup flag region:   768M----(768M+16M-1)  : 0x30000000 + 0x1000000 - 0x10 *
 *
 */

#define CPU1_RST_BASE_ADRS						0x0000000

#define CPU1_STUB_BASE_ADRS						0xFFFFFF00

#define CPU1_APP_BASE_ADRS_POINTER				0xFFFFFFF0

#define CPU1_APP_BASE_ADRS						0x4000000
#define CPU1_APP_BACKUP_BASE_ADRS				0x30000000
#define CPU1_APP_BACKUP_LEN						0x1000000
#define CPU1_APP_BACKUP_FLAG_ADRS				(CPU1_APP_BACKUP_BASE_ADRS+CPU1_APP_BACKUP_LEN-0x10)
#define CPU1_APP_BACKUP_FLAG1					(0x12345678)
#define CPU1_APP_BACKUP_FLAG2					(0xabcdef78)
#define CPU1_APP_BACKUP_FLAG3					(0x8899aaee)
#define CPU1_APP_BACKUP_FLAG4					(0x5566aa88)

#define TASK_FUNC_HAND_SHAKE_BASE_ADRS			0x8000000
#define TASK_FUNC_VALID_FLAG1_ADRS				(TASK_FUNC_HAND_SHAKE_BASE_ADRS+0)
#define TASK_FUNC_VALID_FLAG1_VALUE				0x34567812
#define TASK_FUNC_VALID_FLAG2_ADRS				(TASK_FUNC_HAND_SHAKE_BASE_ADRS+4)
#define TASK_FUNC_VALID_FLAG2_VALUE				0xaa5599bb

#define TASK_FUNC_PTR1_ADRS						(TASK_FUNC_HAND_SHAKE_BASE_ADRS+8)
#define TASK_FUNC_PTR2_ADRS						(TASK_FUNC_HAND_SHAKE_BASE_ADRS+12)

#define TASK_FUNC_PTR_RSPS_ADRS1				(TASK_FUNC_HAND_SHAKE_BASE_ADRS+16)
#define TASK_FUNC_PTR_RSPS_ADRS2				(TASK_FUNC_HAND_SHAKE_BASE_ADRS+20)

#define TASK_FUNC_REPORT_INTERVAL				1000000
#define TASK_FUNC_WAIT_MAX_TIME					10000000
#define TASK_FUNC_WAIT_REPORT_INTERVAL			(TASK_FUNC_WAIT_MAX_TIME/10)

#define TASK_FUNC_CACHE_SIZE					(0x40)


#define sev() __asm__("sev")
#define wfi() __asm__ __volatile__ ("wfi")
#define wfe() __asm__ __volatile__ ("wfe")


//#define	DBG_PRINT_ON


#ifdef	DBG_PRINT_ON
#define dbg_printf(format, ...) { printf(format, ## __VA_ARGS__);  }
#else
#define dbg_printf(type, ...)
#endif


/* These are the SLCR lock and unlock macros */
#define SlcrUnlock()		Xil_Out32(XPS_SYS_CTRL_BASEADDR + 0x08, 0xDF0DDF0D)
#define SlcrLock()			Xil_Out32(XPS_SYS_CTRL_BASEADDR + 0x04, 0x767B767B)


/**************************** Type Definitions *******************************/

typedef void (*task_func)( void );


/************************** Constant Definitions *****************************/



/************************** Variable Definitions *****************************/



/************************** Function Prototypes ******************************/




#endif /* __TASK_DISPATCH_H_  */

