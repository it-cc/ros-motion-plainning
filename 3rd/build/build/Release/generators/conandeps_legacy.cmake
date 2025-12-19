message(STATUS "Conan: Using CMakeDeps conandeps_legacy.cmake aggregator via include()")
message(STATUS "Conan: It is recommended to use explicit find_package() per dependency instead")

find_package(osqp)
find_package(Ceres)

set(CONANDEPS_LEGACY  osqp::osqp  Ceres::ceres )