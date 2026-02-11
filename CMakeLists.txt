cmake_minimum_required(VERSION 3.16)
project(pong)

# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")


add_subdirectory(vendor/SDL EXCLUDE_FROM_ALL)

# Create game executable target as usual
add_executable(pong WIN32 src/PingPong.c)

target_link_libraries(pong PRIVATE 
    SDL3::SDL3
)

add_custom_command(TARGET pong POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:pong>/images
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/images
            $<TARGET_FILE_DIR:pong>/images
    COMMENT "Copying images to runtime output directory"
)
