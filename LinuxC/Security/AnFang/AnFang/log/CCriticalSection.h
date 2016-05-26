#ifndef __CCRITICALSECTION_H
#define __CCRITICALSECTION_H
#include <pthread.h>

class CCriticalSection
{
public:
	bool bInit;
private:
	pthread_mutex_t mutex;
public:
	CCriticalSection();
	~CCriticalSection();
public:
	void Lock();
	void Unlock();
};
#endif

