#pragma once

/*****************************************************
* add by chenjiyou 2016-04-20
* ���ܣ�ѹ��ͼƬ������ͼƬռ�ÿռ�
******************************************************/

class CCompressPicture
{
public:
    CCompressPicture(void);
    ~CCompressPicture(void);

public:
    // ��ȡͼƬclsid
    BOOL GetImageCLSID(const WCHAR* format, CLSID* pCLSID);

    // ͼƬѹ������
    BOOL TransPicture(LPCWSTR lpSrcFile, LPCWSTR lpDstExt, CStringW &strDstFile,
        const CString &strPath, const CString &strName); 

    // ��һ���������ļ�·�������ļ���Ŀ¼���ļ�������
    BOOL GetFilePathAndName(const CString &strFile, CString &strPath, CString &strName);

    // ִ��ͼƬѹ��
    BOOL CompressPicture(const CString &strFielPath);

};

