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

char server_msg[BUFSIZE] = {0};
int sockfd;
char *destination = "0.0.0.0";
int port_number = 8088;

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
      struct queue_entry *node_to_enqueue_back = queue_pop_head(&wait_queue);
      threaddesc *task_to_enqueue_back = (threaddesc *)node_to_enqueue_back->data;

      pthread_mutex_unlock(&m);
      if (new_task_io->function_number == 2) // write function
      {
        send_message(sockfd, new_task_io->buffer, new_task_io->size);
      }
      else if (new_task_io->function_number == 1) // read function
      {
        while (true)
        {
          ssize_t byte_count = recv_message(sockfd, server_msg, sizeof(server_msg));
        }
        // Put task back into queue once finished
        usleep(1000 * 10000);
        pthread_mutex_lock(&m);
        queue_insert_tail(&task_ready_queue, node_to_enqueue_back);

        setcontext(&task_to_enqueue_back->threadcontext);
        pthread_mutex_unlock(&m);
      }
      else
      {
        continue;
      }
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
  struct queue_entry *c_exec_new_node = queue_new_node(current);
  pthread_mutex_unlock(&m);

  // Create new node for the wait queue
  iothread *new_io_thread = &(iothreadarr[io_numthreads]);
  new_io_thread->function_number = 2;
  new_io_thread->buffer = buf;
  new_io_thread->size = size;

  struct queue_entry *new_io_node = queue_new_node(new_io_thread);

  usleep(1000 * 1000);
  pthread_mutex_lock(&m);
  io_numthreads++;
  queue_insert_tail(&wait_queue, new_io_node);
  queue_insert_tail(&wait_queue, c_exec_new_node);
  pthread_mutex_unlock(&m);
}

char *sut_read()
{
  memset(server_msg, 0, sizeof(server_msg));
  pthread_mutex_lock(&m);
  threaddesc *current = (threaddesc *)queue_peek_front(&task_ready_queue)->data;
  struct queue_entry *c_exec_new_node = queue_new_node(current);
  pthread_mutex_unlock(&m);

  iothread *new_io_thread = &(iothreadarr[io_numthreads]);
  new_io_thread->function_number = 1;
  new_io_thread->buffer = "";
  new_io_thread->size = 0;

  struct queue_entry *new_io_node = queue_new_node(new_io_thread);
  pthread_mutex_lock(&m);
  io_numthreads++;
  queue_insert_tail(&wait_queue, new_io_node);
  queue_insert_tail(&wait_queue, c_exec_new_node);
  pthread_mutex_unlock(&m);
  while (strlen(server_msg) == 0)
  {
    continue;
  }
  return server_msg;
}

void sut_close()
{

  pthread_mutex_lock(&m);
  threaddesc *current = (threaddesc *)queue_peek_front(&task_ready_queue)->data;
  struct queue_entry *c_exec_new_node = queue_new_node(current);
  pthread_mutex_unlock(&m);
  iothread *new_io_thread = &(iothreadarr[io_numthreads]);
  new_io_thread->function_number = 0;
  new_io_thread->buffer = "";
  new_io_thread->size = 0;
  struct queue_entry *new_io_node = queue_new_node(new_io_thread);
  pthread_mutex_lock(&m);
  io_numthreads++;
  queue_insert_tail(&wait_queue, new_io_node);
  queue_insert_tail(&wait_queue, c_exec_new_node);
  pthread_mutex_unlock(&m);
}
