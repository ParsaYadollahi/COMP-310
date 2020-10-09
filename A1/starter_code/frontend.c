#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "messages.h"
#include "a1_lib.h"

#define BUFSIZE 1024

int main(int argc, char *argv[])
{
  int sockfd;
  char user_input[BUFSIZE] = {0};
  char server_msg[BUFSIZE] = {0};

  char *server_name = argv[1];
  int server_pid = atoi(argv[2]);

  if (connect_to_server(server_name, server_pid, &sockfd) < 0)
  {
    fprintf(stderr, "oh no\n");
    return -1;
  }
  while (strcmp(user_input, "quit\n"))
  {
    memset(user_input, 0, sizeof(user_input));
    memset(server_msg, 0, sizeof(server_msg));
    printf(">> ");

    // read user input from command line
    fgets(user_input, BUFSIZE, stdin);

    if (!strcmp(user_input, "exit\n"))
    {
      // Exit from loop and print ciao
      printf("Bye!\n");
      break;
    }
    // send the input to server
    send_message(sockfd, user_input, strlen(user_input));
    // receive a msg from the server
    ssize_t byte_count = recv_message(sockfd, server_msg, sizeof(server_msg));
    if (byte_count <= 0)
    {
      break;
    }
    printf("%s\n", server_msg);
  }

  return 0;
}
