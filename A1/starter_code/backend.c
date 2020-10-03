#include <stdio.h>
#include <string.h>
#include <string.h>

#include "a1_lib.h"
#include "backend.h"

#define BUFSIZE 1024

int main(void)
{
  char *two_param[] = {"add", "multiply", "divide"};
  char *one_param[] = {"factorial", "sleep"};
  char *no_param[] = {"quit", "shutdown", "exit"};
  int sockfd, clientfd;
  char msg[BUFSIZE];
  const char *greeting = "hello, world\n";
  int running = 1;
  int result;

  if (create_server("0.0.0.0", 10000, &sockfd) < 0)
  {
    fprintf(stderr, "oh no\n");
    return -1;
  }

  if (accept_connection(sockfd, &clientfd) < 0)
  {
    fprintf(stderr, "oh no\n");
    return -1;
  }

  while (strcmp(msg, "quit\n"))
  {
    memset(msg, 0, sizeof(msg));
    ssize_t byte_count = recv_message(clientfd, msg, BUFSIZE);

    // TODO: PARSE INPUT HERE
    char *param = strtok(msg, " ");
    if (inArray(param, two_param, 3))
    {
      int x = atoi(strtok(NULL, " "));
      int y = atoi(strtok(NULL, " "));
      if (!strcmp(param, "add"))
      {
        result = addInts(x, y);
      }
      else if (!strcmp(param, "multiply"))
      {
        result = multiplyInts(x, y);
      }
      else if (!strcmp(param, "divide"))
      {
        result = divideFloats((float)x, (float)y);
      }
      printf("%d\n", result);
    }
    else if (inArray(param, one_param, 2))
    {
      int x = atoi(strtok(NULL, " "));
    }
    else if (inArray(param, no_param, 3))
    {
    }
    else
    {
      printf("FFFFFFFFF");
    }

    if (byte_count <= 0)
    {
      break;
    }
    printf("Client: %s\n", msg);
    send_message(clientfd, greeting, strlen(greeting));
  }

  return 0;
}

int addInts(int a, int b)
{
  int result = a + b;
  return result;
}

int multiplyInts(int a, int b)
{
  int result = a * b;
  return result;
}

float divideFloats(float a, float b)
{
  if (b == 0)
  {
    fprintf(stderr, "Cannot divide by 0\n");
    return -1;
  }
  float result = a / (float)(b);
  return result;
}
int sleep(int x);
// make the calculator sleep for x seconds – this is blocking
uint64_t factorial(int x)
{
  if (x < 0)
  {
    return (uint64_t)0;
  }
  uint64_t result = 0;
  for (int i = 0; i < x; i++)
  {
    result *= i;
  }
  return result;
}

bool inArray(char *word, char **cmd, int len_cmd)
{
  for (int i = 0; i < len_cmd; i++)
  {
    if (!strcmp(cmd[i], word))
    {
      return true;
    }
  }
  return false;
}
