LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE    := substrate
LOCAL_SRC_FILES := $(TARGET_ARCH)/libsubstrate.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := substrate-dvm
LOCAL_SRC_FILES := $(TARGET_ARCH)/libsubstrate-dvm.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := test.cy 
LOCAL_SRC_FILES := test.cpp
LOCAL_LDLIBS := -llog
LOCAL_LDLIBS += -L$(LOCAL_PATH)/$(TARGET_ARCH) -lsubstrate-dvm -lsubstrate
include $(BUILD_SHARED_LIBRARY)