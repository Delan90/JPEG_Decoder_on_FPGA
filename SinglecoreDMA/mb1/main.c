#include "comik.h"
#include "global_memmap.h"
#include "5kk03-utils.h"
//#include "jpeg.h"
//#include "surfer.jpg.h"

extern int jpeg_dec_main();

int main (void)
{
	// Sync with the monitor.
	mk_mon_sync();
	// Enable stack checking.
	start_stack_check();

    jpeg_dec_main();
	
	
	// Signal the monitor we are done.
	mk_mon_debug_tile_finished();
	return 0;
}
