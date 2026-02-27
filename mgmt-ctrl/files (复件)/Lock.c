#include "Lock.h"

pthread_mutex_t CreateLock()
{
	pthread_mutex_t mutex;
	pthread_mutex_init(&mutex,NULL);
	return mutex;
}

INT32 Lock(pthread_mutex_t* mutex,INT32 passMicroSeconds)
{
	INT32 waitResult;
	if(passMicroSeconds == 0)
	{
		waitResult = pthread_mutex_lock(mutex);
	}
	else
	{
		struct timespec abs_timeout;
		abs_timeout.tv_sec = passMicroSeconds;
		abs_timeout.tv_nsec = 0;
		waitResult = pthread_mutex_timedlock(mutex,&abs_timeout);
	}
	switch(waitResult)
	{
	case 0:
		return RETURN_OK;
	case ETIMEDOUT:
		return RETURN_TIMEOUT;
	default:
		return RETURN_FAILED;
	} 
}


INT32 Unlock(pthread_mutex_t* mutex)
{
	 return pthread_mutex_unlock(mutex);
}

pthread_cond_t CreateEvent()
{
	pthread_cond_t cond;
	pthread_cond_init(&cond,NULL);
	return cond;
}

INT32 GetEvent(pthread_cond_t* cond,pthread_mutex_t* mutex)
{
	int i = pthread_cond_wait(cond,mutex);
	return i;
}

INT32 SetEvent(pthread_cond_t* cond)
{
	int i = pthread_cond_broadcast(cond);
	return i;
}
