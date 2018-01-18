LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := AndroidBootstrap0.cpp
LOCAL_MODULE := AndroidBootstrap0
LOCAL_MODULE_FILENAME := AndroidBootstrap0
LOCAL_LDLIBS := -llog
LOCAL_CFLAGS := -fvisibility=hidden
include $(BUILD_SHARED_LIBRARY)

LOCAL_SRC_FILES := AndroidCydia.cpp
LOCAL_MODULE := AndroidCydia.cy
LOCAL_MODULE_FILENAME := AndroidCydia.cy
LOCAL_LDLIBS := -llog
LOCAL_CFLAGS := -fvisibility=hidden
include $(BUILD_SHARED_LIBRARY)

LOCAL_SRC_FILES := AndroidLoader.cpp
LOCAL_MODULE := AndroidLoader
LOCAL_MODULE_FILENAME := AndroidLoader
LOCAL_LDLIBS := -llog
LOCAL_CFLAGS := -fvisibility=hidden
include $(BUILD_SHARED_LIBRARY)

LOCAL_SRC_FILES := ApiDvm.cpp
LOCAL_MODULE := substrate-dvm
LOCAL_MODULE_FILENAME := substrate-dvm
LOCAL_LDLIBS := -llog
LOCAL_CFLAGS := -fvisibility=hidden
include $(BUILD_SHARED_LIBRARY)

LOCAL_SRC_FILES := ApiJava.cpp
LOCAL_MODULE := substrate
LOCAL_MODULE_FILENAME := substrate
LOCAL_LDLIBS := -llog
LOCAL_CFLAGS := -fvisibility=hidden
include $(BUILD_SHARED_LIBRARY)

LOCAL_SRC_FILES := DalvikLoader.cpp
LOCAL_MODULE := DalvikLoader.cy
LOCAL_MODULE_FILENAME := DalvikLoader.cy
LOCAL_LDLIBS := -llog
LOCAL_CFLAGS := -fvisibility=hidden
include $(BUILD_SHARED_LIBRARY)

LOCAL_SRC_FILES := SubstrateJNI.cpp
LOCAL_MODULE := SubstrateJNI
LOCAL_MODULE_FILENAME := SubstrateJNI
LOCAL_LDLIBS := -llog
LOCAL_CFLAGS := -fvisibility=hidden
include $(BUILD_SHARED_LIBRARY)

LOCAL_SRC_FILES := Run.cpp Unix.cpp
LOCAL_MODULE := SubstrateRun
LOCAL_MODULE_FILENAME := SubstrateRun
LOCAL_LDLIBS := -llog
LOCAL_CFLAGS := -fvisibility=hidden
include $(BUILD_EXECUTABLE)