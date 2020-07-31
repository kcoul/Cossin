if(${JUCE_LIB_LOCAL})
    if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/juce")
        add_subdirectory(juce)
    else()
        message(FATAL_ERROR "Could not find any juce folder in the project root")
    endif()
else()
    find_package(JUCE CONFIG REQUIRED)

    if(NOT JUCE_FOUND)
        message(FATAL_ERROR "Could not find any juce package on your system")
    endif()
endif()

// Adding bare modules
juce_add_module(ff_meters ALIAS_NAMESPACE foleys)

// Adding libraries
add_subdirectory(jaut)
add_subdirectory(yaml-cpp)
