# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt

# Utility rule file for install-safestack.

# Include the progress variables for this target.
include lib/safestack/CMakeFiles/install-safestack.dir/progress.make

lib/safestack/CMakeFiles/install-safestack:
	cd /home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt/lib/safestack && /usr/bin/cmake -DCMAKE_INSTALL_COMPONENT=safestack -P /home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt/cmake_install.cmake

install-safestack: lib/safestack/CMakeFiles/install-safestack
install-safestack: lib/safestack/CMakeFiles/install-safestack.dir/build.make

.PHONY : install-safestack

# Rule to build all files generated by this target.
lib/safestack/CMakeFiles/install-safestack.dir/build: install-safestack

.PHONY : lib/safestack/CMakeFiles/install-safestack.dir/build

lib/safestack/CMakeFiles/install-safestack.dir/clean:
	cd /home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt/lib/safestack && $(CMAKE_COMMAND) -P CMakeFiles/install-safestack.dir/cmake_clean.cmake
.PHONY : lib/safestack/CMakeFiles/install-safestack.dir/clean

lib/safestack/CMakeFiles/install-safestack.dir/depend:
	cd /home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt /home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt/lib/safestack /home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt /home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt/lib/safestack /home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt/lib/safestack/CMakeFiles/install-safestack.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : lib/safestack/CMakeFiles/install-safestack.dir/depend

