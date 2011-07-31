#include "stdafx.h"
#include "Player.h"
#include "MPluginManager.h"


#ifdef _WIN32

class CZikiPlayerPlugin
{
public:
	CZikiPlayerPlugin() { m_hModule = NULL; m_QueryPluginIF = NULL; }
	virtual ~CZikiPlayerPlugin() { if (m_hModule) FreeLibrary(m_hModule); }

	bool Open(LPCTSTR szModule)
	{
		if (m_hModule)
		{
			FreeLibrary(m_hModule);
			m_QueryPluginIF = NULL;
		}

		m_hModule = LoadLibrary(szModule);
		if (m_hModule == NULL)
			return false;

		return Attach(m_hModule);
	}

	bool Attach(HMODULE hModule)
	{
		m_hModule = hModule;
		m_QueryPluginIF = (ZikiPlayerQueryPluginIF_t)GetProcAddress(m_hModule, SZ_FUNC_ZP_QUERY_PLUGIN_IF);
		return m_QueryPluginIF != NULL;
	}

	void Detach() { m_hModule = NULL; m_QueryPluginIF = NULL; }

	MLRESULT ZikiPlayerQueryPluginIF(
		int nIndex,
		MPInterfaceType *pInterfaceType,
		IXStr *strDescription,
		LPVOID *lpInterface
		)
	{
		if (m_QueryPluginIF)
			return m_QueryPluginIF(nIndex, pInterfaceType, strDescription, lpInterface);
		else
			return ERR_FALSE;
	}

protected:
	HMODULE						m_hModule;
	ZikiPlayerQueryPluginIF_t	m_QueryPluginIF;

};

#else

class CZikiPlayerPlugin
{
public:
	CZikiPlayerPlugin() { m_QueryPluginIF = NULL; }
	virtual ~CZikiPlayerPlugin() { }

	bool Open(LPCTSTR szModule)
	{
		return false;
	}

	void Detach() { }

	MLRESULT ZikiPlayerQueryPluginIF(
		int nIndex,
		MPInterfaceType *pInterfaceType,
		IXStr *strDescription,
		LPVOID *lpInterface
		)
	{
		if (m_QueryPluginIF)
			return m_QueryPluginIF(nIndex, pInterfaceType, strDescription, lpInterface);
		else
			return ERR_FALSE;
	}

protected:
	ZikiPlayerQueryPluginIF_t	m_QueryPluginIF;

};

#endif

CMPluginManager::CMPluginManager(void)
{
	OBJ_REFERENCE_INIT
}


CMPluginManager::~CMPluginManager(void)
{
}


//
// Detect all the plugins and manage them.
// All plugins are in "Plugins" folder.
//
MLRESULT CMPluginManager::DetectPlugins()
{
	tstring			strPluginFolder, strPlugin;
	CMLFileFind		finder;

	strPluginFolder = GetWorkingFolder();
	strPluginFolder += _T("Plugins");
	DirStringFormat(strPluginFolder);

	if (!finder.OpenDir(strPluginFolder.c_str()))
		return ERR_OK;

	while (finder.FindNext())
	{
		if (finder.IsCurDir() || !FileIsExtSame(finder.GetCurName(), _T(".dll")))
			continue;

		strPlugin = strPluginFolder;
		strPlugin += finder.GetCurName();

		CZikiPlayerPlugin		plugin;

		if (!plugin.Open(strPlugin.c_str()))
			continue;

		for (int i = 0; ; i++)
		{
			MPInterfaceType	ifType;
			MLRESULT		nRet;
			CXStr			strDesc;

			nRet = plugin.ZikiPlayerQueryPluginIF(i, &ifType, &strDesc, NULL);
			if (nRet != ERR_OK)
				break;

			switch (ifType)
			{
			case MPIT_INPUT_DECTOR:
				{
					CMPAutoPtr<IMediaInput>	input;

					nRet = plugin.ZikiPlayerQueryPluginIF(i, &ifType, NULL, (LPVOID*)&input);
					if (nRet != ERR_OK || ifType != MPIT_INPUT)
						break;
				}
				break;
			case MPIT_DECODE:
				{
					CMPAutoPtr<IMediaDecode>	decoder;

					nRet = plugin.ZikiPlayerQueryPluginIF(i, &ifType, NULL, (LPVOID*)&decoder);
					if (nRet != ERR_OK || ifType != MPIT_DECODE)
						break;

					RegisterDecoder(decoder, strPlugin.c_str(), i);
				}
				break;
			}
		}
	}

	return ERR_OK;
}


MLRESULT CMPluginManager::OnInternalDecoderRegister(IMediaDecode *pDecoder)
{
	return RegisterDecoder(pDecoder, _T(""), 0);
}


MLRESULT CMPluginManager::NewInput(LPCXSTR szMediaUrl, IMediaInput **ppInput)
{
	// Use internal input
	return ERR_NOT_SUPPORT;
}


MLRESULT CMPluginManager::NewDecoder(IMediaInput *pInput, IMediaDecode **ppDecoder)
{
	LPCXSTR						szMediaUrl = pInput->GetSource();
	ListDecodersInfo::iterator	it;
	LPCTSTR						szExt;

	*ppDecoder = NULL;

	szExt = FileGetExt(szMediaUrl);
	if (IsEmptyString(szExt))
		return ERR_NOT_SUPPORT_FILE_FORMAT;

	for (it = m_listDecoders.begin(); it != m_listDecoders.end(); ++it)
	{
		DecoderInfo		&info = *it;
		LPCTSTR			szNext;

		szNext = info.strExt.c_str();
		while (szNext)
		{
			if (_tcsicmp(szNext, szExt) == 0)
			{
				CZikiPlayerPlugin	plugin;
				MPInterfaceType		mpit;

				if (!plugin.Open(info.strFileName.c_str()))
					break;	// Try next plugin

				if (plugin.ZikiPlayerQueryPluginIF(info.nIFIndex, &mpit, NULL, (LPVOID *)ppDecoder) != ERR_OK)
					break;	// Try next plugin
				plugin.Detach();

				return ERR_OK;
			}
			szNext = MultiStrNext(szNext);
		}
	}

	return ERR_NOT_SUPPORT_FILE_FORMAT;
}


MLRESULT CMPluginManager::NewOutput(IMediaOutput **ppOutput)
{
	// Use internal output
	return ERR_NOT_SUPPORT;
}


MLRESULT CMPluginManager::GetActiveDSP(IDSP **ppDSP)
{
	return ERR_NOT_SUPPORT;
}


MLRESULT CMPluginManager::GetActiveVis(IVector *pvVis)
{
	return ERR_NOT_SUPPORT;
}


MLRESULT CMPluginManager::RegisterDecoder(IMediaDecode *pDecoder, LPCTSTR szModuleFileName, int nIFIndex)
{
	DecoderInfo		decoderInfo;
	vector<tstring>	vStrExt;
	LPCTSTR			szExt, szExtDesc;

	szExt = pDecoder->GetFileExtentions();
	while (szExt)
	{
		szExtDesc = MultiStrNext(szExt);
		if (szExtDesc)
		{
			decoderInfo.strExt.append(szExt);
			decoderInfo.strExt.insert(decoderInfo.strExt.end(), '\0');

			decoderInfo.strExtDesc.append(szExtDesc);
			decoderInfo.strExtDesc.insert(decoderInfo.strExtDesc.end(), '\0');

			szExt = MultiStrNext(szExtDesc);
		}
		else
			break;
	}
	decoderInfo.strDesc = pDecoder->GetDescription();
	decoderInfo.strFileName = szModuleFileName;
	decoderInfo.nIFIndex = nIFIndex;


	// Is this decoder registered?
	for (ListDecodersInfo::iterator it = m_listDecoders.begin(); it != m_listDecoders.end(); ++it)
	{
		DecoderInfo		&info = *it;
		if (!info.strFileName.empty()
			&& _tcsicmp(info.strFileName.c_str(), szModuleFileName) == 0
			&& info.nIFIndex == nIFIndex)
		{
			info = decoderInfo;
			return true;
		}
	}
	m_listDecoders.push_back(decoderInfo);

	return true;
}


void CMPluginManager::UnregisterDecoder(LPCTSTR szModuleFileName)
{
	ListDecodersInfo::iterator		it, itEnd;

	itEnd = m_listDecoders.end();
	for (it = m_listDecoders.begin(); it != itEnd; ++it)
	{
		DecoderInfo	&item = *it;
		if (_tcscmp(item.strFileName.c_str(), szModuleFileName) == 0)
		{
			m_listDecoders.erase(it);
			return;
		}
	}
}
