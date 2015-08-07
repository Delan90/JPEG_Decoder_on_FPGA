#include <stdio.h>
#include <global_memmap.h>
//#include "5kk03.h"
#include "jpeg.h"
#include "comik.h"
#include "5kk03-utils.h"

//extern unsigned int input_buffer[JPGBUFFER_SIZE / sizeof(int)];
extern int vld_count;
int bit_counter = 0;
int refill_bit_counter = 0;
int buff_sel = 0;
extern unsigned int *cmem_input_circ_buff;
extern unsigned int *ddr_input;

unsigned int FGETC(volatile unsigned int *fi)
{
#ifdef INPUT_DMA
	unsigned int c = ((fi[(vld_count / 4)] << (8 * (vld_count % 4))) >> 24) & 0x00ff;
	vld_count = (vld_count + 1) & TOTAL_BLOCK_SIZE;
#else 
    unsigned int c = ((fi[vld_count / 4] << (8 * (vld_count % 4))) >> 24) & 0x00ff;
	vld_count++;
#endif
	
	bit_counter += 8;
    refill_bit_counter += 8;
	//mk_mon_debug_info(c);
	//mk_mon_debug_info(bit_counter);
	
	//if(bit_counter == 1024) suicide();
	
#ifdef INPUT_DMA
	if( refill_bit_counter == INPUT_READ_SIZE_BITS)
    {
	   refill_bit_counter = 0;
	   hw_dma_receive_addr((unsigned int*)(cmem_input_circ_buff + buff_sel * INPUT_READ_SIZE), (unsigned int*)(ddr_input), INPUT_READ_SIZE, (void*)mb1_dma0_BASEADDR);
	   ddr_input += INPUT_READ_SIZE;
	   buff_sel = CHANGE_BUFFER(buff_sel);
	}
#endif

	return c;
}

int FSEEK(int offset, int start)
{
	vld_count += offset + (start - start);	/* Just to use start... */
	return 0;
}

int FTELL()
{
	return vld_count;
}
