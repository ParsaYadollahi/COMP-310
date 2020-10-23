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
static ucontext_t t1, t2, m;

void f1()
{
  while (true)
  {
    usleep(1000 * 1000);
    printf("Hello world!, this is SUT-One \n");
    swapcontext(&t1, &m);
  }
}

// void f2()
// {
//   while (true)
//   {
//     usleep(1000 * 1000);
//     printf("Hello world!, this is SUT-Two \n");
//     swapcontext(&t2, &m);
//   }
// }

int main()
{
  char f1stack[16 * 1024];
  char f2stack[16 * 1024];

  getcontext(&t1);
  t1.uc_stack.ss_sp = f1stack;
  t1.uc_stack.ss_size = sizeof(f1stack);
  t1.uc_link = &m;
  makecontext(&t1, f1, 0);

  // getcontext(&t2);
  // t2.uc_stack.ss_sp = f2stack;
  // t2.uc_stack.ss_size = sizeof(f2stack);
  // t2.uc_link = &m;
  // makecontext(&t2, f2, 0);

  while (true)
  {
    swapcontext(&m, &t1);
    // swapcontext(&m, &t2);
  }

  return 0;
}

void sut_init()
{
  // Initialize kernel-level threads
  pthread_t c_exec;
  pthread_t i_exec;

  pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

  pthread_create(&c_exec, NULL, sut_init, NULL);
  pthread_create(&i_exec, NULL, sut_init, NULL);
  pthread_join(c_exec, NULL);
}

// bool sut_create(sut_task_f fn)
// {
//   return true;
// }
