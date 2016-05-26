#include "StdAfx.h"
#include "CompressPicture.h"

#include <atlimage.h>
#include <gdiPlus.h>
using namespace Gdiplus;
#pragma comment(lib, "gdiplus.lib")

CCompressPicture::CCompressPicture(void)
{
}


CCompressPicture::~CCompressPicture(void)
{
}

// ��ȡͼƬclsid
BOOL CCompressPicture::GetImageCLSID(const WCHAR* format, CLSID* pCLSID)
{
    UINT num = 0;
    UINT size = 0;

    ImageCodecInfo* pImageCodecInfo = NULL;
    GetImageEncodersSize(&num, &size);
    if(size == 0){
        return FALSE;
    }
    pImageCodecInfo = (ImageCodecInfo *)(malloc(size));
    if(pImageCodecInfo == NULL)
        return FALSE;
    GetImageEncoders(num, size, pImageCodecInfo); 

    // Find for the support of format for image in the windows
    for(UINT i = 0; i < num; ++i)
    { 
        //MimeType: Depiction for the program image  
        if( wcscmp(pImageCodecInfo[i].MimeType, format) == 0)
        {  
            *pCLSID = pImageCodecInfo[i].Clsid; 
            free(pImageCodecInfo); 
            return TRUE;  
        }  
    }  
    free(pImageCodecInfo);  
    return FALSE;
}

// ͼƬѹ��
BOOL CCompressPicture::TransPicture(LPCWSTR lpSrcFile, LPCWSTR lpDstExt, CStringW &strDstFile,
    const CString &strPath, const CString &strName)
{
    USES_CONVERSION;
    CFileFind   find;  
#ifdef UNICODE
    if (!find.FindFile(lpSrcFile))//�ļ������� 
#else
    if (!find.FindFile(W2A(lpSrcFile)))//�ļ�������  
#endif

    {
        find.Close();
        return FALSE;
    }
    find.Close();

    Image *psrcImg = NULL;   
    psrcImg = Image::FromFile(lpSrcFile);
    if (!psrcImg || psrcImg->GetLastStatus() != Ok)
    {
        return 0;
    }
    int srcWidth = psrcImg->GetWidth();   
    int srcHeight = psrcImg->GetHeight();
    
    // ��ȴ���2000���ػ�߶ȴ���2000���ض�ѹ��������ѹ��
    if (2000 > srcHeight && 2000 > srcWidth)
    {
        // �ļ����ƸĻ�ȥ
        delete psrcImg;
        CStringW strInitName;
        strInitName = strPath + strName;
        CFile::Rename(lpSrcFile, strInitName);
        return TRUE;
    }

    //Construct   a   Graphics   object   based   on   the   image.   
    //Graphics   imgGraphics(psrcImg);  
    int n = 2;
    int nSaveWidth = srcWidth/n;
    int nSaveHeight = srcHeight/n;

    Bitmap *pBitmap = ::new Bitmap(nSaveWidth,nSaveHeight);   
    Graphics bmpGraphics(pBitmap);   
    //bmpGraphics.DrawImage(psrcImg,0,0,srcWidth,srcHeight);   
    bmpGraphics.DrawImage(psrcImg,RectF(0,0,nSaveWidth,nSaveHeight),0,0,srcWidth,srcHeight,UnitPixel);
    delete psrcImg;
    bmpGraphics.ReleaseHDC(NULL);

    //Save   the   altered   image.
    LPWSTR lpExt = PathFindExtensionW(lpSrcFile); 
    LPWSTR lpExtDst = NULL;

    lpDstExt++;

    LPWSTR   lpEncoder;  
    switch(*lpDstExt)   
    {   
    case   L'J':   
    case   L'j':   
        lpEncoder=L"image/jpeg";   
        lpExtDst = L".jpeg";  
        break;   
    case   L'P':   
    case   L'p':   
        lpEncoder=L"image/png";   
        lpExtDst = L".png"; 
        break;   
    case   L'B':   
    case   L'b':   
        lpEncoder=L"image/bmp";  
        lpExtDst = L".bmp"; 
        break;   
    case   L'G':   
    case   L'g':   
        lpEncoder=L"image/gif";   
        lpExtDst = L".gif"; 
        break;   
    case   L't':   
    case   L'T':   
        lpEncoder=L"image/tiff";   
        lpExtDst = L".tiff"; 
        break;   
    default:   
        lpEncoder=L"image/jpeg";   
        lpExtDst = L".jpeg"; 
    }   

    // ����֮ǰ���ļ���
    CStringW strPicPath;
    strPicPath = strPath + strName;

    CLSID   imgClsid;   
    GetImageCLSID(lpEncoder,&imgClsid);
    Status statusSave = pBitmap->Save(strPicPath,&imgClsid,NULL); 

    ::DeleteFileW(lpSrcFile);

    strDstFile = CStringW(strPicPath);
    ::delete pBitmap;

    return (statusSave == Ok ) ? TRUE : FALSE;
}

// ��һ���������ļ�·�������ļ���Ŀ¼���ļ�������
BOOL CCompressPicture::GetFilePathAndName(const CString &strFile, CString &strPath, CString &strName)
{
    int pos = strFile.ReverseFind('\\');
    if (0 >= pos)
    {
        pos = strFile.ReverseFind('/');
    }

    if (0 >= pos)
    {
        strName = strFile;
        return FALSE;
    }

    strPath = strFile.Left(pos+1);
    strName = strFile.Right(strFile.GetLength() - pos - 1);
    return TRUE;
}

// ִ��ͼƬѹ��
BOOL CCompressPicture::CompressPicture(const CString &strFielPath)
{
    CString strNewFileName;  // ѹ������ļ���
    CString strTmpFileName;  // ѹ��ǰ����ʱ�ļ���
    CString strFilePath;     // �ļ�·��
    CString strFileName;     // �ļ���

    // ��ȡ�ļ���չ�����ļ�·�����ļ���
    LPWSTR lpExt = PathFindExtensionW(strFielPath); 
    GetFilePathAndName(strFielPath, strFilePath, strFileName);

    // ����Ƿ�����Ҫѹ����ͼƬ��ʽ
    CString strExt(lpExt);
    if (0 != strExt.CompareNoCase(_T(".jpg"))  &&
        0 != strExt.CompareNoCase(_T(".jpeg")) &&
        0 != strExt.CompareNoCase(_T(".bmp"))  &&
        0 != strExt.CompareNoCase(_T(".png"))  &&
        0 != strExt.CompareNoCase(_T(".tiff")) &&
        0 != strExt.CompareNoCase(_T(".gif")))
    {
        return FALSE;
    }

    // ������ԭ�ļ�
    SYSTEMTIME st;
    GetLocalTime(&st);
    CString strTmp;
    strTmp.Format(_T("TMP%02d%03d"), st.wSecond, st.wMilliseconds);
    strTmpFileName = strFilePath + strTmp + lpExt;
    CFile::Rename(strFielPath, strTmpFileName);

    // ѹ��ͼƬ
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    TransPicture((LPCWSTR)strTmpFileName, lpExt, strNewFileName, strFilePath, strFileName);
    GdiplusShutdown(gdiplusToken);

    return TRUE;
}
