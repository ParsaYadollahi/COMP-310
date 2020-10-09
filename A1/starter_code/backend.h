#include <stdint.h>

void addInts(int a, int b, int clientdf);      // add two integers
void multiplyInts(int a, int b, int clientdf); // multiple two integers
void divideFloats(float a, float b, int clientdf);
// divide float numbers (report divide by zero error)
int sleepy(int x, int clientdf);
// make the calculator sleep for x seconds – this is blocking
void factorial(int x, int clientdf); // return factorial x
bool inArray(char *word, char **cmd, int len_cmd);
void calculations(char *cmd, int x, int y, int clientdf);
void two_params(char *cmd, int x, int clientdf);
void logic(char *msg, char **two_param, char **one_param, int clientdf, int pid);
void shutdown_cmd(int clientdf, int pid);
void strip(char *s);
