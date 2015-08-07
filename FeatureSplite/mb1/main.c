#include <comik.h>
#include <global_memmap.h>
#include <5kk03-utils.h>
#include "c-heap.h"
#include "jpeg.h"
#include <hw_tifu.h>


/* global variables related to the C-HEAP FIFO */
volatile fifo_t* producer = (fifo_t*) mb1_cmemin1_BASEADDR;
volatile fifo_t* consumer = (fifo_t*) mb2_cmemin1_pt_REMOTEADDR;
volatile int* producer_data = (int*) (mb2_cmemin1_pt_REMOTEADDR + 1 * sizeof(fifo_t));
volatile int* consumer_data = (int*) (mb2_cmemin1_BASEADDR + 1 * sizeof(fifo_t));

/* synchronization variable that indicates that C-HEAP FIFO is initialized. */
volatile int* fifo_sync = (int*) (mb2_cmemin1_pt_REMOTEADDR + 1 * sizeof(fifo_t) + 64 * sizeof(int));
volatile int* fifo_sync_data = (int*) (mb1_cmemout1_BASEADDR + 1 * sizeof(fifo_t) + 64 * sizeof(int));

extern int jpeg_dec_main();


int main (void)
{
	// Sync with the monitor.
	mk_mon_sync();
	TIME t1,t2,diff;
	
	// Enable stack checking.
	start_stack_check();
	
	clear_frame_buffer();
	
	t1 = hw_tifu_systimer_get(); 
	
    /*Initalize the C-Heap communication*/

    /* FBlock is used as token for communication between Core1 & Core2 */
    unsigned int token_size = sizeof(FBlock);

    /* FIFO can store 6 tokens. */
    unsigned int fifo_size = 1;

    /*Init FiFo*/
    cheap_init_fifo(producer, consumer, token_size, fifo_size, (unsigned int)producer_data, (unsigned int) consumer_data,
            mb1_dma1_BASEADDR, mb1_cmemout1_BASEADDR,
            mb2_dma1_BASEADDR, mb2_cmemout1_BASEADDR);


    /* signal to core 2 that the FIFO is initialized and can be read from. */
    *fifo_sync_data = 1;
    hw_dma_send_addr((void *)fifo_sync, (int*)fifo_sync_data, sizeof(int),(void *)mb1_dma1_BASEADDR);
	jpeg_dec_main();
	t2 = hw_tifu_systimer_get(); 
	
	diff = t2 - t1;
	
	mk_mon_debug_info(LO_64(diff));
    mk_mon_debug_info(0XFDDE);
	// Signal the monitor we are done.
	mk_mon_debug_tile_finished();
	return 0;
}
