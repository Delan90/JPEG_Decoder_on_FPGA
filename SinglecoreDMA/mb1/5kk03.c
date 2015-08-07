#include <stdio.h>

#include "5kk03.h"
#include "jpeg.h"
#include <global_memmap.h>

//extern unsigned int input_buffer[JPGBUFFER_SIZE / sizeof(int)];
extern int vld_count;

unsigned int FGETC(volatile unsigned int *fi)
{
    unsigned int c = ((fi[vld_count / 4] << (8 * (vld_count % 4))) >> 24) & 0x00ff;
    vld_count++;

#if 0
    if(vld_count == 480)
    {
       vld_count = 0;

       hw_dma_receive_addr((int*)(mb1_cmemout0_BASEADDR), (volatile unsigned int *)(shared_pt_REMOTEADDR+1024*1024*4+vld_count),
                     120, (void*)mb1_dma0_BASEADDR);

       while(hw_dma_status_addr((void*)mb1_dma0_BASEADDR)) {}

       mk_mon_debug_info(((fi[480 / 4] << (8 * (vld_count % 4))) >> 24) & 0x00ff);
       
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
