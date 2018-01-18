/*
 * Unix.cpp
 *
 *  Created on: 2016Äê5ÔÂ7ÈÕ
 *      Author: Administrator
 */
#include "Common.h"


#define SYS_utimensat 0

void domount(const char* target,bool readonly)
{
	int mountflags = MS_REMOUNT;
	if(readonly)
		mountflags |= MS_RDONLY;
	int mountstatu = -1;
	dosafe(mountstatu, mount("",target,"none",mountflags,0))
	if(mountstatu == -1)
		return;
}

void replaceLinkerEnv(const char* linkerpath,const char* oldenv,const char* newenv)
{
	if(strlen(oldenv) != strlen(newenv))
	{
		showerror(EINVAL);
		return;
	}
	int linkerfd = -1;
	dosafe(linkerfd, open(linkerpath, O_RDONLY));
	if(linkerfd == -1)
		return;
	int status;
	struct stat st;
	dosafe(status, fstat(linkerfd, &st));
	if(status != -1)
	{
		void* base = MAP_FAILED;
		dosafe(base, mmap(0, st.st_size,PROT_READ | PROT_WRITE, MAP_PRIVATE, linkerfd, 0));
		if(base != MAP_FAILED)
		{
			void* searchpt;
			void* curp = base;
			bool finded = false;
			while(searchpt = memmem(curp, st.st_size, oldenv, strlen(oldenv)))//±¬ËÑÌæ»»
			{//first modify memory
				finded = true;
				memcpy(searchpt, newenv, strlen(oldenv));
				curp =(void*)((char*)curp + strlen(oldenv));
			}
			if(finded)
			{//second modify file
				char rplpath[256];
				sprintf(rplpath, "%s.rpl", linkerpath);
				int linkerrplfd = -1;
				dosafe(linkerrplfd, open(rplpath, O_CREAT | O_WRONLY | O_TRUNC, S_IWUSR));
				if(linkerrplfd != -1)
				{
					int writenum;
					dosafe(writenum, write(linkerrplfd, base, st.st_size));
					struct timespec time[2] =
					{
						(time_t)st.st_atime,
						(long)st.st_atime_nsec,
						(time_t)st.st_mtime,
						(long)st.st_mtime_nsec
					};
					dosafe(status, syscall(SYS_utimensat, linkerrplfd, 0, time, 0));
					dosafe(status, fchmod(linkerrplfd, (mode_t)st.st_mode));
					dosafe(status, fchown(linkerrplfd, st.st_uid, st.st_gid));
					close(linkerrplfd);
				}
				dosafe(status, rename(rplpath, linkerpath));
				unlink(rplpath);
			}
			munmap(base, st.st_size);
		}
	}
	close(linkerfd);
}
