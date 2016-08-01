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


/************************** Function Implementation ******************************/



void task_func_print1( void )
{
	static int i_call_cnt1=0;

	i_call_cnt1++;

	if( 0 == (i_call_cnt1%TASK_FUNC_REPORT_INTERVAL) )
	{
	    xil_printf("No. %010d call for task_func_print1 1 1.\n\r", i_call_cnt1 );
	}

}

void task_func_print2( void )
{
	static int i_call_cnt2=0;

	i_call_cnt2++;

	if( 0 == (i_call_cnt2%TASK_FUNC_REPORT_INTERVAL) )
	{
	    xil_printf("No. %010d call for task_func_print2 2 2.\n\r", i_call_cnt2 );
	}

}


/* ======================================== */
void rst_stub_instruction_report( void )
{
	unsigned int *puiCpu1RstAdrs = (unsigned int *)(CPU1_RST_BASE_ADRS);

	xil_printf("CPU 1 reset address: 0x%08x.\n\r", (unsigned int)puiCpu1RstAdrs);

	xil_printf("\n\rCPU 1 instruction in reset address read by  CPU 0:\n\r");
	xil_printf( "%08x %08x %08x %08x %08x %08x %08x %08x\n\r",
						puiCpu1RstAdrs[0], puiCpu1RstAdrs[1], puiCpu1RstAdrs[2], puiCpu1RstAdrs[3],
						puiCpu1RstAdrs[4], puiCpu1RstAdrs[5], puiCpu1RstAdrs[6], puiCpu1RstAdrs[7] );
	xil_printf( "%08x %08x %08x %08x %08x %08x %08x %08x\n\r",
						puiCpu1RstAdrs[8], puiCpu1RstAdrs[9], puiCpu1RstAdrs[10], puiCpu1RstAdrs[11],
						puiCpu1RstAdrs[12], puiCpu1RstAdrs[13], puiCpu1RstAdrs[14], puiCpu1RstAdrs[15] );
	xil_printf( "%08x %08x %08x %08x %08x %08x %08x %08x\n\r",
						puiCpu1RstAdrs[16], puiCpu1RstAdrs[17], puiCpu1RstAdrs[18], puiCpu1RstAdrs[19],
						puiCpu1RstAdrs[20], puiCpu1RstAdrs[21], puiCpu1RstAdrs[22], puiCpu1RstAdrs[23] );
	xil_printf( "%08x %08x %08x %08x %08x %08x %08x %08x\n\r",
						puiCpu1RstAdrs[24], puiCpu1RstAdrs[25], puiCpu1RstAdrs[26], puiCpu1RstAdrs[27],
						puiCpu1RstAdrs[28], puiCpu1RstAdrs[29], puiCpu1RstAdrs[30], puiCpu1RstAdrs[31] );

}


/*

Table B1-3 Offsets from exception base addresses
Exception 
offset
Exception that is vectored at that offset from:
Monitor exception base addressa
a. This column applies only if the Security Extensions are implemented.
Base address for all other exceptions
0x00 Not used Reset
0x04 Not used Undefined Instruction
0x08 Secure Monitor Call (SMC) Supervisor Call (SVC)
*/
void rst_stub_install( void )
{
	unsigned int *puiCpu1RstAdrs = (unsigned int *)(CPU1_RST_BASE_ADRS);
	xil_printf("CPU 1 reset address: 0x%08x.\n\r", (unsigned int)puiCpu1RstAdrs);

	/*  
	CPU 1 stub instruction address: 0xFFFFFF00 = '#255'
	__asm__ ("mvn	pc, #255\n");
	  10086c:	e3e0f0ff	mvn pc, #255	; 0xff
	*/
	puiCpu1RstAdrs[0] = 0xe3e0f0ff;

	Xil_DCacheFlushRange( (unsigned int)puiCpu1RstAdrs, 0x20);
	
	/* Barrier */
	asm(
		"dsb\n\t"
		"isb"
	);
}


/* Function to get the machine code. */
void rst_stub_asm_check( void )
{
	/*  CPU 1 stub instruction address: 0xFFFFFF00   */

	/*  
	__asm__ ("mvn	pc, #255\n");
	  10086c:	e3e0f0ff	mvn pc, #255	; 0xff
	*/
	__asm__ ("mvn	pc, #255\n");
}


void stub_adrs_instruction_report( void )
{
	unsigned int *puiCpu1StubAdrs = (unsigned int *)(0xFFFFFF00);

	xil_printf("CPU 1 stub address: 0x%08x.\n\r", (unsigned int)0xFFFFFF00);

	xil_printf("\n\rCPU 1 instruction in stub address read by  CPU 0:\n\r");
	xil_printf( "%08x %08x %08x %08x %08x %08x %08x %08x\n\r",
						puiCpu1StubAdrs[0], puiCpu1StubAdrs[1], puiCpu1StubAdrs[2], puiCpu1StubAdrs[3],
						puiCpu1StubAdrs[4], puiCpu1StubAdrs[5], puiCpu1StubAdrs[6], puiCpu1StubAdrs[7] );
	xil_printf( "%08x %08x %08x %08x %08x %08x %08x %08x\n\r",
						puiCpu1StubAdrs[8], puiCpu1StubAdrs[9], puiCpu1StubAdrs[10], puiCpu1StubAdrs[11],
						puiCpu1StubAdrs[12], puiCpu1StubAdrs[13], puiCpu1StubAdrs[14], puiCpu1StubAdrs[15] );
	xil_printf( "%08x %08x %08x %08x %08x %08x %08x %08x\n\r",
						puiCpu1StubAdrs[16], puiCpu1StubAdrs[17], puiCpu1StubAdrs[18], puiCpu1StubAdrs[19],
						puiCpu1StubAdrs[20], puiCpu1StubAdrs[21], puiCpu1StubAdrs[22], puiCpu1StubAdrs[23] );
	xil_printf( "%08x %08x %08x %08x %08x %08x %08x %08x\n\r",
						puiCpu1StubAdrs[24], puiCpu1StubAdrs[25], puiCpu1StubAdrs[26], puiCpu1StubAdrs[27],
						puiCpu1StubAdrs[28], puiCpu1StubAdrs[29], puiCpu1StubAdrs[30], puiCpu1StubAdrs[31] );

}


void stub_adrs_instruction_install( void )
{
	unsigned int *puiCpu1StubAdrs = (unsigned int *)(0xFFFFFF00);

	/*
		# connect to the 2nd CPU core
		connect arm hw -debugdevice cpunr 2
		#0xFFFFFF00: mvn     r0, #15
		mwr 0xFFFFFF00   0xe3e0000f
		#0xFFFFFF00: mov     r1, #0
		mwr 0xFFFFFF04   0xe3a01000
		#0xFFFFFF00: str     r1, [r0]
		mwr 0xFFFFFF08   0xe5801000
		#0xFFFFFF00: wfe
		mwr 0xFFFFFF0c   0xe320f002
		#0xFFFFFF00: ldr     r2, [r0]
		mwr 0xFFFFFF10   0xe5902000
		#0xFFFFFF00: cmp     r2, r1
		mwr 0xFFFFFF14   0xe1520001
		#0xFFFFFF00: beq     0xc
		mwr 0xFFFFFF18   0x0afffffb
		#0xFFFFFF00: mov     pc, r2
		mwr 0xFFFFFF1c   0xe1a0f002
		# program couter of the 2nd CPU core at 0xFFFFFF00
		rwr pc 0xFFFFFF00
		# continue the execution
		con
	 */
	puiCpu1StubAdrs[0] = 0xe3e0000f;
	puiCpu1StubAdrs[1] = 0xe3a01000;
	puiCpu1StubAdrs[2] = 0xe5801000;
	puiCpu1StubAdrs[3] = 0xe320f002;
	puiCpu1StubAdrs[4] = 0xe5902000;
	puiCpu1StubAdrs[5] = 0xe1520001;
	puiCpu1StubAdrs[6] = 0x0afffffb;
	puiCpu1StubAdrs[7] = 0xe1a0f002;
	puiCpu1StubAdrs[8] = 0xe3e020ff;
	puiCpu1StubAdrs[9] = 0xe1a0f002;
	puiCpu1StubAdrs[10] = 0xe3e020ff;
	puiCpu1StubAdrs[11] = 0xe1a0f002;

	/* Flush cache, not necessary if this memory section is non-cacheable or cacheable-writethrough
	    Xil_DCacheFlushRange will mask lower bits
	*/
	Xil_DCacheFlushRange( (unsigned int)puiCpu1StubAdrs, 0x80);
	
	/* Barrier */
	asm(
		"dsb\n\t"
		"isb"
	);
}


void app_instruction_report( void )
{
	/* ==============================================  */
	/*	CPU0  1M----(16M-1) 	: 0x00100000 ----- 0x00100000 + 0xf00000  */
	/*	CPU1 64M----(64+16M-1)	: 0x04000000 ----- 0x04000000 + 0x1000000 */
	unsigned int *puiCpu1ImageAdrs = (unsigned int *)(CPU1_APP_BASE_ADRS);

	xil_printf("CPU 1 application start address: 0x%08lx at 0xFFFFFFF0.\n\r", puiCpu1ImageAdrs);

	xil_printf("\n\rCPU 1 application instruction read by  CPU 0:\n\r");
	xil_printf( "%08x %08x %08x %08x %08x %08x %08x %08x\n\r",
						puiCpu1ImageAdrs[0], puiCpu1ImageAdrs[1], puiCpu1ImageAdrs[2], puiCpu1ImageAdrs[3],
						puiCpu1ImageAdrs[4], puiCpu1ImageAdrs[5], puiCpu1ImageAdrs[6], puiCpu1ImageAdrs[7] );
	xil_printf( "%08x %08x %08x %08x %08x %08x %08x %08x\n\r",
						puiCpu1ImageAdrs[8], puiCpu1ImageAdrs[9], puiCpu1ImageAdrs[10], puiCpu1ImageAdrs[11],
						puiCpu1ImageAdrs[12], puiCpu1ImageAdrs[13], puiCpu1ImageAdrs[14], puiCpu1ImageAdrs[15] );
	xil_printf( "%08x %08x %08x %08x %08x %08x %08x %08x\n\r",
						puiCpu1ImageAdrs[16], puiCpu1ImageAdrs[17], puiCpu1ImageAdrs[18], puiCpu1ImageAdrs[19],
						puiCpu1ImageAdrs[20], puiCpu1ImageAdrs[21], puiCpu1ImageAdrs[22], puiCpu1ImageAdrs[23] );
	xil_printf( "%08x %08x %08x %08x %08x %08x %08x %08x\n\r",
						puiCpu1ImageAdrs[24], puiCpu1ImageAdrs[25], puiCpu1ImageAdrs[26], puiCpu1ImageAdrs[27],
						puiCpu1ImageAdrs[28], puiCpu1ImageAdrs[29], puiCpu1ImageAdrs[30], puiCpu1ImageAdrs[31] );
}


void app_install( void )
{
	/* ======================================== */
	/*
	 * CPU1 64M----(64+16M-1)	: 0x04000000 ----- 0x04000000 + 0x1000000 *
	 * Restore CPU1 program from 0x30000000 *
	 *  CPU1 program backup memory region:   768M----(768M+16M-1)  : 0x30000000 ----- 0x30000000 + 0x1000000 *
	 *  CPU1 program backup flag region:   768M----(768M+16M-1)  : 0x30000000 + 0x1000000 - 0x10 *
	 *
	 */
	unsigned int *puiCpu1ImageAdrs = (unsigned int *)(CPU1_APP_BASE_ADRS);
	unsigned int *puiCpu1StartAdrs = (unsigned int *)CPU1_APP_BASE_ADRS_POINTER;
	unsigned int *puiCpu1ProgramBackupFlag = (unsigned int *)(CPU1_APP_BACKUP_FLAG_ADRS);

	xil_printf("CPU 1 application address: 0x%08x.\n\r", (unsigned int)puiCpu1ImageAdrs);
	xil_printf("CPU 1 application flag address: 0x%08x.\n\r", (unsigned int)puiCpu1ProgramBackupFlag);
	xil_printf("CPU 1 application address location: 0x%08x.\n\r", (unsigned int)puiCpu1StartAdrs);

	if(  ( CPU1_APP_BACKUP_FLAG1 == puiCpu1ProgramBackupFlag[0]) && ( CPU1_APP_BACKUP_FLAG2 == puiCpu1ProgramBackupFlag[1])
			 && ( CPU1_APP_BACKUP_FLAG3 == puiCpu1ProgramBackupFlag[2])  && ( CPU1_APP_BACKUP_FLAG4 == puiCpu1ProgramBackupFlag[3]) )
	{
		xil_printf( "CPU 1 program flag at 0x30000000 is valid. \n\r" );
	}
	else
	{
		xil_printf( "CPU 1 program flag at 0x30000000 is invalid. \n\r" );
		xil_printf( "Backup CPU 1 program in FSBL first. \n\r" );
		xil_printf( "CPU 1 program flag 0: 0x%08x. \n\r", puiCpu1ProgramBackupFlag[0] );
		xil_printf( "CPU 1 program flag 1: 0x%08x. \n\r", puiCpu1ProgramBackupFlag[1] );
		xil_printf( "CPU 1 program flag 2: 0x%08x. \n\r", puiCpu1ProgramBackupFlag[2] );
		xil_printf( "CPU 1 program flag 3: 0x%08x. \n\r", puiCpu1ProgramBackupFlag[3] );
		return;
	}

	*puiCpu1StartAdrs = (unsigned int)puiCpu1ImageAdrs;
	/* Flush cache, not necessary if this memory section is non-cacheable or cacheable-writethrough
	    Xil_DCacheFlushRange will mask lower bits
	*/
	Xil_DCacheFlushRange( (unsigned int)puiCpu1StartAdrs, 32);
	dbg_printf("CPU 1 start address: 0x%08x at 0xFFFFFFF0.\n\r", *puiCpu1StartAdrs);

	/* Invalidate cache, not necessary if this memory section is non-cacheable */
	Xil_DCacheInvalidateRange( CPU1_APP_BACKUP_BASE_ADRS, CPU1_APP_BACKUP_LEN);
	dbg_printf( "Copy CPU 1 program from 0x%08x to 0x%08x.\n\r", CPU1_APP_BACKUP_BASE_ADRS, puiCpu1ImageAdrs );
	memset( puiCpu1ImageAdrs, 0, CPU1_APP_BACKUP_LEN);

	memcpy( puiCpu1ImageAdrs, (void *)CPU1_APP_BACKUP_BASE_ADRS, CPU1_APP_BACKUP_LEN);  // reinstall the application.
	memcpy( puiCpu1ImageAdrs, (void *)0x30000000, 0x1000000);  // backup
	Xil_DCacheFlushRange( (unsigned int)puiCpu1ImageAdrs, 0x1000000);\
	dsb();

}


void rst_cpu1( void )
{
	unsigned int *puiCpu1StartAdrs = (unsigned int *)CPU1_APP_BASE_ADRS_POINTER;

	/* Reset CPU 1 and wait for sometimes. */
	xil_printf("\n\r\n\rCPU0 reset CPU1......\n\r");
	
	/* Unlock the SLCR */
	SlcrUnlock();

	u32 u32CpuRstCtrlReg;

	/* ========================================
		Register (slcr) A9_CPU_RST_CTRL
		A9_CLKSTOP1 5 rw 0x0 CPU 1 clock stop control:
		A9_CLKSTOP0 4 rw 0x0 CPU 0 clock stop control:
		A9_RST1 1 rw 0x0 CPU 0 software reset control:
		A9_RST0 0 rw 0x0 CPU 0 software reset control:
	*/
	u32CpuRstCtrlReg = Xil_In32(XPS_SYS_CTRL_BASEADDR + 0x244);
	u32CpuRstCtrlReg |= (1<<1);  /* Bit 2, A9_RST1   */
	Xil_Out32(XPS_SYS_CTRL_BASEADDR + 0x244, u32CpuRstCtrlReg);
	dbg_printf("Register (slcr) A9_CPU_RST_CTRL written to reset CPU 0: 0x%08x.\n\r", u32CpuRstCtrlReg );
	usleep ( 1 );

	u32CpuRstCtrlReg = Xil_In32(XPS_SYS_CTRL_BASEADDR + 0x244);
	u32CpuRstCtrlReg |= ((1<<1)|(1<<5));  /* Bit 5, A9_CLKSTOP1  */
	Xil_Out32(XPS_SYS_CTRL_BASEADDR + 0x244, u32CpuRstCtrlReg);
	dbg_printf("Register (slcr) A9_CPU_RST_CTRL written to reset CPU 0: 0x%08x.\n\r", u32CpuRstCtrlReg );
	usleep ( 1 );


	/* ======================================== */
	/* Restore CPU1 program from 0x30000000 because data is modified. */
	app_install( );

	/* ==============================================  */
	/*
	 * Modify the reset code here or in boot.s.
	 * Make CPU1 check address at 0xffff fff0 after wake up
	 */
	//rst_stub_instruction_report( );
	rst_stub_install( );
	//rst_stub_instruction_report( );
	stub_adrs_instruction_install(	);
	//rst_adrs_instruction_report( );
	

	/* ======================================== */
	u32CpuRstCtrlReg &= (~(1<<1));  /* Bit 2, A9_RST1   */
	Xil_Out32(XPS_SYS_CTRL_BASEADDR + 0x244, u32CpuRstCtrlReg);
	dbg_printf("Register (slcr) A9_CPU_RST_CTRL written to un-reset CPU 0: 0x%08x.\n\r", u32CpuRstCtrlReg );
	usleep ( 1 );;    /* necessary */

	u32CpuRstCtrlReg &= ( ~((1<<1)|(1<<5)) );  /* Bit 5, A9_CLKSTOP1  */
	Xil_Out32(XPS_SYS_CTRL_BASEADDR + 0x244, u32CpuRstCtrlReg);
	dbg_printf("Register (slcr) A9_CPU_RST_CTRL written to un-reset CPU 0: 0x%08x.\n\r", u32CpuRstCtrlReg );
	usleep ( 1 );;  /* necessary */
	
	/* Lock the SLCR */
	SlcrLock();

	// Stub will clear CPU 1 start address in 0xfffffff0.
	// So set it again
	*puiCpu1StartAdrs = CPU1_APP_BASE_ADRS;
	Xil_DCacheFlushRange( (CPU1_APP_BASE_ADRS_POINTER&0xffffffe0), 32);

	//xil_printf("CPU0 reset CPU1, wait for 1 mili-seconds.\n\r");
	//usleep (  1000  );
	xil_printf("CPU0 wake up CPU1......");
	dsb();
	sev();
	xil_printf("Done.\n\r");
}



int task_dispatch_test( int i )
{
	int	j, k;
	volatile unsigned int *puiValidFlag1	= 	(volatile unsigned int *)TASK_FUNC_VALID_FLAG1_ADRS;
	volatile unsigned int *puiValidFlag2	=	(volatile unsigned int *)TASK_FUNC_VALID_FLAG2_ADRS;
	volatile unsigned int *puiTaskFuncPtr1	=	(volatile unsigned int *)TASK_FUNC_PTR1_ADRS;
	volatile unsigned int *puiTaskFuncPtr2	=	(volatile unsigned int *)TASK_FUNC_PTR2_ADRS;

	volatile unsigned int *puiTaskFuncRspsPtr1	=	(volatile unsigned int *)TASK_FUNC_PTR_RSPS_ADRS1;
	volatile unsigned int *puiTaskFuncRspsPtr2	=	(volatile unsigned int *)TASK_FUNC_PTR_RSPS_ADRS2;


	for( j=1; j<(TASK_FUNC_REPORT_INTERVAL*20); j++ )
	{
    	if( 0 == (j%TASK_FUNC_REPORT_INTERVAL) )
    	{
            xil_printf("No. %08d-%08d call to CPU1 from CPU0.\n\r", i, j);
    	}

        if( 0 == (j%2))
        {
        	*puiTaskFuncPtr1 = (unsigned int)task_func_print1;
    		*puiTaskFuncPtr2 = (unsigned int)task_func_print1;
        }
        else
        {
        	*puiTaskFuncPtr1 = (unsigned int)task_func_print2;
    		*puiTaskFuncPtr2 = (unsigned int)task_func_print2;
        }
       	*puiValidFlag1 = TASK_FUNC_VALID_FLAG1_VALUE;
    	*puiValidFlag2 = TASK_FUNC_VALID_FLAG2_VALUE;

    	/* Flush cache, not necessary if this memory section is non-cacheable or cacheable-writethrough*/
    	Xil_DCacheFlushRange(TASK_FUNC_HAND_SHAKE_BASE_ADRS, TASK_FUNC_CACHE_SIZE);

    	dsb();
    	sev();
		
    	for(k=1; k<TASK_FUNC_WAIT_MAX_TIME; k++ )
    	{
			/* Invalidate cache, not necessary if this memory section is non-cacheable */
        	Xil_DCacheInvalidateRange(TASK_FUNC_HAND_SHAKE_BASE_ADRS, TASK_FUNC_CACHE_SIZE);
			
        	if( ( 0 == (*puiValidFlag1) ) && ( 0 == (*puiValidFlag2) ) )
        	{
            	if( 0 == (j%TASK_FUNC_REPORT_INTERVAL) )
            	{
                    xil_printf("No. %08d-%08d call to CPU1 from CPU0 successed.\n\r", i, j);
            	}
                break;
        	}
        	else if( ( TASK_FUNC_VALID_FLAG1_VALUE == (*puiTaskFuncRspsPtr1) ) && ( TASK_FUNC_VALID_FLAG1_VALUE == (*puiTaskFuncRspsPtr2) ) )
        	{
        		*puiTaskFuncRspsPtr1 =0;
        		*puiTaskFuncRspsPtr2 =0;
            	usleep ( 200000  );
                xil_printf("No. %08d-%08d call to CPU1 from CPU0 has invalid function pointer.\n\r", i, j);
                usleep ( 200000  );
                break;
        	}
        	else if( ( TASK_FUNC_VALID_FLAG2_VALUE == (*puiTaskFuncRspsPtr1) ) && ( TASK_FUNC_VALID_FLAG2_VALUE == (*puiTaskFuncRspsPtr2) ) )
        	{
        		*puiTaskFuncRspsPtr1 =0;
        		*puiTaskFuncRspsPtr2 =0;
            	usleep ( 200000  );
        		xil_printf("No. %08d-%08d call to CPU1 from CPU0 has invalid flag.\n\r", i, j);
        		usleep ( 200000  );
                break;
        	}
        	else
        	{
            	if( 0 == (k%TASK_FUNC_WAIT_REPORT_INTERVAL) )
            	{
                    xil_printf("No. %08d wait for %08d-%08d call to CPU1 from CPU0.\n\r", k, i, j);
            	}
        	}
    	}
    	if( TASK_FUNC_WAIT_MAX_TIME == k )
    	{
            xil_printf("No. %08d-%08d call to CPU1 from CPU0 is time-out.\n\r", i, j);
    	}
	}

    return 0;
}


int main( void )
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
	Xil_SetTlbAttributes(TASK_FUNC_HAND_SHAKE_BASE_ADRS, 0x4de2);
	Xil_SetTlbAttributes(0xfff00000, 0x4de2);

	/* Invalidate cache, not necessary if this memory section is non-cacheable */
	Xil_DCacheInvalidateRange( CPU1_APP_BACKUP_BASE_ADRS, CPU1_APP_BACKUP_LEN);		// CPU 0


    for( i=1; ; i++ )
    {
		rst_cpu1( );
		usleep(2000);

	 	task_dispatch_test( i );
		sleep(5);

	}

    return 0;
}
