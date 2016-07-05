#pragma once

/************************************************************
* 
* ���ܣ�ʹ�� openssl ʵ��aes�ӽ���
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
    * ����: Aes ����
    * ���: source d�����ܵ��ַ���
    *       key ������Կ 
    * ���Σ�strHex ���ܺ��ʮ�������ַ���
    * ����ֵ: ���ܺ������
    ************************************************************/
	std::string AesEncrypt(const std::string& source, const std::string& key,
        std::string& strHex);

    /************************************************************
    * ����: Aes����
    * ���: ciphertext �����ܵ�����
    * ���Σ�
    * ����ֵ: ���ؽ��ܺ������
    ************************************************************/
	std::string AesDecrypt(const std::string& ciphertext, 
        const std::string& key);

    /************************************************************
    * ����: Aes ����
    * ���: source �����ܵ��ַ���
    *       key ������Կ 
    * ���Σ�strHex ���ܺ��ʮ�������ַ���
    * ����ֵ: ���ܺ������
    * ˵�����η���Ϊ�ֶ�ʵ�� PKCS5Padding ���뷢
    ************************************************************/
    std::string AESEcbEncryptPKCS5Padding(const std::string& source, const std::string& key,
        std::string& strHex, unsigned char *out, int& outlen);

     /************************************************************
    * ����: Aes ����
    * ���: ciphertext �����ܵ�������
    *       chiplen ���ĳ���
    *       key ������Կ 
    * ���Σ�
    * ����ֵ: ���ܺ������
    ************************************************************/
    std::string AESEcbDecryptPKCS5Padding(const unsigned char* ciphertext, const int chiplen,
        const std::string& key);

    /************************************************************
    * ����: �ַ���תʮ������
    * ���: pbSrc ��ת���ַ���
    *       nLen ��ת�����ַ����ĳ���
    * ���Σ�pbDest ���ת�����
    * ����ֵ: ����
    ************************************************************/
    void StrToHex(unsigned char *pbDest, const char *pbSrc, int nLen);

    /************************************************************
    * ����: ʮ������ת�ַ���
    * ���: pbSrc ��ת���ַ���
    *       nLen ��ת�����ַ����ĳ���
    * ���Σ�pbDest ���ת�����
    * ����ֵ: ����
    ************************************************************/
    void HexToStr(unsigned char *pbDest, 
        const unsigned char *pbSrc, int nLen);

     /************************************************************
    * ����: UTF8����ת��GBK����
    * ���: lpUTF8Str utf8������ַ���
            nGBKStrLen ���GBK�������С
    * ���Σ�lpGBKStr ���ת�����GBK�ַ���   
    * ����ֵ: 0 ʧ�� ����0�ɹ�Ϊʱ��ת�����ַ���
    ************************************************************/
    int UTF8ToGBK(unsigned char *lpUTF8Str,
        unsigned char *lpGBKStr,int nGBKStrLen);

    /************************************************************
    * ����: GBK����ת����UTF8����
    * ���: lpGBKStr GBK������ַ���
            nUTF8StrLen ���UTF8�������С
    * ���Σ�lpUTF8Str ���ת�����UTF8�ַ���   
    * ����ֵ: 0 ʧ�� ����0�ɹ�Ϊʱ��ת�����ַ���
    ************************************************************/
    int GBKToUTF8(unsigned char * lpGBKStr,
        unsigned char * lpUTF8Str,int nUTF8StrLen);
};

