#pragma once

#include <string>

using namespace std;

class CBase64
{
public:
	CBase64(void);
	~CBase64(void);

	// ����
	string Encode(const unsigned char* Data,int DataByte);

	// ����
	string Decode(const char* Data,int DataByte,int& OutByte);
};

