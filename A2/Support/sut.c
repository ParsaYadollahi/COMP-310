#define _XOPEN_SOURCE
/**
 *  @author: Parsa Yadollahi
 *  @email: parsa.yadollahi@mail.mcgill.ca
 *  @description: backend
 *  @assignment: RPC - assignment 1
 * @studentid: 260869949
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <ucontext.h>
#include <pthread.h>
#include <time.h>

typedef void (*sut_task_f)();
static ucontext_t t1, m;

void f1()
{
  while (true)
  {
    usleep(1000 * 1000);
    printf("Hello world!, this is SUT-One \n");
    swapcontext(&t1, &m);
  }
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

void *i_exec_ftn(void *args)
{
  pthread_mutex_t *m = args;
  while (true)
  {
    pthread_mutex_lock(m);
    printf("2 I-exec thread\n");
    usleep(1000 * 1000);
    printf("2 IEXEC\n");
    pthread_mutex_unlock(m);
    usleep(1000 * 1000);
  }
}

int sut_init()
{
  // Initialize kernel-level threads
  pthread_t c_exec_thread;
  pthread_t i_exec_thread;

  // Initialize pthread mutex - lock
  pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

  pthread_create(&c_exec_thread, NULL, c_exec_ftn, &m);
  pthread_create(&i_exec_thread, NULL, i_exec_ftn, &m);
  pthread_join(c_exec_thread, NULL);
  pthread_join(i_exec_thread, NULL);
}

int main()
{
  char f1stack[16 * 1024];

  getcontext(&t1);
  t1.uc_stack.ss_sp = f1stack;
  t1.uc_stack.ss_size = sizeof(f1stack);
  t1.uc_link = &m;
  makecontext(&t1, f1, 0);

  while (true)
  {
    swapcontext(&m, &t1);
  }

  return 0;
}
