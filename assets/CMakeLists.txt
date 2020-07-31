file(GLOB_RECURSE asset_files "${CMAKE_CURRENT_LIST_DIR}/*.*")

juce_add_binary_data(PluginAssets
    HEADER_NAME Assets.h
    NAMESPACE   Assets
    SOURCES     ${asset_files})

target_link_libraries(Cossin PRIVATE PluginAssets) 
