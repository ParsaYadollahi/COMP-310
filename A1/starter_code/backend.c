#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#include "a1_lib.h"
#include "backend.h"

#define BUFSIZE 1024
#define TRUE 1
#define FALSE 0

int main(void)
{
  // Paramatrer
  char *two_param[] = {"add", "multiply", "divide"};
  char *one_param[] = {"factorial", "sleep"};
  char *no_param[] = {"quit", "shutdown", "exit"};

  int sockfd, clientfd;
  char msg[BUFSIZE];
  const char *greeting = "hello, world\n";
  int running = 1;
  int result;
  int pid, pid1, pid2;
  pid = fork();

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
      result = three_params(param, x, y);
    }
    else if (inArray(param, one_param, 2))
    {
      int x = atoi(strtok(NULL, " "));
      result = two_params(param, x);
    }
    else if (inArray(param, no_param, 3))
    {
      printf("\nQUIT\n");
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
    char result_string[1024];
    sprintf(result_string, "%d", result);

    send_message(clientfd, result_string, strlen(result_string));
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

int sleepy(int x)
{
  sleep(x);
  return 1;
}
// make the calculator sleep for x seconds – this is blocking
uint64_t factorial(int x)
{
  if (x < 0)
  {
    return (uint64_t)0;
  }
  uint64_t result = 1;
  for (int i = 1; i <= x; i++)
  {
    result *= i;
  }
  return result;
}

//////////////////////////////////////////
// CUSTOM FUNCTIONS
//////////////////////////////////////////
bool inArray(char *word, char **cmd, int len_cmd)
{
  for (int i = 0; i < len_cmd; i++)
  {
    if (!strcmp(cmd[i], word))
    {
      return TRUE;
    }
  }
  return FALSE;
}

float three_params(char *cmd, int x, int y)
{
  float result = 0;
  if (!strcmp(cmd, "add"))
  {
    result = addInts(x, y);
  }
  else if (!strcmp(cmd, "multiply"))
  {
    result = multiplyInts(x, y);
  }
  else if (!strcmp(cmd, "divide"))
  {
    result = divideFloats((float)x, (float)y);
  }
  return result;
}

uint64_t two_params(char *cmd, int x)
{
  float result = 0;
  if (!strcmp(cmd, "sleep"))
  {
    result = (uint64_t)sleepy(x);
  }
  else if (!strcmp(cmd, "factorial"))
  {
    result = factorial(x);
  }
  return result;
}
