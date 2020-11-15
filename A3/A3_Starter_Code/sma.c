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

/* Definitions*/
#define MAX_TOP_FREE (128 * 1024) // Max top free block size = 128 Kbytes
//	TODO: Change the Header size if required
#define FREE_BLOCK_HEADER_SIZE 2 * sizeof(char *) + sizeof(int) // Size of the Header in a free memory block
#define META_SIZE sizeof(block_meta)
//	TODO: Add constants here
#define META_SIZE sizeof(struct block_meta)

typedef enum //	Policy type definition
{
  WORST,
  NEXT
} Policy;

char *sma_malloc_error;
block_meta *current;                  // pointing to the current head
block_meta *freeListHeadBlock;        // The block to the HEAD of the DDL
void *freeListHead = NULL;            //	The pointer to the HEAD of the doubly linked free memory list
void *freeListTail = NULL;            //	The pointer to the TAIL of the doubly linked free memory list
unsigned long totalAllocatedSize = 0; //	Total Allocated memory in Bytes
unsigned long totalFreeSize = 0;      //	Total Free memory in Bytes in the free memory list
Policy currentPolicy = WORST;         //	Current Policy
//	TODO: Add any global variables here

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
  else if (ptr > sbrk(0))
  {
    puts("Error: Attempting to free unallocated space!");
  }
  else
  {
    //	Adds the block to the free memory list
    block_meta *ptr_block;
    ptr_block->block = ptr;
    add_block_freeList(ptr_block); // coalesce
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
  excessSize = size + sizeof(newBlock);

  block = sbrk(0);
  block->block = newBlock; // Pointer to the current break to the heap

  void *request = sbrk(size + META_SIZE + excessSize); // request this much space in the heap
  if (request == (void *)-1)
  {
    return NULL; // sbrk failed.
  }

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

  block_meta *head = freeListHeadBlock; /* set head to the beginning of the list */
  block_meta *worst = sbrk(0);

  void *request = sbrk(size + META_SIZE);
  if (request == (void *)-1)
  {
    return NULL; // sbrk failed.
  }

  //	TODO: 	Allocate memory by using Worst Fit Policy
  //	Hint:	Start off with the freeListHead and iterate through the entire list to get the largest block
  while (head != NULL) /* Iterate through the entire list to find the largest block*/
  {
    if (head->size >= size)
    {
      worstBlock = head->block;
      blockFound = 1;
    }
    head = head->next;
  }

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
  void *nextBlock = NULL;
  int excessSize;
  int blockFound = 0;
  block_meta *block;
  block->block = nextBlock;

  //	TODO: 	Allocate memory by using Next Fit Policy
  //	Hint:	Start off with the freeListHead, and keep track of the current position in the free memory list.
  //			The next time you allocate, it should start from the current position.

  //	Checks if appropriate found is found.
  if (blockFound)
  {
    //	Allocates the Memory Block
    allocate_block(block, size, excessSize, 1);
  }
  else
  {
    //	Assigns invalid address if appropriate block not found in free list
    nextBlock = (void *)-2;
  }

  return nextBlock;
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
    minimum = get_blockSize(freeListHead);
  }
  else
  {
    minimum = FREE_BLOCK_HEADER_SIZE;
  }
  addFreeBlock = excessSize > minimum;
  //	If excess free size is big enough
  if (addFreeBlock)
  {
    //	TODO: Create a free block using the excess memory size, then assign it to the Excess Free Block

    // if (freeListHead != NULL)
    // {
    //   puts("---------------HIT free list head = null---------------\n");
    //   printf("-----From free list %d--------\n", fromFreeList);
    // }
    block_meta *free_block = sbrk(0);
    void *request = sbrk(excessSize + META_SIZE);

    free_block->block = excessFreeBlock;
    free_block->size = size;

    //	Checks if the new block was allocated from the free memory list

    if (fromFreeList)
    {
      //	Removes new block and adds the excess free block to the free list
      replace_block_freeList(newBlock, free_block);
    }
    else
    {
      //	Adds excess free block to the free list
      add_block_freeList(free_block);
    }
  }
  //	Otherwise add the excess memory to the new block
  else
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
  block_meta *head = freeListHeadBlock;
  while (head != NULL)
  {
    if (head->block == oldBlock->block)
    {
      head->block = newBlock->block;
    }
    head = head->next;
  }

  //	Updates SMA info
  totalAllocatedSize += (get_blockSize(oldBlock) - get_blockSize(newBlock));
  totalFreeSize += (get_blockSize(newBlock) - get_blockSize(oldBlock));
}

/*
 *	Funcation Name: add_block_freeList
 *	Input type:		void*
 * 	Output type:	void
 * 	Description:	Adds a memory block to the the free memory list
 */
void add_block_freeList(block_meta *excessFreeBlock) // same as coalesce()
{
  //	TODO: 	Add the block to the free list
  //	Hint: 	You could add the free block at the end of the list, but need to check if there
  //			exits a list. You need to add the TAG to the list.
  //			Also, you would need to check if merging with the "adjacent" blocks is possible or not.
  //			Merging would be tideous. Check adjacent blocks, then also check if the merged
  //			block is at the top and is bigger than the largest free block allowed (128kB).
  // if (freeListHead != NULL)
  // {
  //   puts("NULL\n");
  // }
  if (freeListHead == NULL)
  {
    // INIT the tail of the list
    excessFreeBlock->next = NULL;
    excessFreeBlock->prev = NULL;
    excessFreeBlock->free = 1;
    freeListHead = excessFreeBlock->block;
    freeListHeadBlock = excessFreeBlock;
    current = excessFreeBlock;

    // block_meta *tail; /* Tail of the DDL */
    // tail = sbrk(0);
    // tail->next = NULL;              /* make tail of DLL point to the first freeblock */
    // tail->free = 1;                 /* set the tag to free */
    // excessFreeBlock->prev = NULL;   /* First free blocks prev is the tail */
    // tail->block = excessFreeBlock;  /* the tail block is the excess free block */
    // freeListHead = excessFreeBlock; /* move the freelisthead pointer to the tail's block pointer */
    // head = tail;                    /* set the current pointer to the same as the tail */
  }
  else
  {
    current->next = excessFreeBlock; /* the current blocks next points to the block-to-be */
    excessFreeBlock->prev = current; /* the new block prev is going to point to the prev free block */
    excessFreeBlock->next = NULL;    // next == null
    excessFreeBlock->free = 1;       // It is a free block [tag]
    current = excessFreeBlock;       // move the current block to the new block
  }

  //	Updates SMA info
  totalAllocatedSize -= get_blockSize(excessFreeBlock->block);
  totalFreeSize += get_blockSize(excessFreeBlock->block);
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

  //	TODO: Iterate through the Free Block List to find the largest free block and return its size

  return largestBlockSize;
}

/*
* When you call pbrk the space that is included in pbrk in the heap is made of blocks and the first time you make an allocation, you will have 2 blocks (allocated and free block)
* Initially that free block will be head and tail of list. The bloc needs to have tags, and needs a size tag (tells how big it is). For the free list, the block needs prev and next tags - also both blocks need a bit to say if it's allocated or not
* The blocks are spaces in memory, not a struct. The way you keep track is by updating and accessing tags by moving a pointer around
*/

void print_LL()
{
  printf("----Printing_values_in_linkedlist----\n");
  block_meta *head = freeListHeadBlock;
  while (head != NULL)
  {
    printf("head Size %d\n", head->size);
    head = head->next;
    usleep(1000 * 500);
  }
}
