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

pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_t c_exec_thread;

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
  while (true)
  {
    printf("Inside\n");
    usleep(1000 * 1000);
    pthread_mutex_lock(&m);
    if (queue_not_empty(&task_ready_queue) == 1)
    {
      printf("AAAAAAA\n");
      struct Task *x = (struct Task *)queue_peek_front(&task_ready_queue);
      printf("2 PEEK %d\n", x->threadid);

      struct Task *new_task = (struct Task *)queue_pop_head(&task_ready_queue);
      current_task = new_task;
      printf("Tread id = %d\n", new_task->threadid);
      swapcontext(&parent, &new_task->threadcontext);
      //TODO: parent context is c_exec
    }
    else
    {
      printf("Empty\n");
      usleep(1000 * 1000);
    }
    pthread_mutex_unlock(&m);
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

  pthread_create(&c_exec_thread, NULL, c_exec_ftn, NULL);
  // pthread_create(&i_exec_thread, NULL, i_exec_ftn, &m);

  printf("AFTER JOIN\n");
  // pthread_join(i_exec_thread, NULL);
}

bool sut_create(sut_task_f fn)
{
  printf("sut_create\n");
  pthread_mutex_lock(&m);
  struct Task *task_description = &threadarr[0];
  THREAD_ID++;

  getcontext(&(task_description->threadcontext));
  task_description->threadstack = (char *)malloc(THREAD_STACK_SIZE);
  task_description->threadid = THREAD_ID;
  printf("\tsut_create threadID %d\n", task_description->threadid);

  task_description->threadcontext.uc_stack.ss_sp = task_description->threadstack;
  task_description->threadcontext.uc_stack.ss_size = THREAD_STACK_SIZE;
  task_description->threadcontext.uc_link = 0;
  task_description->threadcontext.uc_stack.ss_flags = 0;
  task_description->threadfunc = fn;

  struct Task *x = (struct Task *)queue_peek_front(&task_ready_queue);
  printf("PEEK %d\n", x->threadid);

  makecontext(&(task_description->threadcontext), fn, 0);

  struct queue_entry *tail_task = queue_new_node(task_description);
  queue_insert_tail(&task_ready_queue, tail_task);
  pthread_mutex_unlock(&m);
}

void sut_yield()
{
  THREAD_ID++;
  printf("SUT_YIELD\n");

  struct queue_entry *tail_task = queue_new_node(current_task);
  current_task = (struct Task *)tail_task->data;
  queue_insert_tail(&task_ready_queue, tail_task);

  // TODO: makecontext

  struct Task *new_task = (struct Task *)queue_pop_head(&task_ready_queue)->data;
  swapcontext(&current_task->threadcontext, &new_task->threadcontext);
}

void sut_exit()
{
  //TODO: setcontext - instead of swapcontext
  swapcontext(&current_task->threadcontext, &parent);
}

int main()
{
  sut_init();
  sut_create(f1);
  sut_create(f2);
  printf("DONE\n");
  pthread_join(c_exec_thread, NULL);
}
