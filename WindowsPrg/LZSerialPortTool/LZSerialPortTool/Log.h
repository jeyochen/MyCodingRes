#pragma once

#include <string>

enum LOG_LEVEL{
    LOG_NOLOG = 0, // �������־
    LOG_DEBUG = 1, // ���debug��־
    LOG_INFO = 2, // ���info��־
    LOG_WARN = 4, // ���warning��־
    LOG_ERR = 8, // ���error��־
    LOG_BOTH = 16 // ͬʱ�������Ļ
};

class CLog
{
public:
    ~CLog(void);

    // �������Ψһʵ��
    static CLog* GetInstance();
    
    // �Ƿ�д��־ trueд��־�� false��д��־
    void IsWreiteLog(bool iswrite);

    // ��־�������Ļ
    bool printlog(const char* format ...);

    // ������־����
    void SetLogLevel(int level);

    // ������־����
    int GetLogLevel();

    // ������־�ļ�ǰ׺
    void SetPreName(const char* prename);

    // ������־�ļ�ǰ׺
    std::string GetPreName();

    // дdebug��־
    bool debug_log(const char* format ...);

    // дinfo��־
    bool info_log(const char* format ...);

    // дwarn��־
    bool warn_log(const char* format ...);

    // дerror��־
    bool err_log(const char* format ...);
protected:
    CLog(void);

private:
    bool m_isWriteLog; // �Ƿ�д��־
    int m_logLevel; // ��־����
    std::string m_preName; // ��־�ļ���ǰ׺
    static CLog* m_pInstance;
};

#define  LOG (*(CLog::GetInstance()))

