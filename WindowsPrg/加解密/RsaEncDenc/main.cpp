#include <iostream>
#include <string>
#include <stdio.h>

#include "RsaEncDec.h"
#include "Log.h"

int main(int argc, char *argv[])
{
    CRsaEncDec objRsa;

    std::string text = "!@#%^&*()45";
    text += "12345678901234567890";
    
    LOG.SetLogLevel(DEBUG|INFO|WARN|ERR|BOTH);
    LOG.SetPreName("RsaTest");
    LOG.printlog("text:%s", text.c_str());

    std::string pubKeyFile ="d:\\pubkey.pem";
    std::string priKeyFile ="d:\\prikey.pem";
    objRsa.ReadPublickey(pubKeyFile);
    objRsa.ReadPrivatekey(priKeyFile);

    std::string strHex;
    std::string strChiper;
    std::string strInit;

    //for(int i = 0; i < 200; i++)
    {
        strChiper = objRsa.RsaEncrypt(text, strHex);
        std::cout<<"strHex="<<strHex<<std::endl;

        // 解密测试  --测试12355中文 编码格式为UTF8
        //strHex = "17e7c77c631a9c59adc37272fbbbe6175f88c94dcc632e3ef689cd5d7176a142da69b33f4cb2d46e2b9aa803d6ac61402dfe5d7e3cbee151570d8d44e93ecac9bbc633ba5d368ad49e5579da56b6194501b85f43829c461bd7fe0a302a8a371f75293828b85cc74dbbf1a3bc446d85de8cf416dc78d2efebd92769e30843137e";
        unsigned char u8arr[256] = {0};
        objRsa.StrToHex(u8arr, strHex.c_str(), strHex.length());

        strInit = objRsa.RsaDecrypt(u8arr);

        unsigned char gbk[128] = {0};
        objRsa.UTF8ToGBK((unsigned char*)(strInit.c_str()), gbk, 128);
        std::string strGbk = (char *)gbk;
        std::cout<<"GBK转UTF8: "<<strGbk<<std::endl;

        unsigned char utf8[128] = {0};
        objRsa.GBKToUTF8(gbk, utf8, 128);
        std::string strUtf8 = (char *)utf8;
        std::cout<<"UTF8转GBK: "<<strUtf8<<std::endl;
        Sleep(2000);
    }

    getchar();
    return 0;
}