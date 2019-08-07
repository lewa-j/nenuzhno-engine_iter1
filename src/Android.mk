LOCAL_PATH := $(call my-dir)
ENGINE_DIR = ../../nenuzhno-engine/
include $(CLEAR_VARS)

LOCAL_MODULE    := gles-test
LOCAL_CFLAGS    := -Wall
LOCAL_C_INCLUDES := ../libs/glm ../nenuzhno-engine
LOCAL_SRC_FILES := $(ENGINE_DIR)android_backend.cpp $(ENGINE_DIR)file_system.cpp $(ENGINE_DIR)renderer/glslProg.cpp $(ENGINE_DIR)triangle.cpp
LOCAL_LDLIBS := -llog -lGLESv2 -lm -lOpenSLES
LOCAL_STATIC_LIBRARIES =

include $(BUILD_SHARED_LIBRARY)

$(call import-add-path,/sdcard/AppProjects/libs)
