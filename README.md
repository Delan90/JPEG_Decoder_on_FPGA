# JPEG_Decoder_on_FPGA

The hardware for this project is [Xilinx ML 605 FPGA](http://www.xilinx.com/publications/prod_mktg/ml605_product_brief.pdf).

![My Unicorn](http://www.xilinx.com/Image/40398-10_201309120145451346.jpg)

The code is working on the [Compsoc platform](http://compsoc.eu/)

The reason choose this paltform is:

Has all the ingredients of modern systems on chip
- Multiple processors
- Direct memory access (DMA) coprocessors
- Heterogeneous distributed shared memory hierarchy
- Network on chip with heterogeneous communication characteristics – Memory-mapped I/O

The design details can be found on the [project report](https://github.com/Delan90/JPEG_Decoder_on_FPGA/blob/master/5KK03%20-%20Group%201%20-%20Final%20Report.pdf)

We also  implemented the [benchmarking code](https://github.com/Delan90/JPEG_Decoder_on_FPGA/tree/master/Benchmarking_demo_code) and [document](https://github.com/Delan90/JPEG_Decoder_on_FPGA/blob/master/5KK03_ESL_Benchmarking_Guidelines_07052014.pdf), which is script written by [bash](https://en.wikipedia.org/wiki/Bash_(Unix_shell)) and the aim of this benchmarking part is to test the performance of the JPEG decoder.
