/*
  ==============================================================================

    BufferManager.h
    Created: 4 Sep 2020 9:08:52pm
    Author:  Christopher Jacoby

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

using namespace juce;

class BufferManager : public AudioProcessor
{
public:
    BufferManager()
    {
        
    }
    
    const String getName() const override { return "BufferManager"; }
    
    void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) override
    {
    }
    
    void releaseResources() override {}
    
    void processBlock(AudioBuffer<float> &buffer, MidiBuffer &midiMessages) override
    {
    }
    
    double getTailLengthSeconds() const override
    {
        return 0.0;
    }
    
    /** Returns true if the processor wants MIDI messages. */
    bool acceptsMidi() const override {return false;}
    
    /** Returns true if the processor produces MIDI messages. */
    bool producesMidi() const override {return false;}
    
private:
};
