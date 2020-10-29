#ifndef __SUT_H__
#define __SUT_H__
#include <stdbool.h>
#include <ucontext.h>

typedef void (*sut_task_f)();
#define MAX_THREADS 32
typedef struct __threaddesc
{
  int threadid;
  char *threadstack;
  void *threadfunc;
  ucontext_t threadcontext;
} threaddesc;

typedef struct __iothread
{
  int function_number; // 1 = write, 0 = read
  char *buffer;
  int size;
} iothread;

extern threaddesc threadarr[MAX_THREADS];
extern threaddesc *current_task;
extern int numthreads, curthread;
extern ucontext_t parent;

void sut_init();
bool sut_create(sut_task_f fn);
void sut_yield();
void sut_exit();
void sut_open(char *dest, int port);
void sut_write(char *buf, int size);
void sut_close();
char *sut_read();
void sut_shutdown();

#endif
