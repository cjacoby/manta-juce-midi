#pragma once

#include <chrono>
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
                       public SettingsPanel::Listener,
                       private Manta
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    // MPE
    /**
     * Send MPE messages to device.
     */
    void mpeMidiReset();
    void mpeControlTypeChange(int type);
    
    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;
    
    void timerCallback() override;
    
    void PadVelocityEvent(int row, int column, int id, int velocity) override;
    void PadEvent(int row, int column, int id, int value) override;
    void SliderEvent(int id, int value) override;

private:
    int midiNoteForPad(int padID);
    int mpeControlType {1};
    
    /**
     * Get the current channel if set, or return a new one.
     */
    int getOrSetNewChannel(int padID, int midiNote);
    
    /**
     * Set the currently used channel in the ValueTree state.
     */
    void setActiveChannel(int padID, int channel);
    
    /**
     * Get the currently used channel in the ValueTree state.
     */
    int getActiveChannel(int padID);
    //==============================================================================
    juce::ValueTree state;
    
    // Midi settings
    int padVelocityMax {100};
    int midiRoot {48};
    
    std::unique_ptr<juce::MidiOutput>         virtualMidiOutput;
    std::unique_ptr<juce::MPEZoneLayout>      zoneLayout;
    std::unique_ptr<juce::MPEChannelAssigner> mpeChannelAssigner;
    
    int padState[48] {0};
    SettingsPanel leftPanel;
    HeaderPanel headerPanel;
    MantaPadContainerComponent padDrawerComponent;
    
    void showAudioWindow();
    SafePointer<AudioSetupModal> audioDialogWindow;
    juce::TextButton audioSettings;
//    BufferManager bufferManager;
    
    juce::Random random;
    juce::Slider levelSlider;
    juce::Label  levelLabel;
//    juce::AudioSampleBuffer
    
    std::chrono::system_clock::time_point lastPadEvent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
