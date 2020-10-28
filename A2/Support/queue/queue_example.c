#include "queue.h"
#include <stdio.h>
#include <unistd.h>

#include <pthread.h>

struct Point
{
  int x, y;
};

struct Task
{
  int threadid;
  char *threadstack;
  void *threadfunc;
  ucontext_t threadcontext;
};

int main()
{

  int x = 1;
  int y = 2;
  int z = 3;
  struct Point p1 = {85, 1};
  struct queue q = queue_create();
  queue_init(&q);
  struct queue_entry *node = queue_new_node(&p1);
  queue_insert_tail(&q, node);
  struct Point *p = (struct Point *)queue_peek_front(&q)->data;
  printf("%d\n", p->x);

  printf("Queue empty %d\n", queue_not_empty(&q));

  struct queue_entry *node2 = queue_new_node(&y);
  queue_insert_tail(&q, node2);

  struct queue_entry *node3 = queue_new_node(&z);
  queue_insert_tail(&q, node3);

  struct queue_entry *ptr = queue_pop_head(&q);
  while (ptr)
  {
    printf("popped %d\n", *(int *)ptr->data);

    queue_insert_tail(&q, ptr);
    usleep(1000 * 1000);

    ptr = queue_pop_head(&q);
  }

  return 0;
}
