#pragma once

/*****************************************************
* add by chenjiyou 2016-04-20
* 功能：压缩图片，较少图片占用空间
******************************************************/

class CCompressPicture
{
public:
    CCompressPicture(void);
    ~CCompressPicture(void);

public:
    // 获取图片clsid
    BOOL GetImageCLSID(const WCHAR* format, CLSID* pCLSID);

    // 图片压缩处理
    BOOL TransPicture(LPCWSTR lpSrcFile, LPCWSTR lpDstExt, CStringW &strDstFile,
        const CString &strPath, const CString &strName); 

    // 从一个完整的文件路径里获得文件的目录和文件的名称
    BOOL GetFilePathAndName(const CString &strFile, CString &strPath, CString &strName);

    // 执行图片压缩
    BOOL CompressPicture(const CString &strFielPath);

};

