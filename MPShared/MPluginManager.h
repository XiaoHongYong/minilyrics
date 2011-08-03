#pragma once

#include "../MPlayerLib/IMPlayer.h"

struct DecoderInfo
{
	V_Str		vExt, vExtDesc;		// Supported file extension and descriptions.
	tstring		strDesc;
	tstring		strFileName;
	int			nIFIndex;			// index in ZikiPlayerQueryPluginIF
};

typedef list<DecoderInfo>	ListDecodersInfo;

class CMPluginManager : public IMPluginManager
{
	OBJ_REFERENCE_DECL
public:
	CMPluginManager(void);
	virtual ~CMPluginManager(void);

	virtual MLRESULT DetectPlugins();

	virtual MLRESULT OnInternalDecoderRegister(IMediaDecode *pDecoder);

	virtual MLRESULT NewInput(LPCXSTR szMediaUrl, IMediaInput **ppInput);
	virtual MLRESULT NewDecoder(IMediaInput *pInput, IMediaDecode **ppDecoder);
	virtual MLRESULT NewOutput(IMediaOutput **ppOutput);

	virtual MLRESULT GetActiveDSP(IDSP **ppDSP);
	virtual MLRESULT GetActiveVis(IVector *pvVis);

public:
	const ListDecodersInfo &GetDecodersInfo() const { return m_listDecoders; }

	// decoders
	MLRESULT RegisterDecoder(IMediaDecode *pDecoder, LPCTSTR szModuleFileName, int nIFIndex);
	void UnregisterDecoder(LPCTSTR szModuleFileName);

protected:
	// all decoders that MPlayer loaded.
	ListDecodersInfo		m_listDecoders;

};
