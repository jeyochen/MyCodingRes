/*
 * InitObj.h
 *
 *  Created on: Nov 24, 2011
 *      Author: xwq
 */

#ifndef INITOBJ_H_
#define INITOBJ_H_

#include 	 <pthread.h>
#include "CCriticalSection.h"
#define MAX_PATH  255

class CLog
{
public:
	CLog(bool logLevel = true);
	~CLog();
	int WriteLog(const char *format, ...);
private:
	int GetModuleFilename(char* sModuleName, char* sFileName, int nSize);
	int DeleteExpiredFile(const char *strDirPath, char * strFileName);
	char m_strWorkPath[MAX_PATH + 1];
    char m_strFileName[MAX_PATH];
	CCriticalSection m_mutex;
	bool bLogLevel;
};

#endif /* INITOBJ_H_ */
