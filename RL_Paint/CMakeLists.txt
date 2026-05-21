# CMakeList.txt : CMake project for RL_Paint, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_library (RL_Paint SHARED RL_Paint.cpp)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET RL_Paint PROPERTY CXX_STANDARD 20)
endif()

set(BakkesModSDK "C:/Users/klanton/Downloads/bakkesmodsdk_lowercase")
set(RenderingTools "C:/Users/klanton/source/repos/RenderingTools" )

target_include_directories("RL_Paint" PRIVATE
    "${BakkesModSDK}/include"
    "${RenderingTools}/RenderingTools"
)

add_library(bakkesmod STATIC IMPORTED)
set_target_properties(bakkesmod PROPERTIES
    IMPORTED_LOCATION
    "${BakkesModSDK}/lib/pluginsdk.lib"
)
target_link_libraries("RL_Paint" PRIVATE bakkesmod)

add_library(RenderingTools STATIC IMPORTED)
set_target_properties(RenderingTools PROPERTIES
    IMPORTED_LOCATION_RELEASE
    "${RenderingTools}/x64/Release/RenderingTools.lib"
)
target_link_libraries("RL_Paint" PRIVATE RenderingTools)

