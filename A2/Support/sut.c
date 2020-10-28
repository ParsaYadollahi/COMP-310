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

struct Point
{
  int x, y;
};

void f1()
{
  printf("enter f1\n");
  for (int i = 0; i < 5; i++)
  {
    usleep(1000 * 1000);
    printf("Hello world!, this is SUT-One \n");
    sut_yield();
  }
  printf("sut exit f1\n");
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
  printf("sut exit f2\n");
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

    task_description->threadcontext.uc_stack.ss_sp = task_description->threadstack;
    task_description->threadcontext.uc_stack.ss_size = THREAD_STACK_SIZE;
    task_description->threadcontext.uc_link = 0;
    task_description->threadcontext.uc_stack.ss_flags = 0;
    task_description->threadfunc = fn;

    makecontext(&(task_description->threadcontext), fn, 0);

    pthread_mutex_lock(&m);
    numthreads++;

    struct queue_entry *new_node = queue_new_node(task_description);
    queue_insert_tail(&task_ready_queue, new_node);

    // threaddesc *t = (threaddesc *)queue_peek_front(&task_ready_queue)->data;
    // printf("PEEK %d\n", t->threadid);

    pthread_mutex_unlock(&m);
  }
}

void sut_yield()
{
  struct queue_entry *new_node = queue_new_node(current_task);

  pthread_mutex_lock(&m);
  queue_insert_tail(&task_ready_queue, new_node);
  pthread_mutex_unlock(&m);

  threaddesc *tail_task = (threaddesc *)new_node->data;

  printf("1 Task_ID = %d\n", tail_task->threadid);
  current_task = tail_task;

  makecontext(&(tail_task->threadcontext), tail_task->threadfunc, 0);

  pthread_mutex_lock(&m);
  threaddesc *new_task = (threaddesc *)queue_pop_head(&task_ready_queue)->data;
  pthread_mutex_unlock(&m);
  printf("2 Task_ID = %d\n", new_task->threadid);

  swapcontext(&current_task->threadcontext, &new_task->threadcontext);
}

void sut_exit()
{
  //TODO: setcontext - instead of swapcontext
  setcontext(&current_task->threadcontext);
}

void sut_shutdown()
{
  pthread_join(c_exec_thread, NULL);
}

int main()
{
  sut_init();
  sut_create(f1);
  sut_create(f2);
  sut_shutdown();
  printf("BBBBB\n");
  printf("EMPTY QUEUE %d\n", queue_not_empty(&task_ready_queue));
}

// // EXAMPLE
// // threaddesc *p1 = &(threadarr[numthreads]);
// threadarr[numthreads].threadid = 1;
// printf("A\n");
// struct queue q = queue_create();
// queue_init(&q);
// struct queue_entry *node = queue_new_node(&threadarr[numthreads]);
// queue_insert_tail(&q, node);
// threaddesc *p = (threaddesc *)queue_peek_front(&q)->data;
// printf("%d\n", threadarr[numthreads].threadid);
// // END EXMAPLE

// // EXAMPLE
// struct Point p1 = {85, 1};
// struct queue q = queue_create();
// queue_init(&q);
// struct queue_entry *node = queue_new_node(&p1);
// queue_insert_tail(&q, node);
// struct Point *p = (struct Point *)queue_peek_front(&q)->data;
// printf("%d\n", p->y);
// // END EXMAPLE
