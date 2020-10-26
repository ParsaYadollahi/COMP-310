/**
 *  @author: Parsa Yadollahi
 *  @email: parsa.yadollahi@mail.mcgill.ca
 *  @description: Threads
 *  @assignment: Simple User-Level Thread Scheduler - Assignment 2
 * @studentid: 260869949
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <ucontext.h>
#include <pthread.h>
#include <time.h>

#include "sut.h"
#include "queue/queue.h"

#define MAX_THREADS 32
#define THREAD_STACK_SIZE 1024 * 64

struct Task
{
  int threadid;
  char *threadstack;
  void *threadfunc;
  ucontext_t threadcontext;
};

typedef void (*sut_task_f)();
static ucontext_t parent;
struct queue task_ready_queue;
struct queue wait_queue;
struct Task *current_task;
struct Task threadarr[MAX_THREADS];
int THREAD_ID = 0;

void f1()
{
  for (int i = 0; i < 5; i++)
  {
    usleep(1000 * 1000);
    printf("1 Hello world!, this is SUT-One \n");
    sut_yield();
  }
  sut_exit();
}

void f2()
{
  for (int i = 0; i < 5; i++)
  {
    printf("2 Hello world!, this is SUT-Two \n");
    sut_yield();
  }
  sut_exit();
}

void *c_exec_ftn(void *args)
{
  pthread_mutex_t *m = args;
  while (true)
  {
    pthread_mutex_lock(m);
    printf("1 C-exec thread\n");
    usleep(1000 * 1000);
    printf("1 CEXEC\n");
    pthread_mutex_unlock(m);
    usleep(1000 * 1000);
  }
}

// void *i_exec_ftn(void *args)
// {
//   pthread_mutex_t *m = args;
//   while (true)
//   {
//     pthread_mutex_lock(m);
//     printf("2 I-exec thread\n");
//     usleep(1000 * 1000);
//     printf("2 IEXEC\n");
//     pthread_mutex_unlock(m);
//     usleep(1000 * 1000);
//   }
// }

void sut_init()
{
  task_ready_queue = queue_create();
  queue_init(&task_ready_queue);

  pthread_t c_exec_thread;
  pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

  pthread_create(&c_exec_thread, NULL, c_exec_ftn, &m);
  // pthread_create(&i_exec_thread, NULL, i_exec_ftn, &m);

  pthread_join(c_exec_thread, NULL);
  // pthread_join(i_exec_thread, NULL);
}

bool sut_create(sut_task_f fn)
{
  printf("sut_create\n");
  struct Task *task_description = &threadarr[0];
  THREAD_ID++;

  getcontext(&(task_description->threadcontext));
  task_description->threadstack = (char *)malloc(THREAD_STACK_SIZE);
  task_description->threadid = THREAD_ID;

  task_description->threadcontext.uc_stack.ss_sp = task_description->threadstack;
  task_description->threadcontext.uc_stack.ss_size = THREAD_STACK_SIZE;
  task_description->threadcontext.uc_link = 0;
  task_description->threadcontext.uc_stack.ss_flags = 0;
  task_description->threadfunc = fn;

  struct queue_entry *tail_task = queue_new_node(task_description);
  queue_insert_tail(&task_ready_queue, tail_task);

  current_task = task_description;
  makecontext(&(task_description->threadcontext), fn, 0);

  swapcontext(&parent, &task_description->threadcontext);
}

void sut_yield()
{
  THREAD_ID++;
  printf("SUT_YIELD\n");

  struct queue_entry *tail_task = queue_new_node(current_task);
  current_task = (struct Task *)tail_task->data;
  queue_insert_tail(&task_ready_queue, tail_task);

  struct Task *new_task = (struct Task *)queue_pop_head(&task_ready_queue)->data;
  swapcontext(&current_task->threadcontext, &new_task->threadcontext);
}

void sut_exit()
{
  swapcontext(&current_task->threadcontext, &parent);
}

int main()
{
  sut_init();
  sut_create(f1);
  sut_create(f2);
  printf("DONE\n");
}
