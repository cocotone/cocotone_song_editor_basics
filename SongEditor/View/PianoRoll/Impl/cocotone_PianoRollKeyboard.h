#pragma once

namespace cctn
{
namespace song
{

//==============================================================================
class NonPropotionalKeyboardComponent;

//==============================================================================
class PianoRollKeyboard final
    : public juce::Component
{
public:
    //==============================================================================
    PianoRollKeyboard(int numVisibleOctaves);
    ~PianoRollKeyboard() override;

    //==============================================================================
    void setVisibleBottomKeyNoteNumber(int noteNumber);
    int getVisibleKeySize() const;

    //==============================================================================
    juce::Rectangle<float> getRectangleForKeyForwarding(int midiNoteNumber) const;
    juce::Range<float> getPositionRangeForPianoRollGridHorizontalRow(int midiNoteNumber) const;
    juce::KeyboardComponentBase::NoteAndVelocity getNoteAndVelocityAtPositionForwarding(juce::Point<float> position, bool includeChildComponents = false) const;

private:
    //==============================================================================
    void paint(juce::Graphics& g) override;
    void resized() override;

    //==============================================================================
    std::unique_ptr<cctn::song::NonPropotionalKeyboardComponent> midiKeyboardComponent;
    std::unique_ptr<juce::MidiKeyboardState> dummyMidiKeyboardState;

    // TODO: Move state value.
    juce::uint8 numVisibleWhiteAndBlackKeys;
    juce::uint8 numVisibleWhiteKeys;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PianoRollKeyboard)
};

}
}
