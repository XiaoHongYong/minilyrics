#include <stdio.h>
#include "../base/base.h"
#include "PlayerImp.h"

bool CMsgPipe::Open(const char *szName, mode_t mode)
{
	return false;
}

bool CMsgPipe::IsOK()
{
	return false;
}

bool CMsgPipe::Write(uint32 msg, LPCVOID lpData, size_t len)
{
	return false;
}

bool CMsgPipe::Write(uint32 msg, LPCSTR szData)
{
	return false;
}

bool CMsgPipe::Read(uint32 &msg, string &data)
{
	return false;
}

