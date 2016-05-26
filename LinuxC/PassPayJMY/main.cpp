#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string>

#include "JMYPassPay.h"
int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		fprintf(stdout, "usage PassPay port\n");
        return 0;
	}
	
	int cmd = 0;
	char pay_cmd[1024] = {0};
	
	int port =  atoi(argv[1]);
    if (0 > port || port > 128)
    {
        fprintf(stdout, "port number error! port:%d\n", port);
    }
    printf("Serial port:%d\n", port);
	
	fprintf(stdout, "               ************************************\n");
	fprintf(stdout, "               *      1.发送命令                   \n");
	fprintf(stdout, "               *      2.支付测试                   \n");
	fprintf(stdout, "               *      3.查询余额                   \n");
	fprintf(stdout, "               *      4.读取卡号                   \n");
    fprintf(stdout, "               *      5.测试线程退出               \n");
    fprintf(stdout, "               *      6.稳定性测试                 \n");
	fprintf(stdout, "               *      9.退出程序                   \n");
	fprintf(stdout, "               ************************************\n");
	
	printf("Enter cmd: ");
	scanf("%d", &cmd);
	
	static char temp[4096] = {0};
	while(cmd != 9)
	{
        if(1 == cmd)
        {
            CJMYPassPay pass_pay;
            if (false == pass_pay.OpenSerial(port))
            {
                printf("open serial port:%d failed\n", port);
                goto READ_CMD;
            }

            printf("Enter pay cmd: ");
            scanf("%s", pay_cmd);
            std::string str_cmd = pay_cmd;
            printf("send cmd:%s\n recv:%s\n", str_cmd.c_str(), temp);
            pass_pay.WriteData(str_cmd);
			
			unsigned char readBuf[1024] = {0};
			int readLen = pass_pay.ReadData(readBuf, sizeof(readBuf));
			memset(temp, 0x00, sizeof(temp));
			for (int i = 0; i < readLen; i++)
			{
				sprintf(temp + i * 3, "%02X ", readBuf[i]);
			}
            pass_pay.ClosePort();
		}
        else if (2 == cmd)
        {
            CJMYPassPay pass_pay;
            std::string str_55field;
            std::string str_card;
            std::string sw1sw2;
            if (false == pass_pay.OpenSerial(port))
            {
                printf("open serial port:%d failed\n", port);
                goto READ_CMD;
            }
            if (false == pass_pay.Pay(str_55field, str_card, sw1sw2, 1))
			{
				fprintf(stdout, "pay failed\n");
                goto READ_CMD;
			}
				
			fprintf(stdout, "\n\nstr_55field:%s\nstr_card:%s\n", 
				str_55field.c_str(), str_card.c_str());	
            pass_pay.ClosePort();
        }
		else if (3 == cmd)
		{
            CJMYPassPay pass_pay;
			std::string balance;
            std::string sw1sw2;
            if (false == pass_pay.OpenSerial(port))
            {
                printf("open serial port:%d failed\n", port);
                goto READ_CMD;
            }
			pass_pay.QueryBalance(balance, sw1sw2);
			printf("balance:%s\n", balance.c_str());
            pass_pay.ClosePort();
		}
        else if (4 == cmd)
        {
            CJMYPassPay pass_pay;
            std::string str_card;
            std::string sw1sw2;
            if (false == pass_pay.OpenSerial(port))
            {
                printf("open serial port:%d failed\n", port);
                goto READ_CMD;
            }
			if (false == pass_pay.ReadCardNo(str_card, sw1sw2))
			{
				fprintf(stdout, "ReadCard failed\n");
			}
			fprintf(stdout, "%s\n", str_card.c_str());
            pass_pay.ClosePort();
        }
        else if (5 == cmd)
        {
            CJMYPassPay pass_pay;
            pass_pay.OpenSerial(port);
            for (int i = 0; i < 10; i++)
            {
                sleep(1);
            }
            pass_pay.ClosePort();
            //break;
        }
        else if (6 == cmd)
        {
            CJMYPassPay pass_pay;
            std::string sw1sw2;
            std::string str_card;
            std::string balance;
            
            for(int i = 0; i < 100; i++)
            {
                //  每次够构建对象和释放对象，测试程序稳定性
                CJMYPassPay pass_pay;
                if (false == pass_pay.OpenSerial(port))
                {
                    printf("open serial port:%d failed\n", port);
                    continue;
                }
                fprintf(stdout, "----loop:%d----\n", i  + 1);
                pass_pay.ReadCardNo(str_card, sw1sw2);
                sleep(1);
                pass_pay.QueryBalance(balance, sw1sw2);
                pass_pay.ClosePort();
            }
            
        }
		else if (9 == cmd)
		{
			break;
		}
		
READ_CMD:
		fprintf(stdout, "               ************************************\n");
		fprintf(stdout, "               *      1.发送命令                   \n");
		fprintf(stdout, "               *      2.支付测试                   \n");
		fprintf(stdout, "               *      3.查询余额                   \n");
		fprintf(stdout, "               *      4.读取卡号                   \n");
        fprintf(stdout, "               *      5.测试线程退出               \n");
        fprintf(stdout, "               *      6.稳定性测试                 \n");
		fprintf(stdout, "               *      9.退出程序                   \n");
		fprintf(stdout, "               ************************************\n");
		printf("Enter cmd: ");
		scanf("%d", &cmd);
	}
  
	return 0;
}
