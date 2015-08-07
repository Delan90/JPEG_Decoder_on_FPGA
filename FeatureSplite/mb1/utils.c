/*-----------------------------------------*/
/* File : utils.c, utilities for jfif view */
/* Author : Pierre Guerrier, march 1998	   */
/*-----------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
//#include "5kk03.h"
#include "jpeg.h"
#include <comik.h>
#include <5kk03-utils.h>

#ifdef FILE_IO
/* Prints a data block in frequency space. */
void show_FBlock(FBlock * S)
{
	int i, j;

	for (i = 0; i < 8; i++) {
		for (j = 0; j < 8; j++)
			fprintf(stderr, "\t%d", S->block[i][j]);
		fprintf(stderr, "\n");
	}
}

/* Prints a data block in pixel space. */
void show_PBlock(PBlock * S)
{
	int i, j;

	for (i = 0; i < 8; i++) {
		for (j = 0; j < 8; j++)
			fprintf(stderr, "\t%d", S->block[i][j]);
		fprintf(stderr, "\n");
	}
}

/* Prints the next 800 bits read from file `fi'. */
#ifdef FILE_IO
void bin_dump(FILE * fi)
#else
void bin_dump(unsigned int * fi)
#endif
{
	int i;

	for (i = 0; i < 100; i++) 
    {
		unsigned int bitmask;
#ifdef FILE_IO
		int c = fgetc(fi);
#else
		int c = FGETC(fi);
#endif

		for (bitmask = 0x80; bitmask; bitmask >>= 1)
        {
#ifdef FILE_IO
			fprintf(stderr, "\t%1d", ! !(c & bitmask));
#else
			//printf("\t%d", ! !(c & bitmask));
#endif

        }
#ifdef FILE_IO
		fprintf(stderr, "\n");
#else
		//printf("\n");
#endif
	}
}
#endif
/*-------------------------------------------*/
/* core dump generator for forensic analysis */
/*-------------------------------------------*/

void suicide(void)
{
	int *P;
#ifdef FILE_IO
	fflush(stdout);
	fflush(stderr);
#endif
	P = NULL;
	*P = 1;
}


/*-------------------------------------------*/

#ifdef FILE_IO
void aborted_stream(FILE * fi)
#else
void aborted_stream(volatile unsigned int * fi)
#endif
{
#if FILE_IO
	fprintf(stderr, "%ld:\tERROR:\tAbnormal end of decompression process!\n", ftell(fi));
	fprintf(stderr, "\tINFO:\tTotal skipped bytes %d, total stuffers %d\n", passed, stuffers);

	fclose(fi);

	free_structures();
#else
	//printf("%d:\tERROR:\tAbnormal end of decompression process!\n", FTELL());
	//printf("\tINFO:\tTotal skipped bytes %d, total stuffers %d\n", passed, stuffers);
	free_structures();
#endif

	suicide();
}

/*----------------------------------------------------------*/

/* Returns ceil(N/D). */
int ceil_div(int N, int D)
{
	int i = N / D;

	if (N > D * i)
		i++;
	return i;
}

/* Returns floor(N/D). */
int floor_div(int N, int D)
{
	int i = N / D;

	if (N < D * i)
		i--;
	return i;
}

/*----------------------------------------------------------*/

/* For all components reset DC prediction value to 0. */
void reset_prediction(void)
{
	int i;

	for (i = 0; i < 3; i++)
		comp[i].PRED = 0;
}

/*---------------------------------------------------------*/

/* Transform JPEG number format into usual 2's-complement format. */
int reformat(unsigned long S, int good)
{
	int St;

	if (!good)
		return 0;
	St = 1 << (good - 1);	/* 2^(good-1) */
	if (S < (unsigned long)St)
		return (S + 1 + ((-1) << good));
	else
		return S;
}

/*----------------------------------------------------------*/

void free_structures(void)
{
	int i;

	for (i = 0; i < 4; i++)
		if (QTvalid[i])
			mk_free(QTable[i]);

#if 0
	if (ColorBuffer != NULL)
		mk_free(ColorBuffer);

	if (FrameBuffer != NULL)
		mk_free(FrameBuffer);
#endif
	if (PBuff != NULL)
		mk_free(PBuff);
	if (FBuff != NULL)
		mk_free(FBuff);

	for (i = 0; MCU_valid[i] != -1; i++)
		mk_free(MCU_buff[i]);
}

/*-------------------------------------------*/
/* this is to save final RGB image to disk   */
/* using the sunraster uncompressed format   */
/*-------------------------------------------*/

/* Sun raster header */

typedef struct {
	unsigned long MAGIC;
	unsigned long Width;
	unsigned long Heigth;
	unsigned long Depth;
	unsigned long Length;
	unsigned long Type;
	unsigned long CMapType;
	unsigned long CMapLength;
} sunraster;

#ifdef FILE_IO
void RGB_save(FILE * fo)
#else
void RGB_save(unsigned int * fo)
#endif

{
	sunraster *FrameHeader;
#ifdef FILE_IO
	int i;
#else
	/*variable is not used in this case*/
#endif

	FrameHeader = (sunraster *) mk_malloc(sizeof(sunraster));
	FrameHeader->MAGIC = 0x59a66a95L;
	FrameHeader->Width = 2 * ceil_div(x_size, 2);	/* round to 2 more */
	FrameHeader->Heigth = y_size;
	FrameHeader->Depth = (n_comp > 1) ? 24 : 8;
	FrameHeader->Length = 0;	/* not required in v1.0 */
	FrameHeader->Type = 0;	/* old one */
	FrameHeader->CMapType = 0;	/* truecolor */
	FrameHeader->CMapLength = 0;	/* none */

#ifdef FILE_IO
	fwrite(FrameHeader, sizeof(sunraster), 1, fo);
	for (i = 0; i < y_size; i++)
		fwrite(FrameBuffer + n_comp * i * x_size, n_comp, FrameHeader->Width, fo);
#else
	/*Implement code to write into DDR or something similiar*/
#endif
}

void memmove_jpeg(volatile unsigned char *dest, volatile unsigned char *src,unsigned int size)
{
	int i;
	for (i = 0; i < size ; i++)
	{
		dest[i] = src[i];
	}
}

void memmove_jpeg_int(volatile unsigned int *dest, volatile unsigned char *src,unsigned int size)
{
	unsigned int i;
	for (i = 0; i < ((size/4) + 1); i++)
	{
		dest[i] = pack_4char_int(src[i],src[i+1],src[i+2]);
		mk_mon_debug_info(0XFFFF);
		mk_mon_debug_info(src[i]);
		mk_mon_debug_info(src[i+1]);
		mk_mon_debug_info(src[i+2]);
		mk_mon_debug_info(dest[i]);
	}
}
