#pragma once

#include <string>

enum LOG_LEVEL{
    NOLOG = 0, // �������־
    DEBUG = 1, // ���debug��־
    INFO = 2, // ���info��־
    WARN = 4, // ���warning��־
    ERR = 8, // ���error��־
    BOTH = 16 // ͬʱ�������Ļ
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

