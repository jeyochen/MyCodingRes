#pragma once

#include <string>

enum LOG_LEVEL{
    LOG_NOLOG = 0, // 不输出日志
    LOG_DEBUG = 1, // 输出debug日志
    LOG_INFO = 2, // 输出info日志
    LOG_WARN = 4, // 输出warning日志
    LOG_ERR = 8, // 输出error日志
    LOG_BOTH = 16 // 同时输出到屏幕
};

class CLog
{
public:
    ~CLog(void);

    // 返回类的唯一实例
    static CLog* GetInstance();
    
    // 是否写日志 true写日志， false不写日志
    void IsWreiteLog(bool iswrite);

    // 日志输出到屏幕
    bool printlog(const char* format ...);

    // 设置日志级别
    void SetLogLevel(int level);

    // 返回日志级别
    int GetLogLevel();

    // 设置日志文件前缀
    void SetPreName(const char* prename);

    // 返回日志文件前缀
    std::string GetPreName();

    // 写debug日志
    bool debug_log(const char* format ...);

    // 写info日志
    bool info_log(const char* format ...);

    // 写warn日志
    bool warn_log(const char* format ...);

    // 写error日志
    bool err_log(const char* format ...);
protected:
    CLog(void);

private:
    bool m_isWriteLog; // 是否写日志
    int m_logLevel; // 日志级别
    std::string m_preName; // 日志文件名前缀
    static CLog* m_pInstance;
};

#define  LOG (*(CLog::GetInstance()))

