# Memory-Pool-Buffer-C-Program-

<div align="justify">
This code helps you create buffer with fixed sizes, every time you allocate a buffer it will give you fixed sized section of memory area. There are function to access these memory area viz. mempool_readFromIndex, mempool_readFull and mempool_writeToIndex these are some of function to access data through these memory sections.

First allocation is necessary to use write function from the memory. Once the head memory is allocated for subsequent write function calls, if the sufficient memory is not available in the buffer, the write function allocates the additional buffer and concatenates to the present memory sector context, that way user just need to maintain the head memory context rest of the concatenated memory has been handled by respective read and write functions.

Code tested with 32-bit C Compiler and works as intended and does not support 64-bit compilation. User can modify the variables and bounds accordingly to make it suitable for 64-bit compiler version.
Anyone is free to suggest those modifications as I do not have time to make those changes.
</div>
