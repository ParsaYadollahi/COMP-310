#include <stdint.h>

int addInts(int a, int b);      // add two integers
int multiplyInts(int a, int b); // multiple two integers
float divideFloats(float a, float b);
// divide float numbers (report divide by zero error)
int sleep(int x);
// make the calculator sleep for x seconds – this is blocking
uint64_t factorial(int x); // return factorial x
bool inArray(char *word, char **cmd, int len_cmd);
