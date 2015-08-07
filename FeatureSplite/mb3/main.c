#include <comik.h>
#include <global_memmap.h>
#include <5kk03-utils.h>
#include "c-heap.h"
#include<jpeg.h>

cd_t comp[3];

/*[Start]Core2 -> Core3 Fifo Parameters*/
volatile fifo_t* producer = (fifo_t*) mb2_cmemin3_pt_REMOTEADDR;
volatile fifo_t* consumer = (fifo_t*) mb3_cmemin3_BASEADDR;
volatile int* fifo_sync = (int*) (mb3_cmemin2_BASEADDR + 1 * sizeof(fifo_t) + 64 * sizeof(int));
/*[End]Core2 -> Core3 Fifo Parameters*/

int main (void)
{

    // Sync with the monitor.
    mk_mon_sync();
    // Enable stack checking.
    start_stack_check();


    PBlock NewPBlk;
    int i,j;

    while (*fifo_sync == 0) {} // wait for fifo from Core1  to initialize

    //while(*fifo_sync == 1 || cheap_is_empty(consumer))
    while(*fifo_sync == 1)
    {
        int retval = cheap_read_fifo(consumer, producer, &NewPBlk, NO_BLOCK_IF_FULL);
        if(retval == EXIT_OK)
        {
#if 0
            for(j=0;j<sizeof(PBlock);j++)
                mk_mon_debug_info(NewPBlk[i].linear[j]);
#endif
            mk_mon_debug_info(0xAAAAAAAA);
            //mk_mon_debug_info(cheap_is_empty(consumer));
        }
    }


#if 0
    while (*Signal == 0x01)
    {
        if(flag % 2 == 0)
        {
            while(*(InComingData1) == 0 && *(Signal) == 0x01);

            /*First 4 bytes are not usefull*/
            offset = *InComingData1++;
            /*Perform Color Conversion*/

            //color_conversion(offset);

            //mk_mon_debug_info(0xEEEEEEEE);
            flag++;

        }
        else
        {
            while(*(InComingData2) == 0 && *(Signal) == 0x01);

            /*First 4 bytes are not usefull*/
            InComingData2++;

            //mk_mon_debug_info(0xFFFFFFFF);
            flag++;
            /*Perform Color Conversion*/
        }
    }
#endif

    //mk_mon_debug_info(cheap_is_empty(consumer));
    //mk_mon_debug_info(*fifo_sync);
    // Signal the monitor we are done.
    mk_mon_debug_tile_finished();
    return 0;
}
