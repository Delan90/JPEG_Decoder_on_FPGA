#include <hw_dma.h>
#include "c-heap.h"

/**
 * NoC safe memcpy (that works on words!!!)
 * \param dst The destination address
 * \param src The source address
 * \param w_cnt The number of words to transfer.
 * THIS IS DIFFERENT FROM MEMCPY THAT TAKES BYTES
 */
static inline void memcpy_int(unsigned int * dst,volatile unsigned int * src, int w_cnt)
{
	int i;

	if(dst == src) return;

	for(i = 0; i < w_cnt; i ++) dst[i] = ((volatile int *)src)[i];
}

/**
 * Small wrapper function that will update a field in the remote ADM using the right
 * method (mmio or dma)
 */
static inline void write_param(volatile void *dest, unsigned int source,volatile fifo_t* adm)
{
	if(adm->dma_address > 0) {
		unsigned int *d = (unsigned int *)(adm->dma_data_address);

		// before writing new data in cmemout, check if dma has finished writing previous.
		while(hw_dma_status_addr((void *)adm->dma_address));

		*d = source;
		hw_dma_send_addr(
		    (void *)dest,
		    (void *) d,
		    1, // Write one integer
		    (void *) adm->dma_address);
	} else {
		// Copy one word
		*(unsigned int *)dest = source;
	}
}

void* cheap_claim_data(volatile fifo_t* adm)
{
	if((adm->writec & WRAP_SET_MASK) == (adm->readc & WRAP_SET_MASK) &&
	        (adm->writec & WR_RD_MASK) == (adm->readc & WR_RD_MASK)) {
		/* The FIFO is empty. */
		return NULL;
	} else {
		return (void*)(adm->data_address + (adm->readc & WR_RD_MASK) *
		               adm->token_size);
	}
}

void cheap_release_space(volatile fifo_t* consumer_adm,
                         volatile fifo_t * producer_adm)
{
	if((consumer_adm->readc & WR_RD_MASK) == (consumer_adm->fifo_size - 1)) {
		if((consumer_adm->readc & WRAP_SET_MASK) == WRAP_SET_MASK) {
			consumer_adm->readc = WRAP_RST_MASK;
		} else {
			consumer_adm->readc = WRAP_SET_MASK;
		}
	} else {
		consumer_adm->readc = consumer_adm->readc + 1;
	}

	write_param(&(producer_adm->readc), consumer_adm->readc, consumer_adm);
}

CHeapStatus cheap_read_fifo(volatile fifo_t* consumer_adm,
                            volatile fifo_t * producer_adm,
                            void* data, CHeapTransferType block)
{
	volatile unsigned int* read_pointer;

	do {
		read_pointer = cheap_claim_data(consumer_adm);
	} while(block == BLOCK_IF_FULL && read_pointer == NULL);

	if(read_pointer != NULL) {
		// check if data is located in a remote memory. use dmas then if available.
		if((unsigned int)read_pointer >= REMOTE_BASE_ADDRESS &&
		        consumer_adm->dma_address > 0) {
			volatile unsigned int *d = (unsigned int *)(consumer_adm->dma_data_address);

			// before writing new data in cmemout,
			// check if dma has finished writing previous.
			while(hw_dma_status_addr((void *)consumer_adm->dma_address));

			hw_dma_receive_addr((void *)d,
			                    (void *)read_pointer,
			                    (consumer_adm->token_size)/4,
			                    (void *)consumer_adm->dma_address);
			// copy it to the passed memory.
			memcpy_int(data, d, (consumer_adm->token_size)/4);
		} else {
			memcpy_int(data, read_pointer, (consumer_adm->token_size)/4);
		}

		cheap_release_space(consumer_adm, producer_adm);
		return EXIT_OK;
	} else {
		return EXIT_ERROR;
	}
}
