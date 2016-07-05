#include "Log.h"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <time.h>
#include <sys/timeb.h>
#include <stdarg.h>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#else
#include <unistd.h>
#include <sys/time.h>
#endif

#define  MAX_PATH 255

CLog* CLog::m_pInstance = NULL;

CLog::CLog(void)
    :m_isWriteLog(true)
    ,m_logLevel(DEBUG)
    ,m_preName("log")
{
}


CLog::~CLog(void)
{
    if(m_pInstance)
    {
        delete m_pInstance;
        m_pInstance = NULL;
    }
}

CLog* CLog::GetInstance()
{
    if (NULL == m_pInstance)
    {
        m_pInstance = new CLog();
    }

    return m_pInstance;
}

void CLog::IsWreiteLog(bool iswrite)
{
    m_isWriteLog = iswrite;
}

// ��־�������Ļ
bool CLog::printlog(const char* format ...)
{
    char buf[1024] = {0};
    memset(buf, 0x00, sizeof(buf));
    va_list ap;
    va_start(ap, format);
    vsnprintf(buf, sizeof(buf) - 1, format, ap);
    va_end(ap);

    struct timeb tb;
    ftime(&tb);
    struct tm *timenow = localtime(&tb.time);

    // ȡ��ǰʱ�侫ȷ��΢��
    fprintf(stdout, "[%04d-%02d-%02d %02d:%02d:%02d.%03ld] %s\n", 1900
        + timenow->tm_year, timenow->tm_mon + 1, timenow->tm_mday,
        timenow->tm_hour, timenow->tm_min, timenow->tm_sec, tb.millitm,
        buf);
 
    return true;
}


// ������־����
void CLog::SetLogLevel(int level)
{
    m_logLevel = level;
}

// ������־����
int CLog::GetLogLevel()
{
    return m_logLevel;
}

// ������־�ļ�ǰ׺
void CLog::SetPreName(const char* prename)
{
    m_preName = prename;
}

// ������־�ļ�ǰ׺
std::string CLog::GetPreName()
{
    return m_preName;
}

// дdebug��־
bool CLog::debug_log(const char* format ...)
{
    if(!m_isWriteLog) return false;

    if (m_logLevel & DEBUG)
    {
        FILE *fp = NULL;
        char sLogFile[MAX_PATH + 1] = {0};
        static char buf[1024] = {0};

        struct timeb tb;
        ftime(&tb);
        struct tm *timenow =  localtime(&tb.time);

        sprintf(sLogFile, "%s_%04d%02d%02d.log", m_preName.c_str(),
            1900 + timenow->tm_year, timenow->tm_mon
            + 1, timenow->tm_mday);

        fp = fopen(sLogFile, "a+");
        if (fp)
        {
            memset(buf, 0x00, sizeof(buf));
            va_list ap;
            va_start(ap, format);
            vsnprintf(buf, sizeof(buf) - 1, format, ap);
            va_end(ap);

            struct timeb tb;
            ftime(&tb);
            timenow = localtime(&tb.time);

            fprintf(fp, "[%04d-%02d-%02d %02d:%02d:%02d.%03ld][DEBUG] %s\n", 1900
                + timenow->tm_year, timenow->tm_mon + 1, timenow->tm_mday,
                timenow->tm_hour, timenow->tm_min, timenow->tm_sec, tb.millitm,
                buf);
            
            if (m_logLevel & BOTH)
            {
                fprintf(stdout, "[%04d-%02d-%02d %02d:%02d:%02d.%03ld][DEBUG] %s\n", 1900
                    + timenow->tm_year, timenow->tm_mon + 1, timenow->tm_mday,
                    timenow->tm_hour, timenow->tm_min, timenow->tm_sec, tb.millitm,
                    buf);
            }
            fclose(fp);
            fp = NULL;
        }
    }
    return true;
}

// дinfo��־
bool CLog::info_log(const char* format ...)
{
    if(!m_isWriteLog) return false;

    if (m_logLevel & INFO)
    {
        FILE *fp = NULL;
        char sLogFile[MAX_PATH + 1] = {0};
        static char buf[1024] = {0};
        
        struct timeb tb;
        ftime(&tb);
        struct tm *timenow =  localtime(&tb.time);

        sprintf(sLogFile, "%s_%04d%02d%02d.log", m_preName.c_str(),
            1900 + timenow->tm_year, timenow->tm_mon
            + 1, timenow->tm_mday);

        fp = fopen(sLogFile, "a+");
        if (fp)
        {
            memset(buf, 0x00, sizeof(buf));
            va_list ap;
            va_start(ap, format);
            vsnprintf(buf, sizeof(buf) - 1, format, ap);
            va_end(ap);

            fprintf(fp, "[%04d-%02d-%02d %02d:%02d:%02d.%03ld][INFO ] %s\n", 1900
                + timenow->tm_year, timenow->tm_mon + 1, timenow->tm_mday,
                timenow->tm_hour, timenow->tm_min, timenow->tm_sec, tb.millitm,
                buf);

            if (m_logLevel & BOTH)
            {
                fprintf(stdout, "[%04d-%02d-%02d %02d:%02d:%02d.%03ld][INFO ] %s\n", 1900
                    + timenow->tm_year, timenow->tm_mon + 1, timenow->tm_mday,
                    timenow->tm_hour, timenow->tm_min, timenow->tm_sec, tb.millitm,
                    buf);
            }
            fclose(fp);
            fp = NULL;
        }
    }

    return true;
}

// дwarn��־
bool CLog::warn_log(const char* format ...)
{
    if(!m_isWriteLog) return false;

    if(m_logLevel & WARN)
    {
        FILE *fp = NULL;
        char sLogFile[MAX_PATH + 1] = {0};
        static char buf[1024] = {0};
        
        struct timeb tb;
        ftime(&tb);
        struct tm *timenow = localtime(&tb.time);

        sprintf(sLogFile, "%s_%04d%02d%02d.log", m_preName.c_str(),
            1900 + timenow->tm_year, timenow->tm_mon
            + 1, timenow->tm_mday);

        fp = fopen(sLogFile, "a+");
        if (fp)
        {
            memset(buf, 0x00, sizeof(buf));
            va_list ap;
            va_start(ap, format);
            vsnprintf(buf, sizeof(buf) - 1, format, ap);
            va_end(ap);

            fprintf(fp, "[%04d-%02d-%02d %02d:%02d:%02d.%03ld][WARN ] %s\n", 1900
                + timenow->tm_year, timenow->tm_mon + 1, timenow->tm_mday,
                timenow->tm_hour, timenow->tm_min, timenow->tm_sec, tb.millitm,
                buf);

            if (m_logLevel & BOTH)
            {
                fprintf(stdout, "[%04d-%02d-%02d %02d:%02d:%02d.%03ld][WARN ] %s\n", 1900
                    + timenow->tm_year, timenow->tm_mon + 1, timenow->tm_mday,
                    timenow->tm_hour, timenow->tm_min, timenow->tm_sec, tb.millitm,
                    buf);
            }
            fclose(fp);
            fp = NULL;
        }
    }

    return true;
}

// дerror��־
bool CLog::err_log(const char* format ...)
{
    if(!m_isWriteLog) return false;

    if (m_logLevel & ERR)
    {
        FILE *fp = NULL;
        char sLogFile[MAX_PATH + 1] = {0};
        static char buf[1024] = {0};
        
        struct timeb tb;
        ftime(&tb);
        struct tm *timenow = localtime(&tb.time);

        sprintf(sLogFile, "%s_%04d%02d%02d.log", m_preName.c_str(),
            1900 + timenow->tm_year, timenow->tm_mon
            + 1, timenow->tm_mday);

        fp = fopen(sLogFile, "a+");
        if (fp)
        {
            memset(buf, 0x00, sizeof(buf));
            va_list ap;
            va_start(ap, format);
            vsnprintf(buf, sizeof(buf) - 1, format, ap);
            va_end(ap);

            fprintf(fp, "[%04d-%02d-%02d %02d:%02d:%02d.%03ld][ERR  ] %s\n", 1900
                + timenow->tm_year, timenow->tm_mon + 1, timenow->tm_mday,
                timenow->tm_hour, timenow->tm_min, timenow->tm_sec, tb.millitm,
                buf);

            if (m_logLevel & BOTH)
            {
                fprintf(stdout, "[%04d-%02d-%02d %02d:%02d:%02d.%03ld][ERR  ] %s\n", 1900
                    + timenow->tm_year, timenow->tm_mon + 1, timenow->tm_mday,
                    timenow->tm_hour, timenow->tm_min, timenow->tm_sec, tb.millitm,
                    buf);
            }
            fclose(fp);
            fp = NULL;
        }
    }
    return true;
}