/*
 * ApiDvm.cpp
 *
 *  Created on: 2016年5月17日
 *      Author: lichao
 */

#include "Common.h"
#include "DalvikHeader.h"
#include "jni.h"


#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO, "CydiaSubstrate", __VA_ARGS__)

#define PRIVTAG 0x882002EC
#define LW_SHAPE_THIN 0
#define LW_SHAPE_FAT 1
#define LW_SHAPE_MASK 0x1
#define LW_HASH_STATE_MASK 0x3
#define LW_HASH_STATE_SHIFT 1
#define LW_SHAPE(x) ((x) & LW_SHAPE_MASK)
#define LW_MONITOR(x) ((Monitor*)((x) & ~((LW_HASH_STATE_MASK << LW_HASH_STATE_SHIFT) | LW_SHAPE_MASK)))
#define DALVIK_JNI_RETURN_SHIFT 28
#define DALVIK_JNI_NO_ARG_INFO  0x80000000

enum DalvikJniReturnType
{
	DALVIK_JNI_RETURN_VOID = 0,     /* must be zero */
	DALVIK_JNI_RETURN_FLOAT = 1,
	DALVIK_JNI_RETURN_DOUBLE = 2,
	DALVIK_JNI_RETURN_S8 = 3,
	DALVIK_JNI_RETURN_S4 = 4,
	DALVIK_JNI_RETURN_S2 = 5,
	DALVIK_JNI_RETURN_U2 = 6,
	DALVIK_JNI_RETURN_S1 = 7
};

typedef void (*MethodCallback)(JNIEnv *jni, jclass clazz, void *data);
struct HookMethodList
{
	HookMethodList* next;
	char*			Descriptor;
	MethodCallback	Callback;
	void*			Data;
};

bool MSDebug;
HookMethodList *g_MethodListHeader;
bool isLinkClassCalled = false;
MSImageRef reflibdvm;
MSJavaObjectKey g_ObjKey;

MSImageRef MSGetImageByName(const char *file);
void* MSFindSymbol(MSImageRef image, const char *name);
void MSHookFunction(void *symbol, void *replace, void **result);
void* MSCloseFunction(void* ,void*);

typedef void (*INITFUNC)(void);
void InitLib(void);
__attribute__ ((section (".init_array")))
INITFUNC init_func=&InitLib;//放入单个函数


extern "C"
{
	void dvmLockObject(Thread* self, Object *obj);
	bool dvmUnlockObject(Thread* self, Object *obj);
}

bool (*OlddvmCheckClassAccess)(const ClassObject* accessFrom, const ClassObject* clazz)=0;
bool (*OlddvmCheckFieldAccess)(const ClassObject* accessFrom, const Field* field)=0;
bool (*OlddvmCheckMethodAccess)(const ClassObject* accessFrom, const Method* method)=0;
bool (*OlddvmLinkClass)(ClassObject* clazz)=0;
void (*OlddvmSweepMonitorList)(Monitor** mon, int (*isUnmarkedObject)(void*))=0;
bool NewdvmCheckClassAccess(const ClassObject* accessFrom, const ClassObject* clazz);
bool NewdvmCheckFieldAccess(const ClassObject* accessFrom, const Field* field);
bool NewdvmCheckMethodAccess(const ClassObject* accessFrom, const Method* method);
bool NewdvmLinkClass(ClassObject* clazz);
void NewdvmSweepMonitorList(Monitor** mon, int (*isUnmarkedObject)(void*));

void (*dvmCallMethod)(Thread* self, const Method* method, Object* obj, JValue* pResult, ...)=0;
bool (*dvmCheckClassAccess)(const ClassObject* accessFrom, const ClassObject* clazz)=0;
bool (*dvmCheckFieldAccess)(const ClassObject* accessFrom, const Field* field)=0;
bool (*dvmCheckMethodAccess)(const ClassObject* accessFrom, const Method* method)=0;
void (*dvmClearException)(Thread* self)=0;
Monitor* (*dvmCreateMonitor)(Object* obj)=0;
char* (*dvmDescriptorToName)(const char* str)=0;
ClassObject* (*dvmFindClass)(const char* descriptor, Object* loader)=0;
Method* (*dvmFindDirectMethodByDescriptor)(const ClassObject* clazz, const char* methodName, const char* descriptor)=0;
Object* (*dvmGetException)(Thread* self)=0;
JNIEnvExt* (*dvmGetJNIEnvForThread)()=0;
bool (*dvmInSamePackage)(const ClassObject* class1, const ClassObject* class2)=0;
bool (*dvmLinkClass)(ClassObject* clazz)=0;
void (*dvmLogNativeMethodEntry)(const Method* method, const u4* args)=0;
void (*dvmLogNativeMethodExit)(const Method* method, Thread* self, const JValue returnValue)=0;
ClassObject* (*dvmLookupClass)(const char* descriptor, Object* loader, bool unprepOkay)=0;
char* (*dvmNameToDescriptor)(const char* str)=0;
void (*dvmObjectWait)(Thread* self, Object *obj, s8 msec, s4 nsec, bool interruptShouldThrow)=0;
u4 (*dvmPlatformInvokeHints)(const DexProto* proto)=0;
bool (*dvmPopLocalFrame)(Thread* self)=0;
const char* (*dexProtoGetShorty)(const DexProto* pProto)=0;
bool (*dvmPushLocalFrame)(Thread* self, const Method* method)=0;
void (*dvmSetException)(Thread* self, Object* exception)=0;
void (*dvmSetNativeFunc)(const Method* method, DalvikBridgeFunc func, const u2* insns)=0;
void (*dvmSweepMonitorList)(Monitor** mon, int (*isUnmarkedObject)(void*))=0;
Thread* (*dvmThreadSelf)(void)=0;
void (*dvmUseJNIBridge)(Method* method, void* func)=0;
void (*dvmCheckCallJNIMethod_general)(const u4* args, JValue* pResult, const Method* method, Thread* self)=0;
Object* (*dvmDecodeIndirectRef)(JNIEnv* env, jobject jobj)=0;//2.x 3.x
Object* (*dvmDecodeIndirectRef_)(Thread* self, jobject jobj)=0;//4.x 5.x


void InitLib(void)
{
	reflibdvm = MSGetImageByName("/system/lib/libdvm.so");
	char buf[256];
#define getAddr(name,x,y) \
	name = (typeof(name))MSFindSymbol(reflibdvm, #name);\
	if(!name)\
	{\
		sprintf(buf, "_Z%zu%s%s", x, #name, y);\
		name = (typeof(name))MSFindSymbol(reflibdvm, buf);\
	}
#define getAddr2(name,x,y,z) \
	name = (typeof(name))MSFindSymbol(reflibdvm, #name);\
	if(!name)\
	{\
		sprintf(buf, "_Z%zu%s%s", x, #name, y);\
		name = (typeof(name))MSFindSymbol(reflibdvm, buf);\
		if(!name)\
			name = (typeof(name))MSFindSymbol(reflibdvm, z);\
	}
	getAddr(dvmCallMethod, 13, "P6ThreadPK6MethodP6ObjectP6JValuez");
	getAddr(dvmCheckClassAccess, 19, "PK11ClassObjectS1_");
	getAddr(dvmCheckFieldAccess, 19, "PK11ClassObjectPK5Field");
	getAddr(dvmCheckMethodAccess, 20, "PK11ClassObjectPK6Method");
	getAddr(dvmClearException, 17, "P6Thread");
	getAddr(dvmCreateMonitor, 16, "P6Object");
	getAddr(dvmDescriptorToName, 19, "PKc");
	getAddr(dvmFindClass, 12, "PKcP6Object");
	getAddr(dvmFindDirectMethodByDescriptor, 31, "PK11ClassObjectPKcS3_");
	getAddr(dvmGetException, 15, "P6Thread");
	getAddr(dvmGetJNIEnvForThread, 21, "v");
	getAddr(dvmInSamePackage, 16, "PK11ClassObjectS1_");
	getAddr(dvmLinkClass, 12, "P11ClassObject");
	getAddr(dvmLogNativeMethodEntry, 23, "PK6MethodPKj");
	getAddr(dvmLogNativeMethodExit, 22, "PK6MethodPK6ThreadK6JValue");
	getAddr(dvmLookupClass, 14, "PKcP6Objectb");
	getAddr(dvmNameToDescriptor, 19, "PKc");
	getAddr(dvmObjectWait, 13, "P6ThreadP6Objectxib");
	getAddr(dvmPlatformInvokeHints, 22, "PK8DexProto");
	getAddr(dvmPopLocalFrame, 16, "P6Thread");
	getAddr(dexProtoGetShorty, 17, "PK8DexProto");
	getAddr(dvmPushLocalFrame, 17, "P6ThreadPK6Method");
	getAddr(dvmSetException, 15, "P6ThreadP6Object");
	getAddr(dvmSetNativeFunc, 16, "P6MethodPFvPKjP6JValuePKS_P6ThreadEPKt");
	getAddr(dvmSweepMonitorList, 19, "PP7MonitorPFiPvE");
	getAddr(dvmThreadSelf, 13, "v");
	getAddr(dvmUseJNIBridge, 15, "P6MethodPv");
	getAddr2(dvmCheckCallJNIMethod_general, 29, "PKjP6JValuePK6MethodP6Thread", "_Z21dvmCheckCallJNIMethodPKjP6JValuePK6MethodP6Thread");
	getAddr(dvmDecodeIndirectRef, 20, "P7_JNIEnvP8_jobject");
	dvmDecodeIndirectRef_ = (typeof(dvmDecodeIndirectRef_))MSFindSymbol(reflibdvm, "_Z20dvmDecodeIndirectRefP6ThreadP8_jobject");
	g_ObjKey = MSJavaCreateObjectKey();

}

void computeJniArgInfo(Method* method)
{
	int returnType;
	int jniArgInfo;
	u4 hints;
	method->accessFlags |= ACC_NATIVE;
	const char* sig = dexProtoGetShorty(&method->prototype);
    switch (*(sig++))
    {
    case 'V':
        returnType = DALVIK_JNI_RETURN_VOID;
        break;
    case 'F':
        returnType = DALVIK_JNI_RETURN_FLOAT;
        break;
    case 'D':
        returnType = DALVIK_JNI_RETURN_DOUBLE;
        break;
    case 'J':
        returnType = DALVIK_JNI_RETURN_S8;
        break;
    case 'Z':
    case 'B':
        returnType = DALVIK_JNI_RETURN_S1;
        break;
    case 'C':
        returnType = DALVIK_JNI_RETURN_U2;
        break;
    case 'S':
        returnType = DALVIK_JNI_RETURN_S2;
        break;
    default:
        returnType = DALVIK_JNI_RETURN_S4;
        break;
    }
	
	jniArgInfo = returnType << DALVIK_JNI_RETURN_SHIFT;
	hints = dvmPlatformInvokeHints(&method->prototype);
	if (hints & DALVIK_JNI_NO_ARG_INFO)
	{
		jniArgInfo |= DALVIK_JNI_NO_ARG_INFO;
	}
	else
	{
		jniArgInfo |= hints;
	}
	method->registersSize = method->insSize;
	method->jniArgInfo = jniArgInfo;
	method->outsSize = 0;
}

void HookClass(ClassObject* clazz, MethodCallback callback, void* data)
{
	JValue result;
	Thread* self = dvmThreadSelf();
	ClassObject* java_lang_Thread = dvmFindClass("Ljava/lang/Thread;", 0);

	//利用java.lang.Thread.sleep构造一个void func(long)的Method结构
	Method* java_lang_Thread_sleep= dvmFindDirectMethodByDescriptor(java_lang_Thread, "sleep", "(J)V");
	Method*	classLoaded = (Method*)malloc(sizeof(Method));
	memcpy(classLoaded, java_lang_Thread_sleep, sizeof(Method));
	classLoaded->clazz = clazz;
	classLoaded->name = "classLoaded";
	computeJniArgInfo(classLoaded);
	dvmUseJNIBridge(classLoaded, (void*)callback);
	dvmCallMethod(self, classLoaded, 0, &result, data);
	free(classLoaded);
}

void* GetStoredValue(Object* obj, MSJavaObjectKey key)
{
	Thread* self = dvmThreadSelf();
	dvmLockObject(self, obj);
	if(LW_SHAPE(obj->lock) == LW_SHAPE_FAT)
	{
		Monitor* mon = LW_MONITOR(obj->lock);
		if(obj == mon->obj && mon->next != 0 && mon->next->obj == 0)
		{
			Monitor* cur = mon->next;
			MonitorData* mondata = (MonitorData*)(cur + 1);
			for(int i=0;i < cur->lockCount;i++)
			{
				if(mondata->key == key)
				{
					return mondata->value;
				}
				mondata++;
			}
		}
	}
	dvmUnlockObject(self,obj);
}

bool IsObjectContainKey(const ClassObject* clazz)
{
	if(!clazz->classLoader)
		return false;
	if(LW_SHAPE(clazz->lock) == LW_SHAPE_THIN)
		return false;
	if(g_ObjKey->tag != PRIVTAG || g_ObjKey->deleted)
		return false;
	return GetStoredValue(clazz->classLoader, g_ObjKey) != 0;
}

bool NewdvmLinkClass(ClassObject* clazz)//加载类时调用
{
	bool ret = OlddvmLinkClass(clazz);
	if(ret)
	{
		isLinkClassCalled = true;
		if(clazz->status <= CLASS_VERIFYING && IsObjectContainKey(clazz))
			clazz->status = CLASS_VERIFIED;
		HookMethodList* iter = g_MethodListHeader;
		while(iter)
		{
			if(!strcmp(iter->Descriptor, clazz->descriptor))
				HookClass(clazz, iter->Callback, iter->Data);
			iter = iter->next;
		}
	}
	return ret;
}

void NewdvmSweepMonitorList(Monitor** mon, int (*isUnmarkedObject)(void*))
{
	JNIEnvExt *env;
	Monitor** iter = mon;
	bool flag = false;
	env = dvmGetJNIEnvForThread();
	while(*iter)
	{
		if((*iter)->obj)
		{
			flag = isUnmarkedObject((void*)(*iter)->obj) != 0;
		}
		else if(flag)
		{
			MonitorData* addidata = (MonitorData*)(*iter + 1);
			for(int i=0;i<(*iter)->lockCount;i++)
			{
				if(addidata->clean)
					addidata->clean(addidata->data, (JNIEnv*)env, addidata->value);
				addidata->key->ref--;
				if(addidata->key->ref == 0 && addidata->key->deleted)
					delete addidata->key;
				addidata++;
			}
			pthread_mutex_destroy(&((*iter)->lock));
			free(*iter);
		}
		iter = &(*iter)->next;
	}
	OlddvmSweepMonitorList(mon, isUnmarkedObject);
}

bool NewdvmCheckClassAccess(const ClassObject* accessFrom, const ClassObject* clazz)
{
	if((clazz->accessFlags & ACC_PUBLIC) || dvmInSamePackage(accessFrom, clazz))//OlddvmIsPublicClass(clazz))
		return true;
	else
		return IsObjectContainKey(accessFrom);//只要包含Key则无视java层权限
}

bool NewdvmCheckFieldAccess(const ClassObject* accessFrom, const Field* field)
{
	if(OlddvmCheckFieldAccess(accessFrom, field))
		return true;
	else
		return IsObjectContainKey(accessFrom);//只要包含Key则无视java层权限
}


bool NewdvmCheckMethodAccess(const ClassObject* accessFrom, const Method* method)
{
	if(OlddvmCheckMethodAccess(accessFrom, method))
		return true;
	else
		return IsObjectContainKey(accessFrom);//只要包含Key则无视java层权限
}

JNIEXPORT Object* JNICALL MSDecodeIndirectReference(JNIEnv* env, jobject jobj)
{
	if(dvmDecodeIndirectRef_)
		return dvmDecodeIndirectRef_(((JNIEnvExt*)env)->self, jobj);
	else
		return dvmDecodeIndirectRef(env, jobj);
}

JNIEXPORT void JNICALL MSJavaHookClassLoad(JNIEnv *env, const char *name, void (*callback)(JNIEnv *, jclass, void *), void *data)
{
	char* descriptor = dvmNameToDescriptor(name);
	if(isLinkClassCalled)
	{
		ClassObject* findClass = dvmLookupClass(descriptor, 0, false);
		if(findClass)
			HookClass(findClass, callback, data);
	}
	HookMethodList* node = new HookMethodList;
	node->Descriptor = descriptor;
	node->Callback = callback;
	node->Data = data;
	node->next = g_MethodListHeader;
	g_MethodListHeader = node;
}

JNIEXPORT void JNICALL MSJavaHookBridge(Method *method, DalvikBridgeFunc func, Method** backup)
{
	if(backup)
	{
		Method* old = (Method*)malloc(sizeof(Method));
		memcpy(old, method, sizeof(Method));
		old->accessFlags |= ACC_PRIVATE;
		*backup = old;
	}
	computeJniArgInfo(method);
	dvmSetNativeFunc(method, func, 0);
}

void* JavaMethodCallback(void** stack, unsigned int* data)
{
	typedef void* (*JAVACALLBACK)(JNIEnv*, jobject, jmethodID, void*);
	return ((JAVACALLBACK)data[1])((JNIEnv*)stack[0], (jobject)stack[1], (jmethodID)data[0], &stack[2]);
}

JNIEXPORT void JNICALL MSJavaHookMethod(JNIEnv *env, jclass clazz, jmethodID methodID, void *function, void **result)
{
	Method* method = (Method*)methodID;
	if(MSDebug)
		LOGI("MSJavaHookMethod(%p, %p, %p, %p, %p)", env, clazz, method, function, result);
	if(!env)
	{
		LOGI("MS:Warning: NULL jni argument");
		return;
	}
	if(!clazz)
	{
		LOGI("MS:Warning: NULL class argument");
		return;
	}
	if(!methodID)
	{
		LOGI("MS:Warning: NULL methodID argument");
		return;
	}
	if(!function)
	{
		LOGI("MS:Warning: NULL function argument");
		return;
	}
	if(result)
	{
		Method* newmet = (Method*)malloc(sizeof(Method));
		memcpy(newmet, method, sizeof(Method));
		newmet->accessFlags |= ACC_PRIVATE;
		bool isstatic = (newmet->accessFlags & ACC_STATIC);
		void* funptr = 0;
		switch(*newmet->shorty)
		{
		case 'B':
			if(!isstatic)
				funptr = (void*)env->functions->CallByteMethodV;
			else
				funptr = (void*)env->functions->CallStaticByteMethodV;
			break;
		case 'C':
			if(!isstatic)
				funptr = (void*)env->functions->CallCharMethodV;
			else
				funptr = (void*)env->functions->CallStaticCharMethodV;
			break;
		case 'D':
			if(!isstatic)
				funptr = (void*)env->functions->CallDoubleMethodV;
			else
				funptr = (void*)env->functions->CallStaticDoubleMethodV;
			break;
		case 'F':
			if(!isstatic)
				funptr = (void*)env->functions->CallFloatMethodV;
			else
				funptr = (void*)env->functions->CallStaticFloatMethodV;
			break;
		case 'I':
			if(!isstatic)
				funptr = (void*)env->functions->CallIntMethodV;
			else
				funptr = (void*)env->functions->CallStaticIntMethodV;
			break;
		case 'J':
			if(!isstatic)
				funptr = (void*)env->functions->CallLongMethodV;
			else
				funptr = (void*)env->functions->CallStaticLongMethodV;
			break;
		case 'L':
			if(!isstatic)
				funptr = (void*)env->functions->CallObjectMethodV;
			else
				funptr = (void*)env->functions->CallStaticObjectMethodV;
			break;
		case 'S':
			if(!isstatic)
				funptr = (void*)env->functions->CallShortMethodV;
			else
				funptr = (void*)env->functions->CallStaticShortMethodV;
			break;
		case 'V':
			if(!isstatic)
				funptr = (void*)env->functions->CallVoidMethodV;
			else
				funptr = (void*)env->functions->CallStaticVoidMethodV;
			break;
		case 'Z':
			if(!isstatic)
				funptr = (void*)env->functions->CallBooleanMethodV;
			else
				funptr = (void*)env->functions->CallStaticBooleanMethodV;
			break;
		default:
			LOGI("MS:Error: unknown shorty");
			return;
		}
		if(!funptr)
			return;
		unsigned int* data = new unsigned int[2];
		data[0] = newmet;
		data[1] = funptr;
		*result = MSCloseFunction((void*)JavaMethodCallback, (void*)data);
	}
	computeJniArgInfo(method);
	dvmUseJNIBridge(method, function);
}

JNIEXPORT MSJavaObjectKey JNICALL MSJavaCreateObjectKey()
{
	MSJavaObjectKey key = new MSJavaObjectKey_;
	key->deleted = false;
	key->tag = PRIVTAG;
	key->ref = 0;
	return key;
}

JNIEXPORT void JNICALL MSJavaReleaseObjectKey(MSJavaObjectKey key)
{
	if(!key->deleted)
	{
		key->deleted = true;
		if(key->ref == 0)
			delete key;
	}
}

void* MSJavaGetObjectKey(JNIEnv *env, jobject object, MSJavaObjectKey key)
{
	Object* obj = MSDecodeIndirectReference(env, object);
	if(LW_SHAPE(obj->lock) == LW_SHAPE_FAT)
	{
		if(key->tag == PRIVTAG)
		{
			if(!key->deleted)
				return GetStoredValue(obj, key);
		}
	}
	return 0;
}

void MSJavaSetObjectKey(JNIEnv *env, jobject object, MSJavaObjectKey key, void *value, void (*clean)(void *, JNIEnv *, void *), void *data)
{
	Object* obj = MSDecodeIndirectReference(env, object);
	if(key->tag == PRIVTAG && !key->deleted)
	{
		Thread* self = dvmThreadSelf();
		dvmLockObject(self, obj);
		if(LW_SHAPE(obj->lock) == LW_SHAPE_THIN)
		{
			Object* exc = dvmGetException(self);
			dvmObjectWait(self, obj, -1, -1, false);
			dvmClearException(self);
			if(exc)
				dvmSetException(self, exc);
		}
		if(LW_SHAPE(obj->lock) != LW_SHAPE_THIN)
		{
			Monitor* mon = LW_MONITOR(obj->lock);
			Monitor* node = mon->next;
			if(obj == mon->obj)
			{
				if(mon->next == 0 || mon->next->obj != 0)
				{
					node = (Monitor*)calloc(1, sizeof(Monitor));
					pthread_mutex_init(&node->lock, 0);
					node->next = mon->next;
					mon->next = node;
				}
			}
			int curlock = node->lockCount;
			MonitorData* mondata = (MonitorData*)(node + 1);
			bool find = false;
			for(int i = 0;i < curlock;i++)
			{
				if(key == mondata[i].key)
				{//如果遇到重复key则替换原有数据
					if(mondata[i].clean)
						mondata[i].clean(mondata[i].data, env, mondata[i].value);
					mondata[i].value = value;
					mondata[i].clean = clean;
					mondata[i].data = data;
					find = true;
				}
			}
			if(!find)
			{//如果添加的为全新key则在数组之后添加数据
				node->lockCount = curlock + 1;
				Monitor* newcopy = (Monitor*)realloc(node, sizeof(Monitor) + node->lockCount * sizeof(MonitorData));
				mondata = (MonitorData*)(newcopy + 1);
				key->ref++;
				mondata[curlock].key = key;
				mondata[curlock].value = value;
				mondata[curlock].clean = clean;
				mondata[curlock].data = data;
				mon->next = newcopy;
			}
		}

		dvmUnlockObject(self, obj);
	}
}

void MSJavaBlessClassLoader(JNIEnv *env, jobject loader)
{
	MSJavaSetObjectKey(env, loader, g_ObjKey, (void*)1, 0, 0);
}

