/*
 * 	Clog.cpp
 *
 *  Created on: Nov 24, 2011
 *  Author: wyl
 */
#include "CLog.h"
#include <stdio.h>
#include <string.h>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <dirent.h>
#include <unistd.h>
#include <stdarg.h>
#include <pthread.h>

#define						LOG_MAX_COUNT					3
#define						MAX_PATH						255


int CLog::GetModuleFilename(char* sModuleName, char* sFileName, int nSize)
{
	int ret = -1;
	char sLine[1024] = { 0 };
	void* pSymbol = (void*) "";
	FILE *fp;
	char *pPath;

	fp = fopen("/proc/self/maps", "r");
	if (fp != NULL)
	{
		while (!feof(fp))
		{
			unsigned long start, end;

			if (!fgets(sLine, sizeof(sLine), fp))
				continue;
			if (!strstr(sLine, " r-xp ") || !strchr(sLine, '/'))
				continue;

			sscanf(sLine, "%lx-%lx ", &start, &end);
			if (pSymbol >= (void *) start && pSymbol < (void *) end)
			{
				char *tmp;
				pPath = strchr(sLine, '/');
				tmp = strrchr(pPath, '\n');
				if (tmp)
					*tmp = 0;
				ret = 0;
				strcpy(sFileName, pPath);
			}
		}
		fclose(fp);
	}
	return ret;
}

CLog::CLog(bool logLevel)
{
	bLogLevel = logLevel;
	char sTemp[MAX_PATH]="";
	GetModuleFilename(sTemp, m_strWorkPath, MAX_PATH);

    // 获取文件名
    char *p = strrchr(m_strWorkPath, ('/'));
    memset(m_strFileName, 0, sizeof(m_strFileName));
    if (p != NULL)
    {
        strcpy(m_strFileName, p+1);
        // m_strWorkPath中只保留工作路径
        p[1] = 0;
    }
    else
    {
        strcpy(m_strFileName, "AnFang");
    }
    
}

CLog::~CLog()
{

}

//*******流水函数********
//buf-流水内容字符串
//查找指定文件并找出最久前的一个文件
int CLog::DeleteExpiredFile(const char *strDirPath, char * strFileName)
{
	char tempLog[MAX_PATH] = "";
	strcpy(tempLog, strDirPath);
	strcat(tempLog, "Log");
	int nFileCount = 0;
	time_t curFileTime;
	time_t MostOldFile;
	struct tm *timenow;
	struct tm *timepre;
	int iYear, iMonth, iDay;

	char strDeleteFileName[MAX_PATH] = "";
	bool bFirstFile = true;

	DIR *dirp;
	struct dirent *direntp = NULL;

	if ((dirp = opendir(tempLog)) == NULL)
	{
		return -1;
	}

	while ((direntp = readdir(dirp)) != NULL)
	{
		if (!strcmp(direntp-> d_name, ".") || !strcmp(direntp-> d_name, ".."))
			continue;
		struct stat statbuf;
		char filename[MAX_PATH];
		memset(filename, '\0', MAX_PATH);
		strcpy(filename, tempLog);
		strcat(filename, "/");
		strcat(filename, direntp->d_name);

		if (stat(filename, &statbuf) == -1)
		{
			return -2;
		}
		timenow = localtime(&statbuf.st_mtime);

		if (S_ISDIR(statbuf.st_mode))
		{
			char dirpath[MAX_PATH];
			memset(dirpath, '\0', MAX_PATH);

			strcpy(dirpath, strDirPath);
			strcat(dirpath, "/ ");
			strcat(dirpath, direntp->d_name);
			DeleteExpiredFile(dirpath, strFileName);
		}
		else
		{
			curFileTime = statbuf.st_mtime;
			if (bFirstFile)
			{
				bFirstFile = false;
				MostOldFile = curFileTime;

				strcpy(strDeleteFileName, filename);
			}
			else
			{
				timepre = gmtime(&MostOldFile);
				iYear = timepre->tm_year;
				iMonth = timepre->tm_mon;
				iDay = timepre->tm_mday;

				timenow = gmtime(&curFileTime);

				if (iYear > timenow->tm_year)
				{
					MostOldFile = curFileTime;
					strcpy(strDeleteFileName, filename);
				}
				else if ((iMonth > timenow->tm_mon) && (iYear
						== timenow->tm_year))
				{
					MostOldFile = curFileTime;
					strcpy(strDeleteFileName, filename);
				}
				else if (iDay > timenow->tm_mday && iMonth == timenow->tm_mon
						&& iYear == timenow->tm_year)
				{
					MostOldFile = curFileTime;
					strcpy(strDeleteFileName, filename);
				}
			}
			if (nFileCount++ >= LOG_MAX_COUNT)
			{
				//删除最早的文件
				remove(strDeleteFileName);
			}
		}
	}
	closedir(dirp);
	return 1;
}

int CLog::WriteLog(const char *format, ...)
{
	if (!bLogLevel)
	{
		return 0;
	}
	FILE *fp = NULL;
	time_t now;
	struct tm *timenow;
	char sLogFile[MAX_PATH + 1] = "";
	static char buf[1024] = {0};
	
	if (m_mutex.bInit == false)
	{
		m_mutex.bInit = true;
		//创建日志目录
		sprintf(sLogFile, "%sLog",m_strWorkPath);

		//如果目录不存在，则创建
		struct stat statbuf;
		stat(sLogFile, &statbuf);
		if (ENOENT == errno) //If folder  not exist
		{
			mkdir(sLogFile, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		}
	}
	time(&now);
	timenow = localtime(&now);
	sprintf(sLogFile, "%sLog/log_%s_%04d%02d%02d.log",
			m_strWorkPath, m_strFileName, 1900 + timenow->tm_year, timenow->tm_mon
					+ 1, timenow->tm_mday);

	//删除过期日志
	DeleteExpiredFile(m_strWorkPath, sLogFile);
	m_mutex.Lock();

	fp = fopen(sLogFile, "a+");
	if (fp)
	{
		memset(buf, 0x00, sizeof(buf));
		va_list ap;
		va_start(ap, format);
		vsnprintf(buf, sizeof(buf) - 1, format, ap);
		va_end(ap);
	
		int processID = getpid();
        pthread_t threadID = pthread_self();
		time(&now);
		timenow = localtime(&now);

        // 取当前时间精确到微秒
        timeval usec;
        gettimeofday(&usec, NULL);
		fprintf(fp, "[%04d-%02d-%02d %02d:%02d:%02d.%03d][%d][%X]%s\n", 1900
				+ timenow->tm_year, timenow->tm_mon + 1, timenow->tm_mday,
				timenow->tm_hour, timenow->tm_min, timenow->tm_sec, usec.tv_usec/1000,
				processID, threadID, buf);

		fclose(fp);
	}
	m_mutex.Unlock();
	return 0;
}
