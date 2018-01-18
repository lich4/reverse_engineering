/*
 * AndroidCydia.cpp
 *
 *  Created on: 2016年5月16日
 *      Author: lichao
 */


#include "Common.h"


MSConfig(MSFilterExecutable, "/system/bin/app_process");

void InitLib(void);
typedef void (*INITFUNC)(void);
__attribute__ ((section (".init_array")))
INITFUNC init_func=&InitLib;//放入单个函数

void (*oldIsInstalled)(JNIEnv *, jobject, ...);
bool isInstalled(JNIEnv *env, jobject thiz,jobject host, jint port);
void (*oldWindowObjectCleared)(JNIEnv *, jobject, ...);
void windowObjectCleared(JNIEnv *env, jobject thiz,jobject host, jint port);

jclass com_saurik_substrate_AndroidCydia;
jmethodID com_saurik_substrate_AndroidCydia_windowObjectCleared;
jmethodID android_webkit_BrowserFrame_nativeAddJavascriptInterface;

void BrowserFrameLoadCallback(JNIEnv *env, jclass _class, void *data)
{
	jmethodID android_webkit_BrowserFrame_windowObjectCleared = env->GetStaticMethodID(_class, "windowObjectCleared", "(I)V");
	MSJavaHookMethod(env, _class, android_webkit_BrowserFrame_windowObjectCleared, (void*)windowObjectCleared, (void**)oldWindowObjectCleared);
}

void UtilityLoadCallback(JNIEnv *env, jclass _class, void *data)
{
	jmethodID com_saurik_substrate_Utility_isInstalled = env->GetStaticMethodID(_class, "isInstalled", "()Z");
	MSJavaHookMethod(env, _class, com_saurik_substrate_Utility_isInstalled, (void*)isInstalled, (void**)oldIsInstalled);
}

void InitLib(void)
{
	MSJavaHookClassLoad(0, "android/webkit/BrowserFrame", BrowserFrameLoadCallback, 0);
	MSJavaHookClassLoad(0, "com/saurik/substrate/Utility", UtilityLoadCallback, 0);
}

bool isInstalled(JNIEnv *env, jobject thiz, jobject host, jint port)
{
	return true;
}

void windowObjectCleared(JNIEnv *env, jobject thiz, jobject host, jint port)
{
	if(!com_saurik_substrate_AndroidCydia)
	{
		jclass android_webkit_BrowserFrame = env->GetObjectClass(thiz);
		android_webkit_BrowserFrame_nativeAddJavascriptInterface = env->GetMethodID(android_webkit_BrowserFrame,
				"nativeAddJavascriptInterface", "(ILjava/lang/Object;Ljava/lang/String;Z)V");
		if(!android_webkit_BrowserFrame_nativeAddJavascriptInterface)
		{
			env->ExceptionClear();
			android_webkit_BrowserFrame_nativeAddJavascriptInterface = env->GetMethodID(android_webkit_BrowserFrame,
					"nativeAddJavascriptInterface", "(ILjava/lang/Object;Ljava/lang/String;Z)V");
		}
		jfieldID mContextID = env->GetFieldID(android_webkit_BrowserFrame, "mContext", "Landroid/content/Context;");
		jobject android_webkit_BrowserFrame_mContext = env->GetObjectField(thiz, mContextID);
		jclass android_content_Context = env->FindClass("android/content/Context");
		jmethodID android_content_Context_createPackageContext = env->GetMethodID(android_content_Context,
				"createPackageContext", "(Ljava/lang/String;I)Landroid/content/Context;");
		jmethodID android_content_Context_getClassLoader = env->GetMethodID(android_content_Context,
				"getClassLoader", "()Ljava/lang/ClassLoader;");
		jstring tmp1 = env->NewStringUTF("com.saurik.substrate");
		jobject packageContext = env->CallObjectMethod(android_webkit_BrowserFrame_mContext, android_content_Context_createPackageContext, tmp1, 3);
		//CONTEXT_INCLUDE_CODE | CONTEXT_IGNORE_SECURITY
		jobject classLoader = env->CallObjectMethod(packageContext, android_content_Context_getClassLoader);
		MSJavaBlessClassLoader(env, classLoader);
		jclass java_lang_Class = env->FindClass("java/lang/Class");
		jmethodID java_lang_Class_forName = env->GetStaticMethodID(java_lang_Class, "forName",
				"(Ljava/lang/String;ZLjava/lang/ClassLoader;)Ljava/lang/Class;");
		jstring tmp2 = env->NewStringUTF("com.saurik.substrate.AndroidCydia");
		jobject lAndroidCydia = env->CallStaticObjectMethod(java_lang_Class, java_lang_Class_forName, tmp2, true, classLoader);
		if(env->ExceptionCheck())
			lAndroidCydia = 0;
		com_saurik_substrate_AndroidCydia = (jclass)env->NewGlobalRef(lAndroidCydia);
		com_saurik_substrate_AndroidCydia_windowObjectCleared = env->GetStaticMethodID(com_saurik_substrate_AndroidCydia,
				"windowObjectCleared", "(Landroid/webkit/BrowserFrame;)Ljava/lang/Object;");
	}
	oldWindowObjectCleared(env, thiz, host);
	jobject cydia_ = env->CallStaticObjectMethod(com_saurik_substrate_AndroidCydia, com_saurik_substrate_AndroidCydia_windowObjectCleared, thiz);
	if(cydia_)
	{
		jstring tmp3 = env->NewStringUTF("cydia");
		env->CallVoidMethod(thiz, android_webkit_BrowserFrame_nativeAddJavascriptInterface, host, cydia_, tmp3, 0);
	}
}
