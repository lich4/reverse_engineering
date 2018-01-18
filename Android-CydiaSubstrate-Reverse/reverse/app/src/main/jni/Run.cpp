/*
 * 以下代码均未考虑缓冲区溢出
 */

#include "Common.h"
#include <vector>
using namespace std;

#define arraysize(x) (sizeof(x)/sizeof(x[0]))
#define AID_ROOT             0  /* traditional unix root user */
#define AID_SYSTEM        1000  /* system server */
#define AID_SHELL         2000  /* adb and debug shell user */

void split(const string &s, char delim, vector<string> &elems) {
    stringstream ss(s);
    string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
}

int doremount(const char* path, bool readonly)
{
	int mountflags = MS_REMOUNT;
	if(readonly)
		mountflags |= MS_RDONLY;
	int status;
	dosafe(status, mount("", path, "none", mountflags, 0));
	return status;
}

int domakelink(const char* src, const char* dst)
{
	int status;
	dosafe(status, unlink(dst));
	dosafe(status, symlink(src, dst));
	return status;
}

int dochown(const char* dir)
{
	int ret = -1;
	char map_path[PATH_MAX];
	if(realpath(dir, map_path))
	{
		dosafe(ret, mkdir(map_path, 0755));
		if(ret != -1)
		{
			dosafe(ret, chown(map_path, AID_ROOT, AID_SHELL));
		}
		else
		{
			struct stat fst;
			dosafe(ret, stat(map_path, &fst));
			if(ret != -1)
				dosafe(ret, chmod(map_path, fst.st_mode | S_IXOTH));
		}
	}
	return ret;
}

void Run_do_link(const char** argv)
{
	bool linkresult = false;
	ifstream fmount("/proc/self/mounts");
	string mountpos[] = {"/", "/system", "/system/vendor"};
	map<string,string> mountinfo;//key=path value=prio
	vector<string> lineelems, prioelems;
	string line;
	if(fmount.is_open())
	{
		while(!fmount.eof())
		{
			lineelems.clear();
			prioelems.clear();
			getline(fmount, line);
			split(line, ' ', lineelems);
			if(lineelems.size() >= 4)
			{//format:rootfs / rootfs ro,relatime 0 0
				split(lineelems[3], ',', prioelems);
				if(prioelems.size() > 0 && (prioelems[0] == "rw" || prioelems[0] == "ro"))
					mountinfo[lineelems[1]] = prioelems[0];
			}
		}
	}
	fmount.close();
	for(int i=arraysize(mountpos)-1;i>=0;i--)
	{
		map<string,string>::iterator itor = mountinfo.find(mountpos[i]);
		if(itor != mountinfo.end())
		{
			int status = 0;
			bool isremount = false;
			if(status != -1)
			{
				if((*itor).second == "ro")
				{
					status = doremount(mountpos[i].c_str(), false);
					isremount = true;
				}
			}
			if(status != -1)
			{
				int ret1,ret2,ret3,ret4,ret5,ret6;
				string packagelibpath = "/data/data/com.saurik.substrate/lib/";
				if(status != -1)
					status = domakelink((packagelibpath + "libsubstrate.so").c_str(), "/system/lib/libsubstrate.so");
				if(status != -1)
					status = domakelink((packagelibpath + "libsubstrate-dvm.so").c_str(), "/system/lib/libsubstrate-dvm.so");
				dochown("/vendor");
				dochown("/vendor/lib");
				//LD_LIBRARY_PATH /vendor/lib:/system/lib 可见先搜索vendor
				if(status != -1)
					status = domakelink("/system/lib/liblog.so", "/vendor/lib/liblog!.so");
				if(status != -1)
					status = domakelink((packagelibpath + "libAndroidBootstrap0.so").c_str(), "/vendor/lib/liblog.so");
			}
			if(isremount)
			{
				doremount(mountpos[i].c_str(), true);
			}
			if(status != -1)
				linkresult = true;
			break;
		}
	}
}

void Run_do_patch(const char** argv)
{
	domount("/system",false);
	replaceLinkerEnv("/system/bin/linker","LD_LIBRARY_PATH","CY_LIBRARY_PATH");
}

void Run_do_unlink(const char** argv)
{
	int status;
	dosafe(status, unlink("/vendor/lib/liblog.so"));
	dosafe(status, unlink("/vendor/lib/liblog!.so"));
	dosafe(status, unlink("/system/lib/substrate-dvm.so"));
	dosafe(status, unlink("/system/lib/libsubstrate.so"));
}

void Run_do_unpatch(const char** argv)
{
	domount("/system",false);
	replaceLinkerEnv("/system/bin/linker","CY_LIBRARY_PATH","LD_LIBRARY_PATH");
}

struct hash_data
{
	Elf32_Word hash_bucket_size;
	Elf32_Word hash_chain_size;
	Elf32_Word hash_bucket;
	Elf32_Word hash_chain;
};

void Run_nm(const char** argv)
{
	int filefd;
	dosafe(filefd, open(argv[0], O_RDONLY));
	if(filefd == -1)
		return;
	int status;
	struct stat st;
	dosafe(status, fstat(filefd, &st));
	if(status != -1)
	{
		void* base = MAP_FAILED;
		dosafe(base, mmap(0, st.st_size,PROT_READ | PROT_WRITE, MAP_PRIVATE, filefd, 0));
		if(base != MAP_FAILED)
		{
			Elf32_Ehdr *ehdr = (Elf32_Ehdr *)base;
			Elf32_Phdr *phdr = (Elf32_Phdr*)((char*)ehdr + ehdr->e_phoff);
			Elf32_Dyn *dyn = 0;
			for(int i=0;i<ehdr->e_phnum;i++)
			{
				if(phdr->p_type == PT_DYNAMIC)
				{
					dyn = (Elf32_Dyn*)((char*)ehdr + phdr->p_offset);
				}
				phdr = (Elf32_Phdr *)((char*)phdr + ehdr->e_phentsize);
			}
			if(dyn == 0 || dyn->d_tag == DT_NULL)
			{
				showerror(EINVAL);
			}
			else
			{
				Elf32_Sword tag = dyn->d_tag;
				enum
				{
					i_hash,
					i_strtab,
					i_symtab,
					i_strsz,
					i_syment,
					i_max
				};
				Elf32_Addr offs[i_max]={-1};

				for(tag = dyn->d_tag;tag != 0;tag = dyn->d_tag,dyn++)
				{
					switch(tag)
					{
					case DT_HASH:
						offs[i_hash] = dyn->d_un.d_ptr;
						break;
					case DT_STRTAB:
						offs[i_strtab] = dyn->d_un.d_ptr;
						break;
					case DT_SYMTAB:
						offs[i_symtab] = dyn->d_un.d_ptr;
						break;
					case DT_STRSZ:
						offs[i_strsz] = dyn->d_un.d_ptr;
						break;
					case DT_SYMENT:
						offs[i_syment] = dyn->d_un.d_ptr;
						break;
					}
				}
#ifndef EM_S390
#define EM_S390 22
#endif
#define SH_ENTSIZE_HASH(Ehdr) \
	((Ehdr)->e_machine == EM_ALPHA || ((Ehdr)->e_machine == EM_S390	&& (Ehdr)->e_ident[EI_CLASS] == ELFCLASS64) ? 8 : 4)

				if(offs[i_symtab] != -1 && offs[i_syment] != -1 && offs[i_strtab] != -1 &&
						offs[i_strsz] != -1 && offs[i_hash] != -1)
				{
					hash_data* hashdata = (hash_data*)((Elf32_Addr)base + offs[i_hash] + SH_ENTSIZE_HASH(ehdr));
					for(int i=0;i < hashdata->hash_chain_size;i++)
					{
						Elf32_Sym* sym = (Elf32_Sym*)((Elf32_Addr)base + offs[i_symtab]);
						if(sym->st_shndx != 0 && ELF_ST_TYPE(sym->st_info) == STT_FUNC && sym->st_name <= offs[i_strsz])
						{
							puts((const char*)(offs[i_strtab] + sym->st_name));
						}
						offs[i_symtab] += offs[i_syment];
					}
				}
			}
			munmap(base, st.st_size);
		}
	}

	close(filefd);
}

void Run_rpl(const char** argv)
{
	replaceLinkerEnv(argv[0], argv[1], argv[2]);
}

typedef void(*dofunc)(const char**);

int main(int argc, const char** argv, const char** envp)
{
	if(argc <= 1)
	{
		showerror(EINVAL);
		return -1;
	}
	map<string,dofunc> funcmap;
	funcmap["do_link"] = Run_do_link;
	funcmap["do_unlink"] = Run_do_unlink;
	funcmap["do_patch"] = Run_do_patch;
	funcmap["do_unpatch"] = Run_do_unpatch;
	funcmap["nm"] = Run_nm;
	funcmap["rpl"] = Run_rpl;
	map<string,dofunc>::iterator itor = funcmap.find(argv[1]);
	if(itor == funcmap.end())
	{
		showerror(EINVAL);
		return -1;
	}
	itor->second(argv+2);//传入后面的参数
}
