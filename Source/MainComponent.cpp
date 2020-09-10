#include "MainComponent.h"
#include <iostream>


static juce::ValueTree createRootValueTree()
{
    juce::ValueTree root("manta");
    
    juce::ValueTree pads ("pads");
    for (int i = 0; i < 48; ++i)
    {
        juce::ValueTree pad ("pad" + juce::String(i));
        pad.setProperty ("value", 0, nullptr);
        pad.setProperty ("channel", -1, nullptr);
        pads.appendChild (pad, nullptr);
    }
    root.appendChild (pads, nullptr);
    
    juce::ValueTree sliders ("sliders");
    for (int i = 0; i < 2; ++i)
    {
        juce::ValueTree slider ("slider" + juce::String(i));
        slider.setProperty ("value", 0.0, nullptr);
        slider.setProperty ("normalizedValue", 0.0, nullptr);
        sliders.appendChild (slider, nullptr);
    }
    root.appendChild (sliders, nullptr);
    
    juce::ValueTree buttons ("buttons");
    root.appendChild (buttons, nullptr);
    
    return root;
}

AudioDeviceDrawer::AudioDeviceDrawer(juce::AudioDeviceManager& deviceManager)
: deviceManager(deviceManager), audioSetupComp(deviceManager,
                                               0,     // minimum input channels
                                               256,   // maximum input channels
                                               0,     // minimum output channels,
                                               256,   // maximum output channels
                                               false, // ability to select midi inputs
                                               false, // ability to select midi output device
                                               false, // treat channels as stereo pairs
                                               false) // hide advanced options
{
    addAndMakeVisible (&audioSetupComp);
    addAndMakeVisible (&diagnosticsBox);
    
    diagnosticsBox.setMultiLine (true);
    diagnosticsBox.setReturnKeyStartsNewLine (true);
    diagnosticsBox.setReadOnly (true);
    diagnosticsBox.setScrollbarsShown (true);
    diagnosticsBox.setCaretVisible (false);
    diagnosticsBox.setPopupMenuEnabled (true);
    diagnosticsBox.setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x32ffffff));
    diagnosticsBox.setColour (juce::TextEditor::outlineColourId,    juce::Colour (0x1c000000));
    diagnosticsBox.setColour (juce::TextEditor::shadowColourId,     juce::Colour (0x16000000));
}

AudioDeviceDrawer::~AudioDeviceDrawer()
{
    
}

void AudioDeviceDrawer::paint (juce::Graphics& g)
{
    g.setColour (juce::Colours::grey);
    g.fillRect (getLocalBounds().removeFromRight (proportionOfWidth (0.4f)));
}

void AudioDeviceDrawer::resized()
{
    auto rect = getLocalBounds();
    
    audioSetupComp.setBounds (rect.removeFromLeft (proportionOfWidth (0.6f)));
    rect.reduce (10, 10);
    
    //    auto topLine (rect.removeFromTop (20));
    //    cpuUsageLabel.setBounds (topLine.removeFromLeft (topLine.getWidth() / 2));
    //    cpuUsageText .setBounds (topLine);
    rect.removeFromTop (20);
    
    diagnosticsBox.setBounds (rect);
}

juce::String AudioDeviceDrawer::getListOfActiveBits (const juce::BigInteger& b)
{
    juce::StringArray bits;
    
    for (auto i = 0; i <= b.getHighestBit(); ++i)
        if (b[i])
            bits.add (juce::String (i));
    
    return bits.joinIntoString (", ");
}

void AudioDeviceDrawer::dumpDeviceInfo()
{
    logMessage ("--------------------------------------");
    logMessage ("Current audio device type: " + (deviceManager.getCurrentDeviceTypeObject() != nullptr
                                                 ? deviceManager.getCurrentDeviceTypeObject()->getTypeName()
                                                 : "<none>"));
    
    if (auto* device = deviceManager.getCurrentAudioDevice())
    {
        logMessage ("Current audio device: "   + device->getName().quoted());
        logMessage ("Sample rate: "    + juce::String (device->getCurrentSampleRate()) + " Hz");
        logMessage ("Block size: "     + juce::String (device->getCurrentBufferSizeSamples()) + " samples");
        logMessage ("Bit depth: "      + juce::String (device->getCurrentBitDepth()));
        logMessage ("Input channel names: "    + device->getInputChannelNames().joinIntoString (", "));
        logMessage ("Active input channels: "  + getListOfActiveBits (device->getActiveInputChannels()));
        logMessage ("Output channel names: "   + device->getOutputChannelNames().joinIntoString (", "));
        logMessage ("Active output channels: " + getListOfActiveBits (device->getActiveOutputChannels()));
    }
    else
    {
        logMessage ("No audio device open");
    }
}

void AudioDeviceDrawer::logMessage (const juce::String& m)
{
    diagnosticsBox.moveCaretToEnd();
    diagnosticsBox.insertTextAtCaret (m + juce::newLine);
}

//==============================================================================
MainComponent::MainComponent()
  : state(createRootValueTree()),
    leftPanel(*this),
    padDrawerComponent(state.getChildWithName("pads"))
{
    setSize (800, 600);
    state.addListener (&padDrawerComponent);
//    state.addListener (&bufferManager);
    
    startTimer(5);
    Connect();
    
    // Initialize the midi output
    virtualMidiOutput = juce::MidiOutput::createNewDevice ("Manta");
    zoneLayout.reset (new juce::MPEZoneLayout());
    zoneLayout->setLowerZone(15);
    mpeChannelAssigner.reset (new juce::MPEChannelAssigner (zoneLayout->getLowerZone()));
    
    
    levelSlider.setRange (0.0, 0.9);
    levelSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 100, 20);
    auto gainValue = state.getChildWithName("sliders").getChildWithName("slider0").getPropertyAsValue("normalizedValue", nullptr);
    levelSlider.getValueObject().referTo(gainValue);
    levelLabel.setText ("Noise Level", juce::dontSendNotification);
    audioSettings.setButtonText("Settings");
    
    addAndMakeVisible (headerPanel);
    addAndMakeVisible (audioSettings);
    addAndMakeVisible (leftPanel);
    leftPanel.addListener (this);
    
    addAndMakeVisible (padDrawerComponent);
    addAndMakeVisible (levelSlider);
    addAndMakeVisible (levelLabel);
    
    audioSettings.onClick = [this] { showAudioWindow(); };
    if (juce::RuntimePermissions::isRequired (juce::RuntimePermissions::recordAudio)
        && ! juce::RuntimePermissions::isGranted (juce::RuntimePermissions::recordAudio))
    {
        juce::RuntimePermissions::request (juce::RuntimePermissions::recordAudio,
                                           [&] (bool granted) { setAudioChannels (granted ? 2 : 0, 2); });
    }
    else
    {
        // Specify the number of input and output channels that we want to open
        setAudioChannels (2, 2);
    }
}

MainComponent::~MainComponent()
{
    if (audioDialogWindow != nullptr)
    {
        audioDialogWindow->exitModalState (0);
        
        /// we are shutting down; can't wait for the message manager
        // to eventually delete this
        delete audioDialogWindow;
    }

    shutdownAudio();
}

void MainComponent::showAudioWindow()
{
    bool native = true;
    audioDialogWindow = new AudioSetupModal("Audio Settings", juce::Colours::grey, juce::DocumentWindow::allButtons, deviceManager);
    
    juce::Rectangle<int> area(0, 0, 300, 400);
    
    juce::RectanglePlacement placement ((native ? juce::RectanglePlacement::xLeft
                                         : juce::RectanglePlacement::xRight)
                                        | juce::RectanglePlacement::yTop
                                        | juce::RectanglePlacement::doNotResize);
    
    auto result = placement.appliedTo (area, juce::Desktop::getInstance().getDisplays()
                                       .getMainDisplay().userArea.reduced (20));
    audioDialogWindow->setBounds (result);
    
    audioDialogWindow->setResizable (true, ! native);
    audioDialogWindow->setUsingNativeTitleBar (native);
    audioDialogWindow->setVisible (true);
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
//    auto bounds = getLocalBounds();
////    int rowSize = bounds.getHeight() / 6;
////    int columnSize = bounds.getWidth() / 8;
    
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    juce::Colour bgColor = getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId);
    g.fillAll (bgColor);
//    g.fillAll (bgColor.withAlpha ((float)currentValue / 255));

    g.setFont (juce::Font (16.0f));
    g.setColour (juce::Colours::white);
    g.drawText ("Hello World!", getLocalBounds(), juce::Justification::centred, true);
    
//    juce::Colour fillColor = juce::Colours::orange;
//    for (auto i = 0; i < 6; ++i) {
//        auto rowBox = bounds.removeFromBottom (rowSize);
//
//        for (auto j = 0; j < 8; ++j) {
//            auto cellBox = rowBox.removeFromLeft (columnSize);
//
//            g.setColour (fillColor.withAlpha (padState[i * 8 + j] / 255.0f));
//            g.fillRect (cellBox);
//        }
//    }
}

void MainComponent::resized()
{
    auto bounds = getLocalBounds();
    
    int headerHeight = 30;
    auto headerBounds = bounds.removeFromTop (headerHeight);
    audioSettings.setBounds      (headerBounds.removeFromLeft (200));
    headerPanel.setBounds        (headerBounds);
    
    leftPanel  .setBounds        (bounds.removeFromLeft (200));
    
    auto levelBox = bounds.removeFromBottom (20);
    levelLabel .setBounds        (levelBox.removeFromLeft (200));
    levelSlider.setBounds        (levelBox);
    
    padDrawerComponent.setBounds (bounds);
    // This is called when the MainComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
}

void MainComponent::mpeMidiReset()
{
    juce::MidiBuffer buffer;
    buffer.addEvents (juce::MPEMessages::clearAllZones (), 0, 0, 0);
    buffer.addEvents (juce::MPEMessages::setLowerZone(), 0, 0, 0);
    
    virtualMidiOutput->sendBlockOfMessagesNow(buffer);
}

void MainComponent::mpeControlTypeChange(int type)
{
    mpeControlType = type;
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    // This function will be called when the audio device is started, or when
    // its settings (i.e. sample rate, block size, etc) are changed.
    
    // You can use this function to initialise any resources you might need,
    // but be careful - it will be called on the audio thread, not the GUI thread.
    
    // For more details, see the help for AudioProcessor::prepareToPlay()
//    bufferManager.prepareToPlay (sampleRate, samplesPerBlockExpected);
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    auto* device = deviceManager.getCurrentAudioDevice();
    auto activeInputChannels = device->getActiveInputChannels();
    auto activeOutputChannels = device->getActiveOutputChannels();
    
    auto maxInputChannels  = activeInputChannels .getHighestBit() + 1;
    auto maxOutputChannels = activeOutputChannels.getHighestBit() + 1;

    // Your audio-processing code goes here!
    auto level = (float) levelSlider.getValue();
    
    for (auto channel = 0; channel < bufferToFill.buffer->getNumChannels(); ++channel)
    {
        auto* buffer = bufferToFill.buffer->getWritePointer (channel, bufferToFill.startSample);
        
        for (auto sample = 0; sample < bufferToFill.numSamples; ++sample)
        {
            auto noise = random.nextFloat() * 2.0f - 1.0f;
            buffer[sample] = noise * level;
        }
    }
    
    // For more details, see the help for AudioProcessor::getNextAudioBlock()
    
    // Right now we are not producing any data, in which case we need to clear the buffer
    // (to prevent the output of random noise)

    
    
    //    bufferToFill.clearActiveBufferRegion();
    
    
}

void MainComponent::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.
    
    // For more details, see the help for AudioProcessor::releaseResources()
}

void MainComponent::timerCallback()
{
    Manta::HandleEvents();
}

void MainComponent::PadVelocityEvent(int row, int column, int id, int velocity)
{
    padVelocityMax = (int)fmax(velocity, padVelocityMax);
    float floatVelocity = velocity / (float)padVelocityMax;
    
    int midiNote = midiNoteForPad(id);
    
    juce::MidiMessage note;
    if (velocity > 0)
    {
        int channel = getOrSetNewChannel(id, midiNote);
        std::cout << "noteOn event: " << id << " v" << floatVelocity << " c" << channel << std::endl;
        note = juce::MidiMessage::noteOn (channel, midiNote, floatVelocity);
    }
    else
    {
        int currentChannel = getActiveChannel(id);
        
        mpeChannelAssigner->noteOff (midiNote);
        note = juce::MidiMessage::noteOff(currentChannel, midiRoot + id);
        
        std::cout << "noteOff event: " << id << " c" << currentChannel << std::endl;
        
        // Reset the channel on noteoff to -1
        setActiveChannel(id, -1);
    }
    
    virtualMidiOutput->sendMessageNow (note);
}

void MainComponent::PadEvent(int row, int column, int id, int value)
{
    int padMax = 220;
//    std::cout << "pad event: " << row << " " << column << " " << id << " " << value << std::endl;
    
    // Send poly aftertouch for this note
    int midiNote = midiNoteForPad(id);
    int currentChannel = getOrSetNewChannel(id, midiNote);
    juce::MidiMessage continuousChange;
    switch (mpeControlType)
    {
        case 1: {
            int aftertouchAmount = juce::jlimit(0, 128, (int)((value / (float)padMax) * 128));
            continuousChange = juce::MidiMessage::aftertouchChange(currentChannel, id, aftertouchAmount);
            break;
        }
        case 2: {
            int channelPressureAmount = juce::jlimit(0, 128, (int)((value / (float)padMax) * 128));
            continuousChange = juce::MidiMessage::channelPressureChange(currentChannel, channelPressureAmount);
            break;
        }
        case 3: {
            int pitchWheelPosition = juce::jlimit(0, 0x4000, (int)((value / (float)padMax) * 0x4000));  // 0x4000 is pitch wheel max
            continuousChange = juce::MidiMessage::pitchWheel(currentChannel, pitchWheelPosition);
            break;
        }
        case 4: {
            int controllerAmount = juce::jlimit(0, 128, (int)((value / (float)padMax) * 128));
            continuousChange = juce::MidiMessage::controllerEvent(currentChannel, 74, controllerAmount);
            break;
        }
        default: break;
    }
    
    virtualMidiOutput->sendMessageNow (continuousChange);
    
    if (state.isValid())
    {
        auto padsNode = state.getChildWithName ("pads");
        if (padsNode.isValid() && padsNode.getNumChildren())
        {
            juce::Identifier padID = "pad" + juce::String(id);
            auto padNode = padsNode.getChildWithName (padID);
            padNode.setProperty ("value", value, nullptr);
        }
    }
//    padState[id] = value;
//    repaint();
}


void MainComponent::SliderEvent(int id, int value)
{
    std::cout << "slider event: " << id << " " << value << std::endl;
    juce::Identifier sliderID = "slider" + juce::String(id);
    
    // Value is 65535 when not touching.
    if (value < 4096 && state.isValid())
    {
        auto slidersNode = state.getChildWithName ("sliders");
        if (slidersNode.isValid() && slidersNode.getNumChildren())
        {
            auto sliderNode = slidersNode.getChildWithName (sliderID);
            
            if (value < 50)
                value = 0;
            
            sliderNode.setProperty ("value", value, nullptr);
            sliderNode.setProperty ("normalizedValue", value / 4096.0, nullptr);
        }
    }
}

int MainComponent::midiNoteForPad (int padID)
{
    return midiRoot + padID;
}


int MainComponent::getOrSetNewChannel(int id, int midiNote)
{
    int currentChannel = getActiveChannel(id);
    
    if (currentChannel == -1)
    {
        currentChannel = mpeChannelAssigner->findMidiChannelForNewNote (midiNote);
        setActiveChannel (id, currentChannel);
    }
    
    return currentChannel;
}


void MainComponent::setActiveChannel(int id, int channel)
{
    juce::Identifier padID = "pad" + juce::String(id);
    
    if (state.isValid())
    {
        auto padsNode = state.getChildWithName ("pads");
        if (padsNode.isValid())
        {
            auto padNode = padsNode.getChildWithName (padID);
            padNode.setProperty ("channel", channel, nullptr);
        }
    }
}


int MainComponent::getActiveChannel(int id)
{
    int channel = -1;
    juce::Identifier padID = "pad" + juce::String(id);
    
    if (state.isValid())
    {
        auto padsNode = state.getChildWithName ("pads");
        if (padsNode.isValid())
        {
            auto padNode = padsNode.getChildWithName (padID);
            
            channel = padNode.getProperty ("channel");
        }
    }
    return channel;
}
