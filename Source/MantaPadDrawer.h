/*
  ==============================================================================

    MantaPadDrawer.h
    Created: 15 Jul 2020 7:19:55pm
    Author:  Christopher Jacoby

  ==============================================================================
 SVG PATH Def
 
 M 0 1.25 L 0 5.25 L 1 5.75
 L 1 1.75 L 2 1.25 L 2 6.25
 L 3 6.75 L 4 6.25 L 4 1.25
 L 5 1.75 L 5 5.75 L 6 5.25
 L 6 1.25 L 3.5 0 L 3.5 6
 L 3 6.25 L 2.5 6 L 2.5 0 L 0 1.25
*/

#pragma once

#include <JuceHeader.h>

inline juce::Path getButtonPath()
{
    juce::Path mantaPath;
    mantaPath.clear();
    mantaPath.startNewSubPath (0.0f, 1.25f);
    mantaPath.lineTo          (0.0f, 5.25f);
    mantaPath.lineTo          (1.0f, 5.75f);
    mantaPath.lineTo          (1.0f, 1.75f);
    mantaPath.lineTo          (2.0f, 1.25f);
    mantaPath.lineTo          (2.0f, 6.25f);
    mantaPath.lineTo          (3.0f, 6.75f);
    mantaPath.lineTo          (4.0f, 6.25f);
    mantaPath.lineTo          (4.0f, 1.25f);
    mantaPath.lineTo          (5.0f, 1.75f);
    mantaPath.lineTo          (5.0f, 5.75f);
    mantaPath.lineTo          (6.0f, 5.25f);
    mantaPath.lineTo          (6.0f, 1.25f);
    mantaPath.lineTo          (3.5f, 0.0f);
    mantaPath.lineTo          (3.5f, 6.0f);
    mantaPath.lineTo          (3.0f, 6.25f);
    mantaPath.lineTo          (2.5f, 6.0f);
    mantaPath.lineTo          (2.5f, 0.0f);
    mantaPath.lineTo          (0.0f, 1.25f);
    
    return mantaPath;
}

class PadButtonComponent
: public juce::ShapeButton
{
public:
    /**
     * @param padState ValueTree node of this pad.
     */
    PadButtonComponent(int thisPadID, const juce::ValueTree& padState)
    : juce::ShapeButton("pad" + juce::String(thisPadID),
                        juce::Colours::white.withAlpha(0.2f),
                        juce::Colours::yellow.withAlpha(0.4f),
                        juce::Colours::yellow),
      padState(padState),
      defaultPath(getButtonPath())
    {
    }
    
    ~PadButtonComponent() {}
    
    void paint (juce::Graphics& g) override
    {
//        juce::ShapeButton::paint(g);
//        juce::Colour bgColor = juce::Colours::blue;
//        g.fillAll (bgColor);

        auto bounds = getLocalBounds();
        float padValue = padState.getProperty ("value");

        g.setColour (juce::Colours::white);
        g.drawText (juce::String(padValue), bounds, juce::Justification::centred, true);
    }
    
    void resized() override
    {
        auto bounds = getLocalBounds();
        resizedPath = juce::Path(defaultPath);
        resizedPath.applyTransform (resizedPath.getTransformToScaleToFit (bounds.reduced(2).toFloat(), true));
        setShape (resizedPath, false, true, true);
        juce::ShapeButton::resized();
    }

private:
    int             thisPadID;
    juce::ValueTree padState;
    juce::Path      defaultPath;
    juce::Path      resizedPath;
};


//==============================================================================
/*
*/
class MantaPadContainerComponent
 : public juce::Component,
   public juce::ValueTree::Listener
{
public:
    MantaPadContainerComponent(const juce::ValueTree& padState) : padState(padState)
    {
        // In your constructor, you should add any child components, and
        // initialise any special settings that your component needs.
        padButtons.clear();
        
        if (padState.isValid())
        {
            for (int i = 0; i < 48; ++i)
            {
                padButtons.add(new PadButtonComponent(i, padState.getChildWithName ("pad" + juce::String(i))));
                addAndMakeVisible(padButtons.getLast());
            }
        }
    }

    ~MantaPadContainerComponent() override
    {
    }

    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds();
        int rowSize = bounds.getHeight() / 6;
        int columnSize = bounds.getWidth() / 8;
        
        // (Our component is opaque, so we must completely fill the background with a solid colour)
//        juce::Colour bgColor = getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId);
//        g.fillAll (bgColor);
        //    g.fillAll (bgColor.withAlpha ((float)currentValue / 255));
        
//        g.setFont (juce::Font (16.0f));
//        g.setColour (juce::Colours::white);
        
//        juce::Colour fillColor = juce::Colours::orange;
//        for (auto i = 0; i < 6; ++i) {
//            auto rowBox = bounds.removeFromBottom (rowSize);
//
//            for (auto j = 0; j < 8; ++j) {
//                int padID = i * 8 + j;
//                auto padValueTree = padState.getChildWithName ("pad" + juce::String(padID));
//                float padValue = padValueTree.getProperty ("value");
//
//                auto cellBox = rowBox.removeFromLeft (columnSize);
////                auto padValue = padState[i * 8 + j];
//
////                g.setColour (fillColor.withAlpha (padValue / 255.0f));
////                g.fillRect (cellBox);
//
//                juce::Path mantaPathCopy (mantaPath);
//                mantaPathCopy.applyTransform (mantaPathCopy.getTransformToScaleToFit (cellBox.reduced(2).toFloat(), true));
//
//                g.setColour (juce::Colours::darkkhaki);
//                g.strokePath (mantaPathCopy, juce::PathStrokeType (1.0f));
//                if (padValue > 0)
//                {
//                    g.setColour (fillColor.withAlpha (padValue / 225.0f));
//                    g.fillPath (mantaPathCopy);
//
//                    g.setColour (juce::Colours::white);
//                    g.drawText (juce::String(padValue), cellBox, juce::Justification::centred, true);
//                }
//            }
//        }
    }

    void resized() override
    {
        auto bounds = getLocalBounds();
        int rowSize = bounds.getHeight() / 6;
        int columnSize = bounds.getWidth() / 8;
//        juce::Rectangle<int> buttonSize(columnSize, rowSize);
        
        if (padButtons.size())
        {
            for (auto i = 0; i < 6; ++i)
            {
                auto rowBox = bounds.removeFromBottom (rowSize);

                for (auto j = 0; j < 8; ++j)
                {
                    int padID = i * 8 + j;
                    auto cellBox = rowBox.removeFromLeft (columnSize);
                    padButtons[padID]->setBounds (cellBox.reduced (2));
                }
            }
        }
    }
    
    void valueTreePropertyChanged (juce::ValueTree& v, const juce::Identifier& vtid) override
    {
        if (v.getType().toString().startsWith("pad") && vtid == juce::Identifier("value"))
        {
            std::cout << v.getType().toString() << " " << vtid.toString() << std::endl;
            repaint();
        }
    }

private:
    juce::OwnedArray<PadButtonComponent>    padButtons;
    juce::ValueTree                         padState;
    
    juce::Colour                            offColour{juce::Colours::black};
    juce::Colour                            amberColour{juce::Colours::orange};
    juce::Colour                            redColour{juce::Colours::red};
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MantaPadContainerComponent)
};
