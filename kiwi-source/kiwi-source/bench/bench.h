#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>

#define KSIZE (16)
#define VSIZE (1000)

#define LINE "+-----------------------------+----------------+------------------------------+-------------------+\n"
#define LINE1 "---------------------------------------------------------------------------------------------------\n"

long long get_ustime_sec(void);
void _random_key(char *key,int length);

//structure with fields r, count(requests),numthr (num of the threads)
struct args
{   int rArg;
    long int countArg;
    int numthr;
};

//global vars for the cost of each operation(write/read)
double wrCost;
double rdCost;

//mutexes for globals vars wrCost/rdCost
pthread_mutex_t writeCmut;
pthread_mutex_t readCmut;




