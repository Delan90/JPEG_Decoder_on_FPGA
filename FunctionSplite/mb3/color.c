/*-----------------------------------------*/
/* File : color.c, utilities for jfif view */
/* Author : Pierre Guerrier, march 1998	   */
/*-----------------------------------------*/


#include "5kk03-utils.h"
#include "global_memmap.h"
#include "jpeg.h"
#include "comik.h"
#include "benchmark.h"

/* Ensure number is >=0 and <=255			   */
#define Saturate(n)	((n) > 0 ? ((n) < 255 ? (n) : 255) : 0)

extern int DMA_flag; 
/*---------------------------------------*/
/* rules for color conversion:	         */
/*  r = y		+1.402	v	 */
/*  g = y -0.34414u	-0.71414v	 */
/*  b = y +1.772  u			 */
/* Approximations: 1.402 # 7/5 = 1.400	 */
/*		.71414 # 357/500 = 0.714 */
/*		.34414 # 43/125	= 0.344	 */
/*		1.772  = 443/250	 */
/*---------------------------------------*/
/* Approximations: 1.402 # 359/256 = 1.40234 */
/*		.71414 # 183/256 = 0.71484 */
/*		.34414 # 11/32 = 0.34375 */
/*		1.772 # 227/128 = 1.7734 */
/*----------------------------------*/

extern NewCmp Compnent[3];
volatile unsigned int *FrameBuffer = (unsigned int *) mb3_cmemout1_BASEADDR;

void color_conversion(int ddr_offset, PBlock *MCU_buff, int maxCmp)
{
    int i, j,k;
    unsigned char y, cb, cr;
    signed char rcb, rcr;
    long r, g, b;
    long offset;

#if 1
    if(maxCmp == 1)
    {
        int k = 0;
        for(i = 0; i < 8 ; i++) 
        {
            for(j = 0; j < 8 ; j++) 
            {
                *((int *)(FrameBuffer + j)) = (((unsigned char)(MCU_buff[0].linear[k]) << 16) & 0X00FF0000) |
                    (((unsigned char)(MCU_buff[0].linear[k]) << 8) & 0X0000FF00) |
                    (((unsigned char)(MCU_buff[0].linear[k])) & 0X000000FF) ;
                k++;
            } 
            DMA_SEND_BLOCKING((unsigned int*)(shared_pt_REMOTEADDR + (ddr_offset << 2) + (i << 12)), (unsigned int*)(FrameBuffer),8, (void*)mb1_dma1_BASEADDR,DMA_flag);
        }
    }
    else
    {
#endif
        for (i = 0; i < MCU_sy; i++) 
        {	
            /* pixel rows */
            int ip_0 = i >> Compnent[0].T.VDIV;
            int ip_1 = i >> Compnent[1].T.VDIV;
            int ip_2 = i >> Compnent[2].T.VDIV;
            int inv_ndx_0 = Compnent[0].T.IDX + Compnent[0].T.HS * (ip_0 >> 3);
            int inv_ndx_1 = Compnent[1].T.IDX + Compnent[1].T.HS * (ip_1 >> 3);
            int inv_ndx_2 = Compnent[2].T.IDX + Compnent[2].T.HS * (ip_2 >> 3);
            int ip_0_lsbs = ip_0 & 7;
            int ip_1_lsbs = ip_1 & 7;
            int ip_2_lsbs = ip_2 & 7;
            int i_times_MCU_sx = i * MCU_sx;

            for (j = 0; j < MCU_sx; j++) 
            {	
                /* pixel columns */
                int jp_0 = j >> Compnent[0].T.HDIV;
                int jp_1 = j >> Compnent[1].T.HDIV;
                int jp_2 = j >> Compnent[2].T.HDIV;

                y = MCU_buff[inv_ndx_0 + (jp_0 >> 3)].block[ip_0_lsbs][jp_0 & 7];
                cb = MCU_buff[inv_ndx_1 + (jp_1 >> 3)].block[ip_1_lsbs][jp_1 & 7];
                cr = MCU_buff[inv_ndx_2 + (jp_2 >> 3)].block[ip_2_lsbs][jp_2 & 7];

                rcb = cb - 128;
                rcr = cr - 128;

                r = y + ((359 * rcr) >> 8);
                g = y - ((11 * rcb) >> 5) - ((183 * rcr) >> 8);
                b = y + ((227 * rcb) >> 7);

                offset = 3 * (i_times_MCU_sx + j);

                /* note that this is SunRaster color ordering */
                *((unsigned int*)(FrameBuffer + i * MCU_sy + j)) =  ((Saturate(r) << 16) & 0X00FF0000) | 
                    ((Saturate(g) << 8) & 0X0000FF00) | 
                    (Saturate(b) & 0X000000FF);

            }
        }

        for(k = 0 ; k < MCU_sy ; k++)
        {
#if 1
            DMA_SEND_BLOCKING((unsigned int*)(shared_pt_REMOTEADDR + (ddr_offset << 2) + (k << 12)), (unsigned int*)(FrameBuffer + k * MCU_sy),MCU_sx, (void*)mb1_dma1_BASEADDR,DMA_flag);
#endif
        }
#if 1
    }
#endif

}
