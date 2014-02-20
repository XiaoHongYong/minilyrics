// Thread.cpp: implementation of the CThread class.
//
//////////////////////////////////////////////////////////////////////

#include "../base.h"


JavaVM		*g_jvm = NULL;

void jvm_init(JNIEnv* env)
{
	env->GetJavaVM(&g_jvm);
}
