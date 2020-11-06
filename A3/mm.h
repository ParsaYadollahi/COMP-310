#include <stdio.h>

void *sma_malloc(int size);
void sma_free(void *ptr);
void sma_mallopt(int policy);
void sma_mallinfo();
void *sma_realloc(void *ptr, int size);

int sma_init(void);
