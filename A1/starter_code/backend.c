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

int main(int argc, char *argv[])
{
  // Paramatrer
  char *two_param[] = {"add", "multiply", "divide"};
  char *one_param[] = {"factorial", "sleep"};
  char *no_param[] = {"quit", "shutdown", "exit"};

  int sockfd, clientfd1, clientfd2, clientfd3, clientfd4, clientfd5;
  char msg[BUFSIZE];
  const char *greeting = "hello, world\n";
  int running = 1;
  int result;
  int pid;
  int pid_array[5];
  int clientdf_array[5] = {clientfd1 = 0, clientfd2 = 0, clientfd3 = 0, clientfd4 = 0, clientfd5 = 0};

  char *server_name = argv[1];
  int server_pid = atoi(argv[2]);

  /*
      create server, then fork,
      parent process is the server
      and is waiting for a "shutdown"
      command from the server itself.
      The child process then creates
      5 child processes which
      all accept a connection
  */

  if (create_server(server_name, server_pid, &sockfd) < 0)
  {
    fprintf(stderr, "oh no\n");
    return -1;
  }

  // Parent PID creates the server
  pid = fork();

  /*
    Child takes care of 5 processes
    Child process has return 0
  */
  if (pid == 0)
  {
    /*
      Each child accepts one connection
    */
    for (int i = 0; i < 5; i++)
    {
      pid_array[i] = fork();
      if (pid_array[i] == 0)
      {
        if (accept_connection(sockfd, &clientdf_array[i]) < 0)
        {
          fprintf(stderr, "oh no\n");
          return -1;
        }

        while (strcmp(msg, "quit\n"))
        {
          memset(msg, 0, sizeof(msg));
          ssize_t byte_count = recv_message(clientdf_array[i], msg, BUFSIZE);
          logic(msg, two_param, one_param, no_param, clientdf_array[i]);
          // send_message(clientdf_array[i], result_string, strlen(result_string));

          // TODO: PARSE INPUT HERE
        }
      }
    }
  }

  return 0;
}

void logic(char *msg, char **two_param, char **one_param, char **no_param, int clientdf)
{
  int result = 0;
  char *param = strtok(msg, " ");
  if (inArray(param, two_param, 3))
  {
    int x = atoi(strtok(NULL, " "));
    int y = atoi(strtok(NULL, " "));
    three_params(param, x, y, clientdf);
  }
  else if (inArray(param, one_param, 2))
  {
    int x = atoi(strtok(NULL, " "));
    two_params(param, x, clientdf);
  }
  else if (inArray(param, no_param, 3))
  {
    printf("\nQUIT\n");
  }
  else
  {
    printf("FFFFFFFFF");
  }

  printf("Client: %s\n", msg);
}

////////////////////////
// MATH
////////////////////////
void addInts(int a, int b, int clientdf)
{
  int result = a + b;
  char *result_string = malloc(sizeof(char *) * 3);
  sprintf(result_string, "%d", result);
  send_message(clientdf, result_string, strlen(result_string));
}

void multiplyInts(int a, int b, int clientdf)
{
  int result = a * b;
  char *result_string = malloc(sizeof(char *) * 3);
  sprintf(result_string, "%d", result);
  send_message(clientdf, result_string, strlen(result_string));
}

void divideFloats(float a, float b, int clientdf)
{
  if (b == 0)
  {
    fprintf(stderr, "Cannot divide by 0\n");
    char *division_by_0_error = "Division by zero";
    send_message(clientdf, division_by_0_error, strlen(division_by_0_error));
  }
  float result = a / (float)(b);
  char *result_string = malloc(sizeof(char *) * 3);
  sprintf(result_string, "%.6f", result);
  send_message(clientdf, result_string, strlen(result_string));
}

////////////////////////
// SINGLE
////////////////////////
int sleepy(int x, int clientdf)
{
  sleep(x);
  char *result_string = "";
  send_message(clientdf, result_string, strlen(result_string));
  return TRUE;
}
// make the calculator sleep for x seconds – this is blocking
void factorial(int x, int clientdf)
{
  uint64_t result = 1;
  if (x > 0)
    for (int i = 1; i <= x; i++)
    {
      result *= i;
    }
  char *result_string = malloc(sizeof(char *) * 3);
  sprintf(result_string, "%llu", result);
  send_message(clientdf, result_string, strlen(result_string));
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

void three_params(char *cmd, int x, int y, int clientdf)
{
  float result = 0;
  if (!strcmp(cmd, "add"))
  {
    addInts(x, y, clientdf);
  }
  else if (!strcmp(cmd, "multiply"))
  {
    multiplyInts(x, y, clientdf);
  }
  else if (!strcmp(cmd, "divide"))
  {
    divideFloats((float)x, (float)y, clientdf);
  }
}

void two_params(char *cmd, int x, int clientdf)
{
  float result = 0;
  if (!strcmp(cmd, "sleep"))
  {
    sleepy(x, clientdf);
  }
  else if (!strcmp(cmd, "factorial"))
  {
    factorial(x, clientdf);
  }
}
