#ifndef   _LOCK_H 
#define   _LOCK_H 

//#include <stdef.h>
//#include "StdAfx.h"
//#include <stdef.h>
#include "mgmt_types.h"
#include <pthread.h>
#include <time.h>
#include <errno.h>


//#define TIME_WAIT_FOR_EVER 0;
//xian cheng shuo
pthread_mutex_t CreateLock();
pthread_cond_t CreateEvent();
INT32 Lock(pthread_mutex_t* mutex,INT32 passMicroSeconds);
INT32 Unlock(pthread_mutex_t* mutex);
INT32 GetEvent(pthread_cond_t* cond,pthread_mutex_t* mutex);
INT32 SetEvent(pthread_cond_t* cond);

#endif
