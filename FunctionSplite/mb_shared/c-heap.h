#ifndef cheap_C_HEAP_H
#define cheap_C_HEAP_H

/**
 * The general structure for FIFO administration.
 */
typedef struct fifo_t {
	/* Write counter */
	unsigned int writec;
	/* Read counter */
	unsigned int readc;
	/* The size of the token, in bytes */
	unsigned int token_size;
	/* The size of the fifo, in tokens */
	unsigned int fifo_size;
	/* Address where the fifo data is stored. */
	unsigned int data_address;
	/* Base address of the DMA */
	unsigned int dma_address;
	/* A 1 token buffer needed to send pointer/counter updates */
	unsigned int dma_data_address;
} fifo_t;

/**
 * Symbolic names for the API.
 */
typedef enum {
	EXIT_ERROR = 0,
	EXIT_OK    = 1
} CHeapStatus;
/**
 * Transfer type.
 */
typedef enum {
	NO_BLOCK_IF_FULL = 0,
	BLOCK_IF_FULL = 1
} CHeapTransferType;

#define WRAP_SET_MASK			0x80000000
#define WRAP_RST_MASK			0x00000000
#define WR_RD_MASK				0x7FFFFFFF

#define REMOTE_BASE_ADDRESS 	0x10000000
#define REMOTE_SHARED_ADDRESS 	0x40000000

/************************************************************************
 *  Producer side functions for initialisation and writing to the FIFO. *
 ************************************************************************/

/**
 * Initialise a FIFO on both producer and consumer side. This is to be
 * called by the producer before commencing operation. Note that the
 * consumer must somehow synchronise to assert that it does not
 * attempt to use the FIFO before this function is called.
 *
 * \param producer_adm          	the local FIFO administration
 * \param consumer_adm          	the remote FIFO administration
 * \param token_size            	the token size in bytes
 * \param fifo_size             	the fifo size in tokens
 * \param producer_data_address 	the address used by the producer
 * \param consumer_data_address 	the address used by the consumer
 * \param producer_dma_address  	the dma address used by the producer (0 if no dma should be used)
 * \param producer_dma_data_address A memory space (1 token) the dma is allowed to read from, 0 when no dma is used.
 * \param consumer_dma_address  the dma address used by the consumer (0 if no dma should be used)
 * \param consumer_dma_data_address A memory space (1 token) the dma is allowed to read from, 0 when no dma is used.
 *
 * \returns CHeapStatus.
 */
extern CHeapStatus cheap_init_fifo(volatile fifo_t* producer_adm,
                                   volatile fifo_t * consumer_adm,
                                   unsigned int token_size, unsigned int fifo_size,
                                   unsigned int producer_data_address,
                                   unsigned int consumer_data_address,
                                   unsigned int producer_dma_address,
                                   unsigned int producer_dma_data_address,
                                   unsigned int consumer_dma_address,
                                   unsigned int consumer_dma_data_address
                                  );

/**
 * Write a token to the FIFO. This function is implemented using the
 * three following functions.
 *
 * \param producer_adm          the local FIFO administration
 * \param consumer_adm          the remote FIFO administration
 * \param data                  a pointer to a token in the local memory
 *                              of the producer
 * \param block                 BLOCK_IF_FULL/NO_BLOCK_IF_FULL
 *
 * \returns CHeapStatus.
 */
extern CHeapStatus cheap_write_fifo(volatile fifo_t* producer_adm,
                                    volatile fifo_t * consumer_adm,
                                    void* data, CHeapTransferType block);


/**
 * Claim space to write. Returns zero if the FIFO is full and a
 * pointer to the current write position if a token can be written.
 *
 * \param adm                   the local FIFO administration
 */
extern unsigned int cheap_claim_space(volatile fifo_t* adm);

/**
 * Copy to a remote write pointer what can be found localy where data
 * points to.
 *
 * \param write_pointer         the absolute address to write to
 * \param data                  pointer to the local data
 * \param token_size            the number of bytes to copy (must be a factor
 *                              of four for correct operation)
 */
/*
extern void cheap_remote_write(unsigned int write_pointer, void* data,
                                 unsigned int token_size);
*/
extern void cheap_remote_write(volatile fifo_t *producer_adm, unsigned int write_pointer, void* data,
                               unsigned int token_size);
/**
 * Update the FIFO administration, both locally and remote such that
 * data is made available to the consumer.
 *
 * \param producer_adm          the local FIFO administration
 * \param consumer_adm          the remote FIFO administration
 */
extern void cheap_release_data(volatile fifo_t* producer_adm,
                               volatile fifo_t *
                               consumer_adm);

/**
 * Check whether the producer percieves the FIFO as empty.
 *
 * \param adm                   the local FIFO administration
 */
unsigned int cheap_is_empty(volatile fifo_t* adm);


/************************************************************************
 *              Consumer side functions for reading the FIFO.           *
 ************************************************************************/

/**
 * Claim data, returns NULL if no data is available or a pointer to
 * the next token to be read.
 *
 * \param adm                   the local FIFO administration
 */
extern void* cheap_claim_data(volatile fifo_t* adm);

/**
 * Release space after consuming a token.
 *
 * \param consumer_adm          the local FIFO administration
 * \param producer_adm          the remote FIFO administration
 */
extern void cheap_release_space(volatile fifo_t* consumer_adm,
                                volatile fifo_t *
                                producer_adm);

/**
 * Read a token from the FIFO.
 *
 * \param consumer_adm          the local FIFO administration
 * \param producer_adm          the remote FIFO administration
 * \param data                  a pointer where to place the token in the
 *                              local memory of the consumer
 * \param block                 BLOCK_IF_FULL/NO_BLOCK_IF_FULL
 */
extern CHeapStatus cheap_read_fifo(volatile fifo_t* consumer_adm,
                                   volatile fifo_t * producer_adm,
                                   void* data,CHeapTransferType block);

#endif
