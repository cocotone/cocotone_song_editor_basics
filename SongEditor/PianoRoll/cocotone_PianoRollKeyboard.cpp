namespace cctn
{
namespace song
{

//==============================================================================
PianoRollKeyboard::PianoRollKeyboard()
{
    numVisibleWhiteAndBlackKeys = 24;
    numVisibleWhiteKeys = 14;

    dummyMidiKeyboardState = std::make_unique<juce::MidiKeyboardState>();

    midiKeyboardComponent = std::make_unique<juce::MidiKeyboardComponent>(
        *dummyMidiKeyboardState,
        juce::KeyboardComponentBase::Orientation::verticalKeyboardFacingRight);
    addAndMakeVisible(midiKeyboardComponent.get());

    midiKeyboardComponent->setScrollButtonsVisible(false);
    midiKeyboardComponent->setAvailableRange(48, 48 + numVisibleWhiteAndBlackKeys);
    midiKeyboardComponent->setBlackNoteWidthProportion(0.6f);
    midiKeyboardComponent->setBlackNoteLengthProportion(1.0f);
}

PianoRollKeyboard::~PianoRollKeyboard()
{
    midiKeyboardComponent.reset();
    dummyMidiKeyboardState.reset();
}

//==============================================================================
void PianoRollKeyboard::setVisibleBottomKeyNoteNumber(int noteNumber)
{
    midiKeyboardComponent->setAvailableRange(noteNumber, noteNumber + numVisibleWhiteAndBlackKeys);
    repaint();
}

int PianoRollKeyboard::getVisibleKeySize() const
{
    return numVisibleWhiteAndBlackKeys;
}

//==============================================================================
juce::Rectangle<float> PianoRollKeyboard::getRectangleForKeyForwarding(int midiNoteNumber) const
{
    return midiKeyboardComponent->getRectangleForKey(midiNoteNumber);
}

juce::Range<float> PianoRollKeyboard::getPositionRangeForPianoRollGridHorizontalWidth(int midiNoteNumber) const
{
    const auto my_key_start_position = midiKeyboardComponent->getKeyStartPosition(midiNoteNumber);
    const auto prev_key_start_position = midiKeyboardComponent->getKeyStartPosition(midiNoteNumber - 1);
    const auto next_key_start_position = midiKeyboardComponent->getKeyStartPosition(midiNoteNumber + 1);

    // In case of this is black 
    if (juce::MidiMessage::isMidiNoteBlack(midiNoteNumber))
    {
        return { my_key_start_position, my_key_start_position + midiKeyboardComponent->getBlackNoteWidth() };
    }

    // In case of this is white and prveious is black and next is black
    if (juce::MidiMessage::isMidiNoteBlack(midiNoteNumber - 1) && juce::MidiMessage::isMidiNoteBlack(midiNoteNumber + 1))
    {
        return { prev_key_start_position + midiKeyboardComponent->getBlackNoteWidth(), next_key_start_position };
    }

    // In case of this is white and prveious is black and next is white
    if (juce::MidiMessage::isMidiNoteBlack(midiNoteNumber - 1) && !juce::MidiMessage::isMidiNoteBlack(midiNoteNumber + 1))
    {
        return { prev_key_start_position + midiKeyboardComponent->getBlackNoteWidth(), my_key_start_position + midiKeyboardComponent->getKeyWidth() };
    }

    // In case of prveious is white and this is white and next is black
    if (!juce::MidiMessage::isMidiNoteBlack(midiNoteNumber - 1) && juce::MidiMessage::isMidiNoteBlack(midiNoteNumber + 1))
    {
        return { my_key_start_position, next_key_start_position };
    }

    return { my_key_start_position, next_key_start_position };
}

//==============================================================================
void PianoRollKeyboard::paint(juce::Graphics& g)
{
}

void PianoRollKeyboard::resized()
{
    const auto rect_area = getLocalBounds();
    midiKeyboardComponent->setBounds(rect_area);
    midiKeyboardComponent->setKeyWidth((float)rect_area.getHeight() / (float)numVisibleWhiteKeys);
}

}
}
