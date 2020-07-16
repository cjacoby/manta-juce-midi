#pragma once

#include <JuceHeader.h>
#include "Manta.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent  : public juce::Component,
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
    
    void timerCallback() override;
    
    void PadEvent(int row, int column, int id, int value) override;

private:
    //==============================================================================
    // Your private member variables go here...
    Component drawComponent;
    int currentValues[48] {0};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
