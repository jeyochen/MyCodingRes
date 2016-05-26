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

// 获取图片clsid
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

// 图片压缩
BOOL CCompressPicture::TransPicture(LPCWSTR lpSrcFile, LPCWSTR lpDstExt, CStringW &strDstFile,
    const CString &strPath, const CString &strName)
{
    USES_CONVERSION;
    CFileFind   find;  
#ifdef UNICODE
    if (!find.FindFile(lpSrcFile))//文件不存在 
#else
    if (!find.FindFile(W2A(lpSrcFile)))//文件不存在  
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
    
    // 宽度大于2000像素或高度大于2000像素都压缩，否则不压缩
    if (2000 > srcHeight && 2000 > srcWidth)
    {
        // 文件名称改回去
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

    // 改名之前的文件名
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

// 从一个完整的文件路径里获得文件的目录和文件的名称
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

// 执行图片压缩
BOOL CCompressPicture::CompressPicture(const CString &strFielPath)
{
    CString strNewFileName;  // 压缩后的文件名
    CString strTmpFileName;  // 压缩前的临时文件名
    CString strFilePath;     // 文件路径
    CString strFileName;     // 文件名

    // 获取文件扩展名，文件路径，文件名
    LPWSTR lpExt = PathFindExtensionW(strFielPath); 
    GetFilePathAndName(strFielPath, strFilePath, strFileName);

    // 检查是否是需要压缩的图片格式
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

    // 重命名原文件
    SYSTEMTIME st;
    GetLocalTime(&st);
    CString strTmp;
    strTmp.Format(_T("TMP%02d%03d"), st.wSecond, st.wMilliseconds);
    strTmpFileName = strFilePath + strTmp + lpExt;
    CFile::Rename(strFielPath, strTmpFileName);

    // 压缩图片
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    TransPicture((LPCWSTR)strTmpFileName, lpExt, strNewFileName, strFilePath, strFileName);
    GdiplusShutdown(gdiplusToken);

    return TRUE;
}
