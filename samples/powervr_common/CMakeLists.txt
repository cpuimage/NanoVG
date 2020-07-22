set(powervr_common_src source/es_util.cpp)
set(powervr_common_platform_src source/es_util_win.cpp)
add_library(powervr_common STATIC ${powervr_common_src} ${powervr_common_platform_src})
target_link_libraries(powervr_common ${POWERVR_SDK_LIBRARIES})