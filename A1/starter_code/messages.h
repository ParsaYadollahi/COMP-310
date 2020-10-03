#ifndef RPC_H_
#define RPC_H_

#define CMD_LENGTH 256
#define ARGS_LENGTH 256

typedef struct message_t
{
  char cmd[CMD_LENGTH];
  char param1[ARGS_LENGTH];
  char param2[ARGS_LENGTH];
} message_t;

#endif
