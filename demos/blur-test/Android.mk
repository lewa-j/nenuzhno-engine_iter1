LOCAL_PATH := $(call my-dir)
ENGINE_DIR =../../nenuzhno-engine/
include $(CLEAR_VARS)

LOCAL_MODULE    := nenuzhno-engine
LOCAL_CFLAGS    := -Wall
LOCAL_C_INCLUDES := . ../libs/glm ../nenuzhno-engine
LOCAL_SRC_FILES = $(ENGINE_DIR)android_backend.cpp $(ENGINE_DIR)log.cpp $(ENGINE_DIR)file_system.cpp $(ENGINE_DIR)game/IGame.cpp \
	$(ENGINE_DIR)graphics/gl_utils.cpp $(ENGINE_DIR)graphics/gl_ext.cpp $(ENGINE_DIR)graphics/glsl_prog.cpp $(ENGINE_DIR)graphics/texture.cpp $(ENGINE_DIR)graphics/fbo.cpp $(ENGINE_DIR)graphics/vbo.cpp \
	$(ENGINE_DIR)resource/vtf_loader.cpp $(ENGINE_DIR)resource/dds_loader.cpp \
	test.cpp $(ENGINE_DIR)renderer/BokehBlur.cpp
LOCAL_LDLIBS := -llog -lEGL -lGLESv2
# -lm -lOpenSLES -lz

include $(BUILD_SHARED_LIBRARY)

