// PluginMJCtrl.h : Declaration of the CPluginMJCtrl

#ifndef __PluginMJCTRL_H_
#define __PluginMJCTRL_H_

#include "resource.h"       // main symbols
#include <atlctl.h>

/////////////////////////////////////////////////////////////////////////////
// CPluginMJCtrl
class ATL_NO_VTABLE CPluginMJCtrl : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public IDispatchImpl<IPluginMJCtrl, &IID_IPluginMJCtrl, &LIBID_PluginMJLib>,
	public CComCompositeControl<CPluginMJCtrl>,
	public IPersistStreamInitImpl<CPluginMJCtrl>,
	public IOleControlImpl<CPluginMJCtrl>,
	public IOleObjectImpl<CPluginMJCtrl>,
	public IOleInPlaceActiveObjectImpl<CPluginMJCtrl>,
	public IViewObjectExImpl<CPluginMJCtrl>,
	public IOleInPlaceObjectWindowlessImpl<CPluginMJCtrl>,
	public IPersistStorageImpl<CPluginMJCtrl>,
	public ISpecifyPropertyPagesImpl<CPluginMJCtrl>,
	public IQuickActivateImpl<CPluginMJCtrl>,
	public IDataObjectImpl<CPluginMJCtrl>,
	public IProvideClassInfo2Impl<&CLSID_PluginMJCtrl, NULL, &LIBID_PluginMJLib>,
	public CComCoClass<CPluginMJCtrl, &CLSID_PluginMJCtrl>
{
public:
	CPluginMJCtrl();

DECLARE_REGISTRY_RESOURCEID(IDR_PluginMJCTRL)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CPluginMJCtrl)
	COM_INTERFACE_ENTRY(IPluginMJCtrl)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IViewObjectEx)
	COM_INTERFACE_ENTRY(IViewObject2)
	COM_INTERFACE_ENTRY(IViewObject)
	COM_INTERFACE_ENTRY(IOleInPlaceObjectWindowless)
	COM_INTERFACE_ENTRY(IOleInPlaceObject)
	COM_INTERFACE_ENTRY2(IOleWindow, IOleInPlaceObjectWindowless)
	COM_INTERFACE_ENTRY(IOleInPlaceActiveObject)
	COM_INTERFACE_ENTRY(IOleControl)
	COM_INTERFACE_ENTRY(IOleObject)
	COM_INTERFACE_ENTRY(IPersistStreamInit)
	COM_INTERFACE_ENTRY2(IPersist, IPersistStreamInit)
	COM_INTERFACE_ENTRY(ISpecifyPropertyPages)
	COM_INTERFACE_ENTRY(IQuickActivate)
	COM_INTERFACE_ENTRY(IPersistStorage)
	COM_INTERFACE_ENTRY(IDataObject)
	COM_INTERFACE_ENTRY(IProvideClassInfo)
	COM_INTERFACE_ENTRY(IProvideClassInfo2)
END_COM_MAP()

BEGIN_PROP_MAP(CPluginMJCtrl)
	PROP_DATA_ENTRY("_cx", m_sizeExtent.cx, VT_UI4)
	PROP_DATA_ENTRY("_cy", m_sizeExtent.cy, VT_UI4)
	// Example entries
	// PROP_ENTRY("Property Description", dispid, clsid)
	// PROP_PAGE(CLSID_StockColorPage)
END_PROP_MAP()

BEGIN_MSG_MAP(CPluginMJCtrl)
	CHAIN_MSG_MAP(CComCompositeControl<CPluginMJCtrl>)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
END_MSG_MAP()
// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

BEGIN_SINK_MAP(CPluginMJCtrl)
	//Make sure the Event Handlers have __stdcall calling convention
END_SINK_MAP()

	STDMETHOD(OnAmbientPropertyChange)(DISPID dispid)
	{
		if (dispid == DISPID_AMBIENT_BACKCOLOR)
		{
			SetBackgroundColorFromAmbient();
			FireViewChange();
		}
		return IOleControlImpl<CPluginMJCtrl>::OnAmbientPropertyChange(dispid);
	}



// IViewObjectEx
	DECLARE_VIEW_STATUS(0)

// IPluginMJCtrl
public:
	STDMETHOD(Init)(/*[in]*/ LPDISPATCH pDisp);

	enum { IDD = IDD_MINILYRCISFORMJ };

protected:
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

};

#endif //__PluginMJCTRL_H_
