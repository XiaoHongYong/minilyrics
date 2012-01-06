#include "StdAfx.h"
#include "MapString.h"

LPCTSTR MapString::GetValue(LPCTSTR szKey, LPCTSTR szDefault)
{
	iterator it = find(szKey);
	if (it == end())
		return szDefault;

	return (*it).second.c_str();
}

bool MapString::ContainsKey(LPCTSTR szKey)
{
	return find(szKey) != end();
}
