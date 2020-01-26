/**
    ===============================================================
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <https://www.gnu.org/licenses/>.
    
    Copyright (c) 2019 ElandaSunshine
    ===============================================================
    
    @author Elanda (elanda@elandasunshine.xyz)
    @file   PluginEditor.cpp
    @date   05, October 2019
    
    ===============================================================
 */

#include "PluginEditor.h"

#include "PluginProcessor.h"
#include "PluginStyle.h"
#include "Resources.h"
#include "SharedData.h"
#include <jaut/appdata.h>
#include <jaut/audioprocessorrack.h>
#include <jaut/config.h>
#include <jaut/fontformat.h>
#include <jaut/thememanager.h>

#pragma region Namespace
namespace
{
inline constexpr int Const_HeightHeader = 65;
inline constexpr int Const_HeightFooter = 77;
inline constexpr int Const_PanelMargin  = 2;

inline constexpr int constHeightBody(int height) noexcept
{
    return height - Const_HeightHeader - Const_HeightFooter - Const_PanelMargin * 2;
}

inline String getLocaleName(const jaut::Localisation &locale)
{
    return locale.getInternalLocalisation().getLanguageName()
         + locale.getInternalLocalisation().getCountryCodes().joinIntoString("-");
}

inline void cLog(const String &message)
{
    Time time = Time::getCurrentTime();
    String thread_name = MessageManager::getInstance()->isThisTheMessageThread() ? "MESSAGE " : "AUDIO ";

    String prependix;
    prependix << "[" << thread_name << time.toString(false, true) << "] ";

    Logger::writeToLog(prependix + message);
}
}
#pragma endregion Namespace

#pragma region CossinAudioProcessorEditor
CossinAudioProcessorEditor::CossinAudioProcessorEditor(CossinAudioProcessor &p, AudioProcessorValueTreeState &vts,
                                                       jaut::PropertyMap &map, FFAU::LevelMeterSource &metreSource,
                                                       jaut::AudioProcessorRack &rack, CossinMainEditorWindow &parent,
                                                       bool supportsOpenGl, const String &gpuInfo)
    : processor(p), sourceMetre(metreSource), initialized(false), tooltipServer(this),
#if COSSIN_USE_OPENGL
      glContext(supportsOpenGl ? new OpenGLContext() : nullptr),
#endif
      locale(sharedData->getDefaultLocale()), needsUpdate(false),
      buttonPanningLawSelection("ButtonPanningLawSelection", DrawableButton::ImageRaw),
      buttonSettings("ButtonSettings", DrawableButton::ButtonStyle::ImageRaw),
      metreLevel(FFAU::LevelMeter::Horizontal), optionsPanel(*this, locale), topUnitRackGui(rack, locale),
      atrPanningLaw(map, "PanningLaw"), atrProcessor(map, "SelectedProcessor")
{
    addMouseListener(this, true);

    JT_IS_STANDALONE
    (
        setDefaultLookAndFeel(&lookAndFeel);
    )
    JT_STANDALONE_ELSE
    (
        parent.setLookAndFeel(&lookAndFeel);
    )

    initializeComponents();
    initializeData(parent, gpuInfo);

    attLevel.reset  (new SliderAttachment(vts, "par_master_level",   sliderLevel));
    attMix.reset    (new SliderAttachment(vts, "par_master_mix",     sliderMix));
    attPanning.reset(new SliderAttachment(vts, "par_master_panning", sliderPanning));

    sliderDragEnded(&sliderTabControl);
    startTimer(500);

    initialized = true;
    resized();

    ::cLog("Done initializing Cossin.");
}

CossinAudioProcessorEditor::~CossinAudioProcessorEditor()
{
#if COSSIN_USE_OPENGL
    if (glContext)
    {
        glContext->detach();
        glContext->setRenderer(nullptr);
    }
#endif

    stopTimer();
    sharedData->removeActionListener(this);

    JT_IS_STANDALONE
    (
        setDefaultLookAndFeel(nullptr);
    )
    JT_STANDALONE_ELSE
    (
        getParentComponent()->setLookAndFeel(nullptr);
    )

    JT_NDEBUGGING(if(sharedData->Configuration().getProperty("logToFile", res::Cfg_Standalone).getValue()
                  && JUCEApplicationBase::isStandaloneApp()))
    {
        Logger *logger_to_delete = Logger::getCurrentLogger();
        Logger::setCurrentLogger(nullptr);

        delete logger_to_delete;
    }
}

//======================================================================================================================
void CossinAudioProcessorEditor::initializeData(CossinMainEditorWindow &parent,String gpuInfo)
{
    if(initialized)
    {
        return;
    }
    
    sharedData->addActionListener(this);
    SharedData::ReadLock lock(*sharedData);

#if COSSIN_USE_OPENGL
    const bool accelerated_by_hardware      = sharedData->Configuration().getProperty("hardwareAcceleration",
                                                                                      res::Cfg_Optimization).getValue();
    const bool gl_multisampling_enabled     = sharedData->Configuration().getProperty("useMultisampling",
                                                                                      res::Cfg_Optimization).getValue();
    const bool gl_texture_smoothing_enabled = sharedData->Configuration().getProperty("textureSmoothing",
                                                                                      res::Cfg_Optimization).getValue();

    options[Flag_HardwareAcceleration] = accelerated_by_hardware;
    options[Flag_GlMultisampling]      = gl_multisampling_enabled;
    options[Flag_GlTextureSmoothing]   = gl_texture_smoothing_enabled;

    if(accelerated_by_hardware && glContext)
    {
        glContext->setRenderer(this);
        glContext->setMultisamplingEnabled(gl_multisampling_enabled);
        glContext->setTextureMagnificationFilter(static_cast<OpenGLContext::TextureMagnificationFilter>
                                                (static_cast<int>(gl_texture_smoothing_enabled)));
        glContext->attachTo(parent);
    }
#endif

    // Create logger
    const String session_id  = session.id.toDashedString();
    const String cpu_model   = !SystemStats::getCpuModel() .isEmpty() ? SystemStats::getCpuModel()  : "n/a";
    const String cpu_vendor  = !SystemStats::getCpuVendor().isEmpty() ? " (" + SystemStats::getCpuVendor() + ")" : "";
    const String memory_size = String(SystemStats::getMemorySizeInMegabytes()) + "mb";

    if (!gpuInfo.containsNonWhitespaceChars())
    {
        gpuInfo = "Unknown";
    }

    String message;
    message << "**********************************************************" << newLine
            << "                        --Program--                       " << newLine
            << "App:        " << res::App_Name                              << newLine
            << "Version:    " << res::App_Version                           << newLine
            << "Session-ID: " << session_id                                 << newLine
            << "**********************************************************" << newLine
            << "                        --Machine--                       " << newLine
            << "System:     " << SystemStats::getOperatingSystemName()      << newLine
            << "Memory:     " << memory_size                                << newLine
            << "CPU:        " << cpu_model << cpu_vendor                    << newLine
            << "Graphics:   " << gpuInfo                                    << newLine
            << "**********************************************************" << newLine;

    JT_NDEBUGGING(if(sharedData->Configuration().getProperty("logToFile", res::Cfg_Standalone).getValue()
                  && JUCEApplication::isStandaloneApp()))
    {
        File file = sharedData->AppData().getDir("logs").getFile("session-" + session_id + ".log");

        if (file.getParentDirectory().exists())
        {
            Logger::setCurrentLogger(new FileLogger(file, ""));
        }
    }

    Logger::writeToLog(message);
    ::cLog("Initializing Cossin user interface...");

    // Load all data elements
    reloadConfig(sharedData->Configuration());
    reloadLocale(sharedData->Localisation());
    reloadTheme (sharedData->ThemeManager().getCurrentTheme());
}

void CossinAudioProcessorEditor::initializeComponents()
{
    const Slider::SliderStyle style_rotary  = Slider::RotaryHorizontalVerticalDrag;
    const int current_processor             = static_cast<var>(atrProcessor);

    // Level slider
    sliderLevel.setTextBoxStyle(Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    sliderLevel.setPopupDisplayEnabled(true, true, this, -1);
    sliderLevel.setSliderStyle(style_rotary);
    sliderLevel.addMouseListener(this, true);
    sliderLevel.getProperties().set("CSSize", "small");
    sliderLevel.getProperties().set("CSType", "full");
    addAndMakeVisible(sliderLevel);

    // Mix slider
    sliderMix.setTextBoxStyle(Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    sliderMix.setPopupDisplayEnabled(true, true, this, -1);
    sliderMix.setSliderStyle(style_rotary);
    sliderMix.addMouseListener(this, true);
    sliderMix.getProperties().set("CSSize", "big");
    sliderMix.getProperties().set("CSType", "full");
    addAndMakeVisible(sliderMix);

    // Panning slider
    sliderPanning.setTextBoxStyle(Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    sliderPanning.setPopupDisplayEnabled(true, true, this, -1);
    sliderPanning.setSliderStyle(style_rotary);
    sliderPanning.addMouseListener(this, true);
    sliderPanning.getProperties().set("CSSize", "small");
    sliderPanning.getProperties().set("CSType", "half");
    addAndMakeVisible(sliderPanning);

    // Tab control slider
    sliderTabControl.setTextBoxStyle(Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    sliderTabControl.setSliderStyle(Slider::SliderStyle::LinearBar);
    sliderTabControl.addMouseListener(this, true);
    sliderTabControl.setRange(0.0, 1.0);
    sliderTabControl.setColour(Slider::textBoxOutlineColourId, Colours::transparentBlack);
    sliderTabControl.setVelocityModeParameters(1.0, 1, 0.0, false, ModifierKeys::Flags::noModifiers);
    sliderTabControl.setVelocityBasedMode(false);
    sliderTabControl.addListener(this);
    sliderTabControl.setLookAndFeel(this);
    addAndMakeVisible(sliderTabControl);

    // Panning law selection toggle button
    buttonPanningLaw.setLookAndFeel(this);
    buttonPanningLaw.addListener(this);
    addAndMakeVisible(buttonPanningLaw);
    
    // Settings popup button
    buttonSettings.setLookAndFeel(this);
    buttonSettings.addListener(this);
    addAndMakeVisible(buttonSettings);

    // Level metre display
    metreLevel.setMeterSource(&sourceMetre);
    addAndMakeVisible(metreLevel);

    // Panning law selection list
    buttonPanningLawSelection.setLookAndFeel(this);
    buttonPanningLawSelection.addListener(this);
    addChildComponent(buttonPanningLawSelection);

    // Background blur
    backgroundBlur.setAlpha(0.5f * !static_cast<bool>(options[Flag_AnimationComponents]));
    addChildComponent(backgroundBlur);

    // Options panel
    optionsPanel.setCloseButtonCallback([this](Button *button) { buttonClicked(button); });
    addReloadListener(&optionsPanel);
    addChildComponent(optionsPanel);
    
    addReloadListener(&topUnitRackGui);
    addChildComponent(topUnitRackGui);
}

//======================================================================================================================
void CossinAudioProcessorEditor::paint(Graphics &g)
{
    g.setFont(fontTheme);
    paintBasicInterface(g);
}

void CossinAudioProcessorEditor::resized()
{
    if(!initialized)
    {
        return;
    }

    const Rectangle<int> header(0, 0, getWidth(), ::Const_HeightHeader);
    const Rectangle<int> body(0, ::Const_HeightHeader + ::Const_PanelMargin,
                              getWidth(), ::constHeightBody(getHeight()));
    const Rectangle<int> footer(0, getHeight() - ::Const_HeightFooter, getWidth(), ::Const_HeightFooter);

    // Header
    const int header_tab_setting_w = 34;
    const int header_tab_setting_x = header.getRight() - header_tab_setting_w;
    const int header_process_w     = 102;
    const int header_tab_process_w = topUnitRackGui.getProcessorCount() * header_process_w;
    const int header_tab_process_x = header_tab_setting_x - header_tab_process_w;

    sliderTabControl.setBounds(header_tab_process_x, header.getY(), header_tab_process_w, header.getHeight());
    buttonSettings  .setBounds(header_tab_setting_x, header.getY(), header_tab_setting_w, header.getHeight());

    // Body
    backgroundBlur.setBounds(body.getX(), body.getY(), body.getWidth(), body.getHeight());
    
    // Footer
    const int footer_middle = footer.getCentreX();
    const int footer_knob_small_y = footer.getY() + 12;
    const int footer_knob_big_y   = footer.getY() + 39;
    const int footer_panning_y    = footer.getCentreY() + 13;

    sliderLevel  .setBounds(footer_middle - 100,     footer_knob_small_y, 36, 36);
    sliderMix    .setBounds(footer_middle - 30,      footer_knob_big_y - 28, 60, 60);
    sliderPanning.setBounds(footer_middle + 64,      footer_knob_small_y, 36, 36);
    metreLevel   .setBounds(footer.getRight() - 259, footer_knob_big_y - 26, 212, 53);
    buttonPanningLawSelection.setBounds(footer_middle + 107, footer_panning_y, 45, 15);
    buttonPanningLaw         .setBounds(footer_middle + 92,  footer_panning_y, 15, 15);

    // Pop-ups and everything else that doesn't necessarily has to fit into grid (these stand by their own)
    const int options_x = optionsPanel.isShowing() ? getWidth() / 2 - 400
                                                   : (options[Flag_AnimationComponents] ? -::Const_WindowDefaultWidth
                                                                  : getWidth() / 2 - ::Const_WindowDefaultWidth / 2);
    const int options_h = ::constHeightBody(::Const_WindowDefaultHeight);

    optionsPanel.setBounds(options_x, body.getCentreY() - options_h / 2, ::Const_WindowDefaultWidth, options_h);

    JT_IS_STANDALONE({})
    JT_STANDALONE_ELSE
    (
        // Window sizing
        processor.getWindowSize().setBounds(0, 0, getWidth(), getHeight());
    )
}

//======================================================================================================================
void CossinAudioProcessorEditor::paintBasicInterface(Graphics &g) const
{
    const LookAndFeel &lf = getLookAndFeel();

    const Rectangle header(0, 0, getWidth(), ::Const_HeightHeader);
    const Rectangle body(0, ::Const_HeightHeader + ::Const_PanelMargin, getWidth(), ::constHeightBody(getHeight()));
    const Rectangle footer(0, getHeight() - ::Const_HeightFooter, getWidth(), ::Const_HeightFooter);

    const Colour colour_font = lf.findColour(ColourFontId);

    // Entire region
    const int body_background_x = body.getCentreX() - imgBackground.getWidth()  / 2;
    const int body_background_y = body.getCentreY() - imgBackground.getHeight() / 2;

    g.setColour(lf.findColour(ColourContainerBackgroundId));
    g.fillAll();
    g.drawImageAt(imgBackground, body_background_x, body_background_y);

    // Header
    g.setColour(lf.findColour(ColourHeaderBackgroundId));
    g.fillRect(header.getX(), header.getY(), header.getWidth(), header.getHeight());
    g.drawImageAt(imgHeader, header.getRight() - imgHeader.getWidth(), header.getY());
    g.drawImageAt(imgLogo, header.getX() + 10, header.getCentreY() - imgLogo.getHeight() / 2);

    // Footer
    const int footer_middle = footer.getCentreX();
    const int footer_centre = footer.getCentreY();
    const int footer_label_slider_small   = footer_centre + 5;
    const int footer_metre_channel_text_x = footer.getRight() - 270;

    g.setColour(lf.findColour(ColourContainerForegroundId));
    g.fillRect(footer.getX(), footer.getY(), footer.getWidth(), footer.getHeight());

    g.setColour(colour_font);
    jaut::FontFormat::drawSmallCaps(g, locale.translate("control.slider.master.level"),
                                    footer_middle - 107, footer_label_slider_small, 50, 31, Justification::centred);
    jaut::FontFormat::drawSmallCaps(g, locale.translate("control.slider.master.mix"),
                                    footer_middle - 30, footer_centre - 28, 60, 60, Justification::centred);
    jaut::FontFormat::drawSmallCaps(g, locale.translate("control.slider.master.pan"),
                                    footer_middle + 50, footer_label_slider_small, 50, 31, Justification::centred);

    g.setFont(fontTheme.withHeight(12.0f));
    g.drawText("L", footer_metre_channel_text_x, footer.getY() + 13, 10, 14, Justification::left);
    g.drawText("R", footer_metre_channel_text_x, footer.getY() + 30, 10, 14, Justification::left);
}

//======================================================================================================================
void CossinAudioProcessorEditor::reloadConfig(const jaut::Config &config)
{
    ::cLog("Reloading config...");

    const int default_panning_law  = config.getProperty("panning", res::Cfg_Defaults).getValue();
    const int default_processor    = config.getProperty("processor", res::Cfg_Defaults).getValue();

    const auto property_animations = config.getProperty("animations", res::Cfg_Optimization);
    const int  animation_mode      = property_animations.getProperty("mode").getValue();
    const bool animate_effects     = property_animations.getProperty("custom").getProperty("effects").getValue();
    const bool animate_components  = property_animations.getProperty("custom").getProperty("components").getValue();

    if(!jaut::fit_a(animation_mode, 0, 3) || animation_mode == 3)
    {
        options[Flag_AnimationEffects]    = true;
        options[Flag_AnimationComponents] = true;
    }
    else
    {
        options[Flag_AnimationEffects]    = animation_mode == 2 || (animation_mode == 1 && animate_effects);
        options[Flag_AnimationComponents] = animation_mode == 1 && animate_components;
    }

    if(!optionsPanel.isShowing())
    {
        const bool should_animate = static_cast<bool>(options[Flag_AnimationComponents]);

        backgroundBlur.setAlpha(0.5f * !should_animate);
        optionsPanel.setTopLeftPosition(should_animate ? -Const_WindowDefaultWidth
                                        : getWidth() / 2 - (Const_WindowDefaultWidth / 2), optionsPanel.getY());
    }

    listeners.call([&config](ReloadListener &listener)
    {
        listener.reloadConfig(config);
    });

    ::cLog("Config successfully reloaded.");
}

void CossinAudioProcessorEditor::reloadLocale(const jaut::Localisation &locale)
{
    String locale_name = locale.getLanguageFile().getFullPathName().toLowerCase();
    locale_name        = locale_name.isEmpty() ? "default" : locale_name;

    if(lastLocale != locale_name)
    {
        lastLocale = locale_name;
        ::cLog("Loading localisation '" + locale.getLanguageFile().getFileNameWithoutExtension() + "'...");

        this->locale.setCurrentLanguage(locale);

        listeners.call([&locale](ReloadListener &listener)
        {
            listener.reloadLocale(locale);
        });
    }

    ::cLog("Language successfully set.");
}

void CossinAudioProcessorEditor::reloadTheme(const jaut::ThemePointer &theme)
{
    const String theme_id = theme.getId();

    if(lastTheme != theme_id)
    {
        lastTheme = theme_id;
        ::cLog("Loading resources of theme pack '" + theme->getThemeMeta()->getName() + "'...");

        imgBackground  = theme->getImage(res::Png_Cont_Back);
        imgHeader      = theme->getImage(res::Png_Head_Cover);
        imgLogo        = theme->getImage(res::Png_Title);
        imgTabControl  = theme->getImage(res::Png_Tabs);
        imgTabSettings = theme->getImage(res::Png_Tab_Opts);
        imgPanningLaw  = theme->getImage(res::Png_Pan_Law);

        lookAndFeel.reset(theme);

        listeners.call([&theme](ReloadListener &listener)
        {
            listener.reloadTheme(theme);
        });

        sendLookAndFeelChange();
    }

    ::cLog("Resources successfully reloaded.");
}

//======================================================================================================================
bool CossinAudioProcessorEditor::getOption(int flag) const noexcept
{
    const bool flag_is_valid = flag >= 0 && flag < Flag_Num;
    jassert(flag_is_valid);

    return flag_is_valid ? options[flag] : false;
}

void CossinAudioProcessorEditor::setOption(int flag, bool value) noexcept
{    
    if(flag >= 0 && flag < Flag_Num)
    {
        options[flag] = value;
    }
    else
    {
        jassertfalse;
    }
}

const PluginSession &CossinAudioProcessorEditor::getSession() const noexcept
{
    return session;
}

#if COSSIN_USE_OPENGL
bool CossinAudioProcessorEditor::isOpenGLSupported() const noexcept
{
    return glContext != nullptr;
}
#endif

//======================================================================================================================
void CossinAudioProcessorEditor::addReloadListener(ReloadListener *listener)
{
    listeners.add(listener);
}

void CossinAudioProcessorEditor::removeReloadListener(ReloadListener *listener)
{
    listeners.remove(listener);
}

//======================================================================================================================
void CossinAudioProcessorEditor::mouseDown(const MouseEvent&)
{
    if (buttonPanningLawSelection.isVisible())
    {
        if (!buttonPanningLawSelection.isMouseOver(true))
        {
            buttonPanningLawSelection.setVisible(false);
            buttonPanningLaw.setToggleState(false, NotificationType::dontSendNotification);
        }
    }
}

void CossinAudioProcessorEditor::mouseMove(const MouseEvent&)
{}

void CossinAudioProcessorEditor::buttonClicked(Button *button)
{
    if (button == &buttonPanningLawSelection)
    {
        const int panning_mode = std::min(buttonPanningLawSelection.getMouseXYRelative().getX() / 15, 2);

        buttonPanningLaw.setToggleState(false, NotificationType::dontSendNotification);
        buttonPanningLawSelection.setVisible(false);

        if (atrPanningLaw != panning_mode)
        {        
            atrPanningLaw = panning_mode;
        }
    }
    else if (button == &buttonPanningLaw)
    {
        buttonPanningLawSelection.setVisible(buttonPanningLaw.getToggleState());
    }
    else if (button == &buttonSettings || button->getProperties().contains("OptionsPanelClose"))
    {
        if(optionsPanel.isShowing())
        {
            if(options[Flag_AnimationComponents])
            {
                const int panel_width  = ::Const_WindowDefaultWidth;
                const int panel_height = ::constHeightBody(::Const_WindowDefaultHeight);

                ComponentAnimator &animator = Desktop::getInstance().getAnimator();
                animator.animateComponent(&backgroundBlur, backgroundBlur.getBounds(), 0.0f, 200, true, 0, 2);
                animator.animateComponent(&optionsPanel, {-panel_width, 67 + (getHeight() - 146) / 2 - panel_height / 2,
                                                          panel_width, panel_height}, 0.0f, 200, true, 0, 2);
            }
            else
            {
                backgroundBlur.setVisible(false);
            }
            
            optionsPanel.hide();
        }
        else
        {
            optionsPanel.show();
            backgroundBlur.setVisible(true);

            if(options[Flag_AnimationComponents])
            {
                const int panel_width  = ::Const_WindowDefaultWidth;
                const int panel_height = ::constHeightBody(::Const_WindowDefaultHeight);

                ComponentAnimator &animator = Desktop::getInstance().getAnimator();
                animator.animateComponent(&backgroundBlur, backgroundBlur.getBounds(), 0.5f, 200, true, 2, 0);
                animator.animateComponent(&optionsPanel, {getWidth() / 2 - panel_width / 2,
                                                          67 + (getHeight() - 146) / 2 - panel_height / 2,
                                                          panel_width, panel_height}, 1.0f, 250, true, 2, 0);
            }
        }
    }
}

void CossinAudioProcessorEditor::sliderValueChanged(Slider *slider) {}

void CossinAudioProcessorEditor::sliderDragEnded(Slider *slider)
{
    if (slider == &sliderTabControl)
    {
        if(optionsPanel.isShowing())
        {
            buttonClicked(&buttonSettings); // this is meant for hiding the settings pane when the tab control was used
        }

        const int processor_count = topUnitRackGui.getProcessorCount();
        const int processor_index = jmin(JT_FIX(slider->getValue() / (1.0f / processor_count)), processor_count - 1);

        topUnitRackGui.setGui(processor_index, ModifierKeys::getCurrentModifiers().isCtrlDown());
    }
}

//======================================================================================================================
void CossinAudioProcessorEditor::drawToggleButton(Graphics &g, ToggleButton &button, bool, bool)
{
    if (&button == &buttonPanningLaw)
    {
        if (button.getToggleState())
        {
            const LookAndFeel &lf = getLookAndFeel();

            g.setColour(lf.findColour(ColourContainerBackgroundId));
            g.fillAll();

            g.setColour(lf.findColour(ColourComponentForegroundId));
            g.drawRect(0, 0, button.getWidth(), button.getHeight());
        }

        g.drawImage(imgPanningLaw, 2, 3, 10, 10, 10 * static_cast<int>(static_cast<var>(atrPanningLaw)), 0, 10, 10);
    }
}

void CossinAudioProcessorEditor::drawDrawableButton(Graphics &g, DrawableButton &button, bool, bool isDown)
{
    if (&button == &buttonPanningLawSelection)
    {
        const LookAndFeel &lf = getLookAndFeel();
        const Colour colout_container_background = lf.findColour(ColourContainerBackgroundId);

        g.setColour(colout_container_background);
        g.fillAll();

        g.setColour(lf.findColour(ColourComponentForegroundId));
        g.drawRect(0, 0, button.getWidth(), button.getHeight());

        g.setColour(colout_container_background);
        g.drawRect(0, 1, 1, button.getHeight() - 2);

        for (int i = 0; i < res::Pan_Modes_Num; ++i)
        {
            g.drawImage(imgPanningLaw, 15 * i + 2, 3, 10, 10, i * 10, 0, 10, 10);
        }
    }
    else if(&button == &buttonSettings)
    {
        g.drawImage(imgTabSettings, 0, 0, button.getWidth(), button.getHeight(), 0, 65 * isDown,
                    button.getWidth(), button.getHeight());
    }
}

void CossinAudioProcessorEditor::drawLinearSlider(Graphics &g, int width, int height, int x, int y, float sliderPos,
                                                  float sliderMin, float sliderMax, Slider::SliderStyle style,
                                                  Slider &slider)
{
    if (&slider == &sliderTabControl)
    {
        const Rectangle dest(slider.getLocalBounds());
        const int processor_count = topUnitRackGui.getProcessorCount();
        const int processor_index = jmin(JT_FIX(sliderPos / (1.0f / processor_count)), processor_count - 1);
        const Image image_tabs    = imgTabControl.getClippedImage(dest.withTop(dest.getHeight() * processor_index));

        g.drawImageAt(image_tabs, dest.getX(), dest.getY());
        
        return;
    }

    LookAndFeel_V4::drawLinearSlider(g, width, height, x, y, sliderPos, sliderMin, sliderMax, style, slider);
}

//======================================================================================================================
void CossinAudioProcessorEditor::timerCallback()
{
    if(needsUpdate)
    {
        needsUpdate = false;
        reloadAllData();
    }
}

void CossinAudioProcessorEditor::actionListenerCallback(const String &exclusionId)
{
    if(exclusionId != session.id.toDashedString())
    {
        needsUpdate = true;
    }
}

void CossinAudioProcessorEditor::reloadAllData()
{
    MouseCursor::showWaitCursor();

    {
        SharedData::ReadLock lock(*sharedData);

        reloadConfig(sharedData->Configuration());
        reloadLocale(sharedData->Localisation());
        reloadTheme (sharedData->ThemeManager().getCurrentTheme());
    }

    repaint();
    MouseCursor::hideWaitCursor();
}

#if COSSIN_USE_OPENGL
//======================================================================================================================
void CossinAudioProcessorEditor::newOpenGLContextCreated()
{}

void CossinAudioProcessorEditor::renderOpenGL()
{}

void CossinAudioProcessorEditor::openGLContextClosing()
{}
#endif
#pragma endregion CossinAudioProcessorEditor

//======================================================================================================================
#pragma region CossinMainEditorWindow
CossinMainEditorWindow::CossinMainEditorWindow(CossinAudioProcessor &processor, juce::AudioProcessorValueTreeState &vts,
                                               jaut::PropertyMap &properties, FFAU::LevelMeterSource &metreSource,
                                               jaut::AudioProcessorRack &processorRack)
    : AudioProcessorEditor(processor),
      processor(processor), vts(vts), properties(properties), metreSource(metreSource), processorRack(processorRack)
{
    initializeWindow();

#if COSSIN_USE_OPENGL
    testContext.setRenderer(this);
    testContext.attachTo(*this);
#else
    handleAsyncUpdate();
#endif
}

CossinMainEditorWindow::~CossinMainEditorWindow()
{}

//======================================================================================================================
void CossinMainEditorWindow::resized()
{
#if COSSIN_USE_OPENGL
    if (initialized)
#else
    if(editor)
#endif
    {
        editor->setBounds(0, 0, getWidth(), getHeight());
    }
}

//======================================================================================================================
void CossinMainEditorWindow::initializeWindow()
{
    int window_width  = Const_WindowDefaultWidth;
    int window_height = Const_WindowDefaultHeight;

    JT_IS_STANDALONE({})
    JT_STANDALONE_ELSE
    (
        window_width  = processor.getWindowSize().getWidth();
        window_height = processor.getWindowSize().getHeight();
    )

    int window_max_area   = 0;
    int window_max_width  = 0;
    int window_max_height = 0;

    for (auto display : Desktop::getInstance().getDisplays().displays)
    {
        const Rectangle<int> user_area = display.userArea;
        const int area = user_area.getWidth() * user_area.getHeight();

        if (area >= window_max_area)
        {
            window_max_area   = area;
            window_max_width  = user_area.getWidth();
            window_max_height = user_area.getHeight();
        }
    }

    setResizable(true, processor.wrapperType != AudioProcessor::WrapperType::wrapperType_VST3);
    setResizeLimits(Const_WindowDefaultWidth, Const_WindowDefaultHeight, window_max_width, window_max_height);
    setSize(window_width, window_height);
}

#if COSSIN_USE_OPENGL
//======================================================================================================================
void CossinMainEditorWindow::newOpenGLContextCreated()
{
    graphicsCardDetails = String((char*) glGetString(GL_RENDERER));
    isSupported.store(testContext.extensions.glActiveTexture != nullptr);
    triggerAsyncUpdate();
}
#endif

//======================================================================================================================
void CossinMainEditorWindow::handleAsyncUpdate()
{
    bool is_supported = false;

#if COSSIN_USE_OPENGL
    if (initialized)
    {
        return;
    }

    is_supported = isSupported.load();
    testContext.detach();
    testContext.setRenderer(nullptr);
#endif

    String cardInfo = "n/a";

#if COSSIN_USE_OPENGL
    cardInfo = graphicsCardDetails;
#endif

    editor = std::make_unique<CossinAudioProcessorEditor>(processor, vts, properties, metreSource, processorRack, *this,
                                                          is_supported, cardInfo);
    addAndMakeVisible(editor.get());

#if COSSIN_USE_OPENGL
    initialized = true;
#endif
    resized();
}
#pragma endregion CossinMainEditorWindow
