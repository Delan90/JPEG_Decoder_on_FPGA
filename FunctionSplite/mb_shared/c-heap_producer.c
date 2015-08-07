#include <hw_dma.h>
#include "c-heap.h"
#include "5kk03-utils.h"

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
		hw_dma_send_addr((void *)dest,
		                 (void *)d,
		                 1,
		                 (void *) adm->dma_address);
	} else {
		// Copy one word
		*(unsigned int *)dest = source;
	}
}

CHeapStatus cheap_init_fifo(volatile fifo_t* producer_adm,
                            volatile fifo_t * consumer_adm,
                            unsigned int token_size, unsigned int fifo_size,
                            unsigned int producer_data_address,
                            unsigned int consumer_data_address,
                            unsigned int producer_dma_address,
                            unsigned int producer_dma_data_address,
                            unsigned int consumer_dma_address,
                            unsigned int consumer_dma_data_address)
{
	// If we use DMA, every field should be specified.
	if(producer_dma_address > 0 || consumer_dma_address > 0) {
		if(producer_dma_address == 0 || consumer_dma_address == 0 ||
		        producer_dma_data_address == 0 || consumer_dma_data_address == 0) {
			return EXIT_ERROR;
		}
	}

	// Init the producer adm structure.
	producer_adm->writec = WRAP_RST_MASK;
	producer_adm->readc = WRAP_RST_MASK;
	producer_adm->token_size = token_size;
	producer_adm->fifo_size = fifo_size;
	producer_adm->data_address = producer_data_address;
	producer_adm->dma_address = producer_dma_address;
	producer_adm->dma_data_address = producer_dma_data_address;
	// Init the consumer adm structure.
	write_param(&(consumer_adm->writec), WRAP_RST_MASK, producer_adm);
	write_param(&(consumer_adm->readc), WRAP_RST_MASK, producer_adm);
	write_param(&(consumer_adm->token_size), token_size, producer_adm);
	write_param(&(consumer_adm->fifo_size), fifo_size, producer_adm);
	write_param(&(consumer_adm->data_address), consumer_data_address, producer_adm);
	write_param(&(consumer_adm->dma_address), consumer_dma_address, producer_adm);
	write_param(&(consumer_adm->dma_data_address),consumer_dma_data_address, producer_adm);
	return EXIT_OK;
}

void cheap_remote_write(volatile fifo_t *producer_adm, unsigned int write_pointer, void* data,
                        unsigned int token_size)
{
	if(producer_adm->dma_address > 0 && write_pointer >= REMOTE_BASE_ADDRESS) {
		unsigned int *d = (unsigned int *)(producer_adm->dma_data_address);

		// before writing new data in cmemout, check if dma has finished writing the previous.
		while(hw_dma_status_addr((void *)producer_adm->dma_address));

		// Copy the data into the communication memory
		memcpy_int(d, data, token_size/4);
		/* Use DMA  to send data.*/
		hw_dma_send_addr((void *)write_pointer, (void *)d, token_size/4,(void *)producer_adm->dma_address);

		// If data is in DDR, read back to make sure the data is actually written remotely.
		if(write_pointer >= REMOTE_SHARED_ADDRESS) {
			hw_dma_receive_addr((void *)d, (void *)write_pointer, token_size/4, (void *)producer_adm->dma_address);
		}
	} else {
		memcpy_int((void *)write_pointer, data, token_size/4);
	}
}

unsigned int cheap_claim_space(volatile fifo_t* adm)
{
	if((adm->writec & WRAP_SET_MASK) != (adm->readc & WRAP_SET_MASK) &&
	        (adm->writec & WR_RD_MASK) == (adm->readc & WR_RD_MASK)) {
		/* The FIFO is full. */
		return 0;
	} else {
		return adm->data_address + (adm->writec & WR_RD_MASK) *
		       adm->token_size;
	}
}

unsigned int cheap_is_empty(volatile fifo_t* adm)
{
	return (adm->writec & WRAP_SET_MASK) == (adm->readc & WRAP_SET_MASK) &&
	       (adm->writec & WR_RD_MASK) == (adm->readc & WR_RD_MASK);
}

void cheap_release_data(volatile fifo_t* producer_adm,
                        volatile fifo_t * consumer_adm)
{
	if((producer_adm->writec & WR_RD_MASK) == (producer_adm->fifo_size - 1)) {
		if((producer_adm->writec & WRAP_SET_MASK) == WRAP_SET_MASK) {
			producer_adm->writec = WRAP_RST_MASK;
			write_param(&(consumer_adm->writec), WRAP_RST_MASK, producer_adm);
		} else {
			producer_adm->writec = WRAP_SET_MASK;
			write_param(&(consumer_adm->writec), WRAP_SET_MASK, producer_adm);
		}
	} else {
		producer_adm->writec = producer_adm->writec + 1;
		write_param(&(consumer_adm->writec), producer_adm->writec, producer_adm);
	}
}

CHeapStatus cheap_write_fifo(volatile fifo_t* producer_adm,
                             volatile fifo_t * consumer_adm,
                             void* data, CHeapTransferType block)
{
	unsigned int write_pointer;

	do {
		write_pointer = cheap_claim_space(producer_adm);
	} while(block == BLOCK_IF_FULL && write_pointer == 0);

	if(write_pointer != 0) {
		cheap_remote_write(producer_adm, write_pointer, data, producer_adm->token_size);
		cheap_release_data(producer_adm, consumer_adm);
		return EXIT_OK;
	} else {
		return EXIT_ERROR;
	}
}
