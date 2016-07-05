#pragma once

#include <string>

using namespace std;

class CBase64
{
public:
	CBase64(void);
	~CBase64(void);

	// ±àÂë
	string Encode(const unsigned char* Data,int DataByte);

	// ½âÂë
	string Decode(const char* Data,int DataByte,int& OutByte);
};

