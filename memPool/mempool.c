/* ***************************************************************************************
    MIT License
    
    Copyright (c) 2026 Dhananjay Pilankar
    
    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:
    
    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.
    
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
 *************************************************************************************** */

#include "mempool.h"
#include <string.h>

/* **************************************************************************
 *              Macros / Defines
 ************************************************************************** */
#define MEM_POOL_OFFSET(st, m)          ((size_t)&(((st *)0)->m))

/* **************************************************************************
 *              Static Constants
 ************************************************************************** */
static const unsigned long memCtxSize = sizeof(t_Mem);
static const unsigned long memSectorCtxSize = sizeof(t_MemSect);

/* **************************************************************************
 *              Function Definitions
 ************************************************************************** */

/* **************************************************************************
 * Function initializes the memory section for future use
 *  pMem        ->  Pointer to the memory fetched using MEM_POOL_ADDR(Name) macro
 *  Size        ->  Size of memory fetched using MEM_POOL_SIZE(Name) macro
 *  SectCnt     ->  Number of Sectors of memory blocks fetched using MEM_POOL_CNT(Name) macro
 *  SectSize    ->  Size of memory sector fetched using MEM_POOL_SECT_SIZE(Name) macro
 * Returns the start address of the current initialized Heap.
 ************************************************************************** */
void *mempool_init(const void *const pMem, const unsigned long Size, const unsigned long SectCnt, const unsigned long SectSize)
{
    void *p_mem = (void *)pMem;
    unsigned long sector = SectSize;
    unsigned long index = 0;

    // Number of sectors of memory
    ((struct s_Mem *)p_mem)->Sec_Cnt = (unsigned long)SectCnt;
    // Size of each sector
    ((struct s_Mem *)p_mem)->Sec_Size = (unsigned long)SectSize;
    // Start of memory sector descriptors
    ((struct s_Mem *)p_mem)->Mem_Desc_Start = (unsigned long)(((char *)p_mem) + memCtxSize);
    // Start of usable memory sectors
    ((struct s_Mem *)p_mem)->Mem_Start = (unsigned long)(((char *)p_mem) + memCtxSize) + (SectCnt * memSectorCtxSize);
    // Number of used sectors of usable memory
    ((struct s_Mem *)p_mem)->Total_Memory = (unsigned long)Size;

    p_mem = (void *)(*((unsigned long *)(((char *)p_mem) + MEM_POOL_OFFSET(t_Mem, Mem_Desc_Start))));

    for(index = 0; index < SectCnt; index++)
    {
        // Preparing sector headers
        // Resetting all the flags from all the sectors
        *((unsigned long *)(((unsigned long)((char *)p_mem)) + (memSectorCtxSize * index) + MEM_POOL_OFFSET(t_MemSect, Flags))) = MEMSECT_FLAGS_NONE;
        // Next sector start address is copied
        *((unsigned long *)(((unsigned long)((char *)p_mem)) + (memSectorCtxSize * index) + MEM_POOL_OFFSET(t_MemSect, pNext))) = ( (((sector * (index * 1)) + memCtxSize + (memSectorCtxSize * SectCnt)) >= Size) ?\
                                                                                                                                    ((unsigned long)(((char *)p_mem))) :\
                                                                                                                                    ((unsigned long)(((char *)p_mem) + (memSectorCtxSize * (index + 1)))) );
        // The buffer concatenated with NULL buffer
        *((unsigned long *)(((unsigned long)((char *)p_mem)) + (memSectorCtxSize * index) + MEM_POOL_OFFSET(t_MemSect, pConcat))) = 0uL;
        // Usable memory start address for current sector
        *((unsigned long *)(((unsigned long)((char *)p_mem)) + (memSectorCtxSize * index) + MEM_POOL_OFFSET(t_MemSect, pMemSect))) = (((unsigned long)((char *)p_mem)) + ((sector * index) + (memSectorCtxSize * SectCnt)));
        // Resetting the read index to 0
        *((unsigned long *)(((unsigned long)((char *)p_mem)) + (memSectorCtxSize * index) + MEM_POOL_OFFSET(t_MemSect, ReadIndex))) = 0uL;
        // Resetting the write index to 0
        *((unsigned long *)(((unsigned long)((char *)p_mem)) + (memSectorCtxSize * index) + MEM_POOL_OFFSET(t_MemSect, WriteIndex))) = 0uL;
    }

    (void)sector;
    (void)index;
    return (void *)pMem;
}

/* **************************************************************************
 * Function allocates the unallocated memory sector for the user
 *  pMem        ->  Pointer to the top of Heap memory fetched using MEM_POOL_ADDR(Name) macro
 * Returns the Sector Pointer if available else returns NULL
 ************************************************************************** */
void *mempool_alloc(const void *const pMem)
{
    unsigned long index = 0;
    unsigned long sect_cnt = ((struct s_Mem *)pMem)->Sec_Cnt;
    void *mem_zero = (void *)((struct s_Mem *)pMem)->Mem_Desc_Start;
    void *mem_ptr = NULL;

    for(index = 0; index < sect_cnt; index++)
    {
        if(*(((unsigned long *)(((char *)mem_zero) + (memSectorCtxSize * index) + MEM_POOL_OFFSET(t_MemSect, Flags)))) == MEMSECT_FLAGS_NONE)
        {
            *((unsigned long *)(((char *)mem_zero) + (memSectorCtxSize * index) + MEM_POOL_OFFSET(t_MemSect, Flags))) = MEMSECT_FLAGS_USED;
            *((unsigned long *)(((char *)mem_zero) + (memSectorCtxSize * index) + MEM_POOL_OFFSET(t_MemSect, pConcat))) = 0uL;
            *((unsigned long *)(((char *)mem_zero) + (memSectorCtxSize * index) + MEM_POOL_OFFSET(t_MemSect, ReadIndex))) = 0uL;
            *((unsigned long *)(((char *)mem_zero) + (memSectorCtxSize * index) + MEM_POOL_OFFSET(t_MemSect, WriteIndex))) = 0uL;
            mem_ptr = (void *)(((unsigned long)((char *)mem_zero)) + (memSectorCtxSize * index));
            break;
        }
    }

    (void)sect_cnt;
    (void)index;
    return mem_ptr;
}

/* **************************************************************************
 * Function frees the allocated sector
 *  pMemSect    ->  Pointer to memory sector descriptor which needs to be marked
 *                  free for the future use.
 * Returns none.
 ************************************************************************** */
void mempool_free(const void *const pMemSect)
{
    unsigned long flags;
    unsigned long sect_buf_size = 0;
    void *p_mem = (void *)pMemSect;

    if(p_mem != NULL)
    {
        flags = MEMSECT_FLAGS_USED;
    }

    while(flags != MEMSECT_FLAGS_NONE)
    {
        flags = *((unsigned long *)(((char *)p_mem) + MEM_POOL_OFFSET(t_MemSect, Flags)));
        *((unsigned long *)(((char *)p_mem) + MEM_POOL_OFFSET(t_MemSect, Flags))) = MEMSECT_FLAGS_NONE;
        *((unsigned long *)(((char *)p_mem) + MEM_POOL_OFFSET(t_MemSect, ReadIndex))) = 0uL;
        *((unsigned long *)(((char *)p_mem) + MEM_POOL_OFFSET(t_MemSect, WriteIndex))) = 0uL;
        sect_buf_size = ((unsigned long)(((t_MemSect *)*((unsigned long *)(((char *)p_mem) + MEM_POOL_OFFSET(t_MemSect, pNext))))->pMemSect)) -\
                        *((unsigned long *)(((char *)p_mem) + MEM_POOL_OFFSET(t_MemSect, pMemSect)));
        
        if(flags & MEMSECT_FLAGS_CONCAT)
        {
            p_mem = (void *)(*((unsigned long *)(((char *)p_mem) + MEM_POOL_OFFSET(t_MemSect, pConcat))));
        }
        else
        {
            flags = MEMSECT_FLAGS_NONE;
        }
        (void)sect_buf_size;
    }
}

/* **************************************************************************
 * Function used for reading data from the memory, everytime you read data
 * the read pointer is incremented
 *  pMemSect    ->  Pointer to memory sector descriptor to be read from read index
 *  pTarget     ->  Pointer to target buffer
 *  TargetSize  ->  Size of the target buffer
 *  ReadCount   ->  Number of bytes to be read
 * Returns the number of bytes copied to target buffer, zero if error
 ************************************************************************** */
unsigned long mempool_readFromIndex(const void *const pMemSect, void *pTarget,\
                                        const unsigned long TargetSize, const unsigned long ReadCount)
{
    void *p_mem = (void *)pMemSect;
    void *p_read = NULL;
    void *p_head = p_mem;
    void *p_out = pTarget;
    unsigned long bytes_read = 0;
    unsigned long read_index = 0;
    unsigned long write_index = 0;
    unsigned long sect_buf_size = 0;
    unsigned long flags = 0;
    unsigned long read_processed = ReadCount;
    unsigned long read_count = 0;

    if(read_processed > *((unsigned long *)(((char *)p_mem) + MEM_POOL_OFFSET(t_MemSect, WriteIndex))))
    {
        read_processed = *((unsigned long *)(((char *)p_mem) + MEM_POOL_OFFSET(t_MemSect, WriteIndex))) -\
                            *((unsigned long *)(((char *)p_mem) + MEM_POOL_OFFSET(t_MemSect, ReadIndex)));
    }

    if((p_mem != NULL) && (pTarget != NULL))
    {
        // Get Read Pointer
        flags = *((unsigned long *)(((char *)p_mem) + MEM_POOL_OFFSET(t_MemSect, Flags)));
        read_index = *((unsigned long *)(((char *)p_mem) + MEM_POOL_OFFSET(t_MemSect, ReadIndex)));
        write_index = *((unsigned long *)(((char *)p_mem) + MEM_POOL_OFFSET(t_MemSect, WriteIndex)));
        sect_buf_size = ((unsigned long)(((t_MemSect *)(*((unsigned long *)(((char *)p_mem) + MEM_POOL_OFFSET(t_MemSect, pNext)))))->pMemSect)) -\
                        *((unsigned long *)(((char *)p_mem) + MEM_POOL_OFFSET(t_MemSect, pMemSect)));
        
        if(read_index >= write_index)
        {
            // No data to be read
            read_processed = 0;
        }
        // Walking towards read index concatenated buffer by buffer
        while((read_index > sect_buf_size) && (read_processed != 0))
        {
            if(flags & MEMSECT_FLAGS_CONCAT)
            {
                read_index -= sect_buf_size;
                p_mem = (void *)(*((unsigned long *)(((char *)p_mem) + MEM_POOL_OFFSET(t_MemSect, pConcat))));
            }
            flags = *((unsigned long *)(((char *)p_mem) + MEM_POOL_OFFSET(t_MemSect, Flags)));
            sect_buf_size = ((unsigned long)(((t_MemSect *)(*((unsigned long *)(((char *)p_mem) + MEM_POOL_OFFSET(t_MemSect, pNext)))))->pMemSect)) -\
                            *((unsigned long *)(((char *)p_mem) + MEM_POOL_OFFSET(t_MemSect, pMemSect)));
        }
        // Read data sector by sector
        while(read_processed > 0)
        {
            bytes_read = 0;
            flags = *((unsigned long *)(((char *)p_mem) + MEM_POOL_OFFSET(t_MemSect, Flags)));
            p_read = (void *)(*((unsigned long *)(((char *)p_mem) + MEM_POOL_OFFSET(t_MemSect, pMemSect))) + read_index);
            sect_buf_size = ((unsigned long)(((t_MemSect *)(*((unsigned long *)(((char *)p_mem) + MEM_POOL_OFFSET(t_MemSect, pNext)))))->pMemSect)) -\
                            *((unsigned long *)(((char *)p_mem) + MEM_POOL_OFFSET(t_MemSect, pMemSect)));
            
            if(TargetSize < read_processed)
            {
                // Reading partial as not enough space in target buffer
                bytes_read = TargetSize;
            }
            else
            {
                // Reading as per read count value
                bytes_read = read_processed;
            }

            if(sect_buf_size < bytes_read)
            {
                bytes_read = sect_buf_size;
            }

            if((sect_buf_size - read_index) < bytes_read)
            {
                bytes_read = sect_buf_size - read_index;
            }

            if((read_count + bytes_read) > TargetSize)
            {
                // Since target buffer is exhausted
                break;
            }

            memcpy(p_out, p_read, bytes_read);
            *((unsigned long *)(((char *)p_head) + MEM_POOL_OFFSET(t_MemSect, ReadIndex))) += bytes_read;
            read_processed -= bytes_read;
            read_count += bytes_read;
            p_out = (void *)((unsigned long)((char *)p_out + bytes_read));

            if(flags & MEMSECT_FLAGS_CONCAT)
            {
                p_mem = (void *)(*((unsigned long *)(((char *)p_mem) + MEM_POOL_OFFSET(t_MemSect, pConcat))));
            }
            else
            {
                break;
            }

            if(read_count >= ReadCount)
            {
                break;
            }

            if(*((unsigned long *)(((char *)p_head) + MEM_POOL_OFFSET(t_MemSect, ReadIndex))) >=\
                *((unsigned long *)(((char *)p_head) + MEM_POOL_OFFSET(t_MemSect, WriteIndex))))
            {
                // Read completed
                break;
            }

            read_index = 0;
        }
    }

    (void)read_index;
    (void)sect_buf_size;
    (void)bytes_read;
    return read_count;
}

/* **************************************************************************
 * Function reads entire buffer data of data limited to size of Target Buffer
 * read pointer index is not incremented while execution data read from Index 0
 *  pMemSect    ->  Pointer to memory sector descriptor to be read from read index
 *  pTarget     ->  Pointer to target buffer
 *  TargetSize  ->  Size of the target buffer
 * Returns the number of bytes copied to target buffer, zero if error
 ************************************************************************** */
unsigned long mempool_readFull(const void *const pMemSect, void *pTarget, const unsigned long TargetSize)
{
    void *p_mem = (void *)pMemSect;
    void *p_read = NULL;
    void *p_out = pTarget;
    unsigned long bytes_read = 0;
    unsigned long sect_buf_size = 0;
    unsigned long flags = 0;
    unsigned long data_present = 1;
    unsigned long read_processed = TargetSize;
    unsigned long read_count = 0;

    if(read_processed > *((unsigned long *)(((char *)p_mem) + MEM_POOL_OFFSET(t_MemSect, WriteIndex))))
    {
        read_processed = *((unsigned long *)(((char *)p_mem) + MEM_POOL_OFFSET(t_MemSect, WriteIndex)));
    }

    if(read_processed == 0)
    {
        // No data writted in the memory
        return 0;
    }

    if((p_mem != NULL) && (pTarget != NULL))
    {
        while(data_present == 1)
        {
            flags = *((unsigned long *)(((char *)p_mem) + MEM_POOL_OFFSET(t_MemSect, Flags)));
            p_read = (void *)(*((unsigned long *)(((char *)p_mem) + MEM_POOL_OFFSET(t_MemSect, pMemSect))));
            sect_buf_size = ((unsigned long)(((t_MemSect *)(*((unsigned long *)(((char *)p_mem) + MEM_POOL_OFFSET(t_MemSect, pNext)))))->pMemSect)) -\
                            *((unsigned long *)(((char *)p_mem) + MEM_POOL_OFFSET(t_MemSect, pMemSect)));
            
            if(read_processed < sect_buf_size)
            {
                bytes_read = read_processed;
            }
            else
            {
                bytes_read = sect_buf_size;
            }

            if((read_count + bytes_read) > TargetSize)
            {
                // Since target buffer is exhausted
                break;
            }

            memcpy(p_out, p_read, bytes_read);
            read_processed -= bytes_read;
            read_count += bytes_read;
            if(flags & MEMSECT_FLAGS_CONCAT)
            {
                p_mem = (void *)(*((unsigned long *)(((char *)p_mem) + MEM_POOL_OFFSET(t_MemSect, pConcat))));
            }
            else
            {
                data_present = 0;
            }
            if(read_processed == 0)
            {
                data_present = 0;
            }
            else
            {
                p_out = (void *)((unsigned long)((char *)p_out + bytes_read));
            }
        }
    }

    (void)bytes_read;
    (void)flags;
    (void)sect_buf_size;
    (void)read_processed;
    return read_count;
}

/* **************************************************************************
 * Function writes data to allocated buffer or adds data to new buffer allocation
 *  pMem        ->  Pointer to the memory fetched using MEM_POOL_ADDR(Name) macro
 *  pMemSect    ->  Pointer to memory sector descriptor where data is to be written
 *  pSouce      ->  Pointer to source buffer from where data needs to be read
 *  SrcSize     ->  Length of the data to be written
 * Returns number of bytes written to the memory sector.
 ************************************************************************** */
unsigned long mempool_writeToIndex(const void *const pMem, const void *const pMemSect,\
                                    const char *const pSource, const unsigned long SrcSize)
{
    void *p_mem = (void *)pMemSect;
    void *p_head = p_mem;
    void *p_write = NULL;
    void *p_next = NULL;
    void *p_src = (void *)pSource;
    unsigned long write_index = 0;
    unsigned long flags = 0;
    unsigned long sect_buf_size = 0;
    unsigned long write_processed = SrcSize;
    unsigned long bytes_to_write = 0;
    unsigned long write_count = 0;

    if((p_mem != NULL) && (pSource != NULL))
    {
        flags = *((unsigned long *)(((char *)p_mem) + MEM_POOL_OFFSET(t_MemSect, Flags)));
        write_index = *((unsigned long *)(((char *)p_head) + MEM_POOL_OFFSET(t_MemSect, WriteIndex)));
        sect_buf_size = ((unsigned long)(((t_MemSect *)(*((unsigned long *)(((char *)p_mem) + MEM_POOL_OFFSET(t_MemSect, pNext)))))->pMemSect)) -\
                            *((unsigned long *)(((char *)p_mem) + MEM_POOL_OFFSET(t_MemSect, pMemSect)));
        
        while((write_index > sect_buf_size) && (write_processed != 0))
        {
            if(flags & MEMSECT_FLAGS_CONCAT)
            {
                write_index -= sect_buf_size;
                p_mem = (void *)(*((unsigned long *)(((char *)p_mem) + MEM_POOL_OFFSET(t_MemSect, pConcat))));
            }
            flags = *((unsigned long *)(((char *)p_mem) + MEM_POOL_OFFSET(t_MemSect, Flags)));
            sect_buf_size = ((unsigned long)(((t_MemSect *)(*((unsigned long *)(((char *)p_mem) + MEM_POOL_OFFSET(t_MemSect, pNext)))))->pMemSect)) -\
                                *((unsigned long *)(((char *)p_mem) + MEM_POOL_OFFSET(t_MemSect, pMemSect)));
        }

        write_count = 0;
        while(write_processed > 0)
        {
            flags = *((unsigned long *)(((char *)p_mem) + MEM_POOL_OFFSET(t_MemSect, Flags)));
            p_write = (void *)((*((unsigned long *)(((char *)p_mem) + MEM_POOL_OFFSET(t_MemSect, pMemSect)))) + write_index);
            sect_buf_size = ((unsigned long)(((t_MemSect *)(*((unsigned long *)(((char *)p_mem) + MEM_POOL_OFFSET(t_MemSect, pNext)))))->pMemSect)) -\
                                *((unsigned long *)(((char *)p_mem) + MEM_POOL_OFFSET(t_MemSect, pMemSect)));
            
            if((signed long)(write_processed - (sect_buf_size - write_index)) > 0)
            {
                if((((unsigned long)(*((unsigned long *)(((char *)p_mem) + MEM_POOL_OFFSET(t_MemSect, pConcat))))) == 0uL) &&\
                    (write_processed > (sect_buf_size - write_index)))
                {
                    // New sector allocation needed
                    p_next = mempool_alloc(pMem);
                    if(p_next != NULL)
                    {
                        // Memory Pool Allocation successful
                        *((unsigned long *)(((char *)p_mem) + MEM_POOL_OFFSET(t_MemSect, Flags))) |= MEMSECT_FLAGS_CONCAT;
                        *((unsigned long *)(((char *)p_mem) + MEM_POOL_OFFSET(t_MemSect, pConcat))) = (unsigned long)p_next;
                    }
                    else
                    {
                        // Memory all consumed
                        break;
                    }
                }
                else
                {
                    p_next = (void *)(*((unsigned long *)(((char *)p_mem) + MEM_POOL_OFFSET(t_MemSect, pConcat))));
                }

                if((sect_buf_size - write_index) > write_processed)
                {
                    bytes_to_write = write_processed;
                }
                else
                {
                    bytes_to_write = sect_buf_size - write_index;
                }
            }
            else
            {
                bytes_to_write = write_processed;
                p_next = (void *)p_mem;
            }

            memcpy(p_write, p_src, bytes_to_write);
            write_processed -= bytes_to_write;
            write_count += bytes_to_write;
            *((unsigned long *)(((char *)p_head) + MEM_POOL_OFFSET(t_MemSect, WriteIndex))) += bytes_to_write;
            p_src = (void *)((unsigned long)(((char *)p_src + bytes_to_write)));
            p_mem = (void *)p_next;
            write_index = 0;
        }
    }

    (void)flags;
    (void)write_index;
    (void)sect_buf_size;
    (void)write_processed;
    (void)bytes_to_write;
    return write_count;
}

/* **************************************************************************
 * Function resets write and read pointers of the allocated memory
 *  pMemSect    ->  Pointer to memory sectors start descriptor who needs to be resetted
 * Returns none.
 ************************************************************************** */
void mempool_resetMemory(const void *const pMemSect)
{
    *((unsigned long *)(((char *)pMemSect) + MEM_POOL_OFFSET(t_MemSect, ReadIndex))) = 0uL;
    *((unsigned long *)(((char *)pMemSect) + MEM_POOL_OFFSET(t_MemSect, WriteIndex))) = 0uL;
}

/* **************************************************************************
 * Function lets you know how much data can be extracted during next read cycle
 *  pMemSect    ->  Pointer to memory sector start descriptor
 * Returns number of bytes can be read by next read instruction.
 ************************************************************************** */
unsigned long mempool_availableData(const void *const pMemSect)
{
    return *((unsigned long *)(((char *)pMemSect) + MEM_POOL_OFFSET(t_MemSect, WriteIndex))) -\
                *((unsigned long *)(((char *)pMemSect) + MEM_POOL_OFFSET(t_MemSect, ReadIndex)));
}

/* **************************************************************************
 * Function counts number of used memory sectors
 *  pMem        ->  Pointer to the top of Heap memory fetched using MEM_POOL_ADDR(Name) macro
 * Returns currently allocated sectors.
 ************************************************************************** */
unsigned long mempool_sectUsed(const void *const pMem)
{
    unsigned long index = 0;
    unsigned long sect_cnt = ((struct s_Mem *)pMem)->Sec_Cnt;
    unsigned long used_sect_count = 0;
    void *mem_zero = (void *)((struct s_Mem *)pMem)->Mem_Desc_Start;

    for(index = 0; index < sect_cnt; index++)
    {
        if(*(((unsigned long *)(((char *)mem_zero) + (memSectorCtxSize * index) + MEM_POOL_OFFSET(t_MemSect, Flags)))) & MEMSECT_FLAGS_USED)
        {
            // Sector in use
            used_sect_count += 1;
        }
    }

    (void)sect_cnt;
    (void)index;
    return used_sect_count;
}

/* **************************************************************************
 * Function responds with percentage of memory used out of allocated on RAM
 *  pMem        ->  Pointer to the top of Heap memory fetched using MEM_POOL_ADDR(Name) macro
 * Returns percentage of active usable memory (double type)
 ************************************************************************** */
double mempool_activeSection(const void *const pMem)
{
    unsigned long total_size = ((struct s_Mem *)pMem)->Total_Memory;
    unsigned long sect_cnt = ((struct s_Mem *)pMem)->Sec_Cnt;
    unsigned long sect_size = ((struct s_Mem *)pMem)->Sec_Size;
    return (((((double)(sect_cnt * sect_size)) * 100.0) / (double)total_size));
}

/* End of mempool.c file */
