// PreferencePageBase.cpp: implementation of the CPreferencePageBase class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerAppBase.h"
#include "PreferencePageBase.h"

//////////////////////////////////////////////////////////////////////


CPagePfBase::CPagePfBase(LPCTSTR szIdPage, PreferPageID pfPageId, LPCTSTR szAssociateTabButtonId) :
	CSkinPageHandler(szIdPage, NULL, NULL, 
				 NULL, NULL)
{
	m_strPageId = szIdPage;
	m_pfPageId = pfPageId;
	m_nAssociateTabButtonId = GetIDByName(szAssociateTabButtonId);
}

void CPagePfBase::OnCreate()
{
	CSkinPageHandler::OnCreate();
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

	// Toolbar(Tab) check command
	for (ListPageHandler::iterator it = m_listPageHandler.begin();
		it != m_listPageHandler.end(); ++it)
	{
		CPagePfBase	*pPageChild = (CPagePfBase *)*it;
		if (pPageChild->GetAssociateTabButtonId() == nId)
		{
			SwitchToPage(pPageChild->GetPageId());
			return true;
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

bool CPagePfBase::SetDefaultPreferPage(PreferPageID pfPageIdDefault)
{
	if (m_pfPageId == pfPageIdDefault)
		return true;

	for (ListPageHandler::iterator it = m_listPageHandler.begin();
		it != m_listPageHandler.end(); ++it)
	{
		CPagePfBase	*pPageChild = (CPagePfBase *)*it;
		if (pPageChild->SetDefaultPreferPage(pfPageIdDefault))
		{
			m_nIdPageDefault = pPageChild->GetPageId();
			return true;
		}
	}

	return false;
}

void CPagePfBase::CheckToolbarDefaultPage(LPCTSTR szToolbarId)
{
	CPagePfBase *pPage = (CPagePfBase *)GetPage(m_nIdPageDefault);
	if (pPage)
	{
		CheckToolbarButton(szToolbarId, 
			pPage->GetAssociateTabButtonId(), true);
	}
	else
		ASSERT(0);
}
