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
include lib/ubsan/CMakeFiles/RTUbsan_cxx.x86_64.dir/depend.make

# Include the progress variables for this target.
include lib/ubsan/CMakeFiles/RTUbsan_cxx.x86_64.dir/progress.make

# Include the compile flags for this target's objects.
include lib/ubsan/CMakeFiles/RTUbsan_cxx.x86_64.dir/flags.make

lib/ubsan/CMakeFiles/RTUbsan_cxx.x86_64.dir/ubsan_handlers_cxx.cpp.o: lib/ubsan/CMakeFiles/RTUbsan_cxx.x86_64.dir/flags.make
lib/ubsan/CMakeFiles/RTUbsan_cxx.x86_64.dir/ubsan_handlers_cxx.cpp.o: lib/ubsan/ubsan_handlers_cxx.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object lib/ubsan/CMakeFiles/RTUbsan_cxx.x86_64.dir/ubsan_handlers_cxx.cpp.o"
	cd /home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt/lib/ubsan && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/RTUbsan_cxx.x86_64.dir/ubsan_handlers_cxx.cpp.o -c /home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt/lib/ubsan/ubsan_handlers_cxx.cpp

lib/ubsan/CMakeFiles/RTUbsan_cxx.x86_64.dir/ubsan_handlers_cxx.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/RTUbsan_cxx.x86_64.dir/ubsan_handlers_cxx.cpp.i"
	cd /home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt/lib/ubsan && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt/lib/ubsan/ubsan_handlers_cxx.cpp > CMakeFiles/RTUbsan_cxx.x86_64.dir/ubsan_handlers_cxx.cpp.i

lib/ubsan/CMakeFiles/RTUbsan_cxx.x86_64.dir/ubsan_handlers_cxx.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/RTUbsan_cxx.x86_64.dir/ubsan_handlers_cxx.cpp.s"
	cd /home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt/lib/ubsan && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt/lib/ubsan/ubsan_handlers_cxx.cpp -o CMakeFiles/RTUbsan_cxx.x86_64.dir/ubsan_handlers_cxx.cpp.s

lib/ubsan/CMakeFiles/RTUbsan_cxx.x86_64.dir/ubsan_type_hash.cpp.o: lib/ubsan/CMakeFiles/RTUbsan_cxx.x86_64.dir/flags.make
lib/ubsan/CMakeFiles/RTUbsan_cxx.x86_64.dir/ubsan_type_hash.cpp.o: lib/ubsan/ubsan_type_hash.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object lib/ubsan/CMakeFiles/RTUbsan_cxx.x86_64.dir/ubsan_type_hash.cpp.o"
	cd /home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt/lib/ubsan && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/RTUbsan_cxx.x86_64.dir/ubsan_type_hash.cpp.o -c /home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt/lib/ubsan/ubsan_type_hash.cpp

lib/ubsan/CMakeFiles/RTUbsan_cxx.x86_64.dir/ubsan_type_hash.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/RTUbsan_cxx.x86_64.dir/ubsan_type_hash.cpp.i"
	cd /home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt/lib/ubsan && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt/lib/ubsan/ubsan_type_hash.cpp > CMakeFiles/RTUbsan_cxx.x86_64.dir/ubsan_type_hash.cpp.i

lib/ubsan/CMakeFiles/RTUbsan_cxx.x86_64.dir/ubsan_type_hash.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/RTUbsan_cxx.x86_64.dir/ubsan_type_hash.cpp.s"
	cd /home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt/lib/ubsan && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt/lib/ubsan/ubsan_type_hash.cpp -o CMakeFiles/RTUbsan_cxx.x86_64.dir/ubsan_type_hash.cpp.s

lib/ubsan/CMakeFiles/RTUbsan_cxx.x86_64.dir/ubsan_type_hash_itanium.cpp.o: lib/ubsan/CMakeFiles/RTUbsan_cxx.x86_64.dir/flags.make
lib/ubsan/CMakeFiles/RTUbsan_cxx.x86_64.dir/ubsan_type_hash_itanium.cpp.o: lib/ubsan/ubsan_type_hash_itanium.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object lib/ubsan/CMakeFiles/RTUbsan_cxx.x86_64.dir/ubsan_type_hash_itanium.cpp.o"
	cd /home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt/lib/ubsan && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/RTUbsan_cxx.x86_64.dir/ubsan_type_hash_itanium.cpp.o -c /home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt/lib/ubsan/ubsan_type_hash_itanium.cpp

lib/ubsan/CMakeFiles/RTUbsan_cxx.x86_64.dir/ubsan_type_hash_itanium.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/RTUbsan_cxx.x86_64.dir/ubsan_type_hash_itanium.cpp.i"
	cd /home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt/lib/ubsan && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt/lib/ubsan/ubsan_type_hash_itanium.cpp > CMakeFiles/RTUbsan_cxx.x86_64.dir/ubsan_type_hash_itanium.cpp.i

lib/ubsan/CMakeFiles/RTUbsan_cxx.x86_64.dir/ubsan_type_hash_itanium.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/RTUbsan_cxx.x86_64.dir/ubsan_type_hash_itanium.cpp.s"
	cd /home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt/lib/ubsan && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt/lib/ubsan/ubsan_type_hash_itanium.cpp -o CMakeFiles/RTUbsan_cxx.x86_64.dir/ubsan_type_hash_itanium.cpp.s

lib/ubsan/CMakeFiles/RTUbsan_cxx.x86_64.dir/ubsan_type_hash_win.cpp.o: lib/ubsan/CMakeFiles/RTUbsan_cxx.x86_64.dir/flags.make
lib/ubsan/CMakeFiles/RTUbsan_cxx.x86_64.dir/ubsan_type_hash_win.cpp.o: lib/ubsan/ubsan_type_hash_win.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object lib/ubsan/CMakeFiles/RTUbsan_cxx.x86_64.dir/ubsan_type_hash_win.cpp.o"
	cd /home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt/lib/ubsan && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/RTUbsan_cxx.x86_64.dir/ubsan_type_hash_win.cpp.o -c /home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt/lib/ubsan/ubsan_type_hash_win.cpp

lib/ubsan/CMakeFiles/RTUbsan_cxx.x86_64.dir/ubsan_type_hash_win.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/RTUbsan_cxx.x86_64.dir/ubsan_type_hash_win.cpp.i"
	cd /home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt/lib/ubsan && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt/lib/ubsan/ubsan_type_hash_win.cpp > CMakeFiles/RTUbsan_cxx.x86_64.dir/ubsan_type_hash_win.cpp.i

lib/ubsan/CMakeFiles/RTUbsan_cxx.x86_64.dir/ubsan_type_hash_win.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/RTUbsan_cxx.x86_64.dir/ubsan_type_hash_win.cpp.s"
	cd /home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt/lib/ubsan && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt/lib/ubsan/ubsan_type_hash_win.cpp -o CMakeFiles/RTUbsan_cxx.x86_64.dir/ubsan_type_hash_win.cpp.s

RTUbsan_cxx.x86_64: lib/ubsan/CMakeFiles/RTUbsan_cxx.x86_64.dir/ubsan_handlers_cxx.cpp.o
RTUbsan_cxx.x86_64: lib/ubsan/CMakeFiles/RTUbsan_cxx.x86_64.dir/ubsan_type_hash.cpp.o
RTUbsan_cxx.x86_64: lib/ubsan/CMakeFiles/RTUbsan_cxx.x86_64.dir/ubsan_type_hash_itanium.cpp.o
RTUbsan_cxx.x86_64: lib/ubsan/CMakeFiles/RTUbsan_cxx.x86_64.dir/ubsan_type_hash_win.cpp.o
RTUbsan_cxx.x86_64: lib/ubsan/CMakeFiles/RTUbsan_cxx.x86_64.dir/build.make

.PHONY : RTUbsan_cxx.x86_64

# Rule to build all files generated by this target.
lib/ubsan/CMakeFiles/RTUbsan_cxx.x86_64.dir/build: RTUbsan_cxx.x86_64

.PHONY : lib/ubsan/CMakeFiles/RTUbsan_cxx.x86_64.dir/build

lib/ubsan/CMakeFiles/RTUbsan_cxx.x86_64.dir/clean:
	cd /home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt/lib/ubsan && $(CMAKE_COMMAND) -P CMakeFiles/RTUbsan_cxx.x86_64.dir/cmake_clean.cmake
.PHONY : lib/ubsan/CMakeFiles/RTUbsan_cxx.x86_64.dir/clean

lib/ubsan/CMakeFiles/RTUbsan_cxx.x86_64.dir/depend:
	cd /home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt /home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt/lib/ubsan /home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt /home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt/lib/ubsan /home/twinturbo/Desktop/ExtraSpace/CheckCBox_Compiler/compiler-rt/lib/ubsan/CMakeFiles/RTUbsan_cxx.x86_64.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : lib/ubsan/CMakeFiles/RTUbsan_cxx.x86_64.dir/depend

