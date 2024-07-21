#pragma once

namespace cctn
{
namespace song
{

//==============================================================================
struct SongEditorNoteBasic
{
    double startPositionInSeconds;
    double endPositionInSeconds;
    juce::int64 noteNumber;
    juce::String lyric;
    juce::String extraPhoneme;

    JUCE_LEAK_DETECTOR(SongEditorNoteBasic)
};

//==============================================================================
struct SongEditorNoteExtended
    : public SongEditorNoteBasic
{
    bool isSelected;

    JUCE_LEAK_DETECTOR(SongEditorNoteExtended)
};

//==============================================================================
struct SongEditorDocumentData
{
    juce::Array<SongEditorNoteExtended> notes;

    JUCE_LEAK_DETECTOR(SongEditorDocumentData)
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

}
}
