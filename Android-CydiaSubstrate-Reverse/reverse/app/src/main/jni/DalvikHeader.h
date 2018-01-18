/*
 * DalvikHeader.h
 *
 *  Created on: 2016Äê5ÔÂ15ÈÕ
 *      Author: Administrator
 */

#ifndef JNI_DALVIKHEADER_H_
#define JNI_DALVIKHEADER_H_

typedef uint8_t             u1;
typedef uint16_t            u2;
typedef uint32_t            u4;
typedef uint64_t            u8;
typedef int8_t              s1;
typedef int16_t             s2;
typedef int32_t             s4;
typedef int64_t             s8;

struct Object;
union JValue
{
    u1      z;
    s1      b;
    u2      c;
    s2      s;
    s4      i;
    s8      j;
    float   f;
    double  d;
    Object* l;
};

typedef enum PrimitiveType
{
    PRIM_NOT        = -1,       /* value is not a primitive type */
    PRIM_BOOLEAN    = 0,
    PRIM_CHAR       = 1,
    PRIM_FLOAT      = 2,
    PRIM_DOUBLE     = 3,
    PRIM_BYTE       = 4,
    PRIM_SHORT      = 5,
    PRIM_INT        = 6,
    PRIM_LONG       = 7,
    PRIM_VOID       = 8,
    PRIM_MAX
} PrimitiveType;

enum
{
    ACC_PUBLIC       = 0x00000001,       // class, field, method, ic
    ACC_PRIVATE      = 0x00000002,       // field, method, ic
    ACC_PROTECTED    = 0x00000004,       // field, method, ic
    ACC_STATIC       = 0x00000008,       // field, method, ic
    ACC_FINAL        = 0x00000010,       // class, field, method, ic
    ACC_SYNCHRONIZED = 0x00000020,       // method (only allowed on natives)
    ACC_SUPER        = 0x00000020,       // class (not used in Dalvik)
    ACC_VOLATILE     = 0x00000040,       // field
    ACC_BRIDGE       = 0x00000040,       // method (1.5)
    ACC_TRANSIENT    = 0x00000080,       // field
    ACC_VARARGS      = 0x00000080,       // method (1.5)
    ACC_NATIVE       = 0x00000100,       // method
    ACC_INTERFACE    = 0x00000200,       // class, ic
    ACC_ABSTRACT     = 0x00000400,       // class, method, ic
    ACC_STRICT       = 0x00000800,       // method
    ACC_SYNTHETIC    = 0x00001000,       // field, method, ic
    ACC_ANNOTATION   = 0x00002000,       // class, ic (1.5)
    ACC_ENUM         = 0x00004000,       // class, field, ic (1.5)
    ACC_CONSTRUCTOR  = 0x00010000,       // method (Dalvik only)
    ACC_DECLARED_SYNCHRONIZED = 0x00020000,       // method (Dalvik only)
    ACC_CLASS_MASK =
        (ACC_PUBLIC | ACC_FINAL | ACC_INTERFACE | ACC_ABSTRACT
                | ACC_SYNTHETIC | ACC_ANNOTATION | ACC_ENUM),
    ACC_INNER_CLASS_MASK =
        (ACC_CLASS_MASK | ACC_PRIVATE | ACC_PROTECTED | ACC_STATIC),
    ACC_FIELD_MASK =
        (ACC_PUBLIC | ACC_PRIVATE | ACC_PROTECTED | ACC_STATIC | ACC_FINAL
                | ACC_VOLATILE | ACC_TRANSIENT | ACC_SYNTHETIC | ACC_ENUM),
    ACC_METHOD_MASK =
        (ACC_PUBLIC | ACC_PRIVATE | ACC_PROTECTED | ACC_STATIC | ACC_FINAL
                | ACC_SYNCHRONIZED | ACC_BRIDGE | ACC_VARARGS | ACC_NATIVE
                | ACC_ABSTRACT | ACC_STRICT | ACC_SYNTHETIC | ACC_CONSTRUCTOR
                | ACC_DECLARED_SYNCHRONIZED),
};

struct DexClassLookup
{
    int     size;                       // total size, including "size"
    int     numEntries;                 // size of table[]; always power of 2
    struct
	{
        u4      classDescriptorHash;    // class descriptor hash code
        int     classDescriptorOffset;  // in bytes, from start of DEX
        int     classDefOffset;         // in bytes, from start of DEX
    } table[1];
};

struct DexOptHeader
{
    u1  magic[8];           /* includes version number */
    u4  dexOffset;          /* file offset of DEX header */
    u4  dexLength;
    u4  depsOffset;         /* offset of optimized DEX dependency table */
    u4  depsLength;
    u4  optOffset;          /* file offset of optimized data tables */
    u4  optLength;
    u4  flags;              /* some info flags */
    u4  checksum;           /* adler32 checksum covering deps/opt */
};

enum
{
	kSHA1DigestLen = 20,
    kSHA1DigestOutputLen = kSHA1DigestLen*2 +1
};

struct DexHeader
{
    u1  magic[8];           /* includes version number */
    u4  checksum;           /* adler32 checksum */
    u1  signature[kSHA1DigestLen]; /* SHA-1 hash */
    u4  fileSize;           /* length of entire file */
    u4  headerSize;         /* offset to start of next section */
    u4  endianTag;
    u4  linkSize;
    u4  linkOff;
    u4  mapOff;
    u4  stringIdsSize;
    u4  stringIdsOff;
    u4  typeIdsSize;
    u4  typeIdsOff;
    u4  protoIdsSize;
    u4  protoIdsOff;
    u4  fieldIdsSize;
    u4  fieldIdsOff;
    u4  methodIdsSize;
    u4  methodIdsOff;
    u4  classDefsSize;
    u4  classDefsOff;
    u4  dataSize;
    u4  dataOff;
};

struct DexStringId
{
    u4 stringDataOff;      /* file offset to string_data_item */
};

struct DexTypeId
{
    u4  descriptorIdx;      /* index into stringIds list for type descriptor */
};

struct DexFieldId
{
    u2  classIdx;           /* index into typeIds list for defining class */
    u2  typeIdx;            /* index into typeIds for field type */
    u4  nameIdx;            /* index into stringIds for field name */
};

struct DexMethodId
{
    u2  classIdx;           /* index into typeIds list for defining class */
    u2  protoIdx;           /* index into protoIds for method prototype */
    u4  nameIdx;            /* index into stringIds for method name */
};

struct DexClassDef
{
    u4  classIdx;           /* index into typeIds for this class */
    u4  accessFlags;
    u4  superclassIdx;      /* index into typeIds for superclass */
    u4  interfacesOff;      /* file offset to DexTypeList */
    u4  sourceFileIdx;      /* index into stringIds for source file name */
    u4  annotationsOff;     /* file offset to annotations_directory_item */
    u4  classDataOff;       /* file offset to class_data_item */
    u4  staticValuesOff;    /* file offset to DexEncodedArray */
};

struct DexProtoId
{
    u4  shortyIdx;          /* index into stringIds for shorty descriptor */
    u4  returnTypeIdx;      /* index into typeIds list for return type */
    u4  parametersOff;      /* file offset to type_list for parameter types */
};

struct DexLink
{
    u1  bleargh;
};

struct DexFile
{
    const DexOptHeader* pOptHeader;
    const DexHeader*    pHeader;
    const DexStringId*  pStringIds;
    const DexTypeId*    pTypeIds;
    const DexFieldId*   pFieldIds;
    const DexMethodId*  pMethodIds;
    const DexProtoId*   pProtoIds;
    const DexClassDef*  pClassDefs;
    const DexLink*      pLinkData;
    const DexClassLookup* pClassLookup;
    const void*         pRegisterMapPool;       // RegisterMapClassPool
    const u1*           baseAddr;
    int                 overhead;
};

struct DexProto
{
    const DexFile* dexFile;     /* file the idx refers to */
    u4 protoIdx;                /* index into proto_ids table of dexFile */
};

struct ClassObject;
struct Object
{
    ClassObject*    clazz;
    u4              lock;
};

struct MemMapping
{
    void*   addr;           /* start of data */
    size_t  length;         /* length of data */
    void*   baseAddr;       /* page-aligned base address */
    size_t  baseLength;     /* length of mapping */
};

struct DvmDex
{
    DexFile*            pDexFile;
    const DexHeader*    pHeader;
    struct StringObject** pResStrings;
    struct ClassObject** pResClasses;
    struct Method**     pResMethods;
    struct Field**      pResFields;
    struct AtomicCache* pInterfaceCache;
    bool                isMappedReadOnly;
    MemMapping          memMap;
    pthread_mutex_t     modLock;
};

enum ClassStatus
{
    CLASS_ERROR         = -1,
    CLASS_NOTREADY      = 0,
    CLASS_IDX           = 1,    /* loaded, DEX idx in super or ifaces */
    CLASS_LOADED        = 2,    /* DEX idx values resolved */
    CLASS_RESOLVED      = 3,    /* part of linking */
    CLASS_VERIFYING     = 4,    /* in the process of being verified */
    CLASS_VERIFIED      = 5,    /* logically part of linking; done pre-init */
    CLASS_INITIALIZING  = 6,    /* class init in progress */
    CLASS_INITIALIZED   = 7,    /* ready to go */
};

struct InitiatingLoaderList
{
    Object**  initiatingLoaders;
    int       initiatingLoaderCount;
};

struct InterfaceEntry
{
    ClassObject*    clazz;
    int*            methodIndexArray;
};

struct Field
{
    ClassObject*    clazz;          /* class in which the field is declared */
    const char*     name;
    const char*     signature;      /* e.g. "I", "[C", "Landroid/os/Debug;" */
    u4              accessFlags;
};

struct InstField : Field
{
    int             byteOffset;
};

struct StaticField : Field
{
    JValue          value;          /* initially set from DEX for primitives */
};

struct ArrayObject : Object
{
    u4              length;
    u8              contents[1];
};

struct DataObject : Object
{
    u4              instanceData[1];
};

struct ClassObject : Object
{
    u4              instanceData[4];
    const char*     descriptor;
    char*           descriptorAlloc;
    u4              accessFlags;
    u4              serialNumber;
    DvmDex*         pDvmDex;
    ClassStatus     status;
    ClassObject*    verifyErrorClass;
    u4              initThreadId;
    size_t          objectSize;
    ClassObject*    elementClass;
    int             arrayDim;
    PrimitiveType   primitiveType;
    ClassObject*    super;
    Object*         classLoader;
    InitiatingLoaderList initiatingLoaderList;
    int             interfaceCount;
    ClassObject**   interfaces;
    int             directMethodCount;
    Method*         directMethods;
    int             virtualMethodCount;
    Method*         virtualMethods;
    int             vtableCount;
    Method**        vtable;
    int             iftableCount;
    InterfaceEntry* iftable;
    int             ifviPoolCount;
    int*            ifviPool;
    int             ifieldCount;
    int             ifieldRefCount; // number of fields that are object refs
    InstField*      ifields;
    u4 refOffsets;
    const char*     sourceFile;
    int             sfieldCount;
    StaticField     sfields[0]; /* MUST be last item */
};

typedef void (*DalvikBridgeFunc)(const u4* args, JValue* pResult,
    const Method* method, struct Thread* self);
typedef void (*DalvikNativeFunc)(const u4* args, JValue* pResult);

struct RegisterMap
{
    u1      format;         /* enum RegisterMapFormat; MUST be first entry */
    u1      regWidth;       /* bytes per register line, 1+ */
    u1      numEntries[2];  /* number of entries */
    u1      data[1];
};

struct Method
{
    ClassObject*    clazz;
    u4              accessFlags;
    u2             methodIndex;
    u2              registersSize;  /* ins + locals */
    u2              outsSize;
    u2              insSize;
    const char*     name;
    DexProto        prototype;
    const char*     shorty;
    const u2*       insns;          /* instructions, in memory-mapped .dex */
    int             jniArgInfo;
    DalvikBridgeFunc nativeFunc;
    bool fastJni;
    bool noRef;
    bool shouldTrace;
    const RegisterMap* registerMap;
    bool            inProfile;
};

struct JNIEnvExt
{
    const struct JNINativeInterface* funcTable;     /* must be first */
    const struct JNINativeInterface* baseFuncTable;
    u4      envThreadId;
    Thread* self;
    int     critical;
    struct JNIEnvExt* prev;
    struct JNIEnvExt* next;
};

class AndroidRuntime
{

};

struct Monitor
{
	Thread*			owner;
	int				lockCount;
	Object*			obj;
	Thread*			waitSet;
	pthread_mutex_t	lock;
	Monitor*		next;
	const Method*	ownerMethod;
	u4				ownerPc;
	//MonitorData[]
};

typedef struct MSJavaObjectKey_
{
	u4		tag;
	bool	deleted;
	int		ref;
} *MSJavaObjectKey;

struct MonitorData
{
	MSJavaObjectKey	key;
	void*			value;
typedef void (*CleanCallback)(void *, JNIEnv *, void *);
	CleanCallback	clean;
	void*			data;
};

#endif /* JNI_DALVIKHEADER_H_ */
