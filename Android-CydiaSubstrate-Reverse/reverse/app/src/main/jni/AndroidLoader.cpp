/*
 * AndroidLoader.cpp
 *
 *  Created on: 2016年5月8日
 *      Author: Administrator
 */

#include "Common.h"

/*
 * 注：以下代码均未考虑缓冲区溢出
 */

static bool MSLoaded = false;
static char selfpath[1024]="";

#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO, "CydiaSubstrate", __VA_ARGS__)
#define CydiaSecName ".substrate"
#define LibFilterType "Filter:Library"
#define ExeFilterType "Filter:Executable"

bool UnknownCheck()
{
	char pathname[20];
	for(int i=0;i<16;i++)
	{
		sprintf(pathname,"/dev/input/event%u",i);
		int fdevent;
		while((fdevent = open(pathname,O_RDONLY)) == -1)
		{
			if(errno != EINTR)//无法读取
				return false;
		}
		unsigned char buf[64];
		memset(buf,0,sizeof(buf));

		int ret;
		while((ret = ioctl(fdevent, 0x80404518)) == -1 && errno == EINTR);
		while(close(fdevent) == -1 && errno == EINTR);
		if(ret != -1 && (buf[14] & 8) != 0)
		{
			return true;
		}
	}
	return false;
}

void DoInject(char* libpath)
{//从substrate节获取信息进行匹配决定是否注入该进程
	int dirnamelen = strlen(libpath);
	DIR* libdir = opendir(libpath);
	if(!libdir)
		return;
	dirent* pdirect;
	while(pdirect = readdir(libdir))
	{
		int libnamelen = strlen(pdirect->d_name);
		if(libnamelen > 8 && !strncmp(pdirect->d_name, "lib" ,3) &&
			!strcmp(pdirect->d_name + libnamelen - 6, ".cy.so"))
		{
			char fullpath[256];
			sprintf(fullpath, "%s/%s", libpath, pdirect->d_name);
			int libfd;
			dosafenolog(libfd, open(fullpath, O_RDONLY));
			if(libfd != -1)
			{
				int status;
				struct stat fst;
				dosafenolog(status, fstat(libfd, &fst));
				if(status != -1)
				{
					void* base;
					dosafenolog(base, mmap(0, fst.st_size, PROT_READ, MAP_PRIVATE, libfd, 0));
					dosafenolog(status, close(libfd));
					if(base != MAP_FAILED)
					{
						Elf32_Ehdr *ehdr = (Elf32_Ehdr *)base;
						Elf32_Shdr *shdr_table = (Elf32_Shdr *)((char*)base + ehdr->e_shoff);
						Elf32_Shdr *shdr_stringtable = (shdr_table + ehdr->e_shstrndx);
						char* stringtable = (char*)base + shdr_stringtable->sh_offset;
						bool find = false;
						for(int i=1;i<ehdr->e_shnum;i++)
						{
							if(!strcmp(stringtable + shdr_table[i].sh_name, ".substrate"))
							{
								//.substrate节内容及长度
								char* info = (char*)base + shdr_table[i].sh_offset;
								int infolen = shdr_table[i].sh_size;
								if(infolen)
								{

									while(true)
									{
										int len = strnlen(info, infolen);
										if(infolen == len)
										{
											LOGI("MS:Error: corrupted MSConfig(...)");
											break;
										}
										if(len)
										{
											char* p = (char*)memchr((void*)info, '=', len-1);
											if(!p)
											{
												LOGI("MS:Error:  invalid MSConfig(...)");
												break;
											}
											if(p - info == 17)
											{
												if(!strncmp(info, "Filter:Executable", 17))
												{
													if(!strcmp(p+1, selfpath))
														find = true;
												}
												else
												{
													LOGI("MS:Error:  unknown MSConfig(...)");
													break;
												}
											}
											else if(p - info == 14)
											{
												if(!strncmp(info, "Filter:Library", 14))
												{
													if(MSGetImageByName(p+1))
														find = true;
												}
												else
												{
													LOGI("MS:Error:  unknown MSConfig(...)");
													break;
												}
											}
										}
									}
								}
								break;
							}
						}
						dosafenolog(status, munmap(base, fst.st_size));
						if(find)
						{
							LOGI("MS:Notice: Loading: %s", fullpath);
							if(!dlopen(fullpath, RTLD_LAZY | RTLD_GLOBAL))
							{
								LOGI("MS:Error: %s", dlerror());
							}
						}
					}
				}
				close(libfd);
			}
		}
	}
	closedir(libdir);
}

bool getline(char* dst, FILE* fp)
{
	char* ret = fgets(dst, 1024, fp);
	if(!ret)
		return false;
	int len = strlen(dst);
	if(len == 0)
		return false;
	if(dst[len-1] == '\n')
	{
		dst[len-1] = '\0';
		return true;
	}
	return false;
}

JNIEXPORT void  JNICALL MSLoadExtensions()
{
	if(MSLoaded)
		return;
	if(UnknownCheck())//未知用途
		return;
	int readlen;
	while((readlen = readlink("/proc/self/exe", selfpath, 1023)) == -1)
	{
		if(errno != EINTR)//无法读取
			return;
	}
	selfpath[readlen] = '\0';
	if(!strcmp(selfpath,"/system/bin/ks") || !strcmp(selfpath,"/system/bin/mcDriverDaemon"))
		return;//耗时操作的程序不检测
	LOGI("MS:Notice: Injecting: %s", selfpath);
	FILE* fp = fopen("/data/data/com.saurik.substrate/permitted.list", "r");
	if(fp)
	{
		char buf[1024];
		char libpath[1024];
		char* inner_ptr = NULL;
		if(getline(buf, fp) && !strcmp(buf,"0"))
		{
			while(getline(buf, fp))
			{
				char* packagename = strtok_r(buf, " ", &inner_ptr);
				sprintf(libpath, "/data/data/%s/lib", packagename);
				DoInject(libpath);
			}
		}
		fclose(fp);
	}
}



