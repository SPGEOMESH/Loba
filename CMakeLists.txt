cmake_minimum_required(VERSION 3.24)
set(PROJECT_LANGUAGES "CXX")
set(CMAKE_CXX_STANDARD 20)
set(PROJECT_FAMILY_NAME "Loba")
set(PROJECT_VERSION "1.0.4")
set(PROJECT_VERSION_NAME "ClementineBirthday")
set(PROJECT_VERSION_MONTH "August")
set(PROJECT_VERSION_YEAR "2026")
set(PROJECT_WORKING_RDIRS "_T")
set(DGCALC_DEPENDENCY_TEST "")
set(DGCALC_DEPENDENCY_LIB "")
set(DGCALC_DEPENDENCY_SRC ${DGCALC_DEPENDENCY_TEST} "")
set(GROUPS "THIRD")
set(THIRD_DEPENDENCY_TEST "Catch2")
set(THIRD_DEPENDENCY_LIB "")
set(THIRD_DEPENDENCY_SRC ${THIRD_DEPENDENCY_TEST})
set(ACCEL "") # "Cuda", "Metal", "" (none)
set(ACCEL_DEPENDENCIES "") # cuBLAS, cuRAND, ...
set(ACCEL_INSTALL_DIR "") # /usr/local, $ENV{CUDA_HOME}, $ENV{METAL_HOME}, ...


# set(LOCAL_ROOT "")
if (LOCAL_ROOT)
    include(${LOCAL_ROOT}/src/dgcalc.1.cmake)
else()
    execute_process(COMMAND dgcalc source 1 OUTPUT_VARIABLE SOURCE1_CODE)
    cmake_language(EVAL CODE "${SOURCE1_CODE}")
endif()
set(PROJECT_NAME "${PROJECT_FAMILY_NAME}${PROJECT_VERSION}${PROJECT_VERSION_NAME}_${PROJECT_VERSION_MONTH}${PROJECT_VERSION_YEAR}")
project(${PROJECT_NAME} VERSION ${PROJECT_VERSION} LANGUAGES ${PROJECT_LANGUAGES})
if (LOCAL_ROOT)
    include(${LOCAL_ROOT}/src/dgcalc.2.cmake)
else()
    execute_process(COMMAND dgcalc source 2 OUTPUT_VARIABLE SOURCE2_CODE)
    cmake_language(EVAL CODE "${SOURCE2_CODE}")
endif()




########################################################

# temporary solutions for libraries
# set(GSL_HOME "/opt/homebrew/opt/gsl")
#set(EIGEN_HOME "/opt/homebrew/opt/eigen")
#set(MPI_HOME "/opt/homebrew/Cellar/open-mpi/5.0.6")
#set(SILO_HOME "/Applications/VisIt.app/Contents/Resources/3.4.2/darwin-arm64")


# target_include_directories(${PROJECT_NAME} PRIVATE "${GSL_HOME}/include")
#target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE "${MPI_HOME}/include")
#target_include_directories(${PROJECT_NAME} PRIVATE "${EIGEN_HOME}/include")
#target_include_directories(${PROJECT_NAME} PRIVATE "${SILO_HOME}/include/silo/include")

#target_link_libraries(${PROJECT_NAME} PUBLIC "${GSL_HOME}/lib/libgsl.dylib")
#target_link_libraries(${PROJECT_NAME} PUBLIC "${MPI_HOME}/lib/libmpi.dylib")
#target_link_libraries(${PROJECT_NAME} PUBLIC "${SILO_HOME}/lib/libsiloh5.dylib")
#target_link_directories(${PROJECT_NAME} PUBLIC "${EIGEN_HOME}/lib")
#target_link_directories(${PROJECT_NAME} PUBLIC "${GSL_HOME}/lib")
#target_link_directories(${PROJECT_NAME} PUBLIC "${MPI_HOME}/lib")
#target_link_directories(${PROJECT_NAME} PUBLIC "${SILO_HOME}")



