/*
 * =====================================================================================
 *
 *	Filename:  		sma.c
 *
 *  Description:	Base code for Assignment 3 for ECSE-427 / COMP-310
 *
 *  Version:  		1.0
 *  Created:  		6/11/2020 9:30:00 AM
 *  Revised:  		-
 *  Compiler:  		gcc
 *
 *  Author:  		Mohammad Mushfiqur Rahman
 *
 *  Instructions:   Please address all the "TODO"s in the code below and modify
 * 					them accordingly. Feel free to modify the "PRIVATE" functions.
 * 					Don't modify the "PUBLIC" functions (except the TODO part), unless
 * 					you find a bug! Refer to the Assignment Handout for further info.
 * =====================================================================================
 */

/* Includes */
#include "sma.h" // Please add any libraries you plan to use inside this file
#include <limits.h>
#include <stdio.h>

/* Definitions*/
#define MAX_TOP_FREE (128 * 1024) // Max top free block size = 128 Kbytes
//	TODO: Change the Header size if required
#define FREE_BLOCK_HEADER_SIZE 2 * sizeof(char *) + sizeof(int) // Size of the Header in a free memory block
#define META_SIZE sizeof(block_meta)
//	TODO: Add constants here
#define PROGRAM_BREAK sbrk(0)

typedef enum //	Policy type definition
{
  WORST,
  NEXT
} Policy;

char *sma_malloc_error;
block_meta *current;                  // pointing to the current head
block_meta *freeListHeadBlock;        // The block to the HEAD of the DDL
block_meta *freeListTailBlock;        // The block to the HEAD of the DDL
void *freeListHead = NULL;            //	The pointer to the HEAD of the doubly linked free memory list
void *freeListTail = NULL;            //	The pointer to the TAIL of the doubly linked free memory list
unsigned long totalAllocatedSize = 0; //	Total Allocated memory in Bytes
unsigned long totalFreeSize = 0;      //	Total Free memory in Bytes in the free memory list
Policy currentPolicy = WORST;         //	Current Policy
//	TODO: Add any global variables here
int freeing;

/*
 * =====================================================================================
 *	Public Functions for SMA
 * =====================================================================================
 */

/*
 *	Funcation Name: sma_malloc
 *	Input type:		int
 * 	Output type:	void*
 * 	Description:	Allocates a memory block of input size from the heap, and returns a
 * 					pointer pointing to it. Returns NULL if failed and sets a global error.
 */
void *sma_malloc(int size)
{
  freeing = 0;
  void *pMemory = NULL;
  // Checks if the free list is empty
  if (freeListHead == NULL)
  {
    // Allocate memory by increasing the Program Break
    pMemory = allocate_pBrk(size);
  }
  // If free list is not empty
  else
  {
    // Allocate memory from the free memory list
    pMemory = allocate_freeList(size);

    // If a valid memory could NOT be allocated from the free memory list
    if (pMemory == (void *)-2)
    {
      // Allocate memory by increasing the Program Break
      pMemory = allocate_pBrk(size);
    }
  }

  // Validates memory allocation
  if (pMemory < 0 || pMemory == NULL)
  {
    sma_malloc_error = "Error: Memory allocation failed!";
    return NULL;
  }

  // Updates SMA Info
  totalAllocatedSize += size;
  return pMemory;
}

/*
 *	Funcation Name: sma_free
 *	Input type:		void*
 * 	Output type:	void
 * 	Description:	Deallocates the memory block pointed by the input pointer
 */
void sma_free(void *ptr)
{
  //	Checks if the ptr is NULL - bp = ptr
  if (ptr == NULL)
  {
    puts("Error: Attempting to free NULL!");
  }
  //	Checks if the ptr is beyond Program Break
  else if (ptr < PROGRAM_BREAK)
  {
    //	Adds the block to the free memory list
    block_meta *ptr_block = get_block_ptr(ptr);

    freeing = 1;
    add_block_freeList(ptr_block); // top 984 of this is already free | need to free bottom 40
    // i.e. the brk() (top) of the allocated block is sbrk(0 - 984)
  }
}

/*
 *	Funcation Name: sma_mallopt
 *	Input type:		int
 * 	Output type:	void
 * 	Description:	Specifies the memory allocation policy
 */
void sma_mallopt(int policy)
{
  // Assigns the appropriate Policy
  if (policy == 1)
  {
    currentPolicy = WORST;
  }
  else if (policy == 2)
  {
    currentPolicy = NEXT;
  }
}

/*
 *	Funcation Name: sma_mallinfo
 *	Input type:		void
 * 	Output type:	void
 * 	Description:	Prints statistics about current memory allocation by SMA.
 */
void sma_mallinfo()
{
  //	Finds the largest Contiguous Free Space (should be the largest free block)
  int largestFreeBlock = get_largest_freeBlock();
  char str[60];

  //	Prints the SMA Stats
  sprintf(str, "Total number of bytes allocated: %lu", totalAllocatedSize);
  puts(str);
  sprintf(str, "Total free space: %lu", totalFreeSize);
  puts(str);
  sprintf(str, "Size of largest contigious free space (in bytes): %d", largestFreeBlock);
  puts(str);
}

/*
 *	Funcation Name: sma_realloc
 *	Input type:		void*, int
 * 	Output type:	void*
 * 	Description:	Reallocates memory pointed to by the input pointer by resizing the
 * 					memory block according to the input size.
 */
void *sma_realloc(void *ptr, int size)
{
  // TODO: 	Should be similar to sma_malloc, except you need to check if the pointer address had been previously allocated.
  // Hint:	Check if you need to expand or contract the memory. If new size is smaller, then
  //			chop off the current allocated memory and add to the free list. If new size is bigger
  //			then check if there is sufficient adjacent free space to expand, otherwise find a new block
  //			like sma_malloc.
  //			Should not accept a NULL pointer, and the size should be greater than 0.
  if (ptr != 0)
  {
    sma_malloc(size);
  }
  else if (ptr != 0 && get_block_ptr(ptr) != NULL)
  {
    sma_free(ptr);
    sma_malloc(size);
  }
}

/*
 * =====================================================================================
 *	Private Functions for SMA
 * =====================================================================================
 */

/*
 *	Funcation Name: allocate_pBrk
 *	Input type:		int
 * 	Output type:	void*
 * 	Description:	Allocates memory by increasing the Program Break
 */
void *allocate_pBrk(int size)
{
  void *newBlock = NULL;
  int excessSize;
  block_meta *block;

  //	TODO: 	Allocate memory by incrementing the Program Break by calling sbrk() or brk()
  //	Hint:	Getting an exact "size" of memory might not be the best idea. Why?
  //			Also, if you are getting a larger memory, you need to put the excess in the free list
  excessSize = size - META_SIZE; // = 984

  block = PROGRAM_BREAK;      // Add the block to the bottom of the 1024
  newBlock = sbrk(META_SIZE); // request this much space in the heap
  if (block->block == (void *)-1)
  {
    return NULL; // sbrk failed.
  }
  block->block = newBlock; // Pointer to the current break to the heap

  if (size == 0)
    return (NULL);

  //	Allocates the Memory Block
  allocate_block(block, size, excessSize, 0);
  return newBlock;
}

/*
 *	Funcation Name: allocate_freeList
 *	Input type:		int
 * 	Output type:	void*
 * 	Description:	Allocates memory from the free memory list
 */
void *allocate_freeList(int size)
{
  void *pMemory = NULL;

  if (currentPolicy == WORST)
  {
    // Allocates memory using Worst Fit Policy

    pMemory = allocate_worst_fit(size);
  }
  else if (currentPolicy == NEXT)
  {
    // Allocates memory using Next Fit Policy
    pMemory = allocate_next_fit(size);
  }
  else
  {
    pMemory = NULL;
  }

  return pMemory;
}

/*
 *	Funcation Name: allocate_worst_fit
 *	Input type:		int
 * 	Output type:	void*
 * 	Description:	Allocates memory using Worst Fit from the free memory list
 */
void *allocate_worst_fit(int size)
{
  void *worstBlock = NULL;
  int excessSize;
  int blockFound = 0;
  int max = freeListHeadBlock->size;

  block_meta *worst = PROGRAM_BREAK;

  //	TODO: 	Allocate memory by using Worst Fit Policy
  //	Hint:	Start off with the freeListHead and iterate through the entire list to get the largest block
  block_meta *head = freeListHeadBlock; /* set head to the beginning of the list */
  while (head != NULL)                  /* Iterate through the entire list to find the largest block*/
  {
    if (head->size >= max && head->size >= size)
    {
      worst = head->prev;
      worstBlock = head->prev->block;
      blockFound = 1;
      max = head->size;
    }
    head = head->next;
  }

  excessSize = max - size; // Ex: want to allocate 1024 bytes from 500 ==> excess = 1500
  worst->block = worstBlock;
  //	Checks if appropriate block is found.
  if (blockFound)
  {
    //	Allocates the Memory
    allocate_block(worst, size, excessSize, 1);
  }
  else
  {
    //	Assigns invalid address if appropriate block not found in free list
    worstBlock = (void *)-2;
  }
  return worstBlock;
}

/*
 *	Funcation Name: allocate_next_fit
 *	Input type:		int
 * 	Output type:	void*
 * 	Description:	Allocates memory using Next Fit from the free memory list
 */
void *allocate_next_fit(int size)
{
  void *bestBlock = NULL;
  int excessSize;
  int blockFound = 0;
  block_meta *best = PROGRAM_BREAK;
  int min = INT_MAX;

  //	TODO: 	Allocate memory by using Next Fit Policy
  //	Hint:	Start off with the freeListHead, and keep track of the current position in the free memory list.
  //			The next time you allocate, it should start from the current position.
  block_meta *head = freeListHeadBlock; /* set head to the beginning of the list */
  while (head != NULL)                  /* Iterate through the entire list to find the largest block*/
  {

    if (head->size <= min && head->size >= size)
    {
      bestBlock = head->prev->block;
      blockFound = 1;
      min = head->size;
      best->next = head->next;
      best->prev = head->prev;
    }
    head = head->next;
    if (head->next != NULL && head == head->next)
    {
      break;
    }
  }
  excessSize = min - size;

  //	Checks if appropriate found is found.
  if (blockFound)
  {
    //	Allocates the Memory Block
    allocate_block(best, size, excessSize, 1);
  }
  else
  {
    //	Assigns invalid address if appropriate block not found in free list
    bestBlock = (void *)-2;
  }

  return bestBlock;
}

/*
 *	Funcation Name: allocate_block
 *	Input type:		void*, int, int, int
 * 	Output type:	void
 * 	Description:	Performs routine operations for allocating a memory block
 */
void allocate_block(block_meta *newBlock, int size, int excessSize, int fromFreeList)
{
  void *excessFreeBlock; //	pointer for any excess free block
  int addFreeBlock;

  // 	Checks if excess free size is big enough to be added to the free memory list
  //	Helps to reduce external fragmentation

  //	TODO: Adjust the condition based on your Head and Tail size (depends on your TAG system)
  //	Hint: Might want to have a minimum size greater than the Head/Tail sizes
  int minimum;

  if (freeListHead != NULL)
  {
    minimum = META_SIZE;
  }
  else
  {
    minimum = FREE_BLOCK_HEADER_SIZE;
  }

  addFreeBlock = excessSize >= minimum;
  //	If excess free size is big enough
  if (addFreeBlock) // Want to add a free block
  {
    //	TODO: Create a free block using the excess memory size, then assign it to the Excess Free Block
    block_meta *excess_free_block = PROGRAM_BREAK; // on top of the 40 from the block before
    excess_free_block->block = excessFreeBlock;
    excess_free_block->size = excessSize; // 984 bytes || max - allocation

    //	Checks if the new block was allocated from the free memory list

    if (fromFreeList)
    {

      //	Removes new block since allocated and adds the excess free block to the free list
      // (i,e splits the free block in allocated and free)
      replace_block_freeList(newBlock, excess_free_block);
    }
    else
    {
      //	Adds excess free block to the free list
      add_block_freeList(excess_free_block);
    }
  }
  //	Otherwise add the excess memory to the new block
  else // Remove the block from freeList
  {
    //	TODO: Add excessSize to size and assign it to the new Block

    //	Checks if the new block was allocated from the free memory list
    if (fromFreeList)
    {
      //	Removes the new block from the free list
      remove_block_freeList(newBlock);
    }
  }
}

/*
 *	Funcation Name: replace_block_freeList
 *	Input type:		void*, void*
 * 	Output type:	void
 * 	Description:	Replaces old block with the new block in the free list
 */
void replace_block_freeList(block_meta *oldBlock, block_meta *newBlock)
{
  //	TODO: Replace the old block with the new block
  newBlock->next = oldBlock->next;
  newBlock->prev = oldBlock->prev;

  if (oldBlock->prev != NULL)
  {
    oldBlock->prev->next = newBlock;
  }
  if (oldBlock->next != NULL)
  {
    oldBlock->next->prev = newBlock;
  }

  //	Updates SMA info
  totalAllocatedSize += (oldBlock->size - newBlock->size);
  totalFreeSize += (newBlock->size - oldBlock->size);
}

/*
 *	Funcation Name: add_block_freeList
 *	Input type:		void*
 * 	Output type:	void
 * 	Description:	Adds a memory block to the the free memory list
 */
void add_block_freeList(block_meta *excessFreeBlock)
{
  //	TODO: 	Add the block to the free list
  //	Hint: 	You could add the free block at the end of the list, but need to check if there
  //			exits a list. You need to add the TAG to the list.
  //			Also, you would need to check if merging with the "adjacent" blocks is possible or not.
  //			Merging would be tideous. Check adjacent blocks, then also check if the merged
  //			block is at the top and is bigger than the largest free block allowed (128kB).
  if (excessFreeBlock->size > MAX_TOP_FREE)
  {
  }
  else
  {
    if (freeListHead == NULL)
    {
      current = PROGRAM_BREAK;
      excessFreeBlock->block = sbrk(984); // Requesting size amount of space in the heap
      // INIT the tail of the list
      excessFreeBlock->next = NULL;
      excessFreeBlock->prev = NULL;
      excessFreeBlock->free = 1;
      freeListHead = excessFreeBlock->block; // HEAD of freelist
      freeListTail = excessFreeBlock->block; // TAIL of freelist
      freeListHeadBlock = excessFreeBlock;
      current = excessFreeBlock;
      freeListTailBlock = excessFreeBlock;
    }
    else if (freeing == 1)
    {
      // Allocate to the block before = two blocks (1024) - the prevs allocated block (40)
      excessFreeBlock->prev->block += excessFreeBlock->size + META_SIZE;
      excessFreeBlock->prev->size += excessFreeBlock->size + META_SIZE;
      if (excessFreeBlock->next != NULL)
      {
        excessFreeBlock->next->prev = excessFreeBlock->prev;
      }
      if (excessFreeBlock->prev != NULL)
      {
        excessFreeBlock->prev->next = excessFreeBlock->next;
      }
      freeing = 0;
    }
    else
    {
      // TODO: if not from sma_Free, sbrk(984) else sbrk(40)
      excessFreeBlock->block = sbrk(excessFreeBlock->size);
      excessFreeBlock->next = NULL;              // next == null
      excessFreeBlock->prev = freeListTailBlock; // the new block prev is going to point to the prev free block
      excessFreeBlock->free = 1;                 // It is a free block [tag]

      freeListTailBlock->next = excessFreeBlock; // the current blocks next points to the block-to-be
      freeListTailBlock = excessFreeBlock;       // move the current block to the new block
    }

    //	Updates SMA info
    totalAllocatedSize -= excessFreeBlock->size;
    totalFreeSize += excessFreeBlock->size;
  }
}

/*
 *	Funcation Name: remove_block_freeList
 *	Input type:		void*
 * 	Output type:	void
 * 	Description:	Removes a memory block from the the free memory list
 */
void remove_block_freeList(void *block)
{
  //	TODO: 	Remove the block from the free list
  //	Hint: 	You need to update the pointers in the free blocks before and after this block.
  //			You also need to remove any TAG in the free block.

  //	Updates SMA info
  totalAllocatedSize += get_blockSize(block);
  totalFreeSize -= get_blockSize(block);
}

/*
 *	Funcation Name: get_blockSize
 *	Input type:		void*
 * 	Output type:	int
 * 	Description:	Extracts the Block Size
 */
int get_blockSize(void *ptr)
{
  int *pSize;

  //	Points to the address where the Length of the block is stored
  pSize = (int *)ptr;
  pSize--;

  //	Returns the deferenced size
  return *(int *)pSize;
}

/*
 *	Funcation Name: get_largest_freeBlock
 *	Input type:		void
 * 	Output type:	int
 * 	Description:	Extracts the largest Block Size
 */
int get_largest_freeBlock()
{
  int largestBlockSize = 0;

  block_meta *head = freeListHeadBlock;
  while (head != NULL)
  {
    if (head->size > largestBlockSize)
    {
      largestBlockSize = head->size;
    }
    head = head->next;
    if (head->next != NULL && head == head->next)
    {
      break;
    }
  }

  return largestBlockSize;
}

/*
* When you call pbrk the space that is included in pbrk in the heap is made of blocks and the first time you make an allocation, you will have 2 blocks (allocated and free block)
* Initially that free block will be head and tail of list. The bloc needs to have tags, and needs a size tag (tells how big it is). For the free list, the block needs prev and next tags - also both blocks need a bit to say if it's allocated or not
* The blocks are spaces in memory, not a struct. The way you keep track is by updating and accessing tags by moving a pointer around
*/

block_meta *get_block_ptr(void *ptr)
{
  block_meta *head = freeListHeadBlock;
  while (head != NULL)
  {
    if (head->block == ptr || head->block == ptr + META_SIZE) // 40 for the size of the block
    {
      return head;
    }
    head = head->next;
    if (head->next != NULL && head == head->next)
    {
      break;
    }
  }
  return NULL;
}
