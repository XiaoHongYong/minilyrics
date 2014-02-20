
#pragma once

#include <jni.h>

extern JavaVM	*g_jvm;

void jvm_init(JNIEnv* env);
