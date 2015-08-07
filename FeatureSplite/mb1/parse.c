/*-----------------------------------------*/
/* File : parse.c, utilities for jfif view */
/* Author : Pierre Guerrier, march 1998	   */
/*-----------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "c-heap.h"
#include "jpeg.h"
#include <comik.h>
#include <5kk03-utils.h>

/*---------------------------------------------------------------------*/

/* utility and counter to return the number of bits from file */
/* right aligned, masked, first bit towards MSB's		*/

static unsigned char bit_count;	/* available bits in the window */
static unsigned char window;

#ifdef FILE_IO
unsigned long get_bits(FILE * fi, int number)
#else
unsigned long get_bits(volatile unsigned int * fi, int number)
#endif
{
	int i, newbit;
	unsigned long result = 0;
	unsigned char aux, wwindow;

	if (!number)
		return 0;

	for (i = 0; i < number; i++) 
    {
        if (bit_count == 0) 
        {
#ifdef FILE_IO
            wwindow = fgetc(fi);
#else
            wwindow = FGETC(fi);
#endif

            if (wwindow == 0xFF)
#ifdef FILE_IO
                switch (aux = fgetc(fi)) 
#else
                switch (aux = FGETC(fi)) 
#endif
                {	/* skip stuffer 0 byte */
                    case 0xFF:
#ifdef FILE_IO
                        fprintf(stderr, "%ld:\tERROR:\tRan out of bit stream\n", ftell(fi));
#else
                        //printf("%d:\tERROR:\tRan out of bit stream\n", FTELL());
#endif
                        aborted_stream(fi);
                        break;

                    case 0x00:
                        stuffers++;
                        break;

                    default:
                        if (RST_MK(0xFF00 | aux))
                        {
#ifdef FILE_IO
                            fprintf(stderr, "%ld:\tERROR:\tSpontaneously found restart!\n", ftell(fi));
#else
                            //printf("%d:\tERROR:\tSpontaneously found restart!\n", FTELL());
#endif

                        }
#ifdef FILE_IO
                        fprintf(stderr, "%ld:\tERROR:\tLost sync in bit stream\n", ftell(fi));
#else
                        //printf("%d:\tERROR:\tLost sync in bit stream\n", FTELL());
#endif
                        aborted_stream(fi);
                        break;
                }

            bit_count = 8;
        } else
            wwindow = window;
        newbit = (wwindow >> 7) & 1;
        window = wwindow << 1;
        bit_count--;
        result = (result << 1) | newbit;
    }
	return result;
}

void clear_bits(void)
{
	bit_count = 0;
}

#ifdef FILE_IO
unsigned char get_one_bit(FILE * fi)
#else
unsigned char get_one_bit(volatile unsigned int * fi)
#endif
{
	int newbit;
	unsigned char aux, wwindow;

	if (bit_count == 0) {
 #ifdef FILE_IO
 		wwindow = fgetc(fi);
 #else
    wwindow = FGETC(fi);
 #endif

		if (wwindow == 0xFF)
#ifdef FILE_IO
			switch (aux = fgetc(fi)) 
#else
			switch (aux = FGETC(fi)) 
#endif
            {	/* skip stuffer 0 byte */
                case 0xFF:
#ifdef FILE_IO
                    fprintf(stderr, "%ld:\tERROR:\tRan out of bit stream\n", ftell(fi));
#else
                    //printf("%d:\tERROR:\tRan out of bit stream\n", FTELL());
#endif
                    aborted_stream(fi);
                    break;

                case 0x00:
                    stuffers++;
                    break;

                default:
#ifdef FILE_IO
                    if (RST_MK(0xFF00 | aux))
                        fprintf(stderr, "%ld:\tERROR:\tSpontaneously found restart!\n", ftell(fi));
                    fprintf(stderr, "%ld:\tERROR:\tLost sync in bit stream\n", ftell(fi));
#else
                    if (RST_MK(0xFF00 | aux))
                        //printf("%d:\tERROR:\tSpontaneously found restart!\n", FTELL());
                    //printf("%d:\tERROR:\tLost sync in bit stream\n", FTELL());
#endif
                    aborted_stream(fi);
                    break;
            }

		bit_count = 8;
	} else
		wwindow = window;

	newbit = (wwindow >> 7) & 1;
	window = wwindow << 1;
	bit_count--;
	return newbit;
}

/*----------------------------------------------------------*/
#ifdef FILE_IO
unsigned int get_size(FILE * fi)
#else
unsigned int get_size(volatile unsigned int * fi)
#endif

{
	unsigned char aux;

#ifdef FILE_IO
	aux = fgetc(fi);
	return (aux << 8) | fgetc(fi);	/* big endian */
#else
	aux = FGETC(fi);
	return (aux << 8) | FGETC(fi);	/* big endian */
#endif
}

/*----------------------------------------------------------*/

#ifdef FILE_IO
void skip_segment(FILE * fi)
#else
void skip_segment(volatile unsigned int * fi)
#endif
{				/* skip a segment we don't want */
	unsigned int size;
	char tag[5];
	int i;

	size = get_size(fi);
	if (size > 5) {
		for (i = 0; i < 4; i++)
   #ifdef FILE_IO
			tag[i] = fgetc(fi);
   #else
      tag[i] = FGETC(fi);
   #endif
		tag[4] = '\0';
		if (verbose)
		{
#ifdef FILE_IO
		fprintf(stderr, "\tINFO:\tTag is %s\n", tag);
#endif
        }
		size -= 4;
	}
 #ifdef FILE_IO
	fseek(fi, size - 2, SEEK_CUR);
 #else
  FSEEK(size - 2, SEEK_CUR);
 #endif
}

/*----------------------------------------------------------------*/
/* find next marker of any type, returns it, positions just after */
/* EOF instead of marker if end of file met while searching ...	  */
/*----------------------------------------------------------------*/
#ifdef FILE_IO
unsigned int get_next_MK(FILE * fi)
#else
unsigned int get_next_MK(volatile unsigned int * fi)
#endif
{
	unsigned int c;
	int ffmet = 0;
	int locpassed = -1;

	passed--;		/* as we fetch one anyway */

#ifdef FILE_IO
    while ((c = fgetc(fi)) != (unsigned int)EOF) 
#else
	while ((c = FGETC(fi)) != (unsigned int)EOF) 
#endif
    {
        switch (c) 
        {
            case 0xFF:
                ffmet = 1;
                break;
            case 0x00:
                ffmet = 0;
                break;
            default:
                if (locpassed > 1)
                {
#ifdef FILE_IO
                    fprintf(stderr, "NOTE: passed %d bytes\n", locpassed);
#else
                    //printf("NOTE: passed %d bytes\n", locpassed);
#endif
                }

                if (ffmet)
                    return (0xFF00 | c);
                ffmet = 0;
                break;
        }
        locpassed++;
        passed++;
    }

	return (unsigned int)EOF;
}

/*----------------------------------------------------------*/
/* loading and allocating of quantization table             */
/* table elements are in ZZ order (same as unpack output)   */
/*----------------------------------------------------------*/

#ifdef FILE_IO
int load_quant_tables(FILE * fi)
#else
int load_quant_tables(volatile unsigned int * fi)
#endif
{
	char aux;
	unsigned int size, n, i, id, x;

	size = get_size(fi);	/* this is the tables' size */
	n = (size - 2) / 65;

	for (i = 0; i < n; i++) {
   #ifdef FILE_IO
			aux = fgetc(fi);
   #else
      aux = FGETC(fi);
   #endif		
		if (first_quad(aux) > 0) 
        {
#ifdef FILE_IO
			fprintf(stderr, "\tERROR:\tBad QTable precision!\n");
#else
			//printf("\tERROR:\tBad QTable precision!\n");
#endif
			return -1;
		}
		id = second_quad(aux);
		if (verbose)
        {
#ifdef FILE_IO
			fprintf(stderr, "\tINFO:\tLoading table %d\n", id);
#else
			//printf("\tINFO:\tLoading table %d\n", id);
#endif
        }
		QTable[id] = (PBlock *) mk_malloc(sizeof(PBlock));
		if (QTable[id] == NULL) 
        {
#ifdef FILE_IO
			fprintf(stderr, "\tERROR:\tCould not allocate table storage!\n");
#else
			//printf("\tERROR:\tCould not allocate table storage!\n");
#endif
			exit(1);
		}
		QTvalid[id] = 1;
		for (x = 0; x < 64; x++)
    #ifdef FILE_IO
			QTable[id]->linear[x] = fgetc(fi);
    #else
      QTable[id]->linear[x] = FGETC(fi);
    #endif
		/*
		   -- This is useful to print out the table content --
		   for (x = 0; x < 64; x++)
		   fprintf(stderr, "%d\n", QTable[id]->linear[x]);
		 */
	}
	return 0;
}

/*----------------------------------------------------------*/
/* initialise MCU block descriptors	                    */
/*----------------------------------------------------------*/

int init_MCU(void)
{
	int i, j, k, n, hmax = 0, vmax = 0;

	for (i = 0; i < 10; i++)
		MCU_valid[i] = -1;

	k = 0;

	for (i = 0; i < n_comp; i++) {
		if (comp[i].HS > hmax)
			hmax = comp[i].HS;
		if (comp[i].VS > vmax)
			vmax = comp[i].VS;
		n = comp[i].HS * comp[i].VS;

		comp[i].IDX = k;
		for (j = 0; j < n; j++) {
			MCU_valid[k] = i;
			MCU_buff[k] = (PBlock *) mk_malloc(sizeof(PBlock));
			if (MCU_buff[k] == NULL) {
#ifdef FILE_IO
				fprintf(stderr, "\tERROR:\tCould not allocate MCU buffers!\n");
				exit(1);
#endif
			}
			k++;
			if (k == 10) {
#ifdef FILE_IO
				fprintf(stderr, "\tERROR:\tMax subsampling exceeded!\n");
				return -1;
#endif
			}
		}
	}

	MCU_sx = 8 * hmax;
	MCU_sy = 8 * vmax;
	for (i = 0; i < n_comp; i++) {
		comp[i].HDIV = (hmax / comp[i].HS > 1);	/* if 1 shift by 0 */
		comp[i].VDIV = (vmax / comp[i].VS > 1);	/* if 2 shift by one */
	}

	mx_size = ceil_div(x_size, MCU_sx);
	my_size = ceil_div(y_size, MCU_sy);
	rx_size = MCU_sx * floor_div(x_size, MCU_sx);
	ry_size = MCU_sy * floor_div(y_size, MCU_sy);

	return 0;
}

/*----------------------------------------------------------*/
/* this takes care for processing all the blocks in one MCU */
/*----------------------------------------------------------*/

#ifdef FILE_IO
int process_MCU(FILE * fi)
#else
int process_MCU(volatile unsigned int * fi)
#endif
{
	unsigned int i,j;
	long offset;
	int goodrows, goodcolumns;

	if (MCU_column == mx_size) {
		MCU_column = 0;
		MCU_row++;
		if (MCU_row == my_size) {
			in_frame = 0;
			return 0;
		}
		if (verbose)
		{
#ifdef FILE_IO
			fprintf(stderr, "%ld:\tINFO:\tProcessing stripe %d/%d\n", ftell(fi), MCU_row + 1, my_size);
#else
			//printf("%d:\tINFO:\tProcessing stripe %d/%d\n", FTELL(), MCU_row + 1, my_size);
#endif
        }
	}

	for (curcomp = 0; MCU_valid[curcomp] != -1; curcomp++) 
	{
		unpack_block(fi, FBuff, MCU_valid[curcomp]);	/* pass index to HT,QT,pred */

        /*Send FBlock to the Core 2 for IDCT using DMA 1*/
        //mk_mon_debug_info(0xCCCCCCCC);
        //cheap_write_fifo(producer, consumer, FBuff, BLOCK_IF_FULL);

		IDCT(FBuff, MCU_buff[curcomp]);
	}
	
        offset = ((MCU_row * MCU_sy << 10) + (MCU_column * MCU_sx));

	/* YCrCb to RGB color space transform here */
	if (n_comp > 1)
	{
          color_conversion(offset);
    }
	else
	{
#if 0
           memmove_jpeg(ColorBuffer, MCU_buff[0], 64);
#else
         
        for(i = 0; i < 8 ; i++) 
	    {
		  for(j = 0; j < 8 ; j++) 
		  {
			*(FrameBuffer + offset + (i << 10) + j) = ((unsigned char)(MCU_buff[(i<<3) + j]) << 16) & 0X00FF0000;
		  } 
		  //hw_dma_send((void*)(( shared_pt_REMOTEADDR + offset + i * 1024)), ((void*)(FrameBuffer + i * 8)), 8*4, (void*)mb1_dma1_BASEADDR);
	    } 
#endif
	}
#if 0
	/* cut last row/column as needed */
	if ((y_size != ry_size) && (MCU_row == (my_size - 1)))
		goodrows = y_size - ry_size;
	else
		goodrows = MCU_sy;

	if ((x_size != rx_size) && (MCU_column == (mx_size - 1)))
		goodcolumns = x_size - rx_size;
	else
		goodcolumns = MCU_sx;

#ifdef FILE_IO	
	offset = n_comp * (MCU_row * MCU_sy * x_size + MCU_column * MCU_sx);
#else
	offset = ((MCU_row * MCU_sy << 10) + (MCU_column * MCU_sx));
#endif

	for (i = 0; i < goodrows; i++)
	{
#ifdef FILE_IO
		//memmove_jpeg(FrameBuffer + offset + n_comp * i * x_size, ColorBuffer + n_comp * i * MCU_sx,n_comp * goodcolumns);
#else
		//memmove_jpeg_int((FrameBuffer + offset + (i * 1024)), ColorBuffer + n_comp * i * MCU_sx,n_comp * goodcolumns);
#endif
	}

#endif

	MCU_column++;
	return 1;
}
