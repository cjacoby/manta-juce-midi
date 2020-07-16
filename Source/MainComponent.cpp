#include "MainComponent.h"
#include <iostream>

//==============================================================================
MainComponent::MainComponent()
{
    setSize (600, 400);
    
    startTimer(5);
    
    Connect();
    
    addAndMakeVisible(&drawComponent);
}

MainComponent::~MainComponent()
{
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
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
            auto cellBox = rowBox.removeFromLeft (columnSize);
    
            g.setColour (fillColor.withAlpha (currentValues[i * 8 + j] / 255.0f));
            g.fillRect (cellBox);
        }
    }
}

void MainComponent::resized()
{
    // This is called when the MainComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
}

void MainComponent::timerCallback()
{
    Manta::HandleEvents();
}

void MainComponent::PadEvent(int row, int column, int id, int value)
{
    std::cout << "pad event: " << row << " " << column << " " << id << " " << value << std::endl;
    currentValues[id] = value;
    repaint();
}
