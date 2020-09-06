/*
  ==============================================================================

    MantaPadDrawer.h
    Created: 15 Jul 2020 7:19:55pm
    Author:  Christopher Jacoby

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class MantaPadDrawer  : public juce::Component,
                        public juce::ValueTree::Listener
{
public:
    MantaPadDrawer(const juce::ValueTree& padState) : padState(padState)
    {
        // In your constructor, you should add any child components, and
        // initialise any special settings that your component needs.

    }

    ~MantaPadDrawer() override
    {
    }

    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds();
        int rowSize = bounds.getHeight() / 6;
        int columnSize = bounds.getWidth() / 8;
        
        // (Our component is opaque, so we must completely fill the background with a solid colour)
        juce::Colour bgColor = getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId);
        g.fillAll (bgColor);
        //    g.fillAll (bgColor.withAlpha ((float)currentValue / 255));
        
        g.setFont (juce::Font (16.0f));
        g.setColour (juce::Colours::white);
        g.drawText ("Hello World!", getLocalBounds(), juce::Justification::centred, true);
        
        juce::Colour fillColor = juce::Colours::orange;
        for (auto i = 0; i < 6; ++i) {
            auto rowBox = bounds.removeFromBottom (rowSize);
            
            for (auto j = 0; j < 8; ++j) {
                int padID = i * 8 + j;
                auto padValueTree = padState.getChildWithName ("pad" + juce::String(padID));
                float padValue = padValueTree.getProperty ("value");
                
                auto cellBox = rowBox.removeFromLeft (columnSize);
//                auto padValue = padState[i * 8 + j];
                
                g.setColour (fillColor.withAlpha (padValue / 255.0f));
                g.fillRect (cellBox);
                if (padValue > 0)
                {
                    g.setColour (juce::Colours::white);
                    g.drawText (juce::String(padValue), cellBox, juce::Justification::centred, true);
                }
            }
        }
    }

    void resized() override
    {
        // This method is where you should set the bounds of any child
        // components that your component contains..

    }
    
    void valueTreePropertyChanged (juce::ValueTree& v, const juce::Identifier& vtid) override
    {
        repaint();
    }

private:
    juce::ValueTree padState;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MantaPadDrawer)
};
