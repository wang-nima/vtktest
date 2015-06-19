cmake_minimum_required(VERSION 2.8)
 
PROJECT(Decimation)
 
find_package(VTK REQUIRED)
include(${VTK_USE_FILE})
 
add_executable(Decimation MACOSX_BUNDLE Decimation)
 
if(VTK_LIBRARIES)
  target_link_libraries(Decimation ${VTK_LIBRARIES})
else()
  target_link_libraries(Decimation vtkHybrid vtkWidgets)
endif()
