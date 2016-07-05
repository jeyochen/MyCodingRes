#pragma once

/************************************************************
* 
* 功能：使用 openssl 实现aes加解密
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
#include <openssl/aes.h>

class CAesEncDec
{
public:
	CAesEncDec(void);
	~CAesEncDec(void);

    /************************************************************
    * 功能: Aes 加密
    * 入参: source d待加密的字符串
    *       key 加密密钥 
    * 出参：strHex 加密后的十六进制字符串
    * 返回值: 加密后的密文
    ************************************************************/
	std::string AesEncrypt(const std::string& source, const std::string& key,
        std::string& strHex);

    /************************************************************
    * 功能: Aes解密
    * 入参: ciphertext 待解密的密文
    * 出参：
    * 返回值: 返回解密后的明文
    ************************************************************/
	std::string AesDecrypt(const std::string& ciphertext, 
        const std::string& key);

    /************************************************************
    * 功能: Aes 加密
    * 入参: source 待加密的字符串
    *       key 加密密钥 
    * 出参：strHex 加密后的十六进制字符串
    * 返回值: 加密后的密文
    * 说明：次方法为手动实现 PKCS5Padding 补齐发
    ************************************************************/
    std::string AESEcbEncryptPKCS5Padding(const std::string& source, const std::string& key,
        std::string& strHex, unsigned char *out, int& outlen);

     /************************************************************
    * 功能: Aes 加密
    * 入参: ciphertext 待解密的字密文
    *       chiplen 密文长度
    *       key 解密密钥 
    * 出参：
    * 返回值: 解密后的密文
    ************************************************************/
    std::string AESEcbDecryptPKCS5Padding(const unsigned char* ciphertext, const int chiplen,
        const std::string& key);

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
};

