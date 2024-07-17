namespace cctn
{
namespace song
{

//==============================================================================
class NonPropotionalKeyboardComponent
    : public juce::MidiKeyboardComponent
{
public:
    //==============================================================================
    NonPropotionalKeyboardComponent(juce::MidiKeyboardState& state, juce::MidiKeyboardComponent::Orientation orientation)
        : juce::MidiKeyboardComponent(state, orientation)
    {
    }

    ~NonPropotionalKeyboardComponent()
    {
    }

    //==============================================================================
    float NonPropotionalKeyboardComponent::getNonPropotinalKeyWidth() const
    {
        const auto width_of_octave = getKeyWidth() * 7.0f;
        const auto width_of_per_key = width_of_octave / 12.0f;
        return width_of_per_key;
    }

private:
    //==============================================================================
    juce::Range<float> NonPropotionalKeyboardComponent::getKeyPosition(int midiNoteNumber, float targetKeyWidth) const
    {
        const int num_keys_in_octave = 12;
        const auto octave = midiNoteNumber / 12;
        const auto note_number_ignore_octave = midiNoteNumber % 12;
        const auto width_of_octave = targetKeyWidth * 7.0f;
        const auto width_of_per_key = width_of_octave / 12.0f;
        const auto start_of_key = (float)octave * ((float)num_keys_in_octave * width_of_per_key) + ((float)note_number_ignore_octave * width_of_per_key);

        if (!(int)juce::MidiMessage::isMidiNoteBlack(note_number_ignore_octave))
        {
            // In case of white key
            if (note_number_ignore_octave == 0)
            {
                // C
                return { start_of_key, start_of_key + width_of_per_key + (width_of_per_key * 0.5f) };
            }
            else if (note_number_ignore_octave == 2)
            {
                // D
                return { start_of_key - (width_of_per_key * 0.5f), start_of_key + width_of_per_key + (width_of_per_key * 0.5f) };
            }
            else if (note_number_ignore_octave == 4)
            {
                // E
                return { start_of_key - (width_of_per_key * 0.5f), start_of_key + width_of_per_key };
            }
            else if (note_number_ignore_octave == 5)
            {
                // F
                return { start_of_key, start_of_key + width_of_per_key + (width_of_per_key * 0.5f) };
            }
            else if (note_number_ignore_octave == 7)
            {
                // G
                return { start_of_key - (width_of_per_key * 0.5f), start_of_key + width_of_per_key + (width_of_per_key * 0.5f) };
            }
            else if (note_number_ignore_octave == 9)
            {
                // A
                return { start_of_key - (width_of_per_key * 0.5f), start_of_key + width_of_per_key + (width_of_per_key * 0.5f) };
            }
            else if (note_number_ignore_octave == 11)
            {
                // B
                return { start_of_key - (width_of_per_key * 0.5f), start_of_key + width_of_per_key };
            }
        }

        return { start_of_key, start_of_key + width_of_per_key };
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NonPropotionalKeyboardComponent)
};

//==============================================================================
PianoRollKeyboard::PianoRollKeyboard()
{
    numVisibleWhiteAndBlackKeys = 24;
    numVisibleWhiteKeys = 14;

    dummyMidiKeyboardState = std::make_unique<juce::MidiKeyboardState>();

    midiKeyboardComponent = std::make_unique<cctn::song::NonPropotionalKeyboardComponent>(
        *dummyMidiKeyboardState,
        juce::KeyboardComponentBase::Orientation::verticalKeyboardFacingRight);
    addAndMakeVisible(midiKeyboardComponent.get());

    midiKeyboardComponent->setScrollButtonsVisible(false);
    midiKeyboardComponent->setAvailableRange(48, 48 + numVisibleWhiteAndBlackKeys);
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

juce::Range<float> PianoRollKeyboard::getPositionRangeForPianoRollGridHorizontalRow(int midiNoteNumber) const
{
    const auto my_key_start_position = midiKeyboardComponent->getKeyStartPosition(midiNoteNumber);
    const auto prev_key_start_position = midiKeyboardComponent->getKeyStartPosition(midiNoteNumber - 1);
    const auto next_key_start_position = midiKeyboardComponent->getKeyStartPosition(midiNoteNumber + 1);
    const auto non_propotional_key_width = midiKeyboardComponent->getNonPropotinalKeyWidth();

    // In case of this is black 
    if (juce::MidiMessage::isMidiNoteBlack(midiNoteNumber))
    {
        return { my_key_start_position, my_key_start_position + non_propotional_key_width };
    }

    // In case of this is white and prveious is black and next is black
    if (juce::MidiMessage::isMidiNoteBlack(midiNoteNumber - 1) && juce::MidiMessage::isMidiNoteBlack(midiNoteNumber + 1))
    {
        return { prev_key_start_position + non_propotional_key_width, next_key_start_position };
    }

    // In case of this is white and prveious is black and next is white
    if (juce::MidiMessage::isMidiNoteBlack(midiNoteNumber - 1) && !juce::MidiMessage::isMidiNoteBlack(midiNoteNumber + 1))
    {
        return { prev_key_start_position + non_propotional_key_width, next_key_start_position };
    }

    // In case of this is white and prveious is white and next is black
    if (!juce::MidiMessage::isMidiNoteBlack(midiNoteNumber - 1) && juce::MidiMessage::isMidiNoteBlack(midiNoteNumber + 1))
    {
        return { my_key_start_position, next_key_start_position };
    }

    // Fallback
    return { my_key_start_position, my_key_start_position + non_propotional_key_width };
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
