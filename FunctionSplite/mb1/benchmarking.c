#include <comik.h>
#include <global_memmap.h>
#include <5kk03-utils.h>
#include <hw_dma.h>

/* 1. Benchmarking includes and global variables */
#include "hw_tifu.h"
#include "benchmark.h" // Include benchmarking header file
void core1_function();
void bench_mark_fill_pattern_to_cmem();
unsigned int bench_mark_measure_cmem_usage();
void bench_mark_fill_pattern_to_dram();
unsigned int bench_mark_measure_dram_usage();

int DMA_flag = 0;
int bench_dyna_mem_size = 0;
/* End of benchmarking code outside main */


int main (void)
{
	// Sync with the monitor.
	mk_mon_sync();

	// Enable stack checking.
	start_stack_check();
	
	/* 1. Benchmarking stuff (to measure usage of DDR memory) */
	{
	    bench_mark_fill_pattern_to_dram();   
	}
	/* 2. Benchmarking stuff (to measure usage of communication memory) */
	{
	    bench_mark_fill_pattern_to_cmem();
	}
	/* End of Benchmarking stuff before starting decoding */
	
	/* 3. Only benchmark related shared variables, (also includes MB2 and MB3 start and stop signals) */
	TIME start_time_of_decoding,end_time_of_decoding;
	int num_of_dma = 0; 
	
	volatile unsigned int* mb2_end = (unsigned int*)(mb1_cmemin0_BASEADDR + mb1_cmemin0_SIZE - 2*sizeof(unsigned int));	
	volatile unsigned int* mb3_end = (unsigned int*)(mb1_cmemin3_BASEADDR + mb1_cmemin3_SIZE - 2*sizeof(unsigned int));
	volatile unsigned int* mb2_start = (unsigned int*)(mb1_cmemout0_BASEADDR + mb1_cmemout0_SIZE - sizeof(unsigned int)); //Core 2 start flag
	volatile unsigned int* mb3_start = (unsigned int*)(mb1_cmemout0_BASEADDR + mb1_cmemout0_SIZE - 2*sizeof(unsigned int)); //Core 3 start flag
	/* End of sync variables */
	
	/* 4. Initialisation of the start and end signals for inter core communication */
	*mb2_end = 0; *mb3_end = 0;
	*mb2_start = 1; *mb3_start = 1;
	
	/* 5. Please write the part of the code which does initial clearing of display frame within the following braces */
	{
	   clear_frame_buffer();
	}
	
	/* 6. Log start time of the decoder (start the timer), i.e. before signalling MB2 and MB3 to start*/
	start_time_of_decoding = hw_tifu_systimer_get();
	
	/* 7. Sending start to MB2 and MB3 */
	//start core2
	DMA_SEND_BLOCKING((unsigned int*)(mb2_cmemin0_pt_REMOTEADDR + mb2_cmemin0_SIZE - sizeof(unsigned int)), mb2_start, 1, (void *)(mb1_dma0_BASEADDR),DMA_flag);
	//start core3
	DMA_SEND_BLOCKING((unsigned int*)(mb3_cmemin0_pt_REMOTEADDR + mb3_cmemin0_SIZE - sizeof(unsigned int)), mb3_start, 1, (void *)(mb1_dma0_BASEADDR),DMA_flag);
	
	/* 8. Please call your jpeg decoder function within the braces below */
	{ 
	   /* Call your application function here */   
	   tile_main();
	   
	}	 
	/* End of the jpeg decoder function -*"No code pertaining to decoder beyond this 
	line apart from receiving sync flags from MB2 and MB3 and benchmarking code "*- */
	
	/* 9. Wait for the end of job flags to be sent from both MB2 and MB3 */
	while((*((volatile int*)mb2_end) != 1) && (*((volatile int*)mb3_end) != 1));
	
	/* 10. Capture finish time */
	end_time_of_decoding = hw_tifu_systimer_get(); // All cores are finished (capture the time)
	
	/* 11. Code to compute number of DMA channels used in MB1 */
	switch(DMA_flag)
	{
	   case(0X00) : num_of_dma = 0; break;
	   case(0X0F) : num_of_dma = 1; break;
	   case(0XF0) : num_of_dma = 1; break;
	   case(0XFF) : num_of_dma = 2; break;
	   default: num_of_dma = -1;
	}
	
	/* 11. Benchmarking display code starts here */
	mk_mon_error(0XB1,(end_time_of_decoding - start_time_of_decoding)); // Calculate total execution cycles taken by the decoder;
	mk_mon_error(0XB2, num_of_dma);  // Displays the total number of DMA send and receive calls performed in MB1
	mk_mon_error(0XB3, bench_mark_measure_cmem_usage()); // Displays the size of communication memory used by MB1
	mk_mon_error(0XB4, bench_dyna_mem_size); // Displays the size of dynamic memory used by MB1
    mk_mon_error(0XB5, bench_mark_measure_dram_usage()); // Displays the size of the DRAM
	
	/* 12. End of tile construct */
	mk_mon_debug_tile_finished();
	return 0;
}

/* 13. Functions to calculate usage of communication memory */
void bench_mark_fill_pattern_to_cmem()
{
  int i;
 unsigned char *cmem_in0 = (unsigned char*) (mb1_cmemin0_BASEADDR);
 unsigned char *cmem_in1 = (unsigned char*) (mb1_cmemin1_BASEADDR);
 unsigned char *cmem_in2 = (unsigned char*) (mb1_cmemin2_BASEADDR);
 unsigned char *cmem_in3 = (unsigned char*) (mb1_cmemin3_BASEADDR);
  
 unsigned char *cmem_out0 = (unsigned char*) (mb1_cmemout0_BASEADDR);
 unsigned char *cmem_out1 = (unsigned char*) (mb1_cmemout1_BASEADDR);
  
  for( i = 0 ; i < CMEM_IN_SIZE; i++)
  {
    cmem_in0[i] = PATTERN1;
	cmem_in1[i] = PATTERN1;
	cmem_in2[i] = PATTERN1;
	cmem_in3[i] = PATTERN1;
  }
    
  for( i = 0 ; i < CMEM_OUT_SIZE; i++)
  {
    cmem_out0[i] = PATTERN1;
	cmem_out1[i] = PATTERN1;
  }
  
}
unsigned int bench_mark_measure_cmem_usage()
{
  int i;
  int comm_mem_size; 
  
  unsigned char *cmem_in0 = (unsigned char*) (mb1_cmemin0_BASEADDR);
  unsigned char *cmem_in1 = (unsigned char*) (mb1_cmemin1_BASEADDR);
  unsigned char *cmem_in2 = (unsigned char*) (mb1_cmemin2_BASEADDR);
  unsigned char *cmem_in3 = (unsigned char*) (mb1_cmemin3_BASEADDR);
  
   unsigned char *cmem_out0 = (unsigned char*) (mb1_cmemout0_BASEADDR);
  unsigned char *cmem_out1 = (unsigned char*) (mb1_cmemout1_BASEADDR);
  
  comm_mem_size = CMEM_IN_SIZE  * 4 + CMEM_OUT_SIZE * 2 ;
  
  for( i = 0 ; i < CMEM_IN_SIZE; i++)
  {
    comm_mem_size -= (cmem_in0[i] != PATTERN1) ? 1 : 0;
	comm_mem_size -= (cmem_in1[i] != PATTERN1) ? 1 : 0;
	comm_mem_size -= (cmem_in2[i] != PATTERN1) ? 1 : 0;
	comm_mem_size -= (cmem_in3[i] != PATTERN1) ? 1 : 0;
  }
    
  for( i = 0 ; i < CMEM_OUT_SIZE; i++)
  {
    comm_mem_size -= (cmem_out0[i] != PATTERN1) ? 1 : 0;
	comm_mem_size -= (cmem_out1[i] != PATTERN1) ? 1 : 0;
  }
  
  return(((CMEM_IN_SIZE  * 4 + CMEM_OUT_SIZE * 2) - comm_mem_size));
}
/* End of functions to calculate usage of communication memory */

/* 13. Functions to calculate usage of DRAM memory */
void bench_mark_fill_pattern_to_dram() 
{
	int i;
	int *DDR = (int*)(mb1_cmemout0_BASEADDR);
	
	for(i = 0; i < 1024; i++) *(DDR + i) = 0xEFFFFFFE;
	for(i = 0; i < 1024; i++){
		while(hw_dma_status_addr(((void*)mb1_dma0_BASEADDR)));
		hw_dma_send_non_block_addr((unsigned int*)(DDR_pt_REMOTEADDR  + i*4*1024), (int*)DDR, 1024, (void*)mb1_dma0_BASEADDR);
	}
}

unsigned int bench_mark_measure_dram_usage()
{
	int i = 0,j, DDRusage = 0;
	int *DDR = (int*)(mb1_cmemout0_BASEADDR);
	
	for(i = 0; i < 1024 ; i++) {
		hw_dma_receive_addr((void*)DDR, (unsigned int*)(DDR_pt_REMOTEADDR+i*4*1024),1024,(void*)mb1_dma0_BASEADDR);
		for(j = 0; j < 1024; j++){
			if(*(DDR+j) != 0xEFFFFFFE) DDRusage+=4; 
		}
	}
	return(DDRusage);
}
/* End of functions to calculate usage of DRAM memory */

