/**
 *  @author: Parsa Yadollahi
 *  @email: parsa.yadollahi@mail.mcgill.ca
 *  @description: backend
 *  @assignment: RPC - assignment 1
 * @studentid: 260869949
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>

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
  char *no_param[] = {"shutdown\n"};

  int sockfd, clientfd1, clientfd2, clientfd3, clientfd4, clientfd5;
  char msg[BUFSIZE];
  int running = 1;
  int result;
  int pid;
  int pid_array[5];
  int clientdf_array[5] = {clientfd1 = 0, clientfd2 = 0, clientfd3 = 0, clientfd4 = 0, clientfd5 = 0};

  // get command line inputs from user
  char *server_name = argv[1];
  int server_pid = atoi(argv[2]);

  // Create the server
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
      // Child process
      if (pid_array[i] == 0)
      {
        // Accept a connection
        if (accept_connection(sockfd, &clientdf_array[i]) < 0)
        {
          fprintf(stderr, "oh no\n");
          return -1;
        }

        // Loop until we get a message from the frontend
        while (1)
        {
          memset(msg, 0, sizeof(msg));
          ssize_t byte_count = recv_message(clientdf_array[i], msg, BUFSIZE);
          if (byte_count <= 0)
          {
            break;
          }
          // Send message along with other parameters to logic function
          logic(msg, two_param, one_param, clientdf_array[i], pid);
        }
      }
    }
  }

  return 0;
}

/**
 *  The logic of the code
 *  Determines which math functions to call given the frontends parameters
 *
 *  @params:
 *    msg:            message sent by frontend
 *    two_param:      array containing all functions that need 2 paramters.
 *    one_param:      array containing all functions that need 1 paramters.
 *    clientdf:       the client number for the frontend
 *    pid:            pid of the parent
 *  @return:    VOID
 */
void logic(char *msg, char **two_param, char **one_param, int clientdf, int pid)
{
  int result = 0;
  // Get the first paramter of the message
  char *param = strtok(msg, " ");
  // If the parameter is (+ , x , ÷)
  if (inArray(param, two_param, 3))
  {
    // Extract two other parameters
    int x = atoi(strtok(NULL, " "));
    int y = atoi(strtok(NULL, " "));
    // Calculate result
    calculations(param, x, y, clientdf);
  }
  // If parameter is (factorial or sleep)
  else if (inArray(param, one_param, 2))
  {
    // Extract second parameter
    int x = atoi(strtok(NULL, " "));
    // Calculate
    two_params(param, x, clientdf);
  }
  // If command is (shutdown or quit)
  else if (!strcmp("shutdown\n", param) || !strcmp("quit\n", param))
  {
    // Shutdown
    shutdown_cmd(clientdf, pid);
  }
  // User sent an unrecognized command
  else
  {
    char result_string[1024];
    strip(param);
    sprintf(result_string, "Error: Command \"%s\" not found\n", param);
    send_message(clientdf, result_string, strlen(result_string));
  }

  printf("Client: %s\n", msg);
}

////////////////////////
// MATH
////////////////////////
/**
 *  Add integers function
 *
 *  @params:
 *    a:      first argument.
 *    b:      second argument.
 *    clientdf:      the client number for the frontend
 *  @return:    VOID
 */
void addInts(int a, int b, int clientdf)
{
  int result = a + b;
  char *result_string = malloc(sizeof(char *) * 3);
  sprintf(result_string, "%d", result);
  send_message(clientdf, result_string, strlen(result_string));
}

/**
 *  Multiply function
 *
 *  @params:
 *    a:      first argument.
 *    b:      second argument.
 *    clientdf:      the client number for the frontend
 *  @return:    VOID
 */
void multiplyInts(int a, int b, int clientdf)
{
  int result = a * b;
  char *result_string = malloc(sizeof(char *) * 3);
  sprintf(result_string, "%d", result);
  send_message(clientdf, result_string, strlen(result_string));
}

/**
 *  Division function
 *
 *  @params:
 *    a:      first argument.
 *    b:      second argument.
 *    clientdf:      the client number for the frontend
 *  @return:    VOID
 */
void divideFloats(float a, float b, int clientdf)
{
  if (b == 0)
  {
    fprintf(stderr, "Cannot divide by 0\n");
    char *division_by_0_error = "Error: Division by zero";
    send_message(clientdf, division_by_0_error, strlen(division_by_0_error));
  }
  float result = a / (float)(b);
  char *result_string = malloc(sizeof(char *) * 3);
  sprintf(result_string, "%.6f", result);
  send_message(clientdf, result_string, strlen(result_string));
}

////////////////////////
// SINGLE VARIABLE
////////////////////////
/**
 *  Make the calculator sleep for x seconds – this is blocking
 *
 *  @params:
 *    x:      first argument.
 *    clientdf:      the client number for the frontend
 *  @return:    TRUE if success
 */
int sleepy(int x, int clientdf)
{
  sleep(x);
  // Need to malloc else char* wont persevere (won't print)
  char *result_string = malloc(sizeof(char *) * 3);
  // Send back a new line
  sprintf(result_string, "%s", " ");
  send_message(clientdf, result_string, strlen(result_string));
  return TRUE;
}

/**
 *  Factorial functions
 *
 *  @params:
 *    x:      first argument.
 *    clientdf:      the client number for the frontend
 *  @return:    VOID
 */
void factorial(int x, int clientdf)
{
  // Base case = 1
  uint64_t result = 1;
  // Inductive step
  if (x > 0)
    for (int i = 1; i <= x; i++)
    {
      result *= i;
    }
  // Return a message to frontend
  char *result_string = malloc(sizeof(char *) * 3);
  sprintf(result_string, "%llu", result);
  send_message(clientdf, result_string, strlen(result_string));
}

//////////////////////////////////////////
// CUSTOM FUNCTIONS
//////////////////////////////////////////
/**
 *  Determine if string in array.
 *
 *  @params:
 *    word:   the word looking for.
 *    cmd:      list of commands.
 *    len_cmd:      length of list cmd.
 *  @return:    TRUE, Found the command in the array
 *              FASLE Did not find the command in the array
 */
bool inArray(char *word, char **cmd, int len_cmd)
{
  // Iterate though entire array
  for (int i = 0; i < len_cmd; i++)
  {
    if (!strcmp(cmd[i], word))
    {
      return TRUE;
    }
  }
  return FALSE;
}

/**
 *  Calculations for the math part (+, x, ÷).
 *
 *  @params:
 *    cmd:   the command.
 *    x:      first argument.
 *    y:      second argument.
 *    clientdf:      the client number for the frontend
 *  @return:    VOID
 */
void calculations(char *cmd, int x, int y, int clientdf)
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

/**
 *  Sleep and Factorial functions
 *
 *  @params:
 *    cmd:   the word looking for.
 *    x:      first argument.
 *    clientdf:      the client number for the frontend
 *  @return:    VOID
 */
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

/**
 *  shutdown command
 *
 *  @params:
 *    clientdf:      the client number for the frontend
 *    pid:   the PID of the parent process.
 *  @return:    VOID
 */
void shutdown_cmd(int clientdf, int pid)
{
  kill(pid, SIGKILL);
  char *result_string = "Bye!\n";
  send_message(clientdf, result_string, strlen(result_string));
}

/**
 *  Remove \n and \t from given string - in place
 *
 *  @params:
 *    s:      the string you want to modify
 *  @return:    VOID
 */
void strip(char *s)
{
  char *p2 = s;
  while (*s != '\0')
  {
    if (*s != '\t' && *s != '\n')
    {
      *p2++ = *s++;
    }
    else
    {
      ++s;
    }
  }
  *p2 = '\0';
}
