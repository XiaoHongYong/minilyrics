// PreferencePageBase.cpp: implementation of the CPreferencePageBase class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PreferencePageBase.h"
#include "SkinComboBox.h"

//////////////////////////////////////////////////////////////////////


CPagePfBase::CPagePfBase(LPCTSTR szIdPage) :
	CSkinPageHandler(szIdPage, NULL, NULL, 
				 NULL, NULL)
{
	m_strPageId = szIdPage;
}

void CPagePfBase::OnCreate()
{
	CSkinPageHandler::OnCreate();

	m_nAnimationShowFLeft = NewAnimationObj(_T("MoveFromLeft"), m_strPageId.c_str());
	m_nAnimationShowFRight = NewAnimationObj(_T("MoveFromRight"), m_strPageId.c_str());
	m_nAnimationHide2Left = NewAnimationObj(_T("MoveToLeft"), m_strPageId.c_str());
	m_nAnimationHide2Right = NewAnimationObj(_T("MoveToRight"), m_strPageId.c_str());
}

void CPagePfBase::OnDestroy()
{
	m_pSkinWnd->UnregisterUIObjNotifyHandler(this);

	ClearOptions();

	CSkinPageHandler::OnDestroy();
}

bool CPagePfBase::OnCustomCommand(int nId)
{
	if (CSkinPageHandler::OnCustomCommand(nId))
		return true;

	{
		// Bool
		VOptBool::iterator	it, itEnd;
		tstring		strValue;
		itEnd = m_vOptBool.end();
		for (it = m_vOptBool.begin(); it != itEnd; ++it)
		{
			OptBool		&opt = *it;
			if (opt.nCtrl == nId)
			{
				bool	bValue;

				CSkinNStatusButton *pButton = (CSkinNStatusButton*)GetUIObjectById(opt.nCtrl, CSkinNStatusButton::ClassName());
				if (pButton)
				{
					bValue = Tobool(pButton->GetStatus());
					CMPlayerSettings::SetSettings(opt.eventType, opt.strSection.c_str(),
						opt.strSettingName.c_str(), bValue, opt.eventType != ET_INVALID);
					return true;
				}
			}
		}
	}

	return false;
}

void CPagePfBase::AddOptBool(EventType evtType, LPCTSTR szSection, LPCTSTR szSettingName, bool bDefValue, LPCTSTR szCtrlId)
{
	OptBool	opt;
	opt.eventType = evtType;
	opt.strSection = szSection;
	opt.strSettingName = szSettingName;
	opt.nDefaultValue = bDefValue;
	opt.nCtrl = GetIDByName(szCtrlId);

	m_vOptBool.push_back(opt);
}

void CPagePfBase::InitCheckButtons()
{
	{
		//
		// Check Combo Str
		//
		VOptComboStr::iterator	it, itEnd;
		tstring		strValue;
		itEnd = m_vOptComboStr.end();
		for (it = m_vOptComboStr.begin(); it != itEnd; ++it)
		{
			OptComboStr		&opt = *it;
			CSkinComboBox	*pCombBox = (CSkinComboBox *)m_pSkinWnd->GetUIObjectById(opt.nIDWidgetCombo, CSkinComboBox::ClassName());
			if (!pCombBox)
				continue;

			m_pSkinWnd->RegisterUIObjNotifyHandler(opt.nIDWidgetCombo, this);

			strValue = g_Profile.GetProfileString(opt.strSection.c_str(), opt.strSettingName.c_str(), opt.strDefaultValue.c_str());
			if (strValue.empty())
				strValue = opt.strDefaultValue;
			else
				opt.strDefaultValue = strValue;
			for (int i = 0; i < (int)opt.vValues.size(); i++)
			{
				if (_tcsicmp(opt.vValues[i].c_str(), strValue.c_str()) == 0)
				{
					pCombBox->SetCurSel(i);
					break;
				}
			}
		}
	}

	{
		//
		// Init Check Bool Buttons
		//
		VOptBool::iterator	it, itEnd;
		itEnd = m_vOptBool.end();
		for (it = m_vOptBool.begin(); it != itEnd; ++it)
		{
			OptBool		&opt = *it;
			bool		bValue;

			bValue = g_Profile.GetProfileBool(opt.strSection.c_str(), opt.strSettingName.c_str(), opt.nDefaultValue);
			CSkinNStatusButton *pButton = (CSkinNStatusButton*)GetUIObjectById(opt.nCtrl, CSkinNStatusButton::ClassName());
			if (pButton)
				pButton->SetStatus(bValue);
			else
				ERRLOG1(_T("Control: %s is NOT defined in skin."), m_pSkinWnd->GetSkinFactory()->GetStringOfID(opt.nCtrl));
		}
	}
}

void CPagePfBase::OnUIObjNotify(IUIObjNotify *pNotify)
{
	if (pNotify->IsKindOf(CSkinComboBox::ClassName()))
	{
		//
		// Process Combo Str select changed command
		//
		VOptComboStr::iterator	it, itEnd;
		tstring		strValue;
		itEnd = m_vOptComboStr.end();
		for (it = m_vOptComboStr.begin(); it != itEnd; ++it)
		{
			OptComboStr		&opt = *it;
			if (opt.nIDWidgetCombo != pNotify->nID
				|| ((CSkinComboBoxEventNotify *)pNotify)->cmd != CSkinComboBoxEventNotify::C_SEL_CHANGED)
				continue;

			CSkinComboBox	*pCombbox = (CSkinComboBox *)pNotify->pUIObject;
			int				nSel;
			nSel = pCombbox->GetCurSel();

			if (nSel >= 0 && nSel < (int)opt.vValues.size())
			{
				if (_tcsicmp(opt.vValues[nSel].c_str(), opt.strDefaultValue.c_str()) != 0)
				{
					// Value changed...
					opt.strDefaultValue = opt.vValues[nSel];
					CMPlayerSettings::SetSettings(opt.eventType, opt.strSection.c_str(),
						opt.strSettingName.c_str(), opt.vValues[nSel].c_str(), opt.eventType != ET_INVALID);
				}
			}
			return;
		}
	}
}

void CPagePfBase::ClearOptions()
{
	m_vOptBool.clear();
	m_vOptComboStr.clear();
}

int CPagePfBase::NewAnimationObj(LPCTSTR szAnimationName, LPCTSTR szTargetUIObj)
{
	CSkinFactory	*pSkinFactory = m_pSkinWnd->GetSkinFactory();
	CUIObject *pObj = pSkinFactory->CreateUIObject(m_pSkinWnd, CSkinAnimationObj::ClassName());
	if (!pObj)
		return SKIN_UIOBJ_ID_UNDEFINE;

	pObj->m_nId = pSkinFactory->AllocCustomID();
	pObj->SetProperty(szAnimationName, szTargetUIObj);

	m_pSkinWnd->GetRootContainer()->AddUIObject(pObj);

	return pObj->m_nId;
}

//////////////////////////////////////////////////////////////////////

CPreferencePageBase::CPreferencePageBase(UINT nIDTemplate, LPCTSTR szCaption) : CBaseDialog(nIDTemplate), m_strCaption(szCaption)
{
	m_pParentPage = NULL;
	m_PageID = PAGE_UNKNOWN;
}

CPreferencePageBase::~CPreferencePageBase()
{

}

BOOL CPreferencePageBase::OnInitDialog()
{
	if (!CBaseDialog::OnInitDialog())
		return FALSE;

	m_wndHelper.Init(this);

	return TRUE;
}

void CPreferencePageBase::AddOptBool(EventType evtType, LPCTSTR szSection, LPCTSTR szSettingName, bool bDefValue, int nIDCtrl)
{
	OptBool	opt;
	opt.eventType = evtType;
	opt.strSection = szSection;
	opt.strSettingName = szSettingName;
	opt.nDefaultValue = bDefValue;
	opt.nCtrl = nIDCtrl;

	m_vOptBool.push_back(opt);
}

void CPreferencePageBase::AddOptBoolStr(EventType evtType, LPCTSTR szSection, LPCTSTR szSettingName, LPCTSTR szDefValue, int nIDCtrl)
{
	OptBoolStr	opt;
	opt.eventType = evtType;
	opt.strSection = szSection;
	opt.strSettingName = szSettingName;
	opt.strDefaultValue = szDefValue;
	opt.nCtrl = nIDCtrl;

	m_vOptBoolStr.push_back(opt);
}

void CPreferencePageBase::InitCheckButtons()
{
	{
		//
		// Check Combo Str
		//
		VOptComboStr::iterator	it, itEnd;
		tstring		strValue;
		itEnd = m_vOptComboStr.end();
		for (it = m_vOptComboStr.begin(); it != itEnd; ++it)
		{
			OptComboStr		&opt = *it;
			CWidgetComboBox	cbCtrl;
			cbCtrl.Attach(this, opt.nIDWidgetCombo);

			strValue = g_Profile.GetProfileString(opt.strSection.c_str(), opt.strSettingName.c_str(), opt.strDefaultValue.c_str());
			if (strValue.empty())
				strValue = opt.strDefaultValue;
			else
				opt.strDefaultValue = strValue;
			for (int i = 0; i < (int)opt.vValues.size(); i++)
			{
				if (_tcsicmp(opt.vValues[i].c_str(), strValue.c_str()) == 0)
				{
					cbCtrl.SetCurSel(i);
					break;
				}
			}
			cbCtrl.Detach();
		}
	}

	{
		//
		// Check Radio Str Buttons
		//
		VOptRadioStr::iterator	it, itEnd;
		tstring		strValue;
		itEnd = m_vOptRadioStr.end();
		for (it = m_vOptRadioStr.begin(); it != itEnd; ++it)
		{
			OptRadioStr		&opt = *it;

			strValue = g_Profile.GetProfileString(opt.strSection.c_str(), opt.strSettingName.c_str(), opt.strDefaultValue.c_str());
			if (strValue.empty())
				strValue = opt.strDefaultValue;
			for (int i = 0; i < (int)opt.vValues.size(); i++)
			{
				if (_tcsicmp(opt.vValues[i].c_str(), strValue.c_str()) == 0)
				{
					CheckButton(opt.vCtrls[i], true);
					break;
				}
			}
		}
	}

	{
		//
		// Check Radio Int Buttons
		//
		VOptRadioInt::iterator	it, itEnd;
		int		nValue;
		itEnd = m_vOptRadioInt.end();
		for (it = m_vOptRadioInt.begin(); it != itEnd; ++it)
		{
			OptRadioInt		&opt = *it;

			nValue = g_Profile.GetProfileInt(opt.strSection.c_str(), opt.strSettingName.c_str(), opt.nDefaultValue);
			for (int i = 0; i < (int)opt.vValues.size(); i++)
			{
				if (opt.vValues[i] == nValue)
				{
					CheckButton(opt.vCtrls[i], true);
					break;
				}
			}
		}
	}

	{
		//
		// Init Check BoolStr Buttons
		//
		VOptBoolStr::iterator	it, itEnd;
		tstring		strValue;
		itEnd = m_vOptBoolStr.end();
		for (it = m_vOptBoolStr.begin(); it != itEnd; ++it)
		{
			OptBoolStr		&opt = *it;

			strValue = g_Profile.GetProfileString(opt.strSection.c_str(), opt.strSettingName.c_str(), opt.strDefaultValue.c_str());
			if (IsTRUE(strValue.c_str()))
				CheckButton(opt.nCtrl, true);
			else
				CheckButton(opt.nCtrl, false);
		}
	}

	{
		//
		// Init Check Bool Buttons
		//
		VOptBool::iterator	it, itEnd;
		itEnd = m_vOptBool.end();
		for (it = m_vOptBool.begin(); it != itEnd; ++it)
		{
			OptBool		&opt = *it;
			bool		bValue;

			bValue = g_Profile.GetProfileBool(opt.strSection.c_str(), opt.strSettingName.c_str(), opt.nDefaultValue);
			if (bValue)
				CheckButton(opt.nCtrl, true);
			else
				CheckButton(opt.nCtrl, false);
		}
	}
}

void CPreferencePageBase::OnSize(int cx, int cy)
{
	m_wndHelper.OnResize();

	CBaseDialog::OnSize(cx, cy);
}

bool CPreferencePageBase::ProcessCommand(UINT uID, UINT nNotifyCode)
{
	{
		//
		// Process Combo Str select changed command
		//
		VOptComboStr::iterator	it, itEnd;
		tstring		strValue;
		itEnd = m_vOptComboStr.end();
		for (it = m_vOptComboStr.begin(); it != itEnd; ++it)
		{
			OptComboStr		&opt = *it;
			if (opt.nIDWidgetCombo != uID || nNotifyCode != CBN_SELCHANGE)
				continue;

			CWidgetComboBox	cbCtrl;
			int				nSel;
			cbCtrl.Attach(this, opt.nIDWidgetCombo);
			nSel = cbCtrl.GetCurSel();

			if (nSel >= 0 && nSel < (int)opt.vValues.size())
			{
				if (_tcsicmp(opt.vValues[nSel].c_str(), opt.strDefaultValue.c_str()) != 0)
				{
					// Value changed...
					opt.strDefaultValue = opt.vValues[nSel];
					CMPlayerSettings::SetSettings(opt.eventType, opt.strSection.c_str(),
						opt.strSettingName.c_str(), opt.vValues[nSel].c_str(), opt.eventType != ET_INVALID);
				}
			}
			cbCtrl.Detach();
			return true;
		}
	}

	{
		//
		// Process Radio Buttons Command
		//
		VOptRadioStr::iterator	it, itEnd;
		tstring		strValue;
		itEnd = m_vOptRadioStr.end();
		for (it = m_vOptRadioStr.begin(); it != itEnd; ++it)
		{
			OptRadioStr		&opt = *it;

			for (int i = 0; i < (int)opt.vCtrls.size(); i++)
			{
				if (opt.vCtrls[i] == uID)
				{
					CMPlayerSettings::SetSettings(opt.eventType, opt.strSection.c_str(),
						opt.strSettingName.c_str(), opt.vValues[i].c_str(), opt.eventType != ET_INVALID);
					return true;
				}
			}
		}
	}

	{
		//
		// Process Radio int Buttons Command
		//
		VOptRadioInt::iterator	it, itEnd;
		itEnd = m_vOptRadioInt.end();
		for (it = m_vOptRadioInt.begin(); it != itEnd; ++it)
		{
			OptRadioInt		&opt = *it;

			for (int i = 0; i < (int)opt.vCtrls.size(); i++)
			{
				if (opt.vCtrls[i] == uID)
				{
					CMPlayerSettings::SetSettings(opt.eventType, opt.strSection.c_str(),
						opt.strSettingName.c_str(), opt.vValues[i], opt.eventType != ET_INVALID);
					return true;
				}
			}
		}
	}

	{
		// Bool str
		VOptBoolStr::iterator	it, itEnd;
		tstring		strValue;
		itEnd = m_vOptBoolStr.end();
		for (it = m_vOptBoolStr.begin(); it != itEnd; ++it)
		{
			OptBoolStr		&opt = *it;
			if (opt.nCtrl == uID)
			{
				bool	bValue;
				bValue = IsButtonChecked(opt.nCtrl);
				CMPlayerSettings::SetSettings(opt.eventType, opt.strSection.c_str(),
					opt.strSettingName.c_str(), BOOLTOSTR(bValue), opt.eventType != ET_INVALID);
				return true;
			}
		}
	}

	{
		// Bool
		VOptBool::iterator	it, itEnd;
		tstring		strValue;
		itEnd = m_vOptBool.end();
		for (it = m_vOptBool.begin(); it != itEnd; ++it)
		{
			OptBool		&opt = *it;
			if (opt.nCtrl == uID)
			{
				bool	bValue;
				bValue = IsButtonChecked(opt.nCtrl);
				CMPlayerSettings::SetSettings(opt.eventType, opt.strSection.c_str(),
					opt.strSettingName.c_str(), bValue, opt.eventType != ET_INVALID);
				return true;
			}
		}
	}

	return false;
}

void CPreferencePageBase::ClearOptions()
{
	m_vOptComboStr.clear();
	m_vOptRadioStr.clear();
	m_vOptRadioInt.clear();
	m_vOptBoolStr.clear();
	m_vOptBool.clear();
}
