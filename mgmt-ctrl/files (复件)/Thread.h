#ifndef   _THREAD_H
#define   _THREAD_H

#include "mgmt_types.h"
#include <pthread.h>
//#include <process.h>

pthread_t Create_Thread(void (pFun)(void *),void *arg);
pthread_t Create_ThreadAndPriority(INT32 priority,void (pFun)(void *),void *arg);
INT32 Close_Thread(pthread_t threadid);
void ThreadSleep(INT32 tparam);

#endif
