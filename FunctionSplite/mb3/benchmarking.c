#include <comik.h>
#include <global_memmap.h>
#include <5kk03-utils.h>

/* 1. Benchmarking includes and global variables */
#include "hw_tifu.h"
#include "benchmark.h" // Include benchmarking header file
void core3_function();
void bench_mark_fill_pattern_to_cmem();
int bench_mark_measure_cmem_usage();

int DMA_flag = 0;
int bench_dyna_mem_size = 0;
/* End of benchmarking code outside main */

int main (void)
{
	// Sync with the monitor.
	mk_mon_sync();
	// Enable stack checking.
	start_stack_check();
	
	/* 2. Benchmarking stuff (to measure usage of communication memory) */
	{
	    bench_mark_fill_pattern_to_cmem();
	}
	/* End of Benchmarking stuff before starting decoding */
	
	/* 2. Only benchmark related shared variables, (also includes variables for receiving start and sending stop signal) */
	volatile unsigned int *mb3_end = (unsigned int*)(mb3_cmemout0_BASEADDR + mb3_cmemout0_SIZE - 2*sizeof(unsigned int));
	volatile unsigned int *mb3_start = (unsigned int*)(mb3_cmemin0_BASEADDR + mb3_cmemin0_SIZE - sizeof(unsigned int));	
	int num_of_dma = 0; 
	/* End of sync variables */
	
	/* 3. Wait for start signal from MB1 */
	*mb3_start = 0;
	while(*((volatile int*)mb3_start) != 1);
	
	/* 4. All code pertaining to jpeg decoder below this line */
	{
	   /* Call your application function here */   
	   tile_main();
	}
	/* End of the jpeg decoder function -*"No code pertaining to decoder beyond this line apart from sending end of job flags to MB1 and benchmarking stuff "*- */
	
	/* 5. Signal MB1 that work is finished in MB3 */
	*mb3_end = 1;
	DMA_SEND_BLOCKING((unsigned int*)(mb1_cmemin3_pt_REMOTEADDR + mb1_cmemin3_SIZE - 2*sizeof(unsigned int)), mb3_end, 1, (void *)(mb3_dma1_BASEADDR),DMA_flag); //core 3 finish flag
	
	/* 11. Code to compute number of DMA channels used in MB1 */
	switch(DMA_flag)
	{
	   case(0X00) : num_of_dma = 0; break;
	   case(0X0F) : num_of_dma = 1; break;
	   case(0XF0) : num_of_dma = 1; break;
	   case(0XFF) : num_of_dma = 2; break;
	   default: num_of_dma = -1;
	}
	
	/* 6. Benchmarking display code starts here */ 
	mk_mon_error(0XB2, num_of_dma);  // Displays the total number of DMA send and receive calls performed in MB3
	mk_mon_error(0XB3, bench_mark_measure_cmem_usage()); // Displays the size of communication memory used by MB3
    mk_mon_error(0XB4, bench_dyna_mem_size); // Displays the size of dynamic memory used by MB3
	
	mk_mon_debug_tile_finished();
	return 0;
}

/* 7. Functions to calculate usage of communication memory */
void bench_mark_fill_pattern_to_cmem()
{
  int i;
  unsigned char *cmem_in0 = (unsigned char*) (mb3_cmemin0_BASEADDR);
  unsigned char *cmem_in1 = (unsigned char*) (mb3_cmemin1_BASEADDR);
  unsigned char *cmem_in2 = (unsigned char*) (mb3_cmemin2_BASEADDR);
  unsigned char *cmem_in3 = (unsigned char*) (mb3_cmemin3_BASEADDR);
  
  unsigned char *cmem_out0 = (unsigned char*) (mb3_cmemout0_BASEADDR);
  unsigned char *cmem_out1 = (unsigned char*) (mb3_cmemout1_BASEADDR);
  
  for( i = 0 ; i < CMEM_IN_SIZE; i++)
  {
    cmem_in0[i] = PATTERN3;
	cmem_in1[i] = PATTERN3;
	cmem_in2[i] = PATTERN3;
	cmem_in3[i] = PATTERN3;
  }
    
  for( i = 0 ; i < CMEM_OUT_SIZE; i++)
  {
    cmem_out0[i] = PATTERN3;
	cmem_out1[i] = PATTERN3;
  }
  
}

int bench_mark_measure_cmem_usage()
{
  int i;
  int comm_mem_size; 
  
  unsigned char *cmem_in0 = (unsigned char*) (mb3_cmemin0_BASEADDR);
  unsigned char *cmem_in1 = (unsigned char*) (mb3_cmemin1_BASEADDR);
  unsigned char *cmem_in2 = (unsigned char*) (mb3_cmemin2_BASEADDR);
  unsigned char *cmem_in3 = (unsigned char*) (mb3_cmemin3_BASEADDR);
  
  unsigned char *cmem_out0 = (unsigned char*) (mb3_cmemout0_BASEADDR);
  unsigned char *cmem_out1 = (unsigned char*) (mb3_cmemout1_BASEADDR);
  
  comm_mem_size = CMEM_IN_SIZE  * 4 + CMEM_OUT_SIZE * 2 ;
  
  for( i = 0 ; i < CMEM_IN_SIZE; i++)
  {
    comm_mem_size -= (cmem_in0[i] != PATTERN3) ? 1 : 0;
	comm_mem_size -= (cmem_in1[i] != PATTERN3) ? 1 : 0;
	comm_mem_size -= (cmem_in2[i] != PATTERN3) ? 1 : 0;
	comm_mem_size -= (cmem_in3[i] != PATTERN3) ? 1 : 0;
  }
    
  for( i = 0 ; i < CMEM_OUT_SIZE; i++)
  {
    comm_mem_size -= (cmem_out0[i] != PATTERN3) ? 1 : 0;
	comm_mem_size -= (cmem_out1[i] != PATTERN3) ? 1 : 0;
  }
  
  return(((CMEM_IN_SIZE  * 4 + CMEM_OUT_SIZE * 2) - comm_mem_size));
}
/* End of functions to calculate usage of communication memory */
