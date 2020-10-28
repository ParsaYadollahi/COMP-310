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
#include "queue.h"

#define MAX_THREADS 32
#define THREAD_STACK_SIZE 1024 * 64

typedef void (*sut_task_f)();
struct queue task_ready_queue;
struct queue wait_queue;
int numthreads, curthread;
ucontext_t parent;
threaddesc *current_task;
threaddesc threadarr[MAX_THREADS];
threaddesc *task_description;

static pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_t c_exec_thread;

void f1()
{
  for (int i = 0; i < 5; i++)
  {
    usleep(1000 * 1000);
    printf("Hello world!, this is SUT-One \n");
    sut_yield();
  }
  sut_exit();
}

void f2()
{
  for (int i = 0; i < 5; i++)
  {
    usleep(1000 * 500);
    printf("Hello world!, this is SUT-Two \n");
    sut_yield();
  }
  sut_exit();
}

void *c_exec_ftn(void *args)
{
  while (true)
  {
    usleep(1000 * 1000);
    if (queue_not_empty(&task_ready_queue) == 1)
    {
      pthread_mutex_lock(&m);
      threaddesc *new_task = (threaddesc *)queue_peek_front(&task_ready_queue)->data;
      printf("\t\t\tThread id = %d\n", new_task->threadid);
      pthread_mutex_unlock(&m);
      current_task = new_task;

      swapcontext(&parent, &new_task->threadcontext);
      printf("AAA\n");
    }
    else
    {
      usleep(1000 * 1000);
    }
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

  numthreads = 0;
  pthread_mutex_lock(&m);

  task_ready_queue = queue_create();
  queue_init(&task_ready_queue);

  pthread_mutex_unlock(&m);

  pthread_create(&c_exec_thread, NULL, c_exec_ftn, NULL);
  // pthread_create(&i_exec_thread, NULL, i_exec_ftn, &m);

  // pthread_join(i_exec_thread, NULL);
}

bool sut_create(sut_task_f fn)
{
  if (numthreads <= 15)
  {

    printf("sut_create\n");
    task_description = &(threadarr[numthreads]);

    getcontext(&(task_description->threadcontext));
    task_description->threadstack = (char *)malloc(THREAD_STACK_SIZE);
    task_description->threadid = numthreads;
    numthreads++;

    task_description->threadcontext.uc_stack.ss_sp = task_description->threadstack;
    task_description->threadcontext.uc_stack.ss_size = THREAD_STACK_SIZE;
    task_description->threadcontext.uc_link = 0;
    task_description->threadcontext.uc_stack.ss_flags = 0;
    task_description->threadfunc = fn;

    makecontext(&(task_description->threadcontext), task_description->threadfunc, 0);

    pthread_mutex_lock(&m);

    struct queue_entry *new_node = queue_new_node(task_description);
    queue_insert_tail(&task_ready_queue, new_node);

    pthread_mutex_unlock(&m);
  }
}

void sut_yield()
{
  pthread_mutex_lock(&m);

  struct queue_entry *old_node = queue_pop_head(&task_ready_queue);
  threaddesc *old_task = (threaddesc *)old_node->data;

  queue_insert_tail(&task_ready_queue, old_node);
  pthread_mutex_unlock(&m);

  current_task = old_task;

  pthread_mutex_lock(&m);
  threaddesc *new_task = (threaddesc *)queue_peek_front(&task_ready_queue)->data;
  pthread_mutex_unlock(&m);

  swapcontext(&old_task->threadcontext, &new_task->threadcontext);
}

void sut_exit()
{
  setcontext(&current_task->threadcontext);
}

void sut_shutdown()
{
  pthread_join(c_exec_thread, NULL);
  pthread_cancel(c_exec_thread);
}

int main()
{
  sut_init();
  sut_create(f1);
  sut_create(f2);
  sut_shutdown();
}
