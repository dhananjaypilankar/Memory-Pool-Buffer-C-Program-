#include <stdio.h>
#include <string.h>
#include "./memPool/mempool.h"

// MEM_POOL_DECLARE(Name_Of_Variable, Sector_Count, Buffer_Size_Of_Sector);
MEM_POOL_DECLARE(test, 20, 32);

void *pMemory = NULL;

const char testAlphabetsUpper[] = { "ABCDEFGHIJKLMNOPQRSTUVWXYZ" };
const char testAlphabetsLower[] = { "abcdefghijklmnopqrstuvwxyz" };
const char testNumbers[] = { "1234567890" };
char testRead[1024];

void memPoolOperations(void)
{
    void *p_mem_pool_1 = NULL;
    void *p_mem_pool_2 = NULL;
    unsigned long wrote = 0;
    unsigned long read = 0;
    
    pMemory = mempool_init(MEM_POOL_ADDR(test), MEM_POOL_SIZE(test), MEM_POOL_SECT_CNT(test), MEM_POOL_SECT_SIZE(test));
    printf("Memory Pool Initialized: %d with size: %d\r\n", pMemory, MEM_POOL_SIZE(test));
    p_mem_pool_1 = mempool_alloc(pMemory);
    printf("Memory Pool Allocated: %lu\r\n", (unsigned long)p_mem_pool_1);

    wrote = mempool_writeToIndex(pMemory, p_mem_pool_1, testAlphabetsUpper, strlen((char *)testAlphabetsUpper));
    printf("Data Written in Memory 1: %lu\r\n", wrote);

    p_mem_pool_2 = mempool_alloc(pMemory);
    printf("Memory Pool Allocated: %lu\r\n", (unsigned long)p_mem_pool_2);

    wrote = mempool_writeToIndex(pMemory, p_mem_pool_2, testAlphabetsLower, strlen((char *)testAlphabetsLower));
    printf("Data Written in Memory 2: %lu\r\n", wrote);

    wrote = mempool_writeToIndex(pMemory, p_mem_pool_1, testAlphabetsLower, strlen((char *)testAlphabetsLower));
    printf("Data Written in Memory 1: %lu\r\n", wrote);

    mempool_resetMemory(p_mem_pool_1);

    wrote = mempool_writeToIndex(pMemory, p_mem_pool_1, testAlphabetsLower, strlen((char *)testAlphabetsLower));
    printf("Data Written in Memory 1: %lu\r\n", wrote);

    wrote = mempool_writeToIndex(pMemory, p_mem_pool_1, testNumbers, strlen((char *)testNumbers));
    printf("Data Written in Memory 1: %lu\r\n", wrote);

    memset(testRead, 0, sizeof(testRead));
    read = mempool_readFromIndex(p_mem_pool_1, testRead, sizeof(testRead), 10);
    printf("Data read from Memory 1: %s\r\n", testRead);

    memset(testRead, 0, sizeof(testRead));
    read = mempool_readFromIndex(p_mem_pool_1, testRead, sizeof(testRead), 10);
    printf("Data read from Memory 1: %s\r\n", testRead);

    memset(testRead, 0, sizeof(testRead));
    read = mempool_readFromIndex(p_mem_pool_1, testRead, sizeof(testRead), 10);
    printf("Data read from Memory 1: %s\r\n", testRead);

    memset(testRead, 0, sizeof(testRead));
    read = mempool_readFromIndex(p_mem_pool_1, testRead, sizeof(testRead), 6);
    printf("Data read from Memory 1: %s\r\n", testRead);

    memset(testRead, 0, sizeof(testRead));
    read = mempool_readFull(p_mem_pool_1, testRead, sizeof(testRead));
    printf("Data read from Memory 1: %s\r\n", testRead);

    printf("Total Active Memory: %0.2f%%\r\n", mempool_activeSection(pMemory));

    printf("Total Allocated Sectors: %lu\r\n", mempool_sectUsed(pMemory));

    mempool_free(p_mem_pool_1);
    printf("Total Allocated Sectors after heap 1 free %lu\r\n", mempool_sectUsed(pMemory));
    mempool_free(p_mem_pool_2);
    printf("Total Allocated Sectors after heap free again: %lu\r\n", mempool_sectUsed(pMemory));
}

int main(void)
{
    printf("Memory Allocator Test Begins\r\n******************************\r\n");
    memPoolOperations();
    printf("\r\n******************************\r\nMemory Allocator Test Ends");
    return 0;
}

/* End of Code */

/* 
 * Build syntax
 * 
 * gcc -O0 -I./memPool -g memPool/mempool.c testMemPool.c -o testMemPool
 * 
 * */
