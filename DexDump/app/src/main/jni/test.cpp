#include <jni.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <android/log.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include "dexfile.h"
#include "substrate.h"

#define TAG "HOOKTEST"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
MSConfig(MSFilterLibrary, "libdvm.so"); // 要hook的so库，后缀名即可。

struct DexFile* new_dexFileParse(const unsigned char* data, size_t length, int flags);
struct DexFile* (*old_dexFileParse)(const unsigned char* data, size_t length, int flags);

bool getProcPackage(int pid, char* buf)
{
    char filename[256];
    sprintf(filename,"/proc/%d/cmdline",pid);
    FILE *f = fopen(filename,"r");
    if (f)
    {
        fread(buf,256,1,f);
        fclose(f);
    }
}

const char* targetlist[]=
{
       "com.b.wallet",
};
#define arraysize(x) (sizeof(x)/sizeof(x[0]))

struct DexFile*  new_dexFileParse(const unsigned char* data, size_t length, int flags)
{
    char name[256];
    char filename[256];
    FILE *fp = 0;
    bool isinlist = false;
    int err = 0;
    struct DexFile* pDexFile = 0;
    const int MinSize = 0x600000;

    getProcPackage(getpid(),name);

    for(int i = 0;i<arraysize(targetlist) && !isinlist;i++)
    {
        if(!strcmp(name, targetlist[i]))
            isinlist = true;
    }

    pDexFile = old_dexFileParse(data, length, flags);
    if(length < MinSize)
    {
        return pDexFile;
    }
    if(isinlist)
    {
        LOGI("dexFileParse for %s %d", name, length);
        sprintf(filename, "/data/data/%s/unpatch-%s-%d.odex", name, name, getpid());
        LOGI("patch %s",filename);
        fp = fopen(filename, "wb");
        if (!fp)
        {
            err = 10;
            LOGI("fopen failed! %s", strerror(errno));
            goto error;
        }
        else
        {
            LOGI("fopen success!");
        }
    }
    else
    {
        return pDexFile;
    }

    do
    {
        u4 dexoff;
        u4 classdefsize;
        u4 max = 0, min = 0xffffffff;
        struct DexOptHeader* pOptHeader = 0;
        struct DexHeader* pHeader = 0;

        if(pDexFile == 0)
        {
            err = 1;
            break;
        }
        if(data == 0)
        {
            err = 2;
            break;
        }

        u1* copydata = new u1[length];
        long curpos = length;
        memcpy(copydata, data, length);
        pOptHeader = (struct DexOptHeader*)copydata;
        dexoff = pOptHeader->dexOffset;
        pHeader = (struct DexHeader*)(copydata + pOptHeader->dexOffset);
        classdefsize = pHeader->classDefsSize;

        if(pOptHeader->magic[0] != 'd' || pOptHeader->magic[1] != 'e' || pOptHeader->magic[2] != 'y' ||
            pHeader->magic[0] != 'd' || pHeader->magic[1] != 'e' || pHeader->magic[2] != 'x')
        {
            LOGI("dex header error!");
            err = 3;
            delete []copydata;
            break;
        }
        LOGI("dex header ok!");

        //get memory segment
        unsigned int beginarr[1024];
        unsigned int endarr[1024];
        int segnum = 0;
        {
            const char* mapname = "/proc/self/maps";
            char allstring[0x20000];
            memset(allstring, 0 ,0x20000);
            FILE* mapfile = fopen(mapname, "r");
            if(mapfile != 0)
            {
                LOGI("%s open success", mapname);
                fread(allstring, 0x20000, 1, mapfile);
                char* line = strtok(allstring, "\n");
                while(line)
                {
                    if(strlen(line) >= 18)
                    {
                        char* p;
                        char tmp[9]="\0\0\0\0\0\0\0\0";
                        memcpy(tmp, line, 8);
                        beginarr[segnum] = strtoul(tmp, &p, 16);
                        memcpy(tmp, line + 9, 8);
                        endarr[segnum] = strtoul(tmp, &p, 16);
                        line = strtok(0, "\n");
                        segnum++;
                    }
                }
            }
            else
            {
                LOGI("%s open error", mapname);
            }
        }

        {
            //------------------------fixup DexClassDef----------------------------
            struct DexClassDef *pClassDefs = (struct DexClassDef *) (copydata + dexoff + pHeader->classDefsOff);
            u4 classDefsSize = pHeader->classDefsSize;

            {
                //========================fixup DexClassDef classDataOff===============
                u4 max = 0x00000000, min = 0xffffffff, first = 0x00000000;
                first = (u4)(pClassDefs[0].classDataOff + data + dexoff);
                for (int i = 0; i < classDefsSize; i++)
                {
                    if (pClassDefs[i].classDataOff != 0)
                    {
                        u4 cur = (u4)(pClassDefs[i].classDataOff + data + dexoff);
                        if (cur > max)
                            max = cur;
                        if (cur < min)
                            min = cur;
                    }
                }
                LOGI("min=%08x, max=%08x, first=%08x", min, max, first);
                //find max and min in map region
                int index1 = 0, index2 = 0;
                for (; index1 < segnum; index1++) {
                    if (min < endarr[index1])
                        break;
                }
                for (; index2 < segnum; index2++) {
                    if (max < endarr[index2])
                        break;
                }
                min = beginarr[index1];
                max = endarr[index2];
                u1* newregion = new u1[max - min];
                memset(newregion, 0, max - min);
                for(int i = index1;i <= index2;i++)
                {
                    memcpy((void*)(newregion + beginarr[i] - min), (void*)(beginarr[i]), endarr[i] - beginarr[i]);
                }

                fseek(fp, curpos, SEEK_SET);
                fwrite(newregion,  1,max - min, fp);

                //adjust the offset
                u4 base = curpos + first - min - dexoff;
                u4 index0 = pClassDefs[0].classDataOff;

                LOGI("[0]=%08x,[1]=%08x,[2]=%08x",pClassDefs[0].classDataOff,pClassDefs[1].classDataOff,pClassDefs[2].classDataOff);
                for (int i = 0; i < classDefsSize; i++)
                {
                    if (pClassDefs[i].classDataOff != 0)
                    {
                        pClassDefs[i].classDataOff = base + pClassDefs[i].classDataOff - index0;
                    }
                }

                curpos = ftell(fp);
                delete []newregion;
            }
        }

        fseek(fp, 0, SEEK_SET);
        fwrite(copydata, length, 1, fp);
        fclose(fp);
        delete []copydata;
    }while(0);
error:
    if(err)
        LOGI("err = %d", err);
    return pDexFile;
}

//Substrate entry point
MSInitialize
{
    LOGI("Substrate initialized.");
    MSImageRef image;
    image = MSGetImageByName("/system/lib/libdvm.so"); // 此处要绝对路径
    if (image != NULL)
    {
        LOGI("dvm image: 0x%08X", (void*)image);
        void * dexFileParse = MSFindSymbol(image, "_Z12dexFileParsePKhji");
        if(dexFileParse == NULL)
        {
            LOGI("error find dexFileParse");
        }
        else
        {
            MSHookFunction(dexFileParse,(void*)&new_dexFileParse,(void **)&old_dexFileParse);
            LOGI("success hook dexFileParse");
        }
    }
    else
    {
        LOGI("can not find libdvm.");
    }
}