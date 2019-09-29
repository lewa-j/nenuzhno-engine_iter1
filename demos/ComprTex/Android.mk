LOCAL_PATH := $(call my-dir)
ENGINE_DIR =../../src/

include $(CLEAR_VARS)

LOCAL_MODULE    := nenuzhno-engine
LOCAL_CFLAGS    := -Wall -std=c++11
LOCAL_C_INCLUDES := $(LOCAL_PATH)/. $(LOCAL_PATH)/../../../libs/glm/glm $(LOCAL_PATH)/$(ENGINE_DIR)
LOCAL_SRC_FILES := $(ENGINE_DIR)android_backend.cpp $(ENGINE_DIR)log.cpp $(ENGINE_DIR)system/FileSystem.cpp $(ENGINE_DIR)game/IGame.cpp \
	$(ENGINE_DIR)graphics/gl_utils.cpp $(ENGINE_DIR)graphics/gl_ext.cpp $(ENGINE_DIR)graphics/glsl_prog.cpp $(ENGINE_DIR)graphics/texture.cpp $(ENGINE_DIR)graphics/vbo.cpp $(ENGINE_DIR)graphics/vao.cpp \
	ComprTex.cpp
LOCAL_LDLIBS := -llog -lGLESv2 -lm -lEGL

include $(BUILD_SHARED_LIBRARY)
