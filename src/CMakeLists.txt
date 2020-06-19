file(GLOB_RECURSE asset_files "${CMAKE_CURRENT_LIST_DIR}/assets/*.*")

message ("Assets: ${asset_files}")

juce_add_binary_data(PluginAssets
    HEADER_NAME Assets
    NAMESPACE   Assets
    SOURCES     ${asset_files})

target_link_libraries(Cossin PUBLIC PluginAssets)

target_sources(Cossin PRIVATE
    CossinMain.cpp
    EffectModuleGuis.cpp
    EffectModules.cpp
    MetreLookAndFeel.cpp
    OptionCategories.cpp
    OptionPanel.cpp
    PluginEditor.cpp
    PluginProcessor.cpp
    PluginStyle.cpp
    SharedData.cpp
    ThemeFolder.cpp)
