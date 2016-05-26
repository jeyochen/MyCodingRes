#include "SerialPort.h"
const uint16 BUAD_RATE = 9600;
int main (int argc, char *argv[]) 
{
    CSerialPort serial_port;
    if (argc < 2)
    {
        fprintf (stdout, "usage:SerialPortTest commport.\n");
        return -1;
    }
    int port = atoi (argv[1]);
    if (0 > port || port > 128)
    {
      fprintf (stdout, "port number illege:%d\n", port);
      return -1;
    }
    if (false == serial_port.OpenSerialPort (port))
    {
      fprintf (stdout, "Open serial port %d fail", port);
      return -1;
    }
    if (false == serial_port.SetBaudrate (BUAD_RATE))
    {
      fprintf (stdout, "SetBaudrate fail: %d\n", BUAD_RATE);
      return -1;
    }
    if (false == serial_port.SetParity (8, 1, 'N'))
    {
      fprintf (stdout, "SetParity fail\n");
      return -1;
    }
   
    fprintf(stdout, "                 *********************************\n");
	fprintf(stdout, "                 * 1. 六路全开                    \n");
	fprintf(stdout, "                 * 2. 六路全关                    \n");
	fprintf(stdout, "                 * 3. 输入命令                    \n");
	fprintf(stdout, "                 * 9. 退出程序                    \n");
	fprintf(stdout, "                 *********************************\n");
   
    char cmd[1024] = {0};
	
	printf("Please enter cmd:");
    scanf("%s", cmd);
	int ins = atoi(cmd);
	int len = 0;
    while(9 != ins)    
    {
		bool bwrite = false;
		unsigned char response[1024] = { 0 };
		memset (response, 0, sizeof (response));
		memset(cmd, 0x00, sizeof(cmd));
		if (1 == ins)
		{
		    snprintf(cmd, sizeof(cmd), "%s", "FF 10 000D 0006 0C 00AA00AA00AA00AA00AA00AA");
		}
		else if (2 == ins)
		{
		    snprintf(cmd, sizeof(cmd), "%s", "FF 10 000D 0006 0C 005500550055005500550055");
		}
		else if (3 == ins)
		{
		    scanf("%s", cmd);
		}
		else if (9 == ins)
		{
		    break;
		}
		else
		{
			goto READ_CMD;
		}
		bwrite = serial_port.WriteData(cmd, strlen(cmd));
		if (!bwrite)
		{
		  goto READ_CMD;
		}
		len = serial_port.ReadData (response, sizeof(response) - 1);

		printf("receive len:%d\n", len);
		for (int i = 0; i < len; i++)
		{
			printf ("%02X ", response[i]);
		}
		if (len > 0)
		{
			printf ("\n");
		}
		
READ_CMD:
		fprintf(stdout, "                 *********************************\n");
		fprintf(stdout, "                 * 1. 六路全开                    \n");
		fprintf(stdout, "                 * 2. 六路全关                    \n");
		fprintf(stdout, "                 * 3. 输入命令                    \n");
		fprintf(stdout, "                 * 9. 退出程序                    \n");
		fprintf(stdout, "                 *********************************\n");

		memset(cmd, 0, sizeof(cmd));
		printf("\nPlease enter cmd:");
		scanf("%s", cmd);
	    ins = atoi(cmd);
    }
    return 0;
}


