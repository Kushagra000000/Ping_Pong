cmake_minimum_required(VERSION 3.16)
project(hello)

# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# This assumes the SDL source is available in vendored/SDL
add_subdirectory(vendor/SDL EXCLUDE_FROM_ALL)
add_subdirectory(vendor/SDL_image EXCLUDE_FROM_ALL)

# Create your game executable target as usual
add_executable(hello WIN32 src/PingPong.c)

# Link to the actual SDL3 library.
if (TARGET SDL3_image)
message("@@@@@@@gay@@@@@@@@")
endif()
target_link_libraries(hello PRIVATE 
    SDL3::SDL3
    SDL3_image::SDL3_image
)

# target_link_libraries(MyApp PRIVATE 
#     SDL3::SDL3 
#     SDL3_image::SDL3_image
# )