#include "RsaEncDec.h"
#include "Log.h"

#define  KEY_BYTES 128
// ģ��
#define  MODULUS "b11e9866cb2e86393ea03e4a094aefa42cba45615655ca83a982189dccf844244e6b216fcb0c4eaf7fde6ca878fcaf9e7ebb9bd6e3719f9f1727beeb21f012265fa5126158ec2446fcacd35879ce9e729d66852076651a6b3f1e4d736cad272e208ab6f652e752e77b99ac8b2397ecbc40f62859b3a2b7260ba94b22f238fa8b"

// ��Կָ��
#define  PUBLIC_EXPONENT RSA_F4 //65537

// ˽Կָ��
#define  PRIVATE_EXPONENT "8e445603d35636680abea9e9cc170df020212a9369c6955936319256460a70a97cf79e2eeeb89f0766bd21281dae51de0c4be37c32cad47cf41b6fc5ec47aa3b1f11f25658e17aef98faf293be6eafa5a722bd8ee187f47f311edcef9578b0b6c4ef8bd62bdd5d821dca3040960ea55c31da25618f0b4433cf45cacd1b890a69"

CRsaEncDec::CRsaEncDec(void)
    :m_privateKey(NULL)
    ,m_publicKey(NULL)
{
}


CRsaEncDec::~CRsaEncDec(void)
{
    if (m_publicKey)
    {
        RSA_free(m_publicKey);
        m_publicKey = NULL;
    }
    
    if (m_privateKey)
    {
        RSA_free(m_privateKey);
        m_privateKey = NULL;
    }
}

bool CRsaEncDec::ReadPublickey(const std::string& keyfile, bool readfile)
{
    if (m_publicKey != NULL)
    {
        RSA_free(m_publicKey);
        m_publicKey = NULL;
    }

    if (NULL == (m_publicKey = RSA_new())) return false;

    if (readfile)
    {
        FILE* hPubKeyFile = fopen(keyfile.c_str(), "rb");
        if( hPubKeyFile == NULL )
        {
            LOG.err_log("�򿪹�Կ�ļ�ʧ��:%s, [%s:%d]", 
                keyfile.c_str(), __FILE__, __LINE__);
            return false; 
        }

        RSA* res = PEM_read_RSA_PUBKEY(hPubKeyFile, &m_publicKey, 0, 0);
        fclose(hPubKeyFile);
        hPubKeyFile = NULL;

        if (!res)
        {
            LOG.err_log("��ȡ��Կʧ��![%s:%d]", __FILE__, __LINE__);
            return false;
        }
    }
    else
    {
        BIGNUM *bnn = BN_new(); 
        BIGNUM *bne = BN_new();
        BIGNUM *bnd = BN_new();

        BN_hex2bn(&bnn, MODULUS);
        BN_set_word(bne, PUBLIC_EXPONENT);
        BN_hex2bn(&bnd, PRIVATE_EXPONENT);

        m_publicKey->n = bnn;
        m_publicKey->e = bne;
        m_publicKey->d = bnd;
        return true;
    }
    
    return true;
}

bool CRsaEncDec::ReadPrivatekey(const std::string& keyfile, bool readfile)
{
    if (m_privateKey != NULL)
    {
        RSA_free(m_privateKey);
        m_privateKey = NULL;
    }

    if (NULL == (m_privateKey = RSA_new())) return false;

    if (readfile)
    {
        FILE* hPriKeyFile = fopen(keyfile.c_str(), "rb");
        if( hPriKeyFile == NULL )
        {
            LOG.err_log("��˽Կ�ļ�ʧ��:%s, [%s:%d]", 
                keyfile.c_str(), __FILE__, __LINE__);
            return false; 
        }
        RSA* res = PEM_read_RSAPrivateKey(hPriKeyFile, &m_privateKey, 0, 0);
        fclose(hPriKeyFile);
        hPriKeyFile = NULL;

        if (!res)
        {
            LOG.err_log("��ȡ˽Կʧ��![%s:%d]", __FILE__, __LINE__);
            return false;
        }
    }
    else
    {
        BIGNUM *bnn = BN_new(); 
        BIGNUM *bne = BN_new();
        BIGNUM *bnd = BN_new();

        BN_hex2bn(&bnn, MODULUS);
        BN_set_word(bne, PUBLIC_EXPONENT);
        BN_hex2bn(&bnd, PRIVATE_EXPONENT);

        m_privateKey->n = bnn;
        m_privateKey->e = bne;
        m_privateKey->d = bnd;
    }

    return true;
}

std::string CRsaEncDec::RsaEncrypt(const std::string& strData, std::string& hexStr)
{
    std::string strRet;
    if (strData.empty())
    {
        return "";
    }
    if (NULL == m_publicKey)
    {
        LOG.err_log("���ȶ�ȡ��Կ,[%s:%d]", __FILE__, __LINE__);
        return "";
    }

    int flen = RSA_size(m_publicKey);
    unsigned char* pEncode = new unsigned char[flen];
    memset(pEncode, 0, flen);
    int ret = RSA_public_encrypt(strData.length(), (const unsigned char*)strData.c_str(), 
        pEncode, m_publicKey, RSA_PKCS1_PADDING);
    if (ret >= 0)
    {
        char buf[1024] = {0};
        memset(buf, 0, 1024);
        for (int i = 0; i < ret; i++)
        {
            sprintf(&buf[i*2], "%02X", pEncode[i]);
        }
        hexStr = buf;
        strRet = std::string((char *)pEncode, ret);
    }
    delete[] pEncode;
    pEncode = NULL;
    CRYPTO_cleanup_all_ex_data(); 

    LOG.debug_log("���ܺ������ʮ�����Ʊ�ʾ:%s", hexStr.c_str());

    return strRet;
}

std::string CRsaEncDec::RsaDecrypt(unsigned char *chiper)
{
    if (NULL == chiper)
    {
        LOG.err_log("����Ϊ��![%s:%d]", 
            __FILE__, __LINE__);
        return "";
    }

    std::string strRet;
   
    if (NULL == m_privateKey)
    {
        LOG.warn_log("���ȶ�ȡ˽Կ");
        return "";
    }

    int flen = RSA_size(m_privateKey);
    char* pDecode = new char[flen];
    memset(pDecode, 0, flen);

    int ret = RSA_private_decrypt(flen, chiper,
        (unsigned char*)pDecode, m_privateKey, RSA_PKCS1_PADDING);
    if(ret >= 0)
    {
        strRet = std::string((char*)pDecode, ret);
    }
    delete [] pDecode;
    pDecode = NULL;
    CRYPTO_cleanup_all_ex_data();
    LOG.debug_log("���ܺ��ԭ��:%s", strRet.c_str());
    return strRet;
}

// �ַ���תʮ������
void CRsaEncDec::StrToHex(unsigned char *pbDest, const char *pbSrc, int nLen)
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
void CRsaEncDec::HexToStr(unsigned char *pbDest, 
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
int CRsaEncDec::UTF8ToGBK(unsigned char *lpUTF8Str,
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
int CRsaEncDec::GBKToUTF8(unsigned char * lpGBKStr,
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