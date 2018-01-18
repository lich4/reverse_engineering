#include "Common.h"
#include <unistd.h>

#define HIDDEN __attribute__ ((visibility ("hidden")))
#define NAKED __attribute__ ((naked))

void InitLib(void);
void Init_Cydia(void);//用于替换宿主程序的init_array函数

typedef void (*INITFUNC)(void);

__attribute__ ((section (".init_array")))
INITFUNC init_func=&InitLib;//放入单个函数

void* dlliblogX = 0;//存储系统原始liblog.so模块信息
void* dllibcutils = 0;//存储系统libcutils.so
INITFUNC origin_init = 0;//存储可执行程序原始init_func

void* p__android_log_assert=0;
void* p__android_log_btwrite=0;
void* p__android_log_buf_print=0;
void* p__android_log_buf_write=0;
void* p__android_log_bwrite=0;
void* p__android_log_dev_available=0;
void* p__android_log_print=0;
void* p__android_log_vprint=0;
void* p__android_log_write=0;
void* pandroid_closeEventTagMap=0;
void* pandroid_log_addFilterRule=0;
void* pandroid_log_addFilterString=0;
void* pandroid_log_formatFromString=0;
void* pandroid_log_formatLogLine=0;
void* pandroid_log_format_free=0;
void* pandroid_log_format_new=0;
void* pandroid_log_printLogLine=0;
void* pandroid_log_processBinaryLogBuffer=0;
void* pandroid_log_processLogBuffer=0;
void* pandroid_log_setPrintFormat=0;
void* pandroid_log_shouldPrintLine=0;
void* pandroid_lookupEventTag=0;
void* pandroid_openEventTagMap=0;
void* plogprint_run_tests=0;
void* p__xlog_buf_printf=0;
void* pale_log_output=0;
void* pale_log_output_binary=0;
void* pxlogf_get_level=0;
void* pxlogf_java_tag_is_on=0;
void* pxlogf_java_xtag_is_on=0;
void* pxlogf_native_tag_is_on=0;
void* pxlogf_native_xtag_is_on=0;
void* pxlogf_set_level=0;
void* pxlogf_tag_set_level=0;
void* pxLog_get_mask=0;
void* pxLog_get_module_ptr=0;
void* pxLog_isOn=0;
void* pxLog_modify_all=0;
void* pxLog_set=0;
void* pxLog_set_mask=0;
void* pandroid_log_setColoredOutput=0;
void* p__htclog_init_mask=0;
void* p__htclog_print_private=0;
void* p__htclog_read_masks=0;
void* pLog_StringToMTT=0;
void* p__android_dump_log=0;
void* pcheck_ulogging_enabled=0;
void* p__android_log_buf_vprint=0;
void* p__android_log_switchable_buf_print=0;
void* p__android_log_switchable_print=0;
void* p__write_to_log_kernel_external=0;
void* pandroid_dbgcfg_init=0;
void* pfilterPriToChar=0;
void* pproperty_get=0;
void* pproperty_list=0;
void* pproperty_set=0;

//修改内存块具有写权限
void ModForRW(void* base, int length)
{
	unsigned int pagesize = getpagesize();
	unsigned int beginaddr,endaddr;
	beginaddr = pagesize * ((unsigned int)base / pagesize);
	endaddr = pagesize + pagesize * (((unsigned int)base + length) / pagesize);
	mprotect((void*)beginaddr, endaddr - beginaddr, PROT_READ | PROT_WRITE);
}

void Init_Cydia()
{
	void* dlAndroidLoader = dlopen("/data/data/com.saurik.substrate/lib/libAndroidLoader.so",RTLD_LAZY | RTLD_GLOBAL);
	if(!dlAndroidLoader)
	{
		Log::log("MS:Error: dlsym(): %s\n", dlerror());
	}
	else
	{
		INITFUNC MSLoadExtensions = (INITFUNC)dlsym(dlAndroidLoader, "MSLoadExtensions");
		if(MSLoadExtensions)
		{
			MSLoadExtensions();
		}
		else
		{
			Log::log("MS:Error: dlsym(): %s\n", dlerror());
		}
	}
	if(origin_init)
	{
		origin_init();
	}
}

void FakeRunnerInit()//替换可执行程序模块的init_array函数
{
	void* dllibdl = dlopen("libdl.so", RTLD_LAZY | RTLD_GLOBAL);
	Dl_info liblog_dlinfo;
	soinfo* base;
	if(!dllibdl)
		return;
	if(!dladdr((void*)Init_Cydia, &liblog_dlinfo))
		return;
	//获取伪liblog.so基址(libAndroidBootstrap0.so)
	void* dlliblog = dlopen(liblog_dlinfo.dli_fname, RTLD_LAZY | RTLD_GLOBAL);
	if(dlliblog)
	{
		char filename[256];
		base = (soinfo*)dlliblog;
		int prefixlen = strlen((const char*)base->name) - 3;//.so
		memcpy(filename, base->name, prefixlen);
		memcpy(filename + prefixlen, "!.so", 5);
		dlliblogX = dlopen(filename, RTLD_LAZY | RTLD_GLOBAL);
		if(dlliblogX)
		{//修改init函数
			dllibcutils = dlopen("libcutils.so", RTLD_LAZY | RTLD_GLOBAL);
			const int mod_size = sizeof(soinfo);//init_func的下一个元素

			//修改模块名liblog!.so为liblog.so
			ModForRW(dlliblogX, mod_size);
			strcpy((char*)((soinfo*)dlliblogX)->name, (char*)((soinfo*)dlliblog)->name);

			//修改模块名liblog.so为libAndroidBootstrap0.so
			ModForRW(dlliblog, mod_size);
			strcpy((char*)((soinfo*)dlliblogX)->name, "libAndroidBootstrap0.so");

			//修改可执行程序init_func
			soinfo* elfhost = ((soinfo*)dllibdl)->next;
			origin_init = elfhost->init_func;
			ModForRW((void*)elfhost->init_func, sizeof(void*));//源码中有误
			elfhost->init_func = Init_Cydia;
		}
	}
}

void InitLib(void)
{
	FakeRunnerInit();

	if(!p__android_log_assert && dlliblogX)
		p__android_log_assert = dlsym(dlliblogX, "__android_log_assert");
	if(!p__android_log_assert && dllibcutils)
		p__android_log_assert = dlsym(dllibcutils, "__android_log_assert");

	if(!p__android_log_btwrite && dlliblogX)
		p__android_log_btwrite = dlsym(dlliblogX, "__android_log_btwrite");
	if(!p__android_log_btwrite && dllibcutils)
		p__android_log_btwrite = dlsym(dllibcutils, "__android_log_btwrite");

	if(!p__android_log_buf_print && dlliblogX)
		p__android_log_buf_print = dlsym(dlliblogX, "__android_log_buf_print");
	if(!p__android_log_buf_print && dllibcutils)
		p__android_log_buf_print = dlsym(dllibcutils, "__android_log_buf_print");

	if(!p__android_log_buf_write && dlliblogX)
		p__android_log_buf_write = dlsym(dlliblogX, "__android_log_buf_write");
	if(!p__android_log_buf_write && dllibcutils)
		p__android_log_buf_write = dlsym(dllibcutils, "__android_log_buf_write");

	if(!p__android_log_bwrite && dlliblogX)
		p__android_log_bwrite = dlsym(dlliblogX, "__android_log_bwrite");
	if(!p__android_log_bwrite && dllibcutils)
		p__android_log_bwrite = dlsym(dllibcutils, "__android_log_bwrite");

	if(!p__android_log_dev_available && dlliblogX)
		p__android_log_dev_available = dlsym(dlliblogX, "__android_log_dev_available");
	if(!p__android_log_dev_available && dllibcutils)
		p__android_log_dev_available = dlsym(dllibcutils, "__android_log_dev_available");

	if(!p__android_log_print && dlliblogX)
		p__android_log_print = dlsym(dlliblogX, "__android_log_print");
	if(!p__android_log_print && dllibcutils)
		p__android_log_print = dlsym(dllibcutils, "__android_log_print");

	if(!p__android_log_vprint && dlliblogX)
		p__android_log_vprint = dlsym(dlliblogX, "__android_log_vprint");
	if(!p__android_log_vprint && dllibcutils)
		p__android_log_vprint = dlsym(dllibcutils, "__android_log_vprint");

	if(!p__android_log_write && dlliblogX)
		p__android_log_write = dlsym(dlliblogX, "__android_log_write");
	if(!p__android_log_write && dllibcutils)
		p__android_log_write = dlsym(dllibcutils, "__android_log_write");

	if(!pandroid_closeEventTagMap && dlliblogX)
		pandroid_closeEventTagMap = dlsym(dlliblogX, "android_closeEventTagMap");
	if(!pandroid_closeEventTagMap && dllibcutils)
		pandroid_closeEventTagMap = dlsym(dllibcutils, "android_closeEventTagMap");

	if(!pandroid_log_addFilterRule && dlliblogX)
		pandroid_log_addFilterRule = dlsym(dlliblogX, "android_log_addFilterRule");
	if(!pandroid_log_addFilterRule && dllibcutils)
		pandroid_log_addFilterRule = dlsym(dllibcutils, "android_log_addFilterRule");

	if(!pandroid_log_addFilterString && dlliblogX)
		pandroid_log_addFilterString = dlsym(dlliblogX, "android_log_addFilterString");
	if(!pandroid_log_addFilterString && dllibcutils)
		pandroid_log_addFilterString = dlsym(dllibcutils, "android_log_addFilterString");

	if(!pandroid_log_formatFromString && dlliblogX)
		pandroid_log_formatFromString = dlsym(dlliblogX, "android_log_formatFromString");
	if(!pandroid_log_formatFromString && dllibcutils)
		pandroid_log_formatFromString = dlsym(dllibcutils, "android_log_formatFromString");

	if(!pandroid_log_formatLogLine && dlliblogX)
		pandroid_log_formatLogLine = dlsym(dlliblogX, "android_log_formatLogLine");
	if(!pandroid_log_formatLogLine && dllibcutils)
		pandroid_log_formatLogLine = dlsym(dllibcutils, "android_log_formatLogLine");

	if(!pandroid_log_format_free && dlliblogX)
		pandroid_log_format_free = dlsym(dlliblogX, "android_log_format_free");
	if(!pandroid_log_format_free && dllibcutils)
		pandroid_log_format_free = dlsym(dllibcutils, "android_log_format_free");

	if(!pandroid_log_format_new && dlliblogX)
		pandroid_log_format_new = dlsym(dlliblogX, "android_log_format_new");
	if(!pandroid_log_format_new && dllibcutils)
		pandroid_log_format_new = dlsym(dllibcutils, "android_log_format_new");

	if(!pandroid_log_printLogLine && dlliblogX)
		pandroid_log_printLogLine = dlsym(dlliblogX, "android_log_printLogLine");
	if(!pandroid_log_printLogLine && dllibcutils)
		pandroid_log_printLogLine = dlsym(dllibcutils, "android_log_printLogLine");

	if(!pandroid_log_processBinaryLogBuffer && dlliblogX)
		pandroid_log_processBinaryLogBuffer = dlsym(dlliblogX, "android_log_processBinaryLogBuffer");
	if(!pandroid_log_processBinaryLogBuffer && dllibcutils)
		pandroid_log_processBinaryLogBuffer = dlsym(dllibcutils, "android_log_processBinaryLogBuffer");

	if(!pandroid_log_processLogBuffer && dlliblogX)
		pandroid_log_processLogBuffer = dlsym(dlliblogX, "android_log_processLogBuffer");
	if(!pandroid_log_processLogBuffer && dllibcutils)
		pandroid_log_processLogBuffer = dlsym(dllibcutils, "android_log_processLogBuffer");

	if(!pandroid_log_setPrintFormat && dlliblogX)
		pandroid_log_setPrintFormat = dlsym(dlliblogX, "android_log_setPrintFormat");
	if(!pandroid_log_setPrintFormat && dllibcutils)
		pandroid_log_setPrintFormat = dlsym(dllibcutils, "android_log_setPrintFormat");

	if(!pandroid_log_shouldPrintLine && dlliblogX)
		pandroid_log_shouldPrintLine = dlsym(dlliblogX, "android_log_shouldPrintLine");
	if(!pandroid_log_shouldPrintLine && dllibcutils)
		pandroid_log_shouldPrintLine = dlsym(dllibcutils, "android_log_shouldPrintLine");

	if(!pandroid_lookupEventTag && dlliblogX)
		pandroid_lookupEventTag = dlsym(dlliblogX, "android_lookupEventTag");
	if(!pandroid_lookupEventTag && dllibcutils)
		pandroid_lookupEventTag = dlsym(dllibcutils, "android_lookupEventTag");

	if(!pandroid_openEventTagMap && dlliblogX)
		pandroid_openEventTagMap = dlsym(dlliblogX, "android_openEventTagMap");
	if(!pandroid_openEventTagMap && dllibcutils)
		pandroid_openEventTagMap = dlsym(dllibcutils, "android_openEventTagMap");

	if(!plogprint_run_tests && dlliblogX)
		plogprint_run_tests = dlsym(dlliblogX, "logprint_run_tests");
	if(!plogprint_run_tests && dllibcutils)
		plogprint_run_tests = dlsym(dllibcutils, "logprint_run_tests");

	if(!p__xlog_buf_printf && dlliblogX)
		p__xlog_buf_printf = dlsym(dlliblogX, "__xlog_buf_printf");
	if(!p__xlog_buf_printf && dllibcutils)
		p__xlog_buf_printf = dlsym(dllibcutils, "__xlog_buf_printf");

	if(!pale_log_output && dlliblogX)
		pale_log_output = dlsym(dlliblogX, "ale_log_output");
	if(!pale_log_output && dllibcutils)
		pale_log_output = dlsym(dllibcutils, "ale_log_output");

	if(!pale_log_output_binary && dlliblogX)
		pale_log_output_binary = dlsym(dlliblogX, "ale_log_output_binary");
	if(!pale_log_output_binary && dllibcutils)
		pale_log_output_binary = dlsym(dllibcutils, "ale_log_output_binary");

	if(!pxlogf_get_level && dlliblogX)
		pxlogf_get_level = dlsym(dlliblogX, "xlogf_get_level");
	if(!pxlogf_get_level && dllibcutils)
		pxlogf_get_level = dlsym(dllibcutils, "xlogf_get_level");

	if(!pxlogf_java_tag_is_on && dlliblogX)
		pxlogf_java_tag_is_on = dlsym(dlliblogX, "xlogf_java_tag_is_on");
	if(!pxlogf_java_tag_is_on && dllibcutils)
		pxlogf_java_tag_is_on = dlsym(dllibcutils, "xlogf_java_tag_is_on");

	if(!pxlogf_java_xtag_is_on && dlliblogX)
		pxlogf_java_xtag_is_on = dlsym(dlliblogX, "xlogf_java_xtag_is_on");
	if(!pxlogf_java_xtag_is_on && dllibcutils)
		pxlogf_java_xtag_is_on = dlsym(dllibcutils, "xlogf_java_xtag_is_on");

	if(!pxlogf_native_tag_is_on && dlliblogX)
		pxlogf_native_tag_is_on = dlsym(dlliblogX, "xlogf_native_tag_is_on");
	if(!pxlogf_native_tag_is_on && dllibcutils)
		pxlogf_native_tag_is_on = dlsym(dllibcutils, "xlogf_native_tag_is_on");

	if(!pxlogf_native_xtag_is_on && dlliblogX)
		pxlogf_native_xtag_is_on = dlsym(dlliblogX, "xlogf_native_xtag_is_on");
	if(!pxlogf_native_xtag_is_on && dllibcutils)
		pxlogf_native_xtag_is_on = dlsym(dllibcutils, "xlogf_native_xtag_is_on");

	if(!pxlogf_set_level && dlliblogX)
		pxlogf_set_level = dlsym(dlliblogX, "xlogf_set_level");
	if(!pxlogf_set_level && dllibcutils)
		pxlogf_set_level = dlsym(dllibcutils, "xlogf_set_level");

	if(!pxlogf_tag_set_level && dlliblogX)
		pxlogf_tag_set_level = dlsym(dlliblogX, "xlogf_tag_set_level");
	if(!pxlogf_tag_set_level && dllibcutils)
		pxlogf_tag_set_level = dlsym(dllibcutils, "xlogf_tag_set_level");

	if(!pxLog_get_mask && dlliblogX)
		pxLog_get_mask = dlsym(dlliblogX, "xLog_get_mask");
	if(!pxLog_get_mask && dllibcutils)
		pxLog_get_mask = dlsym(dllibcutils, "xLog_get_mask");

	if(!pxLog_get_module_ptr && dlliblogX)
		pxLog_get_module_ptr = dlsym(dlliblogX, "xLog_get_module_ptr");
	if(!pxLog_get_module_ptr && dllibcutils)
		pxLog_get_module_ptr = dlsym(dllibcutils, "xLog_get_module_ptr");

	if(!pxLog_isOn && dlliblogX)
		pxLog_isOn = dlsym(dlliblogX, "xLog_isOn");
	if(!pxLog_isOn && dllibcutils)
		pxLog_isOn = dlsym(dllibcutils, "xLog_isOn");

	if(!pxLog_modify_all && dlliblogX)
		pxLog_modify_all = dlsym(dlliblogX, "xLog_modify_all");
	if(!pxLog_modify_all && dllibcutils)
		pxLog_modify_all = dlsym(dllibcutils, "xLog_modify_all");

	if(!pxLog_set && dlliblogX)
		pxLog_set = dlsym(dlliblogX, "xLog_set");
	if(!pxLog_set && dllibcutils)
		pxLog_set = dlsym(dllibcutils, "xLog_set");

	if(!pxLog_set_mask && dlliblogX)
		pxLog_set_mask = dlsym(dlliblogX, "xLog_set_mask");
	if(!pxLog_set_mask && dllibcutils)
		pxLog_set_mask = dlsym(dllibcutils, "xLog_set_mask");

	if(!pandroid_log_setColoredOutput && dlliblogX)
		pandroid_log_setColoredOutput = dlsym(dlliblogX, "android_log_setColoredOutput");
	if(!pandroid_log_setColoredOutput && dllibcutils)
		pandroid_log_setColoredOutput = dlsym(dllibcutils, "android_log_setColoredOutput");

	if(!p__htclog_init_mask && dlliblogX)
		p__htclog_init_mask = dlsym(dlliblogX, "__htclog_init_mask");
	if(!p__htclog_init_mask && dllibcutils)
		p__htclog_init_mask = dlsym(dllibcutils, "__htclog_init_mask");

	if(!p__htclog_print_private && dlliblogX)
		p__htclog_print_private = dlsym(dlliblogX, "__htclog_print_private");
	if(!p__htclog_print_private && dllibcutils)
		p__htclog_print_private = dlsym(dllibcutils, "__htclog_print_private");

	if(!p__htclog_read_masks && dlliblogX)
		p__htclog_read_masks = dlsym(dlliblogX, "__htclog_read_masks");
	if(!p__htclog_read_masks && dllibcutils)
		p__htclog_read_masks = dlsym(dllibcutils, "__htclog_read_masks");

	if(!pLog_StringToMTT && dlliblogX)
		pLog_StringToMTT = dlsym(dlliblogX, "Log_StringToMTT");
	if(!pLog_StringToMTT && dllibcutils)
		pLog_StringToMTT = dlsym(dllibcutils, "Log_StringToMTT");

	if(!p__android_dump_log && dlliblogX)
		p__android_dump_log = dlsym(dlliblogX, "__android_dump_log");
	if(!p__android_dump_log && dllibcutils)
		p__android_dump_log = dlsym(dllibcutils, "__android_dump_log");

	if(!pcheck_ulogging_enabled && dlliblogX)
		pcheck_ulogging_enabled = dlsym(dlliblogX, "check_ulogging_enabled");
	if(!pcheck_ulogging_enabled && dllibcutils)
		pcheck_ulogging_enabled = dlsym(dllibcutils, "check_ulogging_enabled");

	if(!p__android_log_buf_vprint && dlliblogX)
		p__android_log_buf_vprint = dlsym(dlliblogX, "__android_log_buf_vprint");
	if(!p__android_log_buf_vprint && dllibcutils)
		p__android_log_buf_vprint = dlsym(dllibcutils, "__android_log_buf_vprint");

	if(!p__android_log_switchable_buf_print && dlliblogX)
		p__android_log_switchable_buf_print = dlsym(dlliblogX, "__android_log_switchable_buf_print");
	if(!p__android_log_switchable_buf_print && dllibcutils)
		p__android_log_switchable_buf_print = dlsym(dllibcutils, "__android_log_switchable_buf_print");

	if(!p__android_log_switchable_print && dlliblogX)
		p__android_log_switchable_print = dlsym(dlliblogX, "__android_log_switchable_print");
	if(!p__android_log_switchable_print && dllibcutils)
		p__android_log_switchable_print = dlsym(dllibcutils, "__android_log_switchable_print");

	if(!p__write_to_log_kernel_external && dlliblogX)
		p__write_to_log_kernel_external = dlsym(dlliblogX, "__write_to_log_kernel_external");
	if(!p__write_to_log_kernel_external && dllibcutils)
		p__write_to_log_kernel_external = dlsym(dllibcutils, "__write_to_log_kernel_external");

	if(!pandroid_dbgcfg_init && dlliblogX)
		pandroid_dbgcfg_init = dlsym(dlliblogX, "android_dbgcfg_init");
	if(!pandroid_dbgcfg_init && dllibcutils)
		pandroid_dbgcfg_init = dlsym(dllibcutils, "android_dbgcfg_init");

	if(!pfilterPriToChar && dlliblogX)
		pfilterPriToChar = dlsym(dlliblogX, "filterPriToChar");
	if(!pfilterPriToChar && dllibcutils)
		pfilterPriToChar = dlsym(dllibcutils, "filterPriToChar");

	if(!pproperty_get && dlliblogX)
		pproperty_get = dlsym(dlliblogX, "property_get");
	if(!pproperty_get && dllibcutils)
		pproperty_get = dlsym(dllibcutils, "property_get");

	if(!pproperty_list && dlliblogX)
		pproperty_list = dlsym(dlliblogX, "property_list");
	if(!pproperty_list && dllibcutils)
		pproperty_list = dlsym(dllibcutils, "property_list");

	if(!pproperty_set && dlliblogX)
		pproperty_set = dlsym(dlliblogX, "property_set");
	if(!pproperty_set && dllibcutils)
		pproperty_set = dlsym(dllibcutils, "property_set");
}

#define EXPORT(x) \
	JNIEXPORT NAKED void JNICALL x()\
	{\
		__asm__("STMFD	SP!, {R0-R3}\n");\
		__asm__("MOV	R12,%0\n"::"r" (p##x));\
		__asm__("LDMFD	SP!, {R0-R3}\n");\
		__asm__(\
				"CMP	R12, #0\n"\
				"BXNE	R12\n"\
				"BX		LR\n"\
				);\
	}

EXPORT(__android_log_assert)
EXPORT(__android_log_btwrite)
EXPORT(__android_log_buf_print)
EXPORT(__android_log_buf_write)
EXPORT(__android_log_bwrite)
EXPORT(__android_log_dev_available)
EXPORT(__android_log_print)
EXPORT(__android_log_vprint)
EXPORT(__android_log_write)
EXPORT(android_closeEventTagMap)
EXPORT(android_log_addFilterRule)
EXPORT(android_log_addFilterString)
EXPORT(android_log_formatFromString)
EXPORT(android_log_formatLogLine)
EXPORT(android_log_format_free)
EXPORT(android_log_format_new)
EXPORT(android_log_printLogLine)
EXPORT(android_log_processBinaryLogBuffer)
EXPORT(android_log_processLogBuffer)
EXPORT(android_log_setPrintFormat)
EXPORT(android_log_shouldPrintLine)
EXPORT(android_lookupEventTag)
EXPORT(android_openEventTagMap)
EXPORT(logprint_run_tests)
EXPORT(__xlog_buf_printf)
EXPORT(ale_log_output)
EXPORT(ale_log_output_binary)
EXPORT(xlogf_get_level)
EXPORT(xlogf_java_tag_is_on)
EXPORT(xlogf_java_xtag_is_on)
EXPORT(xlogf_native_tag_is_on)
EXPORT(xlogf_native_xtag_is_on)
EXPORT(xlogf_set_level)
EXPORT(xlogf_tag_set_level)
EXPORT(xLog_get_mask)
EXPORT(xLog_get_module_ptr)
EXPORT(xLog_isOn)
EXPORT(xLog_modify_all)
EXPORT(xLog_set)
EXPORT(xLog_set_mask)
EXPORT(android_log_setColoredOutput)
EXPORT(__htclog_init_mask)
EXPORT(__htclog_print_private)
EXPORT(__htclog_read_masks)
EXPORT(Log_StringToMTT)
EXPORT(__android_dump_log)
EXPORT(check_ulogging_enabled)
EXPORT(__android_log_buf_vprint)
EXPORT(__android_log_switchable_buf_print)
EXPORT(__android_log_switchable_print)
EXPORT(__write_to_log_kernel_external)
EXPORT(android_dbgcfg_init)
EXPORT(filterPriToChar)
EXPORT(property_get)
EXPORT(property_list)
EXPORT(property_set)



