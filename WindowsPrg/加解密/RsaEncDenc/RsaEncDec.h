#pragma once

/************************************************************
* 
* 功能：使用 openssl 实现Rsa加解密
* 
* create by chenjiyou 2016-06-27
*
*************************************************************/

#include <string>
#include <iostream>
#include <Windows.h>
#include <stdio.h>
#include <assert.h>
#include <openssl/objects.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/err.h>

class CRsaEncDec
{
public:
    CRsaEncDec(void);
    ~CRsaEncDec(void);

public:    
    /************************************************************
    * 功能: 读取公钥文件
    * 入参: keyfile 公钥文件名
    *       readfile 是否读取pem文件 true 读，false不读
    * 返回值: true 成功，false失败
    ************************************************************/
    bool ReadPublickey(const std::string& keyfile, bool readfile = false);

    /************************************************************
    * 功能: 读取私钥文件
    * 入参: keyfile 私钥文件名
    *       readfile 是否读取pem文件， true 读，false不读
    * 返回值: true 成功，false失败
    ************************************************************/
    bool ReadPrivatekey(const std::string& keyfile, bool readfile = false);

    /************************************************************
    * 功能: 公钥加密
    * 入参: strData 待加密内容
    * 出参：hexStr 密文的十六进制表示
    * 返回值: 密文
    ************************************************************/
    std::string RsaEncrypt(const std::string& strData, std::string& hexStr);

    /************************************************************
    * 功能: 私钥解密
    * 入参: chiper 待解密内容
    * 返回值: 明文
    ************************************************************/
    std::string RsaDecrypt(unsigned char *chiper);

    /************************************************************
    * 功能: 字符串转十六进制
    * 入参: pbSrc 待转的字符串
    *       nLen 待转换的字符串的长度
    * 出参：pbDest 存放转换结果
    * 返回值: 明文
    ************************************************************/
    void StrToHex(unsigned char *pbDest, const char *pbSrc, int nLen);

    /************************************************************
    * 功能: 十六进制转字符串
    * 入参: pbSrc 待转的字符串
    *       nLen 待转换的字符串的长度
    * 出参：pbDest 存放转换结果
    * 返回值: 明文
    ************************************************************/
    void HexToStr(unsigned char *pbDest, 
        const unsigned char *pbSrc, int nLen);

     /************************************************************
    * 功能: UTF8编码转到GBK编码
    * 入参: lpUTF8Str utf8编码的字符串
            nGBKStrLen 存放GBK的数组大小
    * 出参：lpGBKStr 存放转换后的GBK字符串   
    * 返回值: 0 失败 大于0成功为时间转换的字符数
    ************************************************************/
    int UTF8ToGBK(unsigned char *lpUTF8Str,
        unsigned char *lpGBKStr,int nGBKStrLen);

    /************************************************************
    * 功能: GBK编码转换到UTF8编码
    * 入参: lpGBKStr GBK编码的字符串
            nUTF8StrLen 存放UTF8的数组大小
    * 出参：lpUTF8Str 存放转换后的UTF8字符串   
    * 返回值: 0 失败 大于0成功为时间转换的字符数
    ************************************************************/
    int GBKToUTF8(unsigned char * lpGBKStr,
        unsigned char * lpUTF8Str,int nUTF8StrLen);

private:
    RSA* m_privateKey; // 私钥
    RSA* m_publicKey; // 公钥
};

