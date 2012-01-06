#pragma once

class MapString : public map<tstring, tstring>
{
public:
	LPCTSTR GetValue(LPCTSTR szKey, LPCTSTR szDefault = _T(""));

	bool ContainsKey(LPCTSTR szKey);

};
