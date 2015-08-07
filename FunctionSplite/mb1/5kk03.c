#include <global_memmap.h>
#include "jpeg.h"
#include "comik.h"
#include "5kk03-utils.h"


extern int vld_count;
int bit_counter = 0;
int refill_bit_counter = 0;
extern int buff_sel;
extern unsigned int *cmem_input_circ_buff;
extern unsigned int *ddr_input;
int ddr_vld_cnt = 0;
extern int ddr_input_chunck_offset;

unsigned char FGETC(unsigned char *fi)
{
#ifdef INPUT_DMA
	unsigned char c = fi[vld_count];
	vld_count = (vld_count + 1) & TOTAL_BLOCK_SIZE;
	ddr_vld_cnt++;
#else 
    unsigned int c = ((fi[vld_count / 4] << (8 * (vld_count % 4))) >> 24) & 0x00ff;
	vld_count++;
#endif
	
	bit_counter += 8;
    refill_bit_counter += 8;
	
#ifdef INPUT_DMA
	if( refill_bit_counter >= INPUT_READ_SIZE_BITS)
    {
	   refill_bit_counter = 0;
	   while(hw_dma_status_addr( (void *) mb1_dma0_BASEADDR));
	   hw_dma_receive_addr((unsigned int*)(cmem_input_circ_buff + buff_sel * INPUT_READ_SIZE_INT), (void*)(&ddr_input[ddr_input_chunck_offset*INPUT_READ_SIZE_INT]), INPUT_READ_SIZE_INT, (void*)mb1_dma0_BASEADDR);
	   ddr_input_chunck_offset++;
	   //ddr_input = (int) (ddr_input + INPUT_READ_SIZE_INT);
	   buff_sel = CHANGE_BUFFER(buff_sel);
	}
#endif

	return c;
}

int FSEEK(int offset, int start)
{
#ifdef INPUT_DMA	
	vld_count += offset;
	ddr_vld_cnt += offset;
	refill_bit_counter += (offset << 3);
	if( refill_bit_counter >= INPUT_READ_SIZE_BITS)
    {
	   int addr;
	   refill_bit_counter = ((offset & 3) << 3);
	   ddr_input_chunck_offset = 0;
	   buff_sel = 0;
	   addr = ddr_vld_cnt >> 2 ;
	   while(hw_dma_status_addr( (void *) mb1_dma0_BASEADDR));
	   hw_dma_receive_addr((unsigned int*)(cmem_input_circ_buff + buff_sel * INPUT_READ_SIZE_INT), (void*)(&ddr_input[addr]), INPUT_READ_SIZE_INT, (void*)mb1_dma0_BASEADDR);
	   ddr_input = (unsigned int *) (ddr_input + addr + ddr_input_chunck_offset * INPUT_READ_SIZE_INT);
	   ddr_input_chunck_offset++;
	   vld_count = (buff_sel * INPUT_READ_SIZE) + (offset & 3);
	   buff_sel = CHANGE_BUFFER(buff_sel);
    }
#else
	vld_count += offset + (start - start);	/* Just to use start... */
#endif	
	return 0;
}

int FTELL()
{
	return vld_count;
}
