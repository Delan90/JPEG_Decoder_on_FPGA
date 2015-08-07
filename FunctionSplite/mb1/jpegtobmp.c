#include<stdio.h>
//#include "hw_dma.h"
#include "comik.h"
#include "global_memmap.h"
#include "5kk03-utils.h"

/* real declaration of global variables here */
/* see jpeg.h for more info			*/
#include "jpeg.h"
//#include "surfer.jpg.h"

/* descriptors for 3 components */
cd_t comp[3];
/* decoded DCT blocks buffer */
PBlock *MCU_buff[10];
/* components of above MCU blocks */
int MCU_valid[10];

/* quantization tables */
PBlock *QTable[4];
int QTvalid[4];

/* Video frame size     */
int x_size, y_size;
/* down-rounded Video frame size (integer MCU) */
int rx_size, ry_size;
/* MCU size in pixels   */
int MCU_sx, MCU_sy;
/* picture size in units of MCUs */
int mx_size, my_size;
/* number of components 1,3 */
int n_comp;

/* MCU after color conversion */
//volatile unsigned char *ColorBuffer;
/* complete final RGB image */
unsigned int *FrameBuffer;
/* scratch frequency buffer */
FBlock *FBuff;
/* scratch pixel buffer */
PBlock *PBuff;
/* frame started ? current component ? */
int in_frame, curcomp;
/* current position in MCU unit */
int MCU_row, MCU_column;

/* input  File stream pointer   */
#ifdef FILE_IO
FILE *fi;
#else
unsigned char *fi;
unsigned int *ddr_input;
unsigned int *cmem_input_circ_buff;
#endif
/* stuff bytes in entropy coded segments */
int stuffers = 0;
/* bytes passed when searching markers */
int passed = 0;

int verbose = 0;

/* Extra global variables for 5kk03 */

int vld_count = 0;		/* Counter used by FGET and FSEEK in 5kk03.c */
//unsigned int input_buffer[JPGBUFFER_SIZE / sizeof(int)];

/* End extra global variables for 5kk03 */

int JpegToBmp();

extern int  bit_counter;
int buff_sel = 0;
void clear_frame_buffer();
extern int benchmark_mb1_dma_counter;
extern int DMA_flag;
extern int bench_dyna_mem_size;

int ddr_input_chunck_offset = 0;

int jpeg_dec_main()
{


        JpegToBmp();
    return 0;

}

/*-----------------------------------------------------------------*/
/*		MAIN		MAIN		MAIN		   */
/*-----------------------------------------------------------------*/

int JpegToBmp()
{
    unsigned int aux, mark;
    int n_restarts, restart_interval, leftover;	/* RST check */
    int i, j;

    /*No Need to do the file operation operation*/
#ifdef FILE_IO
    fi = fopen(file1, "rb");
    if (fi == NULL) 
    {
        return 0;
    }
#else 


#ifdef INPUT_DMA
    // wait for input data to arrive
    //mk_mon_debug_info(0xFF);
    // read input file DRAM (via my cmem-out)
	ddr_input = (unsigned int*)(shared_pt_REMOTEADDR + 1024*1024*4);
	cmem_input_circ_buff = (unsigned int*) (mb1_cmemout0_BASEADDR);

	hw_dma_receive_addr((int*)(cmem_input_circ_buff + buff_sel * INPUT_READ_SIZE_INT), (void*)(&ddr_input[ddr_input_chunck_offset*INPUT_READ_SIZE_INT]), INPUT_READ_SIZE_INT, (void*)mb1_dma0_BASEADDR);
	ddr_input_chunck_offset++;
	buff_sel = CHANGE_BUFFER(buff_sel);
	for (i = 0 ; i < NUM_OF_INIT_BUFF_LOAD-1; i++)
    {
	    while(hw_dma_status_addr( (void *) mb1_dma0_BASEADDR));
		hw_dma_receive_addr((unsigned int*)(cmem_input_circ_buff + buff_sel * INPUT_READ_SIZE_INT), (void*)(&ddr_input[ddr_input_chunck_offset*INPUT_READ_SIZE_INT]), INPUT_READ_SIZE_INT, (void*)mb1_dma0_BASEADDR);
		ddr_input_chunck_offset++;

        buff_sel = CHANGE_BUFFER(buff_sel);
    }

    fi = (unsigned char *)(cmem_input_circ_buff);
#else
    fi = (volatile unsigned int *)(shared_pt_REMOTEADDR+1024*1024*4);
#endif

#endif

    /* First find the SOI marker: */
    aux = get_next_MK(fi);


    if (aux != SOI_MK)
        aborted_stream(fi);

    if (verbose)
    {
#ifdef FILE_IO
        fprintf(stderr, "%ld:\tINFO:\tFound the SOI marker!\n", ftell(fi));
#else
        //printf("%d:\tINFO:\tFound the SOI marker!\n", FTELL());
#endif
    }
    in_frame = 0;
    restart_interval = 0;
    for (i = 0; i < 4; i++)
        QTvalid[i] = 0;


    /* Now process segments as they appear: */
    do {
        mark = get_next_MK(fi);

        //mk_mon_debug_info(0XFFF);
        //mk_mon_debug_info(mark);
        //mk_mon_debug_info(bit_counter);
        //mk_mon_debug_info(0XFFF);

        switch (mark) {
            case SOF_MK:
                if (verbose)
                {
#ifdef FILE_IO
                    fprintf(stderr, "%ld:\tINFO:\tFound the SOF marker!\n", ftell(fi));
#else
                    //printf("%d:\tINFO:\tFound the SOF marker!\n", FTELL());
#endif
                }
                in_frame = 1;
                get_size(fi);	/* header size, don't care */

                /* load basic image parameters */
#ifdef FILE_IO
                fgetc(fi);	/* precision, 8bit, don't care */
#else
                FGETC(fi);	/* precision, 8bit, don't care */
#endif
                y_size = get_size(fi);
                x_size = get_size(fi);



                if (verbose)
                {
#ifdef FILE_IO
                    fprintf(stderr, "\tINFO:\tImage size is %d by %d\n", x_size, y_size);
#else
                    //printf("\tINFO:\tImage size is %d by %d\n", x_size, y_size);
#endif
                }

#ifdef FILE_IO
                n_comp = fgetc(fi);	/* # of components */
#else
                n_comp = FGETC(fi);	/* # of components */
#endif
                if (verbose) 
                {
#ifdef FILE_IO
                    fprintf(stderr, "\tINFO:\t");
#else
                    //printf("\tINFO:\t");
#endif
                    switch (n_comp) 
                    {
#ifdef FILE_IO
                        case 1:
                            fprintf(stderr, "Monochrome");
                            break;
                        case 3:
                            fprintf(stderr, "Color");
                            break;
                        default:
                            fprintf(stderr, "Not a");
                            break;
#else
                        case 1:
                            //printf("Monochrome");
                            break;
                        case 3:
                            //printf("Color");
                            break;
                        default:
                            //printf("Not a");
                            break;

#endif
                    }
#ifdef FILE_IO
                    fprintf(stderr, " JPEG image!\n");
#else
                    //printf(" JPEG image!\n");
#endif
                }

                for (i = 0; i < n_comp; i++) 
                {
#ifdef FILE_IO
                    /* component specifiers */
                    comp[i].CID = fgetc(fi);
                    aux = fgetc(fi);
                    comp[i].HS = first_quad(aux);
                    comp[i].VS = second_quad(aux);
                    comp[i].QT = fgetc(fi);
#else
                    /* component specifiers */
                    comp[i].CID = FGETC(fi);
                    aux = FGETC(fi);
                    comp[i].HS = first_quad(aux);
                    comp[i].VS = second_quad(aux);
                    comp[i].QT = FGETC(fi);
#endif
                }

                if ((n_comp > 1) && verbose)
                {
#ifdef FILE_IO
                    fprintf(stderr,
                            "\tINFO:\tColor format is %d:%d:%d, H=%d\n",
                            comp[0].HS * comp[0].VS, comp[1].HS * comp[1].VS, comp[2].HS * comp[2].VS,
                            comp[1].HS);
#else
#if 0
                    //printf("\tINFO:\tColor format is %d:%d:%d, H=%d\n",
                    comp[0].HS * comp[0].VS, comp[1].HS * comp[1].VS, comp[2].HS * comp[2].VS,
                        comp[1].HS);
#endif
#endif
                }

                if (init_MCU() == -1)
                    aborted_stream(fi);

                /* dimension scan buffer for YUV->RGB conversion */
                /* TODO */			
#if 0
                FrameBuffer = (volatile unsigned char *)mk_malloc((size_t) x_size * y_size * n_comp);
#else
                FrameBuffer = (unsigned int *) mb1_cmemout1_BASEADDR;
#endif		
                //ColorBuffer = (volatile unsigned char *)mk_malloc((size_t) MCU_sx * MCU_sy * n_comp);
#if 0
                FBuff = (FBlock *) mk_malloc(sizeof(FBlock));
#else
                MY_MK_MALLOC(FBuff,FBlock,1);
#endif
#if 0
                PBuff = (PBlock *) mk_malloc(sizeof(PBlock));
#else
                MY_MK_MALLOC(PBuff,PBlock,1);
#endif

                if ((FrameBuffer == NULL) /*|| (ColorBuffer == NULL)*/ || (FBuff == NULL) || (PBuff == NULL)) 
                {
#ifdef FILE_IO
                    fprintf(stderr, "\tERROR:\tCould not allocate pixel storage!\n");
#else
                    //printf("\tERROR:\tCould not allocate pixel storage!\n");
#endif
                }
                break;

            case DHT_MK:
                if (verbose)
                {
#ifdef FILE_IO
                    fprintf(stderr, "%ld:\tINFO:\tDefining Huffman Tables\n", ftell(fi));
#else
                    //printf("%d:\tINFO:\tDefining Huffman Tables\n", FTELL());
#endif
                }
                if (load_huff_tables(fi) == -1)
                    aborted_stream(fi);
                break;

            case DQT_MK:
                if (verbose)
                {
#ifdef FILE_IO
                    fprintf(stderr, "%ld:\tINFO:\tDefining Quantization Tables\n", ftell(fi));
#else
                    //printf("%d:\tINFO:\tDefining Quantization Tables\n", FTELL());
#endif
                }
                if (load_quant_tables(fi) == -1)
                    aborted_stream(fi);
                break;

            case DRI_MK:
                get_size(fi);	/* skip size */
                restart_interval = get_size(fi);
                if (verbose)
                {
#ifdef FILE_IO
                    fprintf(stderr, "%ld:\tINFO:\tDefining Restart Interval %d\n", ftell(fi),restart_interval);
#else
                    //printf("%d:\tINFO:\tDefining Restart Interval %d\n", FTELL(), restart_interval);
#endif
                }
                break;

            case SOS_MK:	/* lots of things to do here */
                //mk_mon_debug_info(01);
                //mk_mon_debug_info(bit_counter);
                //mk_mon_debug_info(02);
                if (verbose)
                {
#ifdef FILE_IO
                    fprintf(stderr, "%ld:\tINFO:\tFound the SOS marker!\n", ftell(fi));
#else
                    //printf("%d:\tINFO:\tFound the SOS marker!\n", FTELL(fi));
#endif
                }
                get_size(fi);	/* don't care */
#ifdef FILE_IO
                aux = fgetc(fi);
#else
                aux = FGETC(fi);
#endif
                if (aux != (unsigned int)n_comp) 
                {
#ifdef FILE_IO
                    fprintf(stderr, "\tERROR:\tBad component interleaving!\n");
#else
                    //printf("\tERROR:\tBad component interleaving!\n");
#endif
                    aborted_stream(fi);
                }

                for (i = 0; i < n_comp; i++) 
                {
#ifdef FILE_IO
                    aux = fgetc(fi);
#else
                    aux = FGETC(fi);
#endif
                    if (aux != comp[i].CID) 
                    {
#ifdef FILE_IO
                        fprintf(stderr, "\tERROR:\tBad Component Order!\n");
#else
                        //printf("\tERROR:\tBad Component Order!\n");
#endif
                        aborted_stream(fi);
                    }
#ifdef FILE_IO
                    aux = fgetc(fi);
#else
                    aux = FGETC(fi);
#endif
                    comp[i].DC_HT = first_quad(aux);
                    comp[i].AC_HT = second_quad(aux);
                }
                get_size(fi);
#ifdef FILE_IO
                fgetc(fi);	/* skip things */
#else
                FGETC(fi);	/* skip things */
#endif

                MCU_column = 0;
                MCU_row = 0;
                clear_bits();
                reset_prediction();

                /* main MCU processing loop here */
                if (restart_interval) 
                {
                    n_restarts = ceil_div(mx_size * my_size, restart_interval) - 1;
                    leftover = mx_size * my_size - n_restarts * restart_interval;
                    /* final interval may be incomplete */

                    for (i = 0; i < n_restarts; i++) 
                    {
                        for (j = 0; j < restart_interval; j++)
                        {
                            process_MCU(fi);
                        }
                        /* proc till all EOB met */

                        aux = get_next_MK(fi);
                        if (!RST_MK(aux)) 
                        {
#ifdef FILE_IO
                            fprintf(stderr, "%ld:\tERROR:\tLost Sync after interval!\n", ftell(fi));
#else
                            //printf("%d:\tERROR:\tLost Sync after interval!\n", FTELL());
#endif
                            aborted_stream(fi);
                        } 
                        else if (verbose)
                        {
                            //printf("%d:\tINFO:\tFound Restart Marker\n", FTELL());
                        }

                        reset_prediction();
                        clear_bits();
                    }	/* intra-interval loop */
                } 
                else
                {
                    leftover = mx_size * my_size;
                }
                /* process till end of row without restarts */
                for (i = 0; i < leftover; i++)
                {
                    process_MCU(fi);
                }
                in_frame = 0;
                //mk_mon_debug_info(0XFEFE);

                //mk_mon_debug_info(0XFEFE);
                break;

            case EOI_MK:
                //mk_mon_debug_info(0XDEADBEE2);
                if (verbose)
                {
#ifdef FILE_IO
                    fprintf(stderr, "%ld:\tINFO:\tFound the EOI marker!\n", ftell(fi));
#else
                    //printf("%d:\tINFO:\tFound the EOI marker!\n", FTELL());
#endif
                }
                if (in_frame)
                {
                    aborted_stream(fi);
                }
                if (verbose)
                {
#ifdef FILE_IO
                    fprintf(stderr, "\tINFO:\tTotal skipped bytes %d, total stuffers %d\n", passed, stuffers);
#else
                    //printf("\tINFO:\tTotal skipped bytes %d, total stuffers %d\n", passed, stuffers);
#endif
                }
#ifdef FILE_IO
                fclose(fi);
#else
                /*Check if something has to be done!!*/
#endif

#ifdef FILE_IO
                //	write_bmp(file2);
#else
                /*Need to implement the function to write in DDR*/	
                //	write_bmp_to_ddr_1();
                //printf_frame_buffer();
#endif
#ifdef FILE_IO
                free_structures();
#else
                /*No Need to do anything as structures are static*/
                //mk_mon_debug_info(0XDEADBEE1);

                //free_structures();
                //mk_mon_debug_info(0XDEADBEE2);
#endif
                /* signal to core 2 that the FIFO is initialized and can be read from. */
                while(cheap_is_empty(producer) != 1)
                {
#if 0

                    mk_mon_debug_info(producer->readc);
                    mk_mon_debug_info(producer->writec);
#endif
                }

                *fifo_sync_data = 0;
                DMA_SEND_BLOCKING((void *)fifo_sync, (int*)fifo_sync_data, sizeof(int),(void *)mb1_dma0_BASEADDR,DMA_flag);
                return 0;
                break;

            case COM_MK:
                if (verbose)
                {
#ifdef FILE_IO
                    fprintf(stderr, "%ld:\tINFO:\tSkipping comments\n", ftell(fi));
#else
                    //printf("%d:\tINFO:\tSkipping comments\n", FTELL());
#endif
                }
                skip_segment(fi);
                break;

            case 0XD9:
                //case 0XD9:
                if (verbose)
                {
#ifdef FILE_IO
                    fprintf(stderr, "%ld:\tERROR:\tRan out of input data!\n", ftell(fi));
#else
                    //printf("%d:\tERROR:\tRan out of input data!\n", FTELL());
#endif
                }

                aborted_stream(fi);

            default:
                if ((mark & MK_MSK) == APP_MK) 
                {
                    if (verbose)
                    {
#ifdef FILE_IO
                        fprintf(stderr, "%ld:\tINFO:\tSkipping application data\n", ftell(fi));
#else
                        //printf("%d:\tINFO:\tSkipping application data\n", FTELL());
#endif
                    }
                    skip_segment(fi);
                    break;
                }
                if (RST_MK(mark)) 
                {
                    reset_prediction();
                    break;
                }
                /* if all else has failed ... */
#ifdef FILE_IO
                fprintf(stderr, "%ld:\tWARNING:\tLost Sync outside scan, %d!\n", ftell(fi), mark);
#else
                //printf("%d:\tWARNING:\tLost Sync outside scan, %d!\n", FTELL(), mark);
#endif
                aborted_stream(fi);
                break;
        }		/* end switch */
    } while (1);

    return 0;
}

#ifdef FILE_IO
void write_bmp(const char *const file2)
{

	FILE *fpBMP;

	int i, j;

	// Header and 3 bytes per pixel
	unsigned long ulBitmapSize = ceil_div(24*x_size, 32)*4*y_size+54; 
	char ucaBitmapSize[4];

	ucaBitmapSize[3] = (ulBitmapSize & 0xFF000000) >> 24;
	ucaBitmapSize[2] = (ulBitmapSize & 0x00FF0000) >> 16;
	ucaBitmapSize[1] = (ulBitmapSize & 0x0000FF00) >> 8;
	ucaBitmapSize[0] = (ulBitmapSize & 0x000000FF);

	/* Create bitmap file */
	fpBMP = fopen(file2, "wb");
	if (fpBMP == 0)
		return;

	/* Write header */
	/* All values are in big endian order (LSB first) */

	// BMP signature + filesize
	fprintf(fpBMP, "%c%c%c%c%c%c%c%c%c%c", 66, 77, ucaBitmapSize[0],
			ucaBitmapSize[1], ucaBitmapSize[2], ucaBitmapSize[3], 0, 0, 0, 0);

	// Image offset, infoheader size, image width
	fprintf(fpBMP, "%c%c%c%c%c%c%c%c%c%c", 54, 0, 0, 0, 40, 0, 0, 0, (x_size & 0x00FF), (x_size & 0xFF00) >> 8);

	// Image height, number of panels, num bits per pixel
	fprintf(fpBMP, "%c%c%c%c%c%c%c%c%c%c", 0, 0, (y_size & 0x00FF), (y_size & 0xFF00) >> 8, 0, 0, 1, 0, 24, 0);

	// Compression type 0, Size of image in bytes 0 because uncompressed
	fprintf(fpBMP, "%c%c%c%c%c%c%c%c%c%c", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	fprintf(fpBMP, "%c%c%c%c%c%c%c%c%c%c", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	fprintf(fpBMP, "%c%c%c%c", 0, 0, 0, 0);

	for (i = y_size - 1; i >= 0; i--) {
		/* in bitmaps the bottom line of the image is at the
		   beginning of the file */
		for (j = 0; j < x_size; j++) {
			putc(FrameBuffer[3 * (i * x_size + j) + 0], fpBMP);
			putc(FrameBuffer[3 * (i * x_size + j) + 1], fpBMP);
			putc(FrameBuffer[3 * (i * x_size + j) + 2], fpBMP);
		}
		for (j = 0; j < x_size % 4; j++)
			putc(0, fpBMP);
	}

	fclose(fpBMP);
}
#else
#if 0
void write_bmp_to_ddr_1()
{

	volatile unsigned int *fpBMP;

	int i, j,a,b,c,d;

	/* Create bitmap file */
	/* fpBMP = fopen(file2, "wb"); */
	fpBMP = (volatile unsigned int *)(shared_pt_REMOTEADDR+3);


	*fpBMP++ = pack_4char_int(0X1,0X2,0X3);    
	for (i = 0 ; i < 12 ; i++)
	{
		*fpBMP++ = pack_4char_int(0XA,0XB,0XC);    
	}
	*fpBMP++ = pack_4char_int(0X4,0X5,0X6);    
}

#endif
void clear_frame_buffer()
{

	unsigned int *fpBMP;

	int i;

	/* Create bitmap file */
	/* fpBMP = fopen(file2, "wb"); */
	fpBMP = (unsigned int *) mb1_cmemout1_BASEADDR;;

	for(i=0 ; i < 1024; i++) 
    {
	  fpBMP[i] = 0X000000ff;//0X000000ff;  /* Write Red */
	}
	
	for (i = 0 ; i < 768 ; i++)
	{
#if 0		
		for (j = 0 ; j < 1024 ; j++)
		{
		    fpBMP[i * 1024 + j] = 0Xff000000;  /* Write Red */                
		}
#else
        DMA_SEND_BLOCKING((int*)((shared_pt_REMOTEADDR + i * 4096)),(int*)fpBMP, 4096, (void*)mb1_dma1_BASEADDR,DMA_flag);
#endif		
	}
}

#if 0
void printf_frame_buffer()
{

	volatile unsigned int *fpBMP;

	int i, j,a,b,c,d;

	/* Create bitmap file */
	/* fpBMP = fopen(file2, "wb"); */
	fpBMP = (volatile unsigned int *)(shared_pt_REMOTEADDR);


	for (i = 0 ; i < y_size ; i++)
	{
		for (j = 0 ; j < x_size ; j++)
		{
			//mk_mon_debug_info(i);                  
			//mk_mon_debug_info(j);                  
			//mk_mon_debug_info(fpBMP[i * 1024 + j]);                  
		}
	}
}

void write_bmp_to_ddr()
{

	volatile unsigned int *fpBMP;

	int i, j,a,b,c,d;

	/* Header and 3 bytes per pixel */
	unsigned long ulBitmapSize = ceil_div(24*x_size, 32)*4*y_size+54; 
	char ucaBitmapSize[4];

	ucaBitmapSize[3] = (ulBitmapSize & 0xFF000000) >> 24;
	ucaBitmapSize[2] = (ulBitmapSize & 0x00FF0000) >> 16;
	ucaBitmapSize[1] = (ulBitmapSize & 0x0000FF00) >> 8;
	ucaBitmapSize[0] = (ulBitmapSize & 0x000000FF);


	/* Create bitmap file */
	/* fpBMP = fopen(file2, "wb"); */
	fpBMP = (volatile unsigned int *)(shared_pt_REMOTEADDR + 3);


	/* Write header */
	/* All values are in big endian order (LSB first) */

	/* BMP signature + filesize */


#if 0
	*fpBMP++ = 66;	*fpBMP++ = 77; *fpBMP++ = ucaBitmapSize[0];
	*fpBMP++ = ucaBitmapSize[1];*fpBMP++ = ucaBitmapSize[2]; *fpBMP++ = ucaBitmapSize[3];
	*fpBMP++ = 0; *fpBMP++ = 0; fpBMP++ = 0;
	*fpBMP++ = 0; *fpBMP++ = 54;*fpBMP++ = 0;
	*fpBMP++ = 0;*fpBMP++ = 0; *fpBMP++ =40;
	*fpBMP++ =0;*fpBMP++ = 0;*fpBMP++ = 0;
	*fpBMP++ =(x_size & 0x00FF);*fpBMP++ = (x_size & 0xFF00) >> 8; *fpBMP++ = 0 ;
	*fpBMP++ = 0;*fpBMP++ = (y_size & 0x00FF);*fpBMP++ = (y_size & 0xFF00) >> 8;
	*fpBMP++ = 0;*fpBMP++ = 0;*fpBMP++ = 1;
	*fpBMP++ = 0;*fpBMP++ = 24;*fpBMP++ = 0;
	*fpBMP++ =0;*fpBMP++ = 0; *fpBMP++ = 0;
	*fpBMP++ = 0;*fpBMP++ = 0;*fpBMP++ = 0;
	*fpBMP++ = 0;*fpBMP++ = 0;*fpBMP++ = 0;
	*fpBMP++ = 0;*fpBMP++ =0;*fpBMP++ = 0;
	*fpBMP++ = 0;*fpBMP++ = 0;*fpBMP++ = 0;
	*fpBMP++ = 0;*fpBMP++ = 0;*fpBMP++ = 0;
	*fpBMP++ = 0;*fpBMP++ = 0; *fpBMP++ =0;
	*fpBMP++ = 0;*fpBMP++ = 0;*fpBMP++ = 0;
#else
	*fpBMP-- = pack_4char_int(66,77,ucaBitmapSize[0]);
	*fpBMP-- = pack_4char_int(ucaBitmapSize[1],ucaBitmapSize[2],ucaBitmapSize[3]);
	*fpBMP-- = pack_4char_int(0,0,0);
	*fpBMP-- = pack_4char_int(0,54,0);
	*fpBMP-- = pack_4char_int(0,0,40);
	*fpBMP-- = pack_4char_int(0,0,0);
	*fpBMP-- = pack_4char_int((x_size & 0x00FF),(x_size & 0xFF00) >> 8,0) ;
	*fpBMP-- = pack_4char_int(0, (y_size & 0x00FF),(y_size & 0xFF00) >> 8);
	*fpBMP-- = pack_4char_int(0, 0,1 );
	*fpBMP-- = pack_4char_int(0, 24,0);
	*fpBMP-- = pack_4char_int(0, 0, 0);
	*fpBMP-- = pack_4char_int(0, 0, 0);
	*fpBMP-- = pack_4char_int(0, 0, 0);
	*fpBMP-- = pack_4char_int(0, 0, 0);
	*fpBMP-- = pack_4char_int(0, 0, 0);
	*fpBMP-- = pack_4char_int(0, 0, 0);
	*fpBMP-- = pack_4char_int(0, 0, 0);
	*fpBMP-- = pack_4char_int(0, 0, 0);


#endif

#if 1
	//for (i = (y_size - 1); i >= 0; i--) 
	for (i = 0; i < y_size; i++) 
	{
		/* in bitmaps the bottom line of the image is at the
		   beginning of the file */
		for (j = 0; j < x_size; j++) 
		{
			*fpBMP-- = pack_4char_int(FrameBuffer[3 * (i * x_size + j) + 0],FrameBuffer[3*(i*x_size+j)+1],FrameBuffer[3*(i*x_size+j)+2]);
		}
#if 0
		for (j = 0; j < x_size % 4; j++)
			*fpBMP++ =0;
#endif
	}
#else

#endif

}

#endif

unsigned int pack_4char_int(unsigned char a,unsigned char b,unsigned char c)
{
	unsigned int z;
	unsigned int d = 0;
	z = ((a) | (((b) << 8) & 0XFF00) | (((c) << 16) & 0XFF0000) | (((d) << 24) & 0XFF000000));
	return (z);
}

#endif
