#include "Thread.h"
#include <unistd.h>

pthread_t Create_Thread(void (pFun)(void *),void *arg)
{
	pthread_t thread_tid = 0;
	void *(*pLinuxFun)(void *);
	pLinuxFun = (void* (*)(void*))pFun;
	pthread_create(&thread_tid,NULL,pLinuxFun,arg);
	return thread_tid;
}

pthread_t Create_ThreadAndPriority(INT32 priority,void (pFun)(void *),void *arg)
{
	pthread_t thread_tid = 0;
	pthread_attr_t prior;
	struct sched_param param;
	void *(*pLinuxFun)(void *);
	pLinuxFun = (void* (*)(void*))pFun;
	if(priority == 0)
	{
		pthread_create(&thread_tid,NULL,pLinuxFun,arg);
		return thread_tid;
	}
	else
	{
		pthread_attr_init(&prior);
		pthread_attr_getschedparam(&prior,&param);
		param.sched_priority = priority;
		pthread_attr_setschedpolicy(&prior,SCHED_RR);
		pthread_attr_setschedparam(&prior,&param);
		pthread_create(&thread_tid,&prior,pLinuxFun,arg);
	}
}


INT32 Close_Thread(pthread_t threadid)
{
	if(threadid != 0)
	{
		pthread_join(threadid,NULL);
		return RETURN_OK;
	}
	return RETURN_FAILED;
}

void ThreadSleep(INT32 tparam)
{
	usleep(tparam*1000);
}

//int main()
//{
//	return 0;
//}
