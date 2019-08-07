LOCAL_PATH := $(call my-dir)
ENGINE_DIR =../../nenuzhno-engine/
include $(CLEAR_VARS)

LOCAL_MODULE    := nenuzhno-engine
LOCAL_CFLAGS    := -Wall
LOCAL_C_INCLUDES := . ../libs/glm ../nenuzhno-engine
LOCAL_SRC_FILES := $(ENGINE_DIR)android_backend.cpp $(ENGINE_DIR)log.cpp $(ENGINE_DIR)system/FileSystem.cpp $(ENGINE_DIR)game/IGame.cpp $(ENGINE_DIR)renderer/camera.cpp $(ENGINE_DIR)/cull/frustum.cpp \
	$(ENGINE_DIR)graphics/gl_utils.cpp $(ENGINE_DIR)graphics/glsl_prog.cpp $(ENGINE_DIR)resource/mesh_loader.cpp $(ENGINE_DIR)renderer/mesh.cpp $(ENGINE_DIR)graphics/vbo.cpp \
	cube.cpp
LOCAL_LDLIBS := -llog -lGLESv2 -lm -lEGL

include $(BUILD_SHARED_LIBRARY)
