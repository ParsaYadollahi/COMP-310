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
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "a1_lib.h"
#include "sut.h"
#include "queue.h"

#define MAX_THREADS 32
#define BUFSIZE 1024
#define THREAD_STACK_SIZE 1024 * 64

typedef void (*sut_task_f)();
struct queue task_ready_queue;
struct queue wait_queue;
int numthreads, curthread, io_numthreads;
ucontext_t parent;
threaddesc threadarr[MAX_THREADS];
threaddesc *task_description;
iothread iothreadarr[MAX_THREADS];

static pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_t c_exec_thread;
pthread_t i_exec_thread;

int sockfd;
char *destination = "0.0.0.0";
int port_number = 8088;

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

void hello1()
{
  int i;
  char sbuf[128];
  sut_open(destination, port_number);
  for (i = 0; i < 3; i++)
  {
    sprintf(sbuf, "ls\n");
    sut_write(sbuf, strlen(sbuf));
    sut_yield();
  }
  // sut_close();
  sut_exit();
}

void *c_exec_ftn()
{
  while (true)
  {
    usleep(1000 * 1000);
    if (queue_not_empty(&task_ready_queue) == 1)
    {
      pthread_mutex_lock(&m);
      threaddesc *new_task_c = (threaddesc *)queue_peek_front(&task_ready_queue)->data;
      printf("C %d\n", new_task_c->threadid);
      pthread_mutex_unlock(&m);
      usleep(1000 * 1000);

      swapcontext(&parent, &new_task_c->threadcontext);
    }
    else
    {
      continue;
    }
  }
}

void *i_exec_ftn()
{
  while (true)
  {
    if (queue_not_empty(&wait_queue) == 1)
    {
      pthread_mutex_lock(&m);
      iothread *new_task_io = (iothread *)queue_pop_head(&wait_queue)->data;
      struct queue_entry *task_to_enqueue_back = queue_pop_head(&wait_queue);
      pthread_mutex_unlock(&m);
      if (new_task_io->function_number == 1) // write function
      {
        char server_msg[BUFSIZE] = {0};
        send_message(sockfd, new_task_io->buffer, new_task_io->size);
        ssize_t byte_count = recv_message(sockfd, server_msg, sizeof(server_msg));
        printf("%s\n", server_msg);
      }
      else if (new_task_io->function_number == 0) // read function
      {
        printf("--REAAAD--\n");
      }
      usleep(1000 * 10000);
      pthread_mutex_lock(&m);
      queue_insert_tail(&task_ready_queue, task_to_enqueue_back);
      pthread_mutex_unlock(&m);
    }
    else
    {
      continue;
    }
  }
}

void sut_init()
{

  numthreads = 0;
  io_numthreads = 0;
  pthread_mutex_lock(&m);

  task_ready_queue = queue_create();
  queue_init(&task_ready_queue);

  wait_queue = queue_create();
  queue_init(&wait_queue);

  pthread_mutex_unlock(&m);

  pthread_create(&c_exec_thread, NULL, c_exec_ftn, NULL);
  pthread_create(&i_exec_thread, NULL, i_exec_ftn, &m);
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

  pthread_mutex_lock(&m);
  threaddesc *new_task = (threaddesc *)queue_peek_front(&task_ready_queue)->data;
  pthread_mutex_unlock(&m);

  swapcontext(&old_task->threadcontext, &new_task->threadcontext);
}

void sut_exit()
{
  threaddesc *last_task = (threaddesc *)queue_peek_front(&task_ready_queue)->data;
  setcontext(&last_task->threadcontext);
}

void sut_shutdown()
{
  pthread_join(c_exec_thread, NULL);
  pthread_join(i_exec_thread, NULL);
  pthread_cancel(c_exec_thread);
  pthread_cancel(i_exec_thread);
}

// Runs on c_exec
void sut_open(char *dest, int port)
{
  if (connect_to_server(dest, port, &sockfd) < 0)
  {
    fprintf(stderr, "oops no connection!\n");
  }
  else
  {
    printf("Connected\n");
  }
}

void sut_write(char *buf, int size)
{

  pthread_mutex_lock(&m);
  threaddesc *current = (threaddesc *)queue_peek_front(&task_ready_queue)->data;
  struct queue_entry *new_node = queue_new_node(current);
  pthread_mutex_unlock(&m);

  iothread *new_io_thread = &(iothreadarr[io_numthreads]);
  new_io_thread->function_number = 1;
  new_io_thread->buffer = buf;
  new_io_thread->size = size;

  struct queue_entry *new_io_node = queue_new_node(new_io_thread);

  usleep(1000 * 1000);
  pthread_mutex_lock(&m);
  queue_insert_tail(&wait_queue, new_io_node);
  queue_insert_tail(&wait_queue, new_node);
  pthread_mutex_unlock(&m);
}

char *sut_read()
{
}

int main()
{
  sut_init();
  sut_create(f1);
  sut_create(f2);
  sut_create(hello1);
  sut_shutdown();
}
