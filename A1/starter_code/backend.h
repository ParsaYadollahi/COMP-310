#include <stdint.h>

void addInts(int a, int b, int clientdf);      // add two integers
void multiplyInts(int a, int b, int clientdf); // multiple two integers
void divideFloats(float a, float b, int clientdf);
// divide float numbers (report divide by zero error)
void sleepy(int x);
// make the calculator sleep for x seconds – this is blocking
void factorial(int x, int clientdf); // return factorial x
bool inArray(char *word, char **cmd, int len_cmd);
void three_params(char *cmd, int x, int y, int clientdf);
void two_params(char *cmd, int x, int clientdf);
void logic(char *msg, char **two_param, char **one_param, char **no_param, int clientdf);
