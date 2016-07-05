#include <iostream>
#include <string>
#include <stdio.h>
#include <string.h>

#include "AesEncDec.h"
#include "Base64.h"
#include "Log.h"

#include <windows.h>

using namespace std;

int main()
{
	CAesEncDec objAes;
    CBase64 objBase64;
    string key = "#$5#&fvAM%M*--!~";
	string text = "1234567890abcdef";
	string strHex;

    LOG.SetLogLevel(DEBUG|INFO|WARN|ERR|BOTH);
    LOG.SetPreName("AesTest");

    //for(int i = 0; i < 100; i++)
    {
        LOG.debug_log("����Aes���ܺͽ��ܹ��ܣ�");

        unsigned char buf[16 * 2] = {0};
        std::string chiperText2;
        int chiperlen = 0;
        objAes.AESEcbEncryptPKCS5Padding(text, 
            key, chiperText2, buf, chiperlen);
        cout<<"���ܺ�Ľ��:"<<chiperText2<<endl;

        std::string text2 = objAes.AESEcbDecryptPKCS5Padding(buf, chiperlen, key);
        cout<<"���ܺ������:"<<text2<<endl;

        unsigned char gbk[128] = {0};
        objAes.UTF8ToGBK((unsigned char*)(text2.c_str()), gbk, 128);
        std::string strGbk = (char *)gbk;
        std::cout<<"GBKתUTF8: "<<strGbk<<std::endl;

        unsigned char utf8[128] = {0};
        objAes.GBKToUTF8(gbk, utf8, 128);
        std::string strUtf8 = (char *)utf8;
        std::cout<<"UTF8תGBK: "<<strUtf8<<std::endl;

        Sleep(2000);
    }
	getchar();

	return 0;
}