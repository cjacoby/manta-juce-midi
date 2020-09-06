/*
  ==============================================================================

    SettingsPanel.h
    Created: 15 Jul 2020 7:32:39pm
    Author:  Christopher Jacoby

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class SettingsPanel  : public juce::Component
{
public:
    SettingsPanel(juce::AudioAppComponent& parent) : parent(parent)
    {
        // In your constructor, you should add any child components, and
        // initialise any special settings that your component needs.
        addAndMakeVisible (&openButton);
        openButton.setButtonText ("Open ...");
        openButton.onClick = [this] { loadAudioFile(); };
    }

    ~SettingsPanel() override
    {
    }

    void paint (juce::Graphics& g) override
    {
        /* This demo code just fills the component's background and
           draws some placeholder text to get you started.

           You should replace everything in this method with your own
           drawing code..
        */

        juce::Colour bgColor = juce::Colours::chartreuse;
        g.fillAll (bgColor);   // clear the background

        g.setColour (juce::Colours::grey);
        g.drawRect (getLocalBounds(), 1);   // draw an outline around the component

        g.setColour (juce::Colours::white);
        g.setFont (14.0f);
        g.drawText ("SettingsPanel", getLocalBounds(),
                    juce::Justification::centred, true);   // draw some placeholder text
    }

    void resized() override
    {
        // This method is where you should set the bounds of any child
        // components that your component contains..
        auto bounds = getLocalBounds();
        
        openButton.setBounds (bounds.removeFromTop(20).reduced(2));
    }
    
    void loadAudioFile()
    {
        parent.shutdownAudio();
        
        juce::FileChooser chooser ("Select a Wave file shorter than 2 seconds to play...",
                                   {},
                                   "*.wav");
        
        if (chooser.browseForFileToOpen())
        {
            auto file = chooser.getResult();
            std::unique_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor (file)); // [2]
            
            if (reader.get() != nullptr)
            {
                auto duration = (float) reader->lengthInSamples / reader->sampleRate;               // [3]
                
                if (duration < 2)
                {
                    fileBuffer.setSize ((int) reader->numChannels, (int) reader->lengthInSamples);  // [4]
                    reader->read (&fileBuffer,                                                      // [5]
                                  0,                                                                //  [5.1]
                                  (int) reader->lengthInSamples,                                    //  [5.2]
                                  0,                                                                //  [5.3]
                                  true,                                                             //  [5.4]
                                  true);                                                            //  [5.5]
                    position = 0;                                                                   // [6]
                    parent.setAudioChannels (0, (int) reader->numChannels);                                // [7]
                }
                else
                {
                    // handle the error that the file is 2 seconds or longer..
                }
            }
        }
    }

private:
    juce::AudioAppComponent&        parent;
    juce::AudioFormatManager        formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource      transportSource;
//    juce::TransportState state;
    juce::AudioSampleBuffer         fileBuffer;
    int position = 0;
    
    juce::TextButton                openButton;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SettingsPanel)
};
