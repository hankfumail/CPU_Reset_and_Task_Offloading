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

/*
 * helloworld.c: simple test application
 */



/***************** Header Files *********************/


#include <stdio.h>

#include "sleep.h"

#include "platform.h"

#include "xil_printf.h"
#include "xil_types.h"
#include "xil_io.h"
#include "xil_mmu.h"
#include "xil_cache.h"
#include "xil_exception.h"

#include "Xpseudo_asm.h"

#include "task_dispatch.h"

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
 *  connect arm hw -debugdevice cpunr 1
 */
		


/***************** Macros (Inline Functions) Definitions *********************/



/**************************** Type Definitions *******************************/



/************************** Constant Definitions *****************************/



/************************** Variable Definitions *****************************/



/************************** Function Prototypes ******************************/


//void print(char *str);



/************************** Function Implementation ******************************/

int task_dispatch_slave( int i )
{
	int iInvalidFuncPtrCnt=0;
	int iInvalidFlagCnt=0;
	static int i_task_func_num=0;
	task_func  cpu1_task_func;
	volatile unsigned int *puiValidFlag1	= 	(volatile unsigned int *)TASK_FUNC_VALID_FLAG1_ADRS;
	volatile unsigned int *puiValidFlag2	=	(volatile unsigned int *)TASK_FUNC_VALID_FLAG2_ADRS;
	volatile unsigned int *puiTaskFuncPtr1	=	(volatile unsigned int *)TASK_FUNC_PTR1_ADRS;
	volatile unsigned int *puiTaskFuncPtr2	=	(volatile unsigned int *)TASK_FUNC_PTR2_ADRS;

	volatile unsigned int *puiTaskFuncRspsPtr1	=	(volatile unsigned int *)TASK_FUNC_PTR_RSPS_ADRS1;
	volatile unsigned int *puiTaskFuncRspsPtr2	=	(volatile unsigned int *)TASK_FUNC_PTR_RSPS_ADRS2;


	/*   Invalidate cache, not necessary if this memory section is non-cacheable */
	Xil_DCacheInvalidateRange(TASK_FUNC_HAND_SHAKE_BASE_ADRS, TASK_FUNC_CACHE_SIZE);

	if( ( TASK_FUNC_VALID_FLAG1_VALUE == (*puiValidFlag1) ) 
		 && ( TASK_FUNC_VALID_FLAG2_VALUE == (*puiValidFlag2) ) )
	{
    	if( (*puiTaskFuncPtr1)  == (*puiTaskFuncPtr2)  )
    	{
    		i_task_func_num++;
    		if( 0 == (i%TASK_FUNC_REPORT_INTERVAL) )
    		{
    			xil_printf("No. %010d task function, address: 0x%08x.\n\r", i_task_func_num, *puiTaskFuncPtr1);
    		}

            cpu1_task_func = (task_func)(*puiTaskFuncPtr1);
    		cpu1_task_func( );

        	*puiValidFlag1 = 0;
        	*puiValidFlag2 = 0;
        	*puiTaskFuncPtr1 = 0;
        	*puiTaskFuncPtr2 = 0;
    	}
    	else
    	{
    		iInvalidFuncPtrCnt++;
    		*puiTaskFuncRspsPtr1 = TASK_FUNC_VALID_FLAG1_VALUE;
    		*puiTaskFuncRspsPtr2 = TASK_FUNC_VALID_FLAG1_VALUE;
            xil_printf("No. %010d wakeup has No.%d invalid function pointer: 0x%08x-%08x.\n\r", 
						i, iInvalidFuncPtrCnt, *puiTaskFuncPtr1, *puiTaskFuncPtr2);
        	usleep ( 200000  );
    	}
	}
	else
	{
		iInvalidFlagCnt++;
		*puiTaskFuncRspsPtr1 = TASK_FUNC_VALID_FLAG2_VALUE;
		*puiTaskFuncRspsPtr2 = TASK_FUNC_VALID_FLAG2_VALUE;
		if( 0 == (iInvalidFlagCnt%10) )
		{
            xil_printf("No. %010d wakeup has No.%d invalid flag: 0x%08x-%08x.\n\r", 
					i, iInvalidFlagCnt, *puiValidFlag1, *puiValidFlag2);
        	usleep ( 200000  );
		}
	}

	dsb();

	 /* Flush cache, not necessary if this memory section is non-cacheable or cacheable-writethrough */
	Xil_DCacheFlushRange(TASK_FUNC_HAND_SHAKE_BASE_ADRS, TASK_FUNC_CACHE_SIZE);

    return 0;
}


int main()
{
	int	i;


    init_platform();
    //disable_caches( );
	//Xil_DCacheFlush( );
	//Xil_DCacheDisable( );
    //Xil_DisableMMU( );

	xil_printf("\n\rXilinx Hello World from CPU %d, USE_AMP:%d \n\r", XPAR_CPU_ID, USE_AMP);
	xil_printf("Release 14.4 %s-%s\r\n",__DATE__,__TIME__);
	

	/* Use non-cache mode for these two memory region */
	Xil_SetTlbAttributes(TASK_FUNC_HAND_SHAKE_BASE_ADRS, 0x4de2 );

    for( i=1; ; i++ )
    {
    	wfe();

    	if( 0 == (i%TASK_FUNC_REPORT_INTERVAL) )
		{
			xil_printf("No. %010d wakeup from CPU 1.\n\r", i);
		}

    	task_dispatch_slave( i );

    }

    return 0;
}
