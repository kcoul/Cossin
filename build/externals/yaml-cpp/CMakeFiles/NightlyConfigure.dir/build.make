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

# Utility rule file for NightlyConfigure.

# Include the progress variables for this target.
include externals/yaml-cpp/CMakeFiles/NightlyConfigure.dir/progress.make

externals/yaml-cpp/CMakeFiles/NightlyConfigure:
	cd /home/elanda/Documents/Development/Juce/Plugins/Cossin/build/externals/yaml-cpp && /home/elanda/JetBrains/app/clion-2020.1.2/bin/cmake/linux/bin/ctest -D NightlyConfigure

NightlyConfigure: externals/yaml-cpp/CMakeFiles/NightlyConfigure
NightlyConfigure: externals/yaml-cpp/CMakeFiles/NightlyConfigure.dir/build.make

.PHONY : NightlyConfigure

# Rule to build all files generated by this target.
externals/yaml-cpp/CMakeFiles/NightlyConfigure.dir/build: NightlyConfigure

.PHONY : externals/yaml-cpp/CMakeFiles/NightlyConfigure.dir/build

externals/yaml-cpp/CMakeFiles/NightlyConfigure.dir/clean:
	cd /home/elanda/Documents/Development/Juce/Plugins/Cossin/build/externals/yaml-cpp && $(CMAKE_COMMAND) -P CMakeFiles/NightlyConfigure.dir/cmake_clean.cmake
.PHONY : externals/yaml-cpp/CMakeFiles/NightlyConfigure.dir/clean

externals/yaml-cpp/CMakeFiles/NightlyConfigure.dir/depend:
	cd /home/elanda/Documents/Development/Juce/Plugins/Cossin/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/elanda/Documents/Development/Juce/Plugins/Cossin /home/elanda/Documents/Development/Juce/Plugins/Cossin/externals/yaml-cpp /home/elanda/Documents/Development/Juce/Plugins/Cossin/build /home/elanda/Documents/Development/Juce/Plugins/Cossin/build/externals/yaml-cpp /home/elanda/Documents/Development/Juce/Plugins/Cossin/build/externals/yaml-cpp/CMakeFiles/NightlyConfigure.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : externals/yaml-cpp/CMakeFiles/NightlyConfigure.dir/depend

