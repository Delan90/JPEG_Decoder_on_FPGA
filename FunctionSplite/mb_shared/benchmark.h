#ifndef BENCHMARK_H
#define BENCHMARK_H

#define CMEM_IN_SIZE  0X4000
#define CMEM_OUT_SIZE 0X1000
#define DDR_pt_REMOTEADDR (shared_pt_REMOTEADDR+ 4*1024*1024 + 4*1024*768)

#define PATTERN1 0XAB
#define PATTERN2 0XCD
#define PATTERN3 0XEF



#define DMA_SEND_BLOCKING(_dest,_src,_size,_channel,_dma_channel_status) \
{hw_dma_send_addr(_dest,_src,_size,_channel);\
 _dma_channel_status |= (_channel == 0x000F0000) ?  0XF : 0XF0;}
#define DMA_RECEIVE_BLOCKING(_dest,_src,_size,_channel,_dma_channel_status) \
{hw_dma_receive_addr(_dest,_src,_size,_channel);\
 _dma_channel_status |= (_channel == 0x000F0000) ?  0XF : 0XF0;}
 #define DMA_SEND_NON_BLOCKING(_dest,_src,_size,_channel,_dma_channel_status) \
{hw_dma_send_non_block_addr(_dest,_src,_size,_channel);\
 _dma_channel_status |= (_channel == 0x000F0000) ?  0XF : 0XF0;}
#define DMA_RECEIVE_NON_BLOCKING(_dest,_src,_size,_channel,_dma_channel_status)\
{hw_dma_receive_non_block_addr(_dest,_src,_size,_channel);\
 _dma_channel_status |= (_channel == 0x000F0000) ?  0XF : 0XF0;}
 
 
#define MY_MK_MALLOC(__ptr,__type,__size)\
{\
   int *intr_ptr,*tempptr,tmp;\
   int new_size; \
   new_size = ((__size) * sizeof(__type))+ (((__size) * sizeof(__type)) & 3) + 2 * sizeof(int);\
   tempptr = mk_malloc(new_size);\
   if(tempptr == NULL) mk_mon_error(0XE1,0XDEAD);\
   intr_ptr = (int *)tempptr;\
   intr_ptr[0] = ((new_size>>2)- 1);\
   tmp = (int)tempptr + sizeof(int);\
   __ptr = (__type *) (tmp);\
   intr_ptr[(new_size >> 2) - 1] = 0XDEADBABE;\
   bench_dyna_mem_size += ((__size) * sizeof(__type));\
}

#define MY_MK_CALLOC(__ptr,__type,__size)\
{\
   int *intr_ptr,*tempptr,tmp;\
   int new_size; \
   new_size = ((__size) * sizeof(__type))+ (((__size) * sizeof(__type)) & 3) + 2 * sizeof(int);\
   tempptr = mk_calloc(new_size);\
   if(tempptr == NULL) mk_mon_error(0XE1,0XDEAD);\
   intr_ptr = (int *)tempptr;\
   intr_ptr[0] = ((new_size>>2)- 1);\
   tmp = (int)tempptr + sizeof(int);\
   __ptr = (__type *) (tmp);\
   intr_ptr[(new_size >> 2) - 1] = 0XDEADBABE;\
   bench_dyna_mem_size += ((__size) * sizeof(__type));\
}

#define MY_FREE(__ptr) \
{ \
  int *intr_ptr,new_size,tmp;\
  tmp = (int)__ptr - sizeof(int);\
  intr_ptr = (int *)(tmp);\
  new_size = intr_ptr[0];\
  if(intr_ptr[new_size] != 0XDEADBABE) mk_mon_error(0XE2,0XDEAD);\
  mk_free(intr_ptr);\
}
 
#endif
