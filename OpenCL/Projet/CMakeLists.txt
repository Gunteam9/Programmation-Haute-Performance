CMAKE_MINIMUM_REQUIRED(VERSION 3.13)
PROJECT("Projet" VERSION 1.0.0 LANGUAGES CXX)

# Variable for the executable name & additional OpenCL file to be copied inside the build directory
SET(EXEC_NAME main)
SET(cl_codes exemple.cl)

# Set C/C++ standard (version)
SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_C_STANDARD 20)
# Add compile flags for warnings
ADD_COMPILE_OPTIONS(-O3 -Wall -Wextra -pedantic)

# Use FindOpenCL through FIND_PACKAGE
FIND_PACKAGE(OpenCL REQUIRED)

# Add all cpp files to the "source_files" variable
FILE(GLOB source_files "*.cpp")

# Copy each file inside "cl_codes" array to the build directory
FOREACH(ADDI IN LISTS cl_codes)
    CONFIGURE_FILE(
            ${CMAKE_CURRENT_SOURCE_DIR}/${ADDI}
            ${CMAKE_CURRENT_BINARY_DIR} COPYONLY
    )
ENDFOREACH()

# Create the executable rule
ADD_EXECUTABLE(${EXEC_NAME} ${source_files})
# Link to OpenCL libraries through cmake 3.1 targets
TARGET_LINK_LIBRARIES(${EXEC_NAME} OpenCL::OpenCL)