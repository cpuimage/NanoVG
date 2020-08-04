#
# Try to find POWERVR SDK library and include path.
# Once done this will define
#
# POWERVR_SDK_FOUND
# POWERVR_SDK_INCLUDE_PATH
# POWERVR_SDK_LIBRARIES

IF (WIN32)
    FIND_PATH(POWERVR_SDK_INCLUDE_PATH pvr_openlib.h
            ${POWERVR_SDK_ROOT_DIR}/include
            /usr/include
            /usr/local/include
            /sw/include
            /opt/local/include
            DOC "The directory where pvr_openlib.h resides")
    # Prefer the static library.
    FIND_LIBRARY(EGL_LIBRARY
            NAMES libEGL.lib
            PATHS
            ${POWERVR_SDK_ROOT_DIR}/lib
            /usr/lib64
            /usr/lib
            /usr/local/lib64
            /usr/local/lib
            /sw/lib
            /opt/local/lib
            DOC "The EGL static library")
    FIND_LIBRARY(GLES_CM_LIBRARY
            NAMES libGLES_CM.lib libGLESv2.lib
            PATHS
            ${POWERVR_SDK_ROOT_DIR}/lib
            /usr/lib64
            /usr/lib
            /usr/local/lib64
            /usr/local/lib
            /sw/lib
            /opt/local/lib
            DOC "The libGLES_CM static library")
    FIND_LIBRARY(GLES_LIBRARY
            NAMES libGLESv2.lib
            PATHS
            ${POWERVR_SDK_ROOT_DIR}/lib
            /usr/lib64
            /usr/lib
            /usr/local/lib64
            /usr/local/lib
            /sw/lib
            /opt/local/lib
            DOC "The GLES static library")
ELSE (WIN32)
    FIND_PATH(POWERVR_SDK_INCLUDE_PATH pvr_openlib.h
            ${POWERVR_SDK_ROOT_DIR}/include
            $ENV{PROGRAMFILES}/GLES/include
            DOC "The directory where pvr_openlib.h resides")
    FIND_LIBRARY(EGL_LIBRARY
            NAMES libEGL
            PATHS
            ${POWERVR_SDK_ROOT_DIR}/bin
            DOC "The EGL library")
    FIND_LIBRARY(GLES_CM_LIBRARY
            NAMES libGLES_CM
            PATHS
            ${POWERVR_SDK_ROOT_DIR}/bin
            DOC "The libGLES_CM library")
    FIND_LIBRARY(GLES_LIBRARY
            NAMES libGLESv2
            PATHS
            ${POWERVR_SDK_ROOT_DIR}/bin
            DOC "The GLES library")
ENDIF (WIN32)

SET(POWERVR_SDK_FOUND "NO")
IF (POWERVR_SDK_INCLUDE_PATH AND GLES_LIBRARY AND GLES_CM_LIBRARY AND EGL_LIBRARY)
    SET(POWERVR_SDK_LIBRARIES ${GLES_LIBRARY} ${GLES_CM_LIBRARY} ${EGL_LIBRARY})
    SET(POWERVR_SDK_FOUND "YES")
    message(STATUS "POWERVR SDK LIBRARIES FOUND")
ENDIF (POWERVR_SDK_INCLUDE_PATH AND GLES_LIBRARY AND GLES_CM_LIBRARY AND EGL_LIBRARY)
INCLUDE(${CMAKE_ROOT}/Modules/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(POWERVR DEFAULT_MSG POWERVR_SDK_LIBRARIES)
