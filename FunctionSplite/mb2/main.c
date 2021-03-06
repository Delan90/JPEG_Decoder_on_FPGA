#include <comik.h>
#include <global_memmap.h>
#include <5kk03-utils.h>
#include "c-heap.h"
#include "benchmark.h"
#include<jpeg.h>
#include <hw_tifu.h>

/*[Start]Core1 -> Core 2 Fifo Parameters*/
volatile fifo_t* producer = (fifo_t*) mb1_cmemin1_pt_REMOTEADDR;
volatile fifo_t* consumer = (fifo_t*) mb2_cmemin1_BASEADDR;
volatile int* fifo_sync = (int*) (mb2_cmemin0_BASEADDR + 1 * sizeof(fifo_t) + 64 * sizeof(int));
/*[End]Core1 -> Core 2 Fifo Parameters*/


/*[Start]Core2 -> Core 3 Fifo Parameters*/
volatile fifo_t* producer1 = (fifo_t*) mb2_cmemin3_BASEADDR;
volatile fifo_t* consumer1 = (fifo_t*) mb3_cmemin3_pt_REMOTEADDR;
volatile int* producer_data1 = (int*) (mb3_cmemin3_pt_REMOTEADDR + 1 * sizeof(fifo_t));
volatile int* consumer_data1 = (int*) (mb3_cmemin3_BASEADDR + 1 * sizeof(fifo_t));

 volatile int* fifo_sync1 = (int*) (mb3_cmemin2_pt_REMOTEADDR + 1 * sizeof(fifo_t) + 64 * sizeof(int));
 volatile int* fifo_sync_data1 = (int*) (mb2_cmemout0_BASEADDR + 1 * sizeof(fifo_t) + 64 * sizeof(int));
/*[End]Core2 -> Core 3 Fifo Parameters*/

extern int DMA_flag; 

#if 0
 int main(void)
#else
int tile_main (void)
#endif
{
    FBlock NewFBlk;
    PBlock NewPBlk;
    TIME t1,t2,diff;
    *fifo_sync = 0;

#if 0
    /* Sync with the monitor. */
    mk_mon_sync();

    /* Enable stack checking. */
    start_stack_check();
#endif

    t1 = hw_tifu_systimer_get(); 

    /* we used integers as tokens in this example*/
    unsigned int token_size = sizeof(PBlock);

    /* the FIFO is sized to fit 4 tokens */
    unsigned int fifo_size=20;

    cheap_init_fifo(producer1, consumer1, token_size, fifo_size, (unsigned int)producer_data1, (unsigned int) consumer_data1,
            mb2_dma1_BASEADDR, mb2_cmemout1_BASEADDR,
            mb3_dma1_BASEADDR, mb3_cmemout1_BASEADDR);

    /* signal to core 3 that the FIFO is initialized and can be read from.*/
    *fifo_sync_data1 = 1;
    DMA_SEND_BLOCKING((void*)fifo_sync1, (int*)fifo_sync_data1, sizeof(int),(void *)mb2_dma0_BASEADDR,DMA_flag);

    while (*fifo_sync == 0) {} // wait for fifo from Core1  to initialize

    while(*fifo_sync == 1)
    {
        //cheap_read_fifo(consumer, producer, &NewFBlk, *fifo_sync ? BLOCK_IF_FULL:NO_BLOCK_IF_FULL);
        int retval = cheap_read_fifo(consumer, producer, &NewFBlk, NO_BLOCK_IF_FULL);
        if(EXIT_OK == retval)
        {
            IDCT(&NewFBlk,&NewPBlk);
#if 0
            mk_mon_debug_info(0xBBBBBBBB);
#endif
            cheap_write_fifo(producer1, consumer1, &NewPBlk, BLOCK_IF_FULL);
        }
    }

    while(cheap_is_empty(producer1) != 1)
    {
#if 0
        mk_mon_debug_info(producer1->readc);
        mk_mon_debug_info(producer1->writec);
#endif

    }

    /* signal to core 3 that the FIFO is initialized and can be read from.*/
    *fifo_sync_data1 = 0;
    DMA_SEND_BLOCKING((void*)fifo_sync1, (int*)fifo_sync_data1, sizeof(int),(void *)mb2_dma0_BASEADDR,DMA_flag);

    t2 = hw_tifu_systimer_get(); 
    diff = t2 - t1;

    mk_mon_error(0XFDDE,LO_64(diff));

#if 0
    /* Signal the monitor we are done. */
    mk_mon_debug_tile_finished();
#endif
    return 0;
}
