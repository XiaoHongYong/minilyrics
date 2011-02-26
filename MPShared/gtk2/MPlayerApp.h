#pragma once

class CMPlayerApp : public CMPlayerAppBase
{
public:
	CMPlayerApp();
	virtual ~CMPlayerApp();

	bool Init(int argc, char *argv[]);

	void Quit();

	BOOL IsRunning();

protected:

};
