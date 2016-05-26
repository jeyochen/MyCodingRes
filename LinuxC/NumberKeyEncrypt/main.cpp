#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "JnaInterface.h"

int main(int argc, char *argv[])
{
	int cmd  = 0;
	
	if (2 != argc)
	{
		fprintf(stdout, "usage:Encrypt port\n");
		return -1;
	}
	
	int port = atoi(argv[1]);
	if (0 > port)
	{
		fprintf(stdout, "port number is error!\n");
		return -1;
	}
	
	SetSerialParam(port);
	
	fprintf(stdout, "               ************************************\n");
	fprintf(stdout, "               *      2.MAC加密                    \n");
	fprintf(stdout, "               *      3.注入密钥                   \n");
	fprintf(stdout, "               *      9.退出程序                   \n");
	fprintf(stdout, "               ************************************\n");
	printf("Enter cmd: ");
	scanf("%d", &cmd);
	// while(9 != cmd)
	{
		if(2 == cmd)
		{
			char strData[64] = {"990123456789"};
			Encrypt(1, strData);
		}
		else if (3 == cmd)
		{
			char strData[128] = {"F15FD36B806E943564D874057E745256DF5CACBA37E80712BFD1A389000000000000000046BE16C0"};
			SetWorkKey(strData);
		}
		// else if(9 == cmd)
		// {
			// break;
		// }
		// fprintf(stdout, "               ************************************\n");
		// fprintf(stdout, "               *      1.PIN加密                    \n");
		// fprintf(stdout, "               *      2.MAC加密                    \n");
		// fprintf(stdout, "               *      3.注入密钥                   \n");
		// fprintf(stdout, "               *      9.退出程序                   \n");
		// fprintf(stdout, "               ************************************\n");
		// printf("Enter cmd: ");
		// scanf("%d", &cmd);
		// char ch = getchar();
	}
	
	return 0;
}
