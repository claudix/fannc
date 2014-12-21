cmake_minimum_required(VERSION 2.6)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

project(fannc)
add_executable(fannc main.c cmd.c)


#Link to FANN library
find_package(Fann REQUIRED)
include_directories(${FANN_INCLUDE_DIR})
set(LIBS ${LIBS} ${FANN_LIBRARY})

#Link to ARGTABLE2 library
find_package(Argtable2 REQUIRED)
include_directories(${ARGTABLE2_INCLUDE_DIR})
set(LIBS ${LIBS} ${ARGTABLE2_LIBRARY})

#Link to math library
set(LIBS ${LIBS} m)

target_link_libraries(fannc ${LIBS})
