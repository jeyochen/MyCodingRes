#include "CCriticalSection.h"

CCriticalSection::CCriticalSection()
{
	bInit = false;
	pthread_mutex_init(&mutex, NULL);
}
CCriticalSection::~CCriticalSection()
{
	pthread_mutex_destroy(&mutex);
}
void CCriticalSection::Lock()
{
	pthread_mutex_lock(&mutex);
}
void CCriticalSection::Unlock()
{
	pthread_mutex_unlock(&mutex);
}
