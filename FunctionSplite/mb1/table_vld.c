/*-----------------------------------------*/
/* File : table_vld.c, utilities for jfif view */
/* Author : Pierre Guerrier, march 1998	   */
/*-----------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
//#include "5kk03.h"
#include "jpeg.h"

/*--------------------------------------*/
/* private huffman.c defines and macros */
/*--------------------------------------*/

/* Memory size of HTables; */
#define MAX_SIZE(hclass)		((hclass)?162:14)

/*--------------------------------------*/
/* some static structures for storage */
/*--------------------------------------*/

static unsigned char DC_Table0[MAX_SIZE(DC_CLASS)], DC_Table1[MAX_SIZE(DC_CLASS)];

static unsigned char AC_Table0[MAX_SIZE(AC_CLASS)], AC_Table1[MAX_SIZE(AC_CLASS)];

static unsigned char *HTable[4] = {
	&DC_Table0[0], &DC_Table1[0],
	&AC_Table0[0], &AC_Table1[0]
};

static int MinCode[4][16];
static int MaxCode[4][16];
static int ValPtr[4][16];

/*----------------------------------------------------------*/
/* Loading of Huffman table, with leaves drop ability	    */
/*----------------------------------------------------------*/

#ifdef FILE_IO
int load_huff_tables(FILE * fi)
#else
int load_huff_tables(unsigned char * fi)
#endif
{
	char aux;
	int size, hclass, id, max;
	int LeavesN, LeavesT, i;
	int AuxCode;

	size = get_size(fi);	/* this is the tables' size */

	size -= 2;

	while (size > 0) {

#ifdef FILE_IO
		aux = fgetc(fi);
#else
    aux = FGETC(fi);
#endif
		hclass = first_quad(aux);	/* AC or DC */
		id = second_quad(aux);	/* table no */
		if (id > 1) {
#ifdef FILE_IO			
			fprintf(stderr, "\tERROR:\tBad HTable identity %d!\n", id);
#endif
			return -1;
		}
		id = HUFF_ID(hclass, id);
		if (verbose)
			{
			#ifdef FILE_IO	
			 fprintf(stderr, "\tINFO:\tLoading Table %d\n", id);
			#endif	
			}
		size--;

		LeavesT = 0;
		AuxCode = 0;

		for (i = 0; i < 16; i++) {
   #ifdef FILE_IO
			LeavesN = fgetc(fi);
   #else
      LeavesN = FGETC(fi);
   #endif
			ValPtr[id][i] = LeavesT;
			MinCode[id][i] = AuxCode * 2;
			AuxCode = MinCode[id][i] + LeavesN;

			MaxCode[id][i] = (LeavesN) ? (AuxCode - 1) : (-1);
			LeavesT += LeavesN;
		}
		size -= 16;

		if (LeavesT > MAX_SIZE(hclass)) {
			max = MAX_SIZE(hclass);
			#ifdef FILE_IO	
			fprintf(stderr, "\tWARNING:\tTruncating Table by %d symbols\n", LeavesT - max);
			#endif
		} else
			max = LeavesT;

		for (i = 0; i < max; i++)
   #ifdef FILE_IO
			HTable[id][i] = fgetc(fi);	/* load in raw order */
   #else
      HTable[id][i] = FGETC(fi);	/* load in raw order */
   #endif
		for (i = max; i < LeavesT; i++)
   #ifdef FILE_IO
			fgetc(fi);	/* skip if we don't load */
   #else
      FGETC(fi);	/* skip if we don't load */
   #endif
		size -= LeavesT;

		if (verbose)
		{
   #ifdef FILE_IO
			fprintf(stderr, "\tINFO:\tUsing %d words of table memory\n", LeavesT);
   #else
      //printf("\tINFO:\tUsing %d words of table memory\n", LeavesT);
   #endif
        }

	}			/* loop on tables */
	return 0;
}

/*-----------------------------------*/
/* extract a single symbol from file */
/* using specified huffman table ... */
/*-----------------------------------*/

#ifdef FILE_IO
unsigned char get_symbol(FILE * fi, int select)
#else
unsigned char get_symbol(unsigned char * fi, int select)
#endif
{
	long code = 0;
	int length;
	int index;

	for (length = 0; length < 16; length++) 
    {
		code = (2 * code) | get_one_bit(fi);
		if (code <= MaxCode[select][length])
			break;
	}

	index = ValPtr[select][length] + code - MinCode[select][length];

	if (index < MAX_SIZE(select / 2))
		return HTable[select][index];

#ifdef FILE_IO
	fprintf(stderr, "%ld:\tWARNING:\tOverflowing symbol table !\n", ftell(fi));
#else
	//printf("%d:\tWARNING:\tOverflowing symbol table !\n", FTELL());
#endif
	return 0;
}
