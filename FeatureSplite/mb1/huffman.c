/*-------------------------------------------*/
/* File : huffman.c, utilities for jfif view */
/* Author : Pierre Guerrier, march 1998	     */
/*-------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>

#include "jpeg.h"

/*--------------------------------------*/
/* private huffman.c defines and macros */
/*--------------------------------------*/

#define HUFF_EOB		0x00
#define HUFF_ZRL		0xF0

/*------------------------------------------*/
/* some constants for on-the-fly IQ and IZZ */
/*------------------------------------------*/

static const int const G_ZZ[] = {
	0, 1, 8, 16, 9, 2, 3, 10,
	17, 24, 32, 25, 18, 11, 4, 5,
	12, 19, 26, 33, 40, 48, 41, 34,
	27, 20, 13, 6, 7, 14, 21, 28,
	35, 42, 49, 56, 57, 50, 43, 36,
	29, 22, 15, 23, 30, 37, 44, 51,
	58, 59, 52, 45, 38, 31, 39, 46,
	53, 60, 61, 54, 47, 55, 62, 63
};

/*-------------------------------------------------*/
/* here we unpack, predict, unquantify and reorder */
/* a complete 8*8 DCT block ...			   */
/*-------------------------------------------------*/

#ifdef FILE_IO
void unpack_block(FILE * fi, FBlock * T, int select)
#else
void unpack_block(volatile unsigned int * fi, FBlock * T, int select)
#endif
{
	unsigned int i, run, cat;
	int value;
	unsigned char symbol;

	/* Init the block with 0's: */
	for (i = 0; i < 64; i++)
		T->linear[i] = 0;

	/* First get the DC coefficient: */
	symbol = get_symbol(fi, HUFF_ID(DC_CLASS, comp[select].DC_HT));
	value = reformat(get_bits(fi, symbol), symbol);

	value += comp[select].PRED;
	comp[select].PRED = value;
	T->linear[0] = value * QTable[(int)comp[select].QT]->linear[0];

	/* Now get all 63 AC values: */
	for (i = 1; i < 64; i++) {
		symbol = get_symbol(fi, HUFF_ID(AC_CLASS, comp[select].AC_HT));
		if (symbol == HUFF_EOB)
			break;
		if (symbol == HUFF_ZRL) {
			i += 15;
			continue;
		}
		cat = symbol & 0x0F;
		run = (symbol >> 4) & 0x0F;
		i += run;
		value = reformat(get_bits(fi, cat), cat);

		/* Dequantify and ZigZag-reorder: */
		T->linear[G_ZZ[i]] = value * QTable[(int)comp[select].QT]->linear[i];
	}
}
