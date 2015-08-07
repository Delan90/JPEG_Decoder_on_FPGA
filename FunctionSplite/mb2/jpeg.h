/* File : jpeg.h, header for all jpeg code */
/* Author: Pierre Guerrier, march 1998     */
/*                                         */
/* 19/01/99  Edited by Koen van Eijk       */

/* Leave structures in memory,output something and dump core in the event
   of a failure: */
#ifndef JPEG_H
#define JPEG_H

/*----------------------------------*/
/* JPEG format parsing markers here */
/*----------------------------------*/

#define SOI_MK	0xFFD8		/* start of image       */
#define APP_MK	0xFFE0		/* custom, up to FFEF */
#define COM_MK	0xFFFE		/* commment segment     */
#define SOF_MK	0xFFC0		/* start of frame       */
#define SOS_MK	0xFFDA		/* start of scan        */
#define DHT_MK	0xFFC4		/* Huffman table        */
#define DQT_MK	0xFFDB		/* Quant. table         */
#define DRI_MK	0xFFDD		/* restart interval     */
#define EOI_MK	0xFFD9		/* end of image         */
#define MK_MSK	0xFFF0

/* is x a restart interval ? */
#define RST_MK(x)	( (0xFFF8&(x)) == 0xFFD0 )

/*-------------------------------------------------------- */
/* all kinds of macros here				*/
/*-------------------------------------------------------- */

#define first_quad(c)   ((c) >> 4)	/* first 4 bits in file order */
#define second_quad(c)  ((c) & 15)

#define HUFF_ID(hclass, id)       (2 * (hclass) + (id))

#define DC_CLASS        0
#define AC_CLASS        1

/*-------------------------------------------------------*/
/* JPEG data types here					*/
/*-------------------------------------------------------*/

typedef union {			/* block of pixel-space values */
	unsigned char block[8][8];
	unsigned char linear[64];
} PBlock;

typedef union {			/* block of frequency-space values */
	int block[8][8];
	int linear[64];
} FBlock;

/* component descriptor structure */

typedef struct {
	unsigned char CID;	/* component ID */
	unsigned char IDX;	/* index of first block in MCU */

	unsigned char HS;	/* sampling factors */
	unsigned char VS;
	unsigned char HDIV;	/* sample width ratios */
	unsigned char VDIV;

	char QT;		/* QTable index, 2bits  */
	char DC_HT;		/* DC table index, 1bit */
	char AC_HT;		/* AC table index, 1bit */
	int PRED;		/* DC predictor value */
} cd_t;

/* Set a 256 kbyte maximum input size. */
#define JPGBUFFER_SIZE      0x40000

/*--------------------------------------------*/
/* global variables here		      */
/*--------------------------------------------*/

/* for every component, useful stuff */
extern cd_t comp[3];
/* decoded component buffer */
extern PBlock *MCU_buff[10];

/* between IDCT and color convert */
/* for every DCT block, component id then -1 */
extern int MCU_valid[10];
/* three quantization tables */
extern PBlock *QTable[4];
/* at most, but seen as four ... */
extern int QTvalid[4];

#ifdef FILE_IO
extern FILE *fi;
#else
extern volatile unsigned int *fi;
#endif

/* picture attributes */
/* Video frame size     */
extern int x_size, y_size;

/* down-rounded Video frame size */
/* in pixel units, multiple of MCU */
extern int rx_size, ry_size;

/* MCU size in pixels   */
extern int MCU_sx, MCU_sy;
/* picture size in units of MCUs */
extern int mx_size, my_size;

/* number of components 1,3 */
extern int n_comp;

/* processing cursor variables */
extern int in_frame, curcomp, MCU_row, MCU_column;
/* current position in MCU unit */

/* RGB buffer storage */
/* MCU after color conversion */
extern volatile unsigned char *ColorBuffer;
/* complete final RGB image */
extern volatile unsigned int *FrameBuffer;
extern PBlock *PBuff;
extern FBlock *FBuff;

/* process statistics */
/* number of stuff bytes in file */
extern int stuffers;
/* number of bytes skipped looking for markers */
extern int passed;

extern int verbose;

/* Counter used by FGET and FSEEK in 5kk03.c */
extern int vld_count;
//extern unsigned int input_buffer[JPGBUFFER_SIZE / sizeof(int)];

/*-----------------------------------------*/
/* prototypes from utils.c		   */
/*-----------------------------------------*/

void show_FBlock(FBlock * S);
void show_PBlock(PBlock * S);

#ifdef FILE_IO
void bin_dump(FILE * fi);
#else
void bin_dump(unsigned int * fi);
#endif

int ceil_div(int N, int D);
int floor_div(int N, int D);
void reset_prediction();
int reformat(unsigned long S, int good);
void free_structures();
void suicide();

#ifdef FILE_IO
void aborted_stream(FILE * fi);
void RGB_save(FILE * fo);
#else
void aborted_stream(volatile unsigned int * fi);
void RGB_save(unsigned int * fo);
#endif

/*-----------------------------------------*/
/* prototypes from parse.c		   */
/*-----------------------------------------*/

void clear_bits();
#ifdef FILE_IO
unsigned long get_bits(FILE * fi, int number);
unsigned char get_one_bit(FILE * fi);
unsigned int get_size(FILE * fi);
unsigned int get_next_MK(FILE * fi);
int load_quant_tables(FILE * fi);
int init_MCU();
void skip_segment(FILE * fi);
int process_MCU(FILE * fi);
#else
unsigned long get_bits(volatile unsigned int * fi, int number);
unsigned char get_one_bit(volatile unsigned int * fi);
unsigned int get_size(volatile unsigned int * fi);
unsigned int get_next_MK(volatile unsigned int * fi);
int load_quant_tables(volatile unsigned int * fi);
int init_MCU();
void skip_segment(volatile unsigned int * fi);
int process_MCU(volatile unsigned int * fi);
#endif

/*-------------------------------------------*/
/* prototypes from fast_idct.c               */
/*-------------------------------------------*/

void IDCT(const FBlock * S, PBlock * T);

/*-----------------------------------------*/
/* prototypes from color.c		   */
/*-----------------------------------------*/

void color_conversion(long);

/*-------------------------------------------*/
/* prototypes from table_vld.c or tree_vld.c */
/*-------------------------------------------*/
#ifdef FILE_IO
int load_huff_tables(FILE * fi);
unsigned char get_symbol(FILE * fi, int select);
#else
int load_huff_tables(volatile unsigned int * fi);
unsigned char get_symbol(volatile unsigned int * fi, int select);
#endif
/*-----------------------------------------*/
/* prototypes from huffman.c 		   */
/*-----------------------------------------*/
#ifdef FILE_IO
void unpack_block(FILE * fi, FBlock * T, int comp);
#else
void unpack_block(volatile unsigned int * fi, FBlock * T, int comp);
#endif
/* unpack, predict, dequantize, reorder on store */

/*-----------------------------------------*/
/* prototypes from jpegtobmp.c		       */
/*-----------------------------------------*/
#ifdef FILE_IO
void write_bmp(const char *const file2);
#else
void write_bmp_to_ddr();
unsigned int pack_4char_int(unsigned char a,unsigned char b,unsigned char c);
#endif
/*-----------------------------------------*/
/* prototypes from 5kk03.c		   */
/*-----------------------------------------*/
unsigned int FGETC(volatile unsigned int *fi);
int FSEEK(int offset, int start);

#endif
