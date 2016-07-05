#pragma once

/************************************************************
* 
* ���ܣ�ʹ�� openssl ʵ��Rsa�ӽ���
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
    * ����: ��ȡ��Կ�ļ�
    * ���: keyfile ��Կ�ļ���
    *       readfile �Ƿ��ȡpem�ļ� true ����false����
    * ����ֵ: true �ɹ���falseʧ��
    ************************************************************/
    bool ReadPublickey(const std::string& keyfile, bool readfile = false);

    /************************************************************
    * ����: ��ȡ˽Կ�ļ�
    * ���: keyfile ˽Կ�ļ���
    *       readfile �Ƿ��ȡpem�ļ��� true ����false����
    * ����ֵ: true �ɹ���falseʧ��
    ************************************************************/
    bool ReadPrivatekey(const std::string& keyfile, bool readfile = false);

    /************************************************************
    * ����: ��Կ����
    * ���: strData ����������
    * ���Σ�hexStr ���ĵ�ʮ�����Ʊ�ʾ
    * ����ֵ: ����
    ************************************************************/
    std::string RsaEncrypt(const std::string& strData, std::string& hexStr);

    /************************************************************
    * ����: ˽Կ����
    * ���: chiper ����������
    * ����ֵ: ����
    ************************************************************/
    std::string RsaDecrypt(unsigned char *chiper);

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

private:
    RSA* m_privateKey; // ˽Կ
    RSA* m_publicKey; // ��Կ
};

