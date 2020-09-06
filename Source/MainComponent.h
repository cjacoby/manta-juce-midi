#pragma once

#include <JuceHeader.h>
#include "Manta.h"

#include "MantaPadDrawer.h"
#include "HeaderPanel.h"
#include "SettingsPanel.h"
//#include "BufferManager.h"

class AudioDeviceDrawer : public juce::Component
{
public:
    AudioDeviceDrawer(juce::AudioDeviceManager& deviceManager);
    ~AudioDeviceDrawer() override;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void dumpDeviceInfo();
    void logMessage (const juce::String& m);
    
private:
    static juce::String getListOfActiveBits (const juce::BigInteger& b);
    
    const juce::AudioDeviceManager& deviceManager;
    juce::AudioDeviceSelectorComponent audioSetupComp;
    juce::TextEditor diagnosticsBox;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioDeviceDrawer)
};

class AudioSetupModal : public juce::DocumentWindow
{
public:
    AudioSetupModal(const juce::String& name, juce::Colour backgroundColour, int buttonsNeeded, juce::AudioDeviceManager& deviceManager): juce::DocumentWindow (name, backgroundColour, buttonsNeeded),
    childComponent(deviceManager)
    {
        setContentOwned (&childComponent, false);
    }
    
    ~AudioSetupModal() override
    {
    }
    
    void closeButtonPressed() override
    {
        delete this;
    }
    
private:
    AudioDeviceDrawer childComponent;
};

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent  : public juce::AudioAppComponent,
                       public juce::Timer,
                       private Manta
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;
    
    void timerCallback() override;
    
    void PadEvent(int row, int column, int id, int value) override;
    void SliderEvent(int id, int value) override;

private:
    //==============================================================================
    juce::ValueTree state;
    
    int padState[48] {0};
    SettingsPanel leftPanel;
    HeaderPanel headerPanel;
    MantaPadDrawer padDrawerComponent;
    
    void showAudioWindow();
    SafePointer<AudioSetupModal> audioDialogWindow;
    juce::TextButton audioSettings;
//    BufferManager bufferManager;
    
    juce::Random random;
    juce::Slider levelSlider;
    juce::Label  levelLabel;
//    juce::AudioSampleBuffer

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
