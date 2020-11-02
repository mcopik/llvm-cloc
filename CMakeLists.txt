cmake_minimum_required(VERSION 3.1)
include(ExternalProject)
project(llvm-cloc)

find_package(LLVM REQUIRED CONFIG PATHS ${LLVM_DIR} NO_DEFAULT_PATH)
add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})

# Verify that the compiler used is the same as in LLVM instrumentation
# Otherwise there might be an ABI incompatibility between our pass and runtime
# library and LLVM.
find_program(LLVM_OPT "opt" PATHS ${LLVM_TOOLS_BINARY_DIR} NO_DEFAULT_PATH)
message(STATUS "Using LLVM opt in ${LLVM_OPT}")
find_program(LLVM_LLC "llc" PATHS ${LLVM_TOOLS_BINARY_DIR} NO_DEFAULT_PATH)
message(STATUS "Using LLVM llc in ${LLVM_LLC}")
# TODO: allow using a different compiler?
#set(CLANG ${CMAKE_C_COMPILER})
#set(CLANGXX ${CMAKE_CXX_COMPILER})


file(GLOB LLVMClocFiles
    "lib/*.cpp"
)
add_library(LLVMClocPass MODULE ${LLVMClocFiles})
set_property(TARGET LLVMClocPass PROPERTY CXX_STANDARD 17)
set_target_properties(LLVMClocPass PROPERTIES POSITION_INDEPENDENT_CODE On)
set_target_properties(LLVMClocPass PROPERTIES LIBRARY_OUTPUT_DIRECTORY lib)
set_target_properties(LLVMClocPass PROPERTIES COMPILE_FLAGS "-fno-rtti -g")

configure_file(bin/llvm-cloc.in bin/llvm-cloc @ONLY)

