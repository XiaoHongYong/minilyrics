
#include "stdafx.h"
#include <windows.h>
#include "filesystemutil.h"

//////////////////////////////////////////////////////////////////////////
// seachFile
// 
unsigned int SearchFile(TCHAR* dir, FILE* fp)
{
	std::vector<std::basic_string<TCHAR> > aStr;
	TravelDir(aStr, std::basic_string<TCHAR> (dir));
	for(std::vector<std::basic_string<TCHAR> >::iterator i = aStr.begin(); i != aStr.end(); i++)
	{
		fwrite(i->c_str(), sizeof(TCHAR), i->length(), fp);
		fwrite(_T("\r\n"), sizeof(TCHAR), i->length(), fp);
	}
	fclose(fp);
	return 0;
}



//////////////////////////////////////////////////////////////////////////
// Check if is a supported file
// 
BOOL IsSupportedPicFile(LPCTSTR lpFileName)			// point to file name

{
	if(lpFileName)
	{
		LPCTSTR pExt = _tcsrchr(lpFileName, _T('.'));
		if(pExt)
		{
			pExt++;
			if((0 == _tcsicmp(_T("mp3"), pExt))
				|| (0 == _tcsicmp(_T("ape"), pExt))
				|| (0 == _tcsicmp(_T("wav"), pExt)))
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}
//////////////////////////////////////////////////////////////////////////
// search files in directory
void TravelDir(std::vector<std::basic_string<TCHAR> >& vsFiles, 
			   const std::basic_string<TCHAR> sDir)
{
	TCHAR* pExtension = NULL;
    WIN32_FIND_DATA data;
    std::basic_string<TCHAR> str = sDir + _T("\\*.*");
    std::basic_string<TCHAR> sTDir = sDir;

    HANDLE hFind = FindFirstFile(str.c_str(), &data);
    if (INVALID_HANDLE_VALUE != hFind) 
    {
        if(FILE_ATTRIBUTE_DIRECTORY == data.dwFileAttributes )
        {
            if((0 != _tcscmp(_T("."), data.cFileName) )
                && (0 != _tcscmp(_T(".."), data.cFileName) ))
            {
				sTDir = sDir + _T("\\") + data.cFileName;
                TravelDir(vsFiles, sTDir);
            }
        }
        else
        {
			if(IsSupportedPicFile(data.cFileName ))
			{
				vsFiles.push_back(sDir + _T("\\") + data.cFileName);
			}
       }

        while(FindNextFile(hFind, &data))
        {
            if(FILE_ATTRIBUTE_DIRECTORY == data.dwFileAttributes)
            {
                if( (0 != _tcscmp(_T("."), data.cFileName) )
                    && (0 != _tcscmp(_T(".."), data.cFileName) ))
                {
					sTDir = sDir + _T("\\") + data.cFileName;
                    TravelDir(vsFiles, sTDir);
                }
            }
            else
            {
				if(IsSupportedPicFile(data.cFileName))
				{
					vsFiles.push_back(sDir + _T("\\") + data.cFileName);
				}
			}
        }// while
    }// if 

}
#if 0 
//////////////////////////////////////////////////////////////////////////
// Get next file from specified directory vector
std::basic_string<TCHAR> GetNextFile(const std::vector<std::basic_string<TCHAR> >& vsDirectories,
									 LPCTSTR pCurrentFile, 
									 BOOL bRandmon)
{
	static BOOL bFirstIn = TRUE;
	if(bFirstIn)
	{
		srand( (unsigned)time( NULL ) );
		bFirstIn = FALSE;
	}

	std::vector<std::basic_string<TCHAR> > vsFileList;
	for(DWORD i = 0; i < vsDirectories.size(); i++)
			TravelDir(vsFileList, vsDirectories[i]);

	if(0 == vsFileList.size()) 
	{
		return std::basic_string<TCHAR>();
	}
	else if (1 == vsFileList.size())
	{
		return vsFileList[0];
	}
	else
	{
		if(TRUE == bRandmon)
		{
			int i = 0;
			do{
				i = rand() * vsFileList.size() / RAND_MAX;
			}while(0 == _tcscmp(vsFileList[i].c_str(), pCurrentFile));

			return vsFileList[i];
		}
		else
		{
			if(NULL == pCurrentFile)
			{
				return vsFileList[0]; 
			}
			else
			{
				for(DWORD j = 0; j < vsFileList.size(); j++)
				{
					if(0 == _tcscmp(vsFileList[j].c_str(), pCurrentFile))
					{
						return vsFileList[(j + 1) % vsFileList.size()];
					}
				}
			}

		}
	}
}



static NOTIFYICONDATA nid; 
//////////////////////////////////////////////////////////////////////////
// add Notify icon
void AddNotifyIconInSysTray(HWND hWnd)
{
    ZeroMemory(&nid, sizeof(NOTIFYICONDATA));
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON_WALLPAPER));
    nid.hWnd = hWnd;
    _tcsncpy(nid.szTip, LoadStringFromRes(IDS_SYS_APP_NAME).c_str(), 64);
    nid.uCallbackMessage = UM_NOTIFY_ICON;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;

    Shell_NotifyIcon(NIM_ADD, &nid);
}
//////////////////////////////////////////////////////////////////////////
// remove Notify icon
void RemoveNotifyIconInSysTray(void)
{
	Shell_NotifyIcon(NIM_DELETE, &nid);
}

#endif
#if 0 

LPGUID pGuid   =   (LPGUID)   &   GUID_USB_DEVICE_ENUMERATOR_INTERFACE_CLASS;  
   
  HDEVINFO                         hardwareDeviceInfo;  
   
  ULONG predictedLength   =   0;  
  ULONG requiredLength   =   0;  
   
  hardwareDeviceInfo   =   SetupDiGetClassDevs   (  
  pGuid,  
  NULL,   //   Define   no   enumerator   (global)  
  NULL,   //   Define   no  
  (DIGCF_PRESENT   |   //   Only   Devices   present  
  DIGCF_INTERFACEDEVICE   )  
  );   //   Function   class   devices.  
   
  if(INVALID_HANDLE_VALUE   ==   hardwareDeviceInfo)  
  {  
  TRACE(_T("Windows   \"SetupDiGetClassDevs\"   API   Fail"),NULL,MB_OK);  
  return   bRetValue;  
  }  
   
  SP_INTERFACE_DEVICE_DATA                         deviceInterfaceData;  
  ZeroMemory(&deviceInterfaceData,sizeof   (SP_INTERFACE_DEVICE_DATA));  
  deviceInterfaceData.cbSize   =   sizeof   (SP_INTERFACE_DEVICE_DATA);  
   
  while(SetupDiEnumDeviceInterfaces   (  
  hardwareDeviceInfo,  
  0,   //   No   care   about   specific   PDOs  
  pGuid,  
  index,   //device   -1  
  &deviceInterfaceData)  
  )  
  {  
  SetupDiGetDeviceInterfaceDetail   (  
  hardwareDeviceInfo,  
  &deviceInterfaceData,  
  NULL,   //   probing   so   no   output   buffer   yet  
  0,   //   probing   so   output   buffer   length   of   zero  
  &requiredLength,  
  NULL  
  );  
   
  PSP_DEVICE_INTERFACE_DETAIL_DATA   FlashDiskInterfaceDetailData   =   NULL;  
  FlashDiskInterfaceDetailData   =   (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc   (requiredLength);  
  ZeroMemory(FlashDiskInterfaceDetailData,sizeof   (SP_DEVICE_INTERFACE_DETAIL_DATA));  
  FlashDiskInterfaceDetailData->cbSize   = sizeof   (SP_DEVICE_INTERFACE_DETAIL_DATA);  
   
  SP_DEVINFO_DATA     device_info_data;  
  ZeroMemory(&device_info_data,sizeof   (SP_DEVINFO_DATA));  
  device_info_data.cbSize   =   sizeof(SP_DEVINFO_DATA);  
   
  predictedLength   =   requiredLength;  
   
  if   (!   SetupDiGetDeviceInterfaceDetail   (  
  hardwareDeviceInfo,  
  &deviceInterfaceData,  
  FlashDiskInterfaceDetailData,  
  predictedLength,  
  &requiredLength,  
  &device_info_data))  
  {  
  // MessageBox(NULL,"Windows   \"SetupDiGetInterfaceDeviceDetail\"   API   Fail",NULL,MB_OK);  
  SAFE_FREE(FlashDiskInterfaceDetailData);  
  break;  
  }  
  //   find   device   interface    
  // TRACE("device_info_data.DevInst   0x%04x\n",device_info_data.DevInst);  
   
  bRetValue   =   IsDev(device_info_data.DevInst);   //–Ë«Û  
  SAFE_FREE(FlashDiskInterfaceDetailData);  
  index   ++;  
   
  if   (bRetValue   ==   TRUE)  
  {  
  break;  
  }  
  }  
   
  SetupDiDestroyDeviceInfoList   (hardwareDeviceInfo); 


#endif