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

#include "queue/queue.h"

#define MAX_THREADS 32
#define THREAD_STACK_SIZE 1024 * 64

typedef void (*sut_task_f)();
static ucontext_t t1, m;
struct queue task_queue;

struct Task
{
  char *threadstack;
  void *threadfunc;
  ucontext_t threadcontext;
};

void f1()
{
  printf("AAAAAAAAA\n");
  while (true)
  {
    usleep(1000 * 1000);
    printf("Hello world!, this is SUT-One \n");
    swapcontext(&t1, &m);
  }
}

// void *c_exec_ftn(void *args)
// {
//   pthread_mutex_t *m = args;
//   while (true)
//   {
//     pthread_mutex_lock(m);
//     printf("1 C-exec thread\n");
//     usleep(1000 * 1000);
//     printf("1 CEXEC\n");
//     pthread_mutex_unlock(m);
//     usleep(1000 * 1000);
//   }
// }

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

int sut_init()
{
  // Initialize the queue
  task_queue = queue_create();
  queue_init(&task_queue);

  struct Task *task_d;
  struct queue_entry *task = queue_new_node(&task_d);
  queue_insert_tail(&task_queue, task);

  // Initialize kernel-level threads
  // pthread_t c_exec_thread;
  // pthread_t i_exec_thread;

  // // Initialize pthread mutex - lock
  // pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

  // pthread_create(&c_exec_thread, NULL, c_exec_ftn, &m);
  // pthread_create(&i_exec_thread, NULL, i_exec_ftn, &m);

  // pthread_join(c_exec_thread, NULL);
  // pthread_join(i_exec_thread, NULL);
}

bool sut_create(sut_task_f fn)
{
  struct Task *task_description;

  // The next task is at the head of the queue
  struct queue_entry *entrie = queue_pop_head(&task_queue);
  task_description = entrie->data;

  getcontext(&(task_description->threadcontext));
  task_description->threadstack = (char *)malloc(THREAD_STACK_SIZE);

  task_description->threadcontext.uc_stack.ss_sp = task_description->threadstack;
  task_description->threadcontext.uc_stack.ss_size = THREAD_STACK_SIZE;
  task_description->threadcontext.uc_link = 0;
  task_description->threadcontext.uc_stack.ss_flags = 0;
  task_description->threadfunc = &fn;

  makecontext(&(task_description->threadcontext), fn, 0, task_description);

  // struct queue_entry *task_context = queue_new_node(&t1);

  // while (true)
  // {

  // }

  return 0;
}

int main()
{
  sut_init();
  sut_create(&f1);
  printf("BBBBBBBBBBBBBB\n");
}
