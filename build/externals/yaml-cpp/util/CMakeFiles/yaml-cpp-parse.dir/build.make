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
CMAKE_COMMAND = /home/elanda/JetBrains/app/clion-2020.1.2/bin/cmake/linux/bin/cmake

# The command to remove a file.
RM = /home/elanda/JetBrains/app/clion-2020.1.2/bin/cmake/linux/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/elanda/Documents/Development/Juce/Plugins/Cossin

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/elanda/Documents/Development/Juce/Plugins/Cossin/build

# Include any dependencies generated for this target.
include externals/yaml-cpp/util/CMakeFiles/yaml-cpp-parse.dir/depend.make

# Include the progress variables for this target.
include externals/yaml-cpp/util/CMakeFiles/yaml-cpp-parse.dir/progress.make

# Include the compile flags for this target's objects.
include externals/yaml-cpp/util/CMakeFiles/yaml-cpp-parse.dir/flags.make

externals/yaml-cpp/util/CMakeFiles/yaml-cpp-parse.dir/parse.cpp.o: externals/yaml-cpp/util/CMakeFiles/yaml-cpp-parse.dir/flags.make
externals/yaml-cpp/util/CMakeFiles/yaml-cpp-parse.dir/parse.cpp.o: ../externals/yaml-cpp/util/parse.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/elanda/Documents/Development/Juce/Plugins/Cossin/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object externals/yaml-cpp/util/CMakeFiles/yaml-cpp-parse.dir/parse.cpp.o"
	cd /home/elanda/Documents/Development/Juce/Plugins/Cossin/build/externals/yaml-cpp/util && /bin/g++-9  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/yaml-cpp-parse.dir/parse.cpp.o -c /home/elanda/Documents/Development/Juce/Plugins/Cossin/externals/yaml-cpp/util/parse.cpp

externals/yaml-cpp/util/CMakeFiles/yaml-cpp-parse.dir/parse.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/yaml-cpp-parse.dir/parse.cpp.i"
	cd /home/elanda/Documents/Development/Juce/Plugins/Cossin/build/externals/yaml-cpp/util && /bin/g++-9 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/elanda/Documents/Development/Juce/Plugins/Cossin/externals/yaml-cpp/util/parse.cpp > CMakeFiles/yaml-cpp-parse.dir/parse.cpp.i

externals/yaml-cpp/util/CMakeFiles/yaml-cpp-parse.dir/parse.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/yaml-cpp-parse.dir/parse.cpp.s"
	cd /home/elanda/Documents/Development/Juce/Plugins/Cossin/build/externals/yaml-cpp/util && /bin/g++-9 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/elanda/Documents/Development/Juce/Plugins/Cossin/externals/yaml-cpp/util/parse.cpp -o CMakeFiles/yaml-cpp-parse.dir/parse.cpp.s

# Object files for target yaml-cpp-parse
yaml__cpp__parse_OBJECTS = \
"CMakeFiles/yaml-cpp-parse.dir/parse.cpp.o"

# External object files for target yaml-cpp-parse
yaml__cpp__parse_EXTERNAL_OBJECTS =

externals/yaml-cpp/util/parse: externals/yaml-cpp/util/CMakeFiles/yaml-cpp-parse.dir/parse.cpp.o
externals/yaml-cpp/util/parse: externals/yaml-cpp/util/CMakeFiles/yaml-cpp-parse.dir/build.make
externals/yaml-cpp/util/parse: externals/yaml-cpp/libyaml-cppd.a
externals/yaml-cpp/util/parse: externals/yaml-cpp/util/CMakeFiles/yaml-cpp-parse.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/elanda/Documents/Development/Juce/Plugins/Cossin/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable parse"
	cd /home/elanda/Documents/Development/Juce/Plugins/Cossin/build/externals/yaml-cpp/util && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/yaml-cpp-parse.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
externals/yaml-cpp/util/CMakeFiles/yaml-cpp-parse.dir/build: externals/yaml-cpp/util/parse

.PHONY : externals/yaml-cpp/util/CMakeFiles/yaml-cpp-parse.dir/build

externals/yaml-cpp/util/CMakeFiles/yaml-cpp-parse.dir/clean:
	cd /home/elanda/Documents/Development/Juce/Plugins/Cossin/build/externals/yaml-cpp/util && $(CMAKE_COMMAND) -P CMakeFiles/yaml-cpp-parse.dir/cmake_clean.cmake
.PHONY : externals/yaml-cpp/util/CMakeFiles/yaml-cpp-parse.dir/clean

externals/yaml-cpp/util/CMakeFiles/yaml-cpp-parse.dir/depend:
	cd /home/elanda/Documents/Development/Juce/Plugins/Cossin/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/elanda/Documents/Development/Juce/Plugins/Cossin /home/elanda/Documents/Development/Juce/Plugins/Cossin/externals/yaml-cpp/util /home/elanda/Documents/Development/Juce/Plugins/Cossin/build /home/elanda/Documents/Development/Juce/Plugins/Cossin/build/externals/yaml-cpp/util /home/elanda/Documents/Development/Juce/Plugins/Cossin/build/externals/yaml-cpp/util/CMakeFiles/yaml-cpp-parse.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : externals/yaml-cpp/util/CMakeFiles/yaml-cpp-parse.dir/depend

