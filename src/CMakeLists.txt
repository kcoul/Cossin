file(GLOB_RECURSE asset_files "${CMAKE_CURRENT_LIST_DIR}/assets/*.*")

juce_add_binary_data(PluginAssets
    HEADER_NAME Assets.h
    NAMESPACE   Assets
    SOURCES     ${asset_files})

target_link_libraries(Cossin PRIVATE PluginAssets)

target_sources(Cossin PRIVATE
    CossinMain.cpp
    EffectModules.cpp
    MetreLookAndFeel.cpp
    OptionCategories.cpp
    OptionPanel.cpp
    PluginEditor.cpp
    PluginProcessor.cpp
    PluginStyle.cpp
    SharedData.cpp
    ThemeFolder.cpp)
