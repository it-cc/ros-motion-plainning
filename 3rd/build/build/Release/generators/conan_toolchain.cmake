

# Conan automatically generated toolchain file
# DO NOT EDIT MANUALLY, it will be overwritten

# Avoid including toolchain file several times (bad if appending to variables like
#   CMAKE_CXX_FLAGS. See https://github.com/android/ndk/issues/323
include_guard()

message(STATUS "Using Conan toolchain: ${CMAKE_CURRENT_LIST_FILE}")

if(${CMAKE_VERSION} VERSION_LESS "3.15")
    message(FATAL_ERROR "The 'CMakeToolchain' generator only works with CMake >= 3.15")
endif()










string(APPEND CONAN_CXX_FLAGS " -m64")
string(APPEND CONAN_C_FLAGS " -m64")
string(APPEND CONAN_SHARED_LINKER_FLAGS " -m64")
string(APPEND CONAN_EXE_LINKER_FLAGS " -m64")



# Extra c, cxx, linkflags and defines


if(DEFINED CONAN_CXX_FLAGS)
  string(APPEND CMAKE_CXX_FLAGS_INIT " ${CONAN_CXX_FLAGS}")
endif()
if(DEFINED CONAN_C_FLAGS)
  string(APPEND CMAKE_C_FLAGS_INIT " ${CONAN_C_FLAGS}")
endif()
if(DEFINED CONAN_SHARED_LINKER_FLAGS)
  string(APPEND CMAKE_SHARED_LINKER_FLAGS_INIT " ${CONAN_SHARED_LINKER_FLAGS}")
endif()
if(DEFINED CONAN_EXE_LINKER_FLAGS)
  string(APPEND CMAKE_EXE_LINKER_FLAGS_INIT " ${CONAN_EXE_LINKER_FLAGS}")
endif()

get_property( _CMAKE_IN_TRY_COMPILE GLOBAL PROPERTY IN_TRY_COMPILE )
if(_CMAKE_IN_TRY_COMPILE)
    message(STATUS "Running toolchain IN_TRY_COMPILE")
    return()
endif()

set(CMAKE_FIND_PACKAGE_PREFER_CONFIG ON)

# Definition of CMAKE_MODULE_PATH
# The root (which is the default builddirs) path of dependencies in the host context
list(PREPEND CMAKE_MODULE_PATH "/home/cc/.conan/data/osqp/0.6.3/_/_/package/2af715f34a7c8c2aeae57b25be0a52c4110dc502/" "/home/cc/.conan/data/glog/0.6.0/_/_/package/a9d362b17f05cef8580ca68487c6299654e427f6/" "/home/cc/.conan/data/gflags/2.2.2/_/_/package/88539f002769027b252fd7a108dc2f1fa9529154/" "/home/cc/.conan/data/xz_utils/5.4.5/_/_/package/6af9cc7cb931c5ad942174fd7838eb655717c709/" "/home/cc/.conan/data/zlib/1.3.1/_/_/package/6af9cc7cb931c5ad942174fd7838eb655717c709/")
# the generators folder (where conan generates files, like this toolchain)
list(PREPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})

# Definition of CMAKE_PREFIX_PATH, CMAKE_XXXXX_PATH
# The Conan local "generators" folder, where this toolchain is saved.
list(PREPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_LIST_DIR} )
list(PREPEND CMAKE_PROGRAM_PATH "/home/cc/.conan/data/osqp/0.6.3/_/_/package/2af715f34a7c8c2aeae57b25be0a52c4110dc502/bin" "/home/cc/.conan/data/ceres-solver/1.14.0/_/_/package/7195fd60b8544268e80a4a2edca60ab3e55f8aa9/bin" "/home/cc/.conan/data/glog/0.6.0/_/_/package/a9d362b17f05cef8580ca68487c6299654e427f6/bin" "/home/cc/.conan/data/gflags/2.2.2/_/_/package/88539f002769027b252fd7a108dc2f1fa9529154/bin" "/home/cc/.conan/data/libunwind/1.8.0/_/_/package/c8c888b1fc83f5e0145e8890c2af3bd4e0005c98/bin" "/home/cc/.conan/data/xz_utils/5.4.5/_/_/package/6af9cc7cb931c5ad942174fd7838eb655717c709/bin" "/home/cc/.conan/data/zlib/1.3.1/_/_/package/6af9cc7cb931c5ad942174fd7838eb655717c709/bin")
list(PREPEND CMAKE_LIBRARY_PATH "/home/cc/.conan/data/osqp/0.6.3/_/_/package/2af715f34a7c8c2aeae57b25be0a52c4110dc502/lib" "/home/cc/.conan/data/ceres-solver/1.14.0/_/_/package/7195fd60b8544268e80a4a2edca60ab3e55f8aa9/lib" "/home/cc/.conan/data/glog/0.6.0/_/_/package/a9d362b17f05cef8580ca68487c6299654e427f6/lib" "/home/cc/.conan/data/gflags/2.2.2/_/_/package/88539f002769027b252fd7a108dc2f1fa9529154/lib" "/home/cc/.conan/data/libunwind/1.8.0/_/_/package/c8c888b1fc83f5e0145e8890c2af3bd4e0005c98/lib" "/home/cc/.conan/data/xz_utils/5.4.5/_/_/package/6af9cc7cb931c5ad942174fd7838eb655717c709/lib" "/home/cc/.conan/data/zlib/1.3.1/_/_/package/6af9cc7cb931c5ad942174fd7838eb655717c709/lib")
list(PREPEND CMAKE_INCLUDE_PATH "/home/cc/.conan/data/osqp/0.6.3/_/_/package/2af715f34a7c8c2aeae57b25be0a52c4110dc502/include" "/home/cc/.conan/data/ceres-solver/1.14.0/_/_/package/7195fd60b8544268e80a4a2edca60ab3e55f8aa9/include" "/home/cc/.conan/data/ceres-solver/1.14.0/_/_/package/7195fd60b8544268e80a4a2edca60ab3e55f8aa9/include/ceres" "/home/cc/.conan/data/eigen/3.4.0/_/_/package/5ab84d6acfe1f23c4fae0ab88f26e3a396351ac9/include/eigen3" "/home/cc/.conan/data/glog/0.6.0/_/_/package/a9d362b17f05cef8580ca68487c6299654e427f6/include" "/home/cc/.conan/data/gflags/2.2.2/_/_/package/88539f002769027b252fd7a108dc2f1fa9529154/include" "/home/cc/.conan/data/libunwind/1.8.0/_/_/package/c8c888b1fc83f5e0145e8890c2af3bd4e0005c98/include" "/home/cc/.conan/data/xz_utils/5.4.5/_/_/package/6af9cc7cb931c5ad942174fd7838eb655717c709/include" "/home/cc/.conan/data/zlib/1.3.1/_/_/package/6af9cc7cb931c5ad942174fd7838eb655717c709/include")



if (DEFINED ENV{PKG_CONFIG_PATH})
set(ENV{PKG_CONFIG_PATH} "/home/cc/ros_ws/project250529/3rd/build/build/Release/generators:$ENV{PKG_CONFIG_PATH}")
else()
set(ENV{PKG_CONFIG_PATH} "/home/cc/ros_ws/project250529/3rd/build/build/Release/generators:")
endif()




# Variables
# Variables  per configuration


# Preprocessor definitions
# Preprocessor definitions per configuration
