LOCAL_PATH := $(call my-dir)
ENGINE_DIR =../../nenuzhno-engine/
include $(CLEAR_VARS)

LOCAL_MODULE    := nenuzhno-engine
LOCAL_CFLAGS    := -Wall -std=c++11
LOCAL_C_INCLUDES := . ../libs/glm ../nenuzhno-engine
LOCAL_SRC_FILES := $(ENGINE_DIR)android_backend.cpp $(ENGINE_DIR)log.cpp $(ENGINE_DIR)system/FileSystem.cpp $(ENGINE_DIR)system/config.cpp $(ENGINE_DIR)game/IGame.cpp $(ENGINE_DIR)/cull/frustum.cpp $(ENGINE_DIR)cull/BoundingBox.cpp\
	$(ENGINE_DIR)graphics/gl_utils.cpp $(ENGINE_DIR)graphics/gl_ext.cpp $(ENGINE_DIR)graphics/glsl_prog.cpp $(ENGINE_DIR)graphics/texture.cpp $(ENGINE_DIR)graphics/vbo.cpp $(ENGINE_DIR)graphics/vao.cpp $(ENGINE_DIR)graphics/fbo.cpp \
	$(ENGINE_DIR)renderer/renderer.cpp $(ENGINE_DIR)renderer/LightingForward.cpp $(ENGINE_DIR)renderer/mesh.cpp $(ENGINE_DIR)renderer/Model.cpp $(ENGINE_DIR)renderer/camera.cpp $(ENGINE_DIR)renderer/font.cpp \
	$(ENGINE_DIR)resource/ResourceManager.cpp $(ENGINE_DIR)resource/vtf_loader.cpp $(ENGINE_DIR)resource/dds_loader.cpp $(ENGINE_DIR)resource/nmf_loader.cpp $(ENGINE_DIR)resource/mesh_loader.cpp \
	$(ENGINE_DIR)scene/Scene.cpp \
	main.cpp mdl_loader.cpp
LOCAL_LDLIBS := -llog -lGLESv2 -lm -lEGL

include $(BUILD_SHARED_LIBRARY)
