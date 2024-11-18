#pragma once

#define KRR_PROJECT_NAME "${CMAKE_PROJECT_NAME}"
#define KRR_PROJECT_DIR "${CMAKE_SOURCE_DIR}"
#define KRR_BUILD_TYPE "${CMAKE_BUILD_TYPE}"

#cmakedefine01  KRR_PLATFORM_WINDOWS 
#cmakedefine01  KRR_PLATFORM_LINUX 
#cmakedefine01	KRR_PLATFORM_UNKNOWN

#cmakedefine01	KRR_BUILD_STARLIGHT

#define KRR_ENABLE_PROFILE	1

#define KRR_CLIPSPACE_RIGHTHANDED 1
#define KRR_CLIPSPACE_Z_FROM_ZERO 1

#define KRR_DEFAULT_RND_SEED	7272
#define OPTIX_LOG_SIZE			4096