#include <comik.h>
#include <global_memmap.h>
#include <5kk03-utils.h>
#include "c-heap.h"
#include<jpeg.h>
#include <hw_tifu.h>

/*[Start]Core2 -> Core3 Fifo Parameters*/
volatile fifo_t* producer = (fifo_t*) mb2_cmemin3_pt_REMOTEADDR;
volatile fifo_t* consumer = (fifo_t*) mb3_cmemin3_BASEADDR;
volatile int* fifo_sync = (int*) (mb3_cmemin2_BASEADDR + 1 * sizeof(fifo_t) + 64 * sizeof(int));
/*[End]Core2 -> Core3 Fifo Parameters*/
   

NewCmp Compnent[3];
int MCU_sx, MCU_sy;

#if 0
int main(void)
#else
int tile_main (void)
#endif
{

    TIME t1,t2,diff;
    *fifo_sync = 0;
#if 0
    /* Sync with the monitor. */
    mk_mon_sync();

    /* Enable stack checking. */
    start_stack_check();
#endif

    t1 = hw_tifu_systimer_get(); 

    volatile int *data = (int*)mb3_cmemin1_BASEADDR;

    int i = 0;
    volatile int row,col = 0;
    volatile int my_size, mx_size = 0;
    volatile int maxCmp = 0;
    PBlock NewPBlk[10];


    while(data[0] != 1 || data[9] !=1);

    /*Extract the Values*/
    MCU_sx = *(data+1);
    MCU_sy = *(data+2);
    mx_size = *(data+3);
    my_size = *(data+4);
    maxCmp = *(data+5);

    if(*(data+6) == 0xFFFFFFFF)
        Compnent[0].Val = 0;
    else
        Compnent[0].Val = *(data+6);

    if(*(data+7) == 0xFFFFFFFF)
        Compnent[1].Val = 0; 
    else
        Compnent[1].Val = *(data+7);

    if(*(data+8) == 0xFFFFFFFF)
        Compnent[2].Val = 0; 
    else
        Compnent[2].Val = *(data+8);

#if 0
    mk_mon_debug_info(MCU_sy);
    mk_mon_debug_info(MCU_sx);
    mk_mon_debug_info(mx_size);
    mk_mon_debug_info(my_size);
    mk_mon_debug_info(maxCmp);
    for(j=0;j<3;i++)
    {
        mk_mon_debug_info(Compnent[j].T.VDIV);
        mk_mon_debug_info(Compnent[j].T.HDIV);
        mk_mon_debug_info(Compnent[j].T.HS);
        mk_mon_debug_info(Compnent[j].T.IDX);
    }

    mk_mon_debug_info(data[1]);
    mk_mon_debug_info(data[2]);
    mk_mon_debug_info(data[3]);
    mk_mon_debug_info(data[4]);
    mk_mon_debug_info(data[5]);
    mk_mon_debug_info(data[6]);
    mk_mon_debug_info(data[7]);
    mk_mon_debug_info(data[8]);

#endif


    while (*fifo_sync == 0) {} // wait for fifo from Core1  to initialize

    while(*fifo_sync == 1)
    {
        int retval = cheap_read_fifo(consumer, producer, &NewPBlk[i], NO_BLOCK_IF_FULL);
        if(retval == EXIT_OK)
        {
            i++;
            if (col == mx_size && i == maxCmp) 
            {
                col = 0;
                row++;
                if (row == my_size) 
                {
                    /* in_frame = 0; */
                    mk_mon_debug_info(0xAAAAAAAA);
                    return 0;
                }
            }
#if 0
            for(j=0;j<sizeof(PBlock);j++)
                mk_mon_debug_info(NewPBlk[i].linear[j]);
#endif
            if(i == maxCmp)
            {
#if 0
                mk_mon_debug_info((row * MCU_sy << 10) + (col * MCU_sx));
                for(j=0;j < maxCmp;j++)
                    for(k=0;k<64;k++)
                        mk_mon_debug_info(NewPBlk[j].linear[k]);

#endif
                color_conversion((row * MCU_sy << 10) + (col * MCU_sx),NewPBlk,maxCmp);
                i=0;
                col++;
            }
#if 0
            mk_mon_debug_info(0xAAAAAAAA);
#endif
        }
    }


    t2 = hw_tifu_systimer_get(); 
    diff = t2 - t1;

    mk_mon_error(0XFDDE,LO_64(diff));

#if 0
    /* Signal the monitor we are done. */
    mk_mon_debug_tile_finished();
#endif

    return 0;
}
