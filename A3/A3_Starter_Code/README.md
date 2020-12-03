to run the code:
gcc -g -w sma.c a3_test.c && ./a.out
or use the Makefile as so:
make run_sma

I recently ran my code for assignment 3 on the Mimi server and unfortunately, it is not running at all, yet it is running perfectly fine on my local and my other computers local.
I have tested the code out on 3 different computers, and they all work fine except for on Mimi.
There seems to be an issue when running my code on Mimi. For the purpose of this assignment would it be possible to test my code on a local dev box.
Thank you for considering this.

For the design of the assignment, I decided to used structs for the blocks. This was especially important for the doubly linked list because I needed a way for each block to be able to reference the next and previous block. Each block contained a few parameters such as size (which was the size of the malloc), a next and previous block pointer that reference the next and previous free blocks and a void pointer \*block which was the address returned by for example sbrk(int).

The sma_malloc() would initialize a new block = sbrk(0) to imitate creating a node in a linkedlist using malloc(). I increment the breakpoint by the size argument given as parameter (this would be my request). The extra space was essentially the size - sizeof(struct block). I would add this excess block into the freelist. One issue here is that when I malloced more than 128kb, my code would break and sbrk(0) would return 16.

For sma_free(), I had a function that would return me the block that contained the ptr. Once I found this block, I would free the allocated space inside of the block and add the block's size to the previous or next block (depending if the previous or next was NULL or not). I would adjust modify the next and previous pointers to make sure that the node was removed from the Doubly linked-list. This is where I think the code starts to break. The biggest issue for this was that I had trouble merging the two free blocks together and modifying the new \*block pointer. I would loose the breaker point here.

For sma_realloc() I did something similar. I knew that sma_free would handle splitting the block and sma_malloc would verify if there was space left to add between free blocks. The only thing I needed to update is to verify if that pointer address had been previously allocated, which I created a new function called get_block_ptr() which would take a pointer and return the block that contained the ptr or NULL if it had not been previously allocated.

Allocate_next_fit and Allocate_worst_fit were quite simple since I just had to iterate through the list of blocks to verify which block was apporiate and send that block to be allocated.

The first three tests pass in this code.

The output of this code using the a3_test.c provided is:
Test 1: Hole finding test...
PASSED

Test 2: Program break expansion test...
PASSED

Test 3: Check for Worst Fit algorithm...
PASSED

Test 4: Check for Next Fit algorithm...
FAILED

Test 5: Check for Reallocation with Next Fit...
FAILED

# Test 6: Print SMA Statistics...

Total number of bytes allocated: 25816920
Total free space: 550056
Size of largest contigious free space (in bytes): 32728
