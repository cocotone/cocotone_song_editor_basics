#pragma once

namespace cctn
{
namespace song
{

//==============================================================================
struct PianoRollNote
{
    double startInSeconds;
    double endInSeconds;
    juce::int64 noteNumber;
    juce::String lyric;
    juce::String extraPhoneme;
    bool isSelected;

    JUCE_LEAK_DETECTOR(PianoRollNote)
};

//==============================================================================
struct PianoRollPreviewData
{
    juce::Array<PianoRollNote> notes;

    JUCE_LEAK_DETECTOR(PianoRollPreviewData)
};

//==============================================================================
struct QueryForFindPianoRollNote
{
    double timeInSeconds{ 0.0 };
    int noteNumber;

    JUCE_LEAK_DETECTOR(QueryForFindPianoRollNote)
};

//==============================================================================
struct QueryForAddPianoRollNote
{
    double startTimeInSeconds{ 0.0 };
    double endTimeInSeconds{ 0.0 };
    int noteNumber;

    JUCE_LEAK_DETECTOR(QueryForAddPianoRollNote)
};

//==============================================================================
class IPianoRollPreviewDataSource
{
public:
    //==============================================================================
    virtual ~IPianoRollPreviewDataSource() = default;

    //==============================================================================
    virtual std::optional<cctn::song::PianoRollPreviewData> getPianoRollPreviewData() = 0;

private:
    //==============================================================================

    JUCE_LEAK_DETECTOR(IPianoRollPreviewDataSource)
};


}
}
