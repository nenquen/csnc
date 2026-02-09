LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := server

include $(XASH3D_CONFIG)

LOCAL_CPPFLAGS += -std=gnu++14 -fno-exceptions -fno-builtin
LOCAL_CFLAGS += -Wall -ffunction-sections -fdata-sections

ifeq ($(TARGET_ARCH_ABI),arm64-v8a)
	LOCAL_CFLAGS += -DXASH_64BIT
endif
ifeq ($(TARGET_ARCH_ABI),x86_64)
	LOCAL_CFLAGS += -DXASH_64BIT
endif

LOCAL_CFLAGS += \
	-DREGAMEDLL_FIXES \
	-DREGAMEDLL_API \
	-DREGAMEDLL_ADD \
	-DUNICODE_FIXES \
	-DBUILD_LATEST \
	-DCLIENT_WEAPONS \
	-DUSE_QSTRING \
	-D_LINUX \
	-DLINUX \
	-DNDEBUG \
	-D_GLIBCXX_USE_CXX11_ABI=0 \
	-D_stricmp=strcasecmp \
	-D_strnicmp=strncasecmp \
	-D_strdup=strdup \
	-D_unlink=unlink \
	-D_snprintf=snprintf \
	-D_vsnprintf=vsnprintf \
	-D_write=write \
	-D_close=close \
	-D_access=access \
	-D_vsnwprintf=vswprintf

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH) \
	$(LOCAL_PATH)/engine \
	$(LOCAL_PATH)/common \
	$(LOCAL_PATH)/dlls \
	$(LOCAL_PATH)/game_shared \
	$(LOCAL_PATH)/pm_shared \
	$(LOCAL_PATH)/regamedll \
	$(LOCAL_PATH)/public \
	$(LOCAL_PATH)/public/regamedll \
	$(LOCAL_PATH)/public/tier0

REGAMEDLL_ALL_SRCS_ABS := \
	$(wildcard $(LOCAL_PATH)/engine/*.cpp) \
	$(wildcard $(LOCAL_PATH)/common/*.cpp) \
	$(wildcard $(LOCAL_PATH)/dlls/*.cpp) \
	$(wildcard $(LOCAL_PATH)/dlls/API/*.cpp) \
	$(wildcard $(LOCAL_PATH)/dlls/addons/*.cpp) \
	$(wildcard $(LOCAL_PATH)/dlls/wpn_shared/*.cpp) \
	$(wildcard $(LOCAL_PATH)/dlls/bot/*.cpp) \
	$(wildcard $(LOCAL_PATH)/dlls/bot/states/*.cpp) \
	$(wildcard $(LOCAL_PATH)/dlls/hostage/*.cpp) \
	$(wildcard $(LOCAL_PATH)/dlls/hostage/states/*.cpp) \
	$(wildcard $(LOCAL_PATH)/game_shared/*.cpp) \
	$(wildcard $(LOCAL_PATH)/game_shared/bot/*.cpp) \
	$(wildcard $(LOCAL_PATH)/pm_shared/*.cpp) \
	$(wildcard $(LOCAL_PATH)/regamedll/*.cpp) \
	$(LOCAL_PATH)/public/FileSystem.cpp \
	$(LOCAL_PATH)/public/interface.cpp \
	$(LOCAL_PATH)/public/MemPool.cpp \
	$(LOCAL_PATH)/public/tier0/dbg.cpp \
	$(LOCAL_PATH)/public/tier0/platform_posix.cpp

REGAMEDLL_ALL_SRCS_ABS := $(filter-out \
	$(LOCAL_PATH)/common/stdc++compat.cpp,\
	$(REGAMEDLL_ALL_SRCS_ABS))

LOCAL_SRC_FILES := $(patsubst $(LOCAL_PATH)/%,%,$(REGAMEDLL_ALL_SRCS_ABS))

LOCAL_LDFLAGS += -Wl,--allow-shlib-undefined

LOCAL_LDLIBS := -ldl -llog

include $(BUILD_SHARED_LIBRARY)
