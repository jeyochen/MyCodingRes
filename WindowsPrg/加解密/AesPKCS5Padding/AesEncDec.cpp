#include "AesEncDec.h"
#include "Log.h"

/************************************************************
* ʹ��openssl_1.o��ʵ��AES���ܺͽ���
* ���ܣ�AES���ܽ���
* 
* create by chenjiyou on 2016-06-23
*************************************************************/

CAesEncDec::CAesEncDec(void)
{
}


CAesEncDec::~CAesEncDec(void)
{
}

std::string CAesEncDec::AesEncrypt(const std::string& source, const std::string& key, std::string& strHex)
{
    EVP_CIPHER_CTX ctx;
    EVP_CIPHER_CTX_init(&ctx);
    int ret = EVP_EncryptInit_ex(&ctx, EVP_aes_128_ecb(), NULL, (const unsigned char*)key.data(), NULL);
    if (1 != ret)
    {
        LOG.err_log("EVP_EncryptInit_ex ʧ��![%s:%d]", __FILE__, __LINE__);
        return "";
    }
    unsigned char* result = new unsigned char[source.length() + 64]; // Ū���㹻��Ŀռ�
    int len1 = 0;
    ret = EVP_EncryptUpdate(&ctx, result, &len1, (const unsigned char*)source.data(), source.length());
    if (1 != ret)
    {
        LOG.err_log("EVP_EncryptInit_ex ʧ��![%s:%d]", __FILE__, __LINE__);
        return "";
    }
    int len2 = 0;
    ret = EVP_EncryptFinal_ex(&ctx, result+len1, &len2); 
    if (1 != ret)
    {
        LOG.err_log("EVP_EncryptFinal_ex ʧ��![%s:%d]", __FILE__, __LINE__);
        return "";
    }
    //std::cout << len1 << "," << len2 << std::endl;
    ret = EVP_CIPHER_CTX_cleanup(&ctx);
    if (1 != ret)
    {
        LOG.err_log("EVP_CIPHER_CTX_cleanup ʧ��![%s:%d]", __FILE__, __LINE__);
        return "";
    }
    std::string res((char*)result, len1+len2);

    // ������ܺ��ʮ�������ַ���
    char* hexResult = new char[(len2 + 1) * 2];
    memset(hexResult, 0, (len2 + 1) * 2);
    for (int i = 0; i < len2; i++)
    {
        sprintf(hexResult + (2 * i), "%02X", result[i]);
    }
    std::string resHex(hexResult, strlen(hexResult));
    strHex = resHex;
    delete[] result;
    delete[] hexResult;
    result = NULL;
    hexResult = NULL;

    // ���ؼ��ܺ��ԭ����
    return res;
}

std::string CAesEncDec::AesDecrypt(const std::string& ciphertext, const std::string& key)
{
    EVP_CIPHER_CTX ctx;
    EVP_CIPHER_CTX_init(&ctx);
    int ret = EVP_DecryptInit_ex(&ctx, EVP_aes_128_ecb(), NULL, (const unsigned char*)key.data(), NULL);
    if (1 != ret)
    {
        LOG.err_log("EVP_DecryptInit_ex ʧ��![%s:%d]", __FILE__, __LINE__);
        return "";
    }
    unsigned char* result = new unsigned char[ciphertext.length() + 64]; // Ū���㹻��Ŀռ�
    int len1 = 0;
    ret = EVP_DecryptUpdate(&ctx, result, &len1, (const unsigned char*)ciphertext.data(), ciphertext.length());
    if (1 != ret)
    {
        LOG.err_log("EVP_DecryptUpdate ʧ��![%s:%d]", __FILE__, __LINE__);
        return "";
    }
    int len2 = 0;
    ret = EVP_DecryptFinal_ex(&ctx, result+len1, &len2); 
    if (1 != ret)
    {
        LOG.err_log("EVP_DecryptFinal_ex ʧ��![%s:%d]", __FILE__, __LINE__);
        return "";
    }
    std::cout << len1 << "," << len2 << std::endl;
    ret = EVP_CIPHER_CTX_cleanup(&ctx);
    if (1 != ret)
    {
        LOG.err_log("EVP_CIPHER_CTX_cleanup ʧ��![%s:%d]", __FILE__, __LINE__);
        return "";
    }
    std::string res((char*)result, len1+len2);
    delete[] result;
    result = NULL;

    return res;
}

std::string CAesEncDec::AESEcbEncryptPKCS5Padding(const std::string& source, const std::string& key,
    std::string& strHex, unsigned char *out, int& outlen)
{
    AES_KEY aes;
    unsigned char Text[AES_BLOCK_SIZE] = {0};
    unsigned char Encrypt[AES_BLOCK_SIZE]  = {0};
    unsigned int surplus_len = 0;
    char b;
    const char *tmp_in = source.c_str();
    int encrypt_quit = 0;

    if (0 >= source.length() || 0 >= key.length())
    {
        LOG.warn_log("�������ַ���Ϊ�ջ���ԿΪ��[%s:%d]", __FILE__, __LINE__);
        return "";
    }

    // set encrypt key
    if (AES_set_encrypt_key((unsigned char*)key.c_str(), key.length() * 8, &aes) < 0)
    {
        LOG.err_log("Unable to set encryption key in AES");
        return "";
    }

    char buf[128 * AES_BLOCK_SIZE] = {0};
    char hexBuf[128 * AES_BLOCK_SIZE] = {0};
    char *tmp_out = buf;

    outlen = 0;
    while(!encrypt_quit)
    {
        memset(Text, 0, sizeof(Text));
        memset(Encrypt, 0, sizeof(Encrypt));
        if(strlen(tmp_in) == 0)
        {
            surplus_len = AES_BLOCK_SIZE;
            b = '\0' + surplus_len;
            memset(Text, b, AES_BLOCK_SIZE);
            encrypt_quit = 1;
        }
        else if(strlen(tmp_in)/AES_BLOCK_SIZE > 0)
        {
            memset(Text, 0, AES_BLOCK_SIZE);   
            memcpy(Text, tmp_in, AES_BLOCK_SIZE);
        }
        else 
        {
            surplus_len = AES_BLOCK_SIZE - strlen(tmp_in);
            b = '\0' + surplus_len;
            memset(Text, b, AES_BLOCK_SIZE);
            memcpy(Text, tmp_in, strlen(tmp_in));
            encrypt_quit = 1;
        }
        
        AES_ecb_encrypt(Text, Encrypt, &aes, AES_ENCRYPT);
        for (int i = 0; i < AES_BLOCK_SIZE; i++)
        {
            sprintf(&hexBuf[strlen(hexBuf)], "%02X", Encrypt[i]);
        }
        memcpy(out, Encrypt, AES_BLOCK_SIZE);
        out += AES_BLOCK_SIZE;
        tmp_in += AES_BLOCK_SIZE;
        outlen += AES_BLOCK_SIZE;
    }
    strHex = hexBuf;

    std::string res = buf;
    return res;
}

std::string CAesEncDec::AESEcbDecryptPKCS5Padding(const unsigned char* ciphertext, const int chiplen,
    const std::string& key)
{
    EVP_CIPHER_CTX ctx;
    EVP_CIPHER_CTX_init(&ctx);
    int ret = EVP_DecryptInit_ex(&ctx, EVP_aes_128_ecb(), NULL, (const unsigned char*)key.data(), NULL);
    if (1 != ret)
    {
        LOG.err_log("EVP_DecryptInit_ex ʧ��![%s:%d]", __FILE__, __LINE__);
        return "";
    }

    unsigned char* result = new unsigned char[chiplen + 64]; // Ū���㹻��Ŀռ�
    int len1 = 0;
    ret = EVP_DecryptUpdate(&ctx, result, &len1, ciphertext, chiplen);
    if (1 != ret)
    {
        LOG.err_log("EVP_DecryptUpdate ʧ��![%s:%d]", __FILE__, __LINE__);
        return "";
    }

    int len2 = 0;
    ret = EVP_DecryptFinal_ex(&ctx, result+len1, &len2); 
    if (1 != ret)
    {
        LOG.err_log("EVP_DecryptFinal_ex ʧ��![%s:%d]", __FILE__, __LINE__);
        return "";
    }

    std::cout << len1 << "," << len2 << std::endl;
    ret = EVP_CIPHER_CTX_cleanup(&ctx);
    if (1 != ret)
    {
        LOG.err_log("EVP_CIPHER_CTX_cleanup ʧ��![%s:%d]", __FILE__, __LINE__);
        return "";
    }

    std::string res((char*)result, len1+len2);
    delete[] result;
    result = NULL;

    return res;
}

// �ַ���תʮ������
void CAesEncDec::StrToHex(unsigned char *pbDest, const char *pbSrc, int nLen)
{
    char h1,h2;
    unsigned char s1,s2;

    for (int i = 0; i < nLen/2; i++)
    {
        h1 = pbSrc[2*i];
        h2 = pbSrc[2*i + 1];

        s1 = toupper(h1) - 0x30;
        if (s1 > 9)
            s1 -= 7;

        s2 = toupper(h2) - 0x30;
        if (s2 > 9)
            s2 -= 7;

        pbDest[i] = s1 * 16 + s2;
    }
}

// ʮ������ת�ַ���
void CAesEncDec::HexToStr(unsigned char *pbDest, 
    const unsigned char *pbSrc, int nLen)
{
    char ddl,ddh;
    for (int i = 0; i < nLen; i++)
    {
        ddh = 48 + pbSrc[i] / 16;
        ddl = 48 + pbSrc[i] % 16;
        if (ddh > 57) ddh = ddh + 7;
        if (ddl > 57) ddl = ddl + 7;
        pbDest[i*2] = ddh;
        pbDest[i*2+1] = ddl;
    }

    pbDest[nLen*2] = '\0';
}

// ����: UTF8����ת��GBK����
int CAesEncDec::UTF8ToGBK(unsigned char *lpUTF8Str,
    unsigned char *lpGBKStr,int nGBKStrLen)
{
    wchar_t * lpUnicodeStr = NULL;
    int nRetLen = 0;

    //���UTF8�ַ���ΪNULL������˳�
    if(!lpUTF8Str)  
        return 0;

    //��ȡת����Unicode���������Ҫ���ַ��ռ䳤��
    nRetLen = ::MultiByteToWideChar(CP_UTF8, 0, 
        (char *)lpUTF8Str, -1 ,NULL, NULL);  

    //ΪUnicode�ַ����ռ�
    lpUnicodeStr = new WCHAR[nRetLen + 1];

    //ת����Unicode����
    nRetLen = ::MultiByteToWideChar(CP_UTF8, 0, 
        (char *)lpUTF8Str, -1, lpUnicodeStr, nRetLen);

    if(!nRetLen)
    {
        if(lpUnicodeStr)
        {
            delete[] lpUnicodeStr;
            lpUnicodeStr = NULL;
        }

        return 0;
    }

    //��ȡת����GBK���������Ҫ���ַ��ռ䳤��
    nRetLen = ::WideCharToMultiByte(CP_ACP, 0, 
        lpUnicodeStr, -1, NULL, NULL, NULL, NULL);

    if(!lpGBKStr)  //���������Ϊ���򷵻�ת������Ҫ�Ŀռ��С
    {
        if(lpUnicodeStr)
        {
            delete []lpUnicodeStr;
            lpUnicodeStr = NULL;
        }

        return nRetLen;
    }

    if(nGBKStrLen < nRetLen)  //���������������Ȳ������˳�
    {
        if(lpUnicodeStr)
        {
            delete []lpUnicodeStr;
            lpUnicodeStr = NULL;
        }

        return 0;
    }

    //ת����GBK����
    nRetLen = ::WideCharToMultiByte(CP_ACP, 0, 
        lpUnicodeStr, -1, (char *)lpGBKStr, nRetLen, NULL, NULL);

    if(lpUnicodeStr)
    {
        delete []lpUnicodeStr;
        lpUnicodeStr = NULL;
    }

    return nRetLen;
}

// ����: GBK����ת����UTF8����
int CAesEncDec::GBKToUTF8(unsigned char * lpGBKStr,
    unsigned char * lpUTF8Str,int nUTF8StrLen)
{
    wchar_t * lpUnicodeStr = NULL;
    int nRetLen = 0;

    if(!lpGBKStr)  //���GBK�ַ���ΪNULL������˳�
        return 0;

    //��ȡת����Unicode���������Ҫ���ַ��ռ䳤��
    nRetLen = ::MultiByteToWideChar(CP_ACP, 0, 
        (char *)lpGBKStr, -1, NULL, NULL); 

    //ΪUnicode�ַ����ռ�
    lpUnicodeStr = new WCHAR[nRetLen + 1];

    //ת����Unicode����
    nRetLen = ::MultiByteToWideChar(CP_ACP, 0, 
        (char *)lpGBKStr, -1, lpUnicodeStr, nRetLen);

    if(!nRetLen)
    {
        if(lpUnicodeStr)
        {
            delete[] lpUnicodeStr;
            lpUnicodeStr = NULL;
        }
        return 0;
    }

    //��ȡת����UTF8���������Ҫ���ַ��ռ䳤��
    nRetLen = ::WideCharToMultiByte(CP_UTF8, 0, 
        lpUnicodeStr, -1, NULL, 0, NULL, NULL);

    //���������Ϊ���򷵻�ת������Ҫ�Ŀռ��С
    if(!lpUTF8Str)
    {
        if(lpUnicodeStr)
        {
            delete[] lpUnicodeStr;
            lpUnicodeStr = NULL;
        }
        return nRetLen;
    }

    if(nUTF8StrLen < nRetLen)  //���������������Ȳ������˳�
    {
        if(lpUnicodeStr)
        {
            delete[] lpUnicodeStr;
            lpUnicodeStr = NULL;
        }
        return 0;
    }

    nRetLen = ::WideCharToMultiByte(CP_UTF8, 0, 
        lpUnicodeStr, -1, (char *)lpUTF8Str, nUTF8StrLen, NULL, NULL);  //ת����UTF8����

    if(lpUnicodeStr)
    {
        delete[] lpUnicodeStr;
        lpUnicodeStr = NULL;
    }

    return nRetLen;
}