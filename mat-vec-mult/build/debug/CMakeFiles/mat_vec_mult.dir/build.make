# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.26

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /opt/homebrew/Cellar/cmake/3.26.3/bin/cmake

# The command to remove a file.
RM = /opt/homebrew/Cellar/cmake/3.26.3/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/kristiansordal/dev/uib/fall23/inf339A/mat-vec-mult

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/kristiansordal/dev/uib/fall23/inf339A/mat-vec-mult/build/debug

# Include any dependencies generated for this target.
include CMakeFiles/mat_vec_mult.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/mat_vec_mult.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/mat_vec_mult.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/mat_vec_mult.dir/flags.make

CMakeFiles/mat_vec_mult.dir/main.cpp.o: CMakeFiles/mat_vec_mult.dir/flags.make
CMakeFiles/mat_vec_mult.dir/main.cpp.o: /Users/kristiansordal/dev/uib/fall23/inf339A/mat-vec-mult/main.cpp
CMakeFiles/mat_vec_mult.dir/main.cpp.o: CMakeFiles/mat_vec_mult.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/kristiansordal/dev/uib/fall23/inf339A/mat-vec-mult/build/debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/mat_vec_mult.dir/main.cpp.o"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/mat_vec_mult.dir/main.cpp.o -MF CMakeFiles/mat_vec_mult.dir/main.cpp.o.d -o CMakeFiles/mat_vec_mult.dir/main.cpp.o -c /Users/kristiansordal/dev/uib/fall23/inf339A/mat-vec-mult/main.cpp

CMakeFiles/mat_vec_mult.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/mat_vec_mult.dir/main.cpp.i"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/kristiansordal/dev/uib/fall23/inf339A/mat-vec-mult/main.cpp > CMakeFiles/mat_vec_mult.dir/main.cpp.i

CMakeFiles/mat_vec_mult.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/mat_vec_mult.dir/main.cpp.s"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/kristiansordal/dev/uib/fall23/inf339A/mat-vec-mult/main.cpp -o CMakeFiles/mat_vec_mult.dir/main.cpp.s

# Object files for target mat_vec_mult
mat_vec_mult_OBJECTS = \
"CMakeFiles/mat_vec_mult.dir/main.cpp.o"

# External object files for target mat_vec_mult
mat_vec_mult_EXTERNAL_OBJECTS =

mat_vec_mult: CMakeFiles/mat_vec_mult.dir/main.cpp.o
mat_vec_mult: CMakeFiles/mat_vec_mult.dir/build.make
mat_vec_mult: /opt/homebrew/lib/libboost_program_options-mt.dylib
mat_vec_mult: /opt/homebrew/lib/libboost_mpi-mt.dylib
mat_vec_mult: /opt/homebrew/lib/libboost_serialization-mt.dylib
mat_vec_mult: /opt/homebrew/Cellar/open-mpi/4.1.5/lib/libmpi.dylib
mat_vec_mult: CMakeFiles/mat_vec_mult.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/kristiansordal/dev/uib/fall23/inf339A/mat-vec-mult/build/debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable mat_vec_mult"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/mat_vec_mult.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/mat_vec_mult.dir/build: mat_vec_mult
.PHONY : CMakeFiles/mat_vec_mult.dir/build

CMakeFiles/mat_vec_mult.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/mat_vec_mult.dir/cmake_clean.cmake
.PHONY : CMakeFiles/mat_vec_mult.dir/clean

CMakeFiles/mat_vec_mult.dir/depend:
	cd /Users/kristiansordal/dev/uib/fall23/inf339A/mat-vec-mult/build/debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/kristiansordal/dev/uib/fall23/inf339A/mat-vec-mult /Users/kristiansordal/dev/uib/fall23/inf339A/mat-vec-mult /Users/kristiansordal/dev/uib/fall23/inf339A/mat-vec-mult/build/debug /Users/kristiansordal/dev/uib/fall23/inf339A/mat-vec-mult/build/debug /Users/kristiansordal/dev/uib/fall23/inf339A/mat-vec-mult/build/debug/CMakeFiles/mat_vec_mult.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/mat_vec_mult.dir/depend

