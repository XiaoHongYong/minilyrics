// PreferencePageBase.cpp: implementation of the CPreferencePageBase class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerAppBase.h"
#include "PreferencePageBase.h"

//////////////////////////////////////////////////////////////////////

UIOBJECT_CLASS_NAME_IMP(CPagePfBase, _T("PagePfBase"))

CPagePfBase::CPagePfBase(PreferPageID pfPageId, LPCTSTR szAssociateTabButtonId)
{
	m_pfPageId = pfPageId;
	m_nAssociateTabButtonId = CSkinApp::GetInstance()->GetSkinFactory()->GetIDByName(szAssociateTabButtonId);
	m_nDefaultPageAssociateTabButtonId = UID_INVALID;
}

CSkinContainer *GetChildPreferPageParent(CSkinContainer *pContainer)
{
	for (int i = 0; i < pContainer->GetChildrenCount(); i++)
	{
		CUIObject *p = pContainer->GetChildByIndex(i);
		if (p->IsContainer())
		{
			if (p->IsKindOf(CPagePfBase::ClassName()))
				return pContainer;
			else
			{
				CSkinContainer *pParent = GetChildPreferPageParent(p->GetContainerIf());
				if (pParent)
					return pParent;
			}
		}
	}

	return NULL;
}

void CPagePfBase::OnInitialUpdate()
{
	CSkinContainer::OnInitialUpdate();

	// Retrieve the default associate tab button id
	CSkinContainer *pPfPageParent = GetChildPreferPageParent(this);
	if (pPfPageParent)
	{
		for (int i = 0; i < pPfPageParent->GetChildrenCount(); i++)
		{
			CUIObject *p = pPfPageParent->GetChildByIndex(i);
			if (p->IsContainer() && p->IsKindOf(CPagePfBase::ClassName()))
			{
				CPagePfBase *pPfPage = (CPagePfBase *)p;
				m_nDefaultPageAssociateTabButtonId = pPfPage->GetAssociateTabButtonId();
				if (!pPfPageParent->HasActivePage())
					pPfPageParent->SwitchToPage(p->GetClassName(), false, 0, false);
				break;
			}
		}
	}
}

void CPagePfBase::OnDestroy()
{
	m_pSkin->UnregisterUIObjNotifyHandler(this);

	ClearOptions();

	CSkinContainer::OnDestroy();
}

bool CPagePfBase::OnSwitchToPageCmd(int nId)
{
	CSkinContainer *pPfPageParent = GetChildPreferPageParent(this);
	if (pPfPageParent)
	{
		for (int i = 0; i < pPfPageParent->GetChildrenCount(); i++)
		{
			CUIObject *p = pPfPageParent->GetChildByIndex(i);
			if (p->IsContainer() && p->IsKindOf(CPagePfBase::ClassName()))
			{
				CPagePfBase *pPfPage = (CPagePfBase *)p;
				if (pPfPage->GetAssociateTabButtonId() == nId)
				{
					pPfPageParent->SwitchToPage(pPfPage->GetClassName(), false, 0, true);
					if (pPfPage->GetPfPageID() != PAGE_UNKNOWN)
					{
						// Write default page in exchange pool data
						SetExPool(SZ_EX_POOL_PF_DEFAULT_PAGE, pPfPage->GetPfPageID());
					}
					return true;
				}
			}
		}
	}

	return false;
}

bool CPagePfBase::OnCustomCommand(int nId)
{
	if (CSkinContainer::OnCustomCommand(nId))
		return true;

	// Toolbar(Tab) check command
	if (OnSwitchToPageCmd(nId))
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
				if (opt.vCtrls[i] == nId)
				{
					CMPlayerSettings::SetSettings(opt.eventType, opt.strSection.c_str(),
						opt.strSettingName.c_str(), opt.vValues[i], opt.eventType != ET_INVALID);

					// Uncheck all buttons, in this group.
					for (int k = 0; k < (int)opt.vCtrls.size(); k++)
					{
						if (k != i)
							CheckButton(opt.vCtrls[k], false);
					}
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
			CSkinComboBox	*pCombBox = (CSkinComboBox *)m_pSkin->GetUIObjectById(opt.nIDWidgetCombo, CSkinComboBox::ClassName());
			if (!pCombBox)
				continue;

			m_pSkin->RegisterUIObjNotifyHandler(opt.nIDWidgetCombo, this);

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
				ERRLOG1(_T("Control: %s is NOT defined in skin."), m_pSkin->GetSkinFactory()->GetStringOfID(opt.nCtrl).c_str());
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
	m_vOptRadioInt.clear();
}

bool CPagePfBase::SetDefaultPreferPage(CSkinContainer *pContainer, PreferPageID pfPageIdDefault)
{
	if (m_pfPageId == pfPageIdDefault)
		return true;

	for (int i = 0; i < pContainer->GetChildrenCount(); i++)
	{
		CUIObject *p = pContainer->GetChildByIndex(i);
		if (p->IsContainer())
		{
			if (p->IsKindOf(CPagePfBase::ClassName()))
			{
				CPagePfBase	*pPageChild = (CPagePfBase *)p;
				if (pPageChild->SetDefaultPreferPage(pPageChild, pfPageIdDefault))
				{
					pPageChild->GetParent()->SwitchToPage(pPageChild->GetClassName(), false, 0, false);
					m_nDefaultPageAssociateTabButtonId = pPageChild->GetAssociateTabButtonId();
					return true;
				}
			}
			else if (SetDefaultPreferPage(p->GetContainerIf(), pfPageIdDefault))
				return true;
		}
	}

	return false;
}

void CPagePfBase::CheckToolbarDefaultPage(LPCTSTR szToolbarId)
{
	if (m_nDefaultPageAssociateTabButtonId != UID_INVALID)
		CheckToolbarButton(GetIDByName(szToolbarId), m_nDefaultPageAssociateTabButtonId, true);
}
