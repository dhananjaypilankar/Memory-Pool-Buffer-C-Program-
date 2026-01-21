#ifndef __MEM_POOL_H__
#define __MEM_POOL_H__

#define MEM_POOL_ALIGN                  4

/* **************************************************************************
                Memory Pool Description
            -------------------------------
            |       Memory Header         |
            -------------------------------
            |     Sector Descriptor 1     |
            -------------------------------
            |               .             |
            |               .             |
            |               .             |
            |               .             |
            -------------------------------
            |     Sector Descriptor n     |
            -------------------------------
            |   Active Memory Sector 1    |
            -------------------------------
            |               .             |
            |               .             |
            |               .             |
            |               .             |
            -------------------------------
            |   Active Memory Sector n    |
            -------------------------------
 ************************************************************************** */

typedef struct s_Mem {      /* Memory Header */
    unsigned long       Mem_Desc_Start;
    unsigned long       Mem_Start;
    unsigned long       Sec_Cnt;
    unsigned long       Sec_Size;
    unsigned long       Total_Memory;
} t_Mem;

typedef struct s_MemSect {  /* Sector Descriptor */
    unsigned long       Flags;
        #define MEMSECT_FLAGS_NONE          0x00uL      // Buffer is free can be allocated for future use
        #define MEMSECT_FLAGS_USED          0x01uL      // Buffer already allocated
        #define MEMSECT_FLAGS_CONCAT        0x10uL      // Concatenated buffer i.e. data is divided in to multiple of them
    struct s_MemSect    *pNext;                         // Linked list pointer
    struct s_MemSect    *pConcat;                       // Next concatenation
    void                *pMemSect;                      // Start of allocated memory
    unsigned long       ReadIndex;                      // Read index
    unsigned long       WriteIndex;                     // Write index
} t_MemSect;

#define MEM_POOL_NAME(Name)                             mem_##Name
#define MEM_POOL_ADDR(Name)                             (char *)&mem_##Name
#define MEM_POOL_SIZE(Name)                             (unsigned long)sizeof(MEM_POOL_NAME(Name))
#define MEM_POOL_SECT_CNT(Name)                         mem_sect_##Name
#define MEM_POOL_SECT_SIZE(Name)                        mem_sect_size_##Name
#define MEM_POOL_CREATE(Name, Size)                     char Name[Size];
#define MEM_POOL_DECLARE(Name, Sectors, Bytes)          MEM_POOL_CREATE( mem_##Name, ( ( ( (sizeof(t_Mem) +\
                                                                    ( Sectors * ( sizeof(t_MemSect) + Bytes ) ) ) + ( MEM_POOL_ALIGN - 1 ) ) ) &\
                                                                    ( ~( MEM_POOL_ALIGN - 1) ) ) ); \
                                                            unsigned long mem_sect_##Name = Sectors;\
                                                            unsigned long mem_sect_size_##Name = Bytes;


/* **************************************************************************
 * Function initializes the memory section for future use
 *  pMem        ->  Pointer to the memory fetched using MEM_POOL_ADDR(Name) macro
 *  Size        ->  Size of memory fetched using MEM_POOL_SIZE(Name) macro
 *  SectCnt     ->  Number of Sectors of memory blocks fetched using MEM_POOL_CNT(Name) macro
 *  SectSize    ->  Size of memory sector fetched using MEM_POOL_SECT_SIZE(Name) macro
 * Returns the start address of the current initialized Heap.
 ************************************************************************** */
void *mempool_init(const void *const pMem, const unsigned long Size, const unsigned long SectCnt, const unsigned long SectSize);

/* **************************************************************************
 * Function allocates the unallocated memory sector for the user
 *  pMem        ->  Pointer to the top of Heap memory fetched using MEM_POOL_ADDR(Name) macro
 * Returns the Sector Pointer if available else returns NULL
 ************************************************************************** */
void *mempool_alloc(const void *const pMem);

/* **************************************************************************
 * Function frees the allocated sector
 *  pMemSect    ->  Pointer to memory sector descriptor which needs to be marked
 *                  free for the future use.
 * Returns none.
 ************************************************************************** */
void mempool_free(const void *const pMemSect);

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
                                        const unsigned long TargetSize, const unsigned long ReadCount);

/* **************************************************************************
 * Function reads entire buffer data of data limited to size of Target Buffer
 * read pointer index is not incremented while execution data read from Index 0
 *  pMemSect    ->  Pointer to memory sector descriptor to be read from read index
 *  pTarget     ->  Pointer to target buffer
 *  TargetSize  ->  Size of the target buffer
 * Returns the number of bytes copied to target buffer, zero if error
 ************************************************************************** */
unsigned long mempool_readFull(const void *const pMemSect, void *pTarget, const unsigned long TargetSize);

/* **************************************************************************
 * Function writes data to allocated buffer or adds data to new buffer allocation
 *  pMem        ->  Pointer to the memory fetched using MEM_POOL_ADDR(Name) macro
 *  pMemSect    ->  Pointer to memory sector descriptor where data is to be written
 *  pSouce      ->  Pointer to source buffer from where data needs to be read
 *  SrcSize     ->  Length of the data to be written
 * Returns number of bytes written to the memory sector.
 ************************************************************************** */
unsigned long mempool_writeToIndex(const void *const pMem, const void *const pMemSect,\
                                    const char *const pSource, const unsigned long SrcSize);

/* **************************************************************************
 * Function resets write and read pointers of the allocated memory
 *  pMemSect    ->  Pointer to memory sectors start descriptor who needs to be resetted
 * Returns none.
 ************************************************************************** */
void mempool_resetMemory(const void *const pMemSect);

/* **************************************************************************
 * Function lets you know how much data can be extracted during next read cycle
 *  pMemSect    ->  Pointer to memory sector start descriptor
 * Returns number of bytes can be read by next read instruction.
 ************************************************************************** */
unsigned long mempool_availableData(const void *const pMemSect);

/* **************************************************************************
 * Function counts number of used memory sectors
 *  pMem        ->  Pointer to the top of Heap memory fetched using MEM_POOL_ADDR(Name) macro
 * Returns currently allocated sectors.
 ************************************************************************** */
unsigned long mempool_sectUsed(const void *const pMem);

/* **************************************************************************
 * Function responds with percentage of memory used out of allocated on RAM
 *  pMem        ->  Pointer to the top of Heap memory fetched using MEM_POOL_ADDR(Name) macro
 * Returns percentage of active usable memory (double type)
 ************************************************************************** */
double mempool_activeSection(const void *const pMem);

#endif                  /* __MEM_POOL_H__ */
