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

# Include any dependencies generated for this target.
include lib/ubsan/CMakeFiles/RTUbsan_dynamic_version_script_dummy.i386.dir/depend.make

# Include the progress variables for this target.
include lib/ubsan/CMakeFiles/RTUbsan_dynamic_version_script_dummy.i386.dir/progress.make

# Include the compile flags for this target's objects.
include lib/ubsan/CMakeFiles/RTUbsan_dynamic_version_script_dummy.i386.dir/flags.make

lib/ubsan/clang_rt.ubsan_standalone-dynamic-i386.vers: lib/sanitizer_common/scripts/gen_dynamic_list.py
lib/ubsan/clang_rt.ubsan_standalone-dynamic-i386.vers: lib/ubsan/ubsan.syms.extra
lib/ubsan/clang_rt.ubsan_standalone-dynamic-i386.vers: lib/linux/libclang_rt.ubsan_standalone-i386.a
lib/ubsan/clang_rt.ubsan_standalone-dynamic-i386.vers: lib/linux/libclang_rt.ubsan_standalone_cxx-i386.a
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Generating version list for clang_rt.ubsan_standalone-dynamic-i386"
	cd /home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt/lib/ubsan && /usr/bin/python3.8 /home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt/lib/sanitizer_common/scripts/gen_dynamic_list.py --version-list --extra ubsan.syms.extra /home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt/lib/linux/libclang_rt.ubsan_standalone-i386.a /home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt/lib/linux/libclang_rt.ubsan_standalone_cxx-i386.a --nm-executable /usr/bin/nm -o /home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt/lib/ubsan/clang_rt.ubsan_standalone-dynamic-i386.vers

lib/ubsan/clang_rt.ubsan_standalone-dynamic-x86_64.vers: lib/sanitizer_common/scripts/gen_dynamic_list.py
lib/ubsan/clang_rt.ubsan_standalone-dynamic-x86_64.vers: lib/ubsan/ubsan.syms.extra
lib/ubsan/clang_rt.ubsan_standalone-dynamic-x86_64.vers: lib/linux/libclang_rt.ubsan_standalone-x86_64.a
lib/ubsan/clang_rt.ubsan_standalone-dynamic-x86_64.vers: lib/linux/libclang_rt.ubsan_standalone_cxx-x86_64.a
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Generating version list for clang_rt.ubsan_standalone-dynamic-x86_64"
	cd /home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt/lib/ubsan && /usr/bin/python3.8 /home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt/lib/sanitizer_common/scripts/gen_dynamic_list.py --version-list --extra ubsan.syms.extra /home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt/lib/linux/libclang_rt.ubsan_standalone-x86_64.a /home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt/lib/linux/libclang_rt.ubsan_standalone_cxx-x86_64.a --nm-executable /usr/bin/nm -o /home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt/lib/ubsan/clang_rt.ubsan_standalone-dynamic-x86_64.vers

lib/ubsan/CMakeFiles/RTUbsan_dynamic_version_script_dummy.i386.dir/dummy.cpp.o: lib/ubsan/CMakeFiles/RTUbsan_dynamic_version_script_dummy.i386.dir/flags.make
lib/ubsan/CMakeFiles/RTUbsan_dynamic_version_script_dummy.i386.dir/dummy.cpp.o: lib/ubsan/dummy.cpp
lib/ubsan/CMakeFiles/RTUbsan_dynamic_version_script_dummy.i386.dir/dummy.cpp.o: lib/ubsan/clang_rt.ubsan_standalone-dynamic-i386.vers
lib/ubsan/CMakeFiles/RTUbsan_dynamic_version_script_dummy.i386.dir/dummy.cpp.o: lib/ubsan/clang_rt.ubsan_standalone-dynamic-x86_64.vers
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object lib/ubsan/CMakeFiles/RTUbsan_dynamic_version_script_dummy.i386.dir/dummy.cpp.o"
	cd /home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt/lib/ubsan && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/RTUbsan_dynamic_version_script_dummy.i386.dir/dummy.cpp.o -c /home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt/lib/ubsan/dummy.cpp

lib/ubsan/CMakeFiles/RTUbsan_dynamic_version_script_dummy.i386.dir/dummy.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/RTUbsan_dynamic_version_script_dummy.i386.dir/dummy.cpp.i"
	cd /home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt/lib/ubsan && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt/lib/ubsan/dummy.cpp > CMakeFiles/RTUbsan_dynamic_version_script_dummy.i386.dir/dummy.cpp.i

lib/ubsan/CMakeFiles/RTUbsan_dynamic_version_script_dummy.i386.dir/dummy.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/RTUbsan_dynamic_version_script_dummy.i386.dir/dummy.cpp.s"
	cd /home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt/lib/ubsan && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt/lib/ubsan/dummy.cpp -o CMakeFiles/RTUbsan_dynamic_version_script_dummy.i386.dir/dummy.cpp.s

RTUbsan_dynamic_version_script_dummy.i386: lib/ubsan/CMakeFiles/RTUbsan_dynamic_version_script_dummy.i386.dir/dummy.cpp.o
RTUbsan_dynamic_version_script_dummy.i386: lib/ubsan/CMakeFiles/RTUbsan_dynamic_version_script_dummy.i386.dir/build.make

.PHONY : RTUbsan_dynamic_version_script_dummy.i386

# Rule to build all files generated by this target.
lib/ubsan/CMakeFiles/RTUbsan_dynamic_version_script_dummy.i386.dir/build: RTUbsan_dynamic_version_script_dummy.i386

.PHONY : lib/ubsan/CMakeFiles/RTUbsan_dynamic_version_script_dummy.i386.dir/build

lib/ubsan/CMakeFiles/RTUbsan_dynamic_version_script_dummy.i386.dir/clean:
	cd /home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt/lib/ubsan && $(CMAKE_COMMAND) -P CMakeFiles/RTUbsan_dynamic_version_script_dummy.i386.dir/cmake_clean.cmake
.PHONY : lib/ubsan/CMakeFiles/RTUbsan_dynamic_version_script_dummy.i386.dir/clean

lib/ubsan/CMakeFiles/RTUbsan_dynamic_version_script_dummy.i386.dir/depend: lib/ubsan/clang_rt.ubsan_standalone-dynamic-i386.vers
lib/ubsan/CMakeFiles/RTUbsan_dynamic_version_script_dummy.i386.dir/depend: lib/ubsan/clang_rt.ubsan_standalone-dynamic-x86_64.vers
	cd /home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt /home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt/lib/ubsan /home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt /home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt/lib/ubsan /home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt/lib/ubsan/CMakeFiles/RTUbsan_dynamic_version_script_dummy.i386.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : lib/ubsan/CMakeFiles/RTUbsan_dynamic_version_script_dummy.i386.dir/depend

