#pragma once

namespace cctn
{
namespace song
{

//==============================================================================
enum class NoteLength
{
    Whole,           // Whole note
    Half,            // Half note
    Quarter,         // Quarter note
    Eighth,          // Eighth note
    Sixteenth,       // Sixteenth note
    ThirtySecond,    // Thirty-second note
    SixtyFourth,     // Sixty-fourth note
    Triplet,         // Triplet
    DottedQuarter,   // Dotted quarter note
    DottedEighth     // Dotted eighth note
};

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
    int noteNumber{ -1 };
    bool snapToQuantizeGrid{ false };

    JUCE_LEAK_DETECTOR(QueryForAddPianoRollNote)
};

//==============================================================================
struct TimeSignature
{
public:
    //==============================================================================
    int bar{ 0 };
    int beat{ 0 };
    int tick{ 0 };

    //==============================================================================
    TimeSignature(int b, int bt, int t)
        : bar(b), beat(bt), tick(t)
    {}

private:
    //==============================================================================
    JUCE_LEAK_DETECTOR(TimeSignature)
};

//==============================================================================
struct TempoAndTimeSignature
{
public:
    //==============================================================================
    double bpm;
    int numerator;
    int denominator;

    //==============================================================================
    TempoAndTimeSignature(double b, int num, int denm)
        : bpm(b), numerator(num), denominator(denm)
    {}

private:
    //==============================================================================
    JUCE_LEAK_DETECTOR(TempoAndTimeSignature)
};

//==============================================================================
struct BeatTimePoint
{
public:
    //==============================================================================
    double beat{ 0.0 };
    double timeInSeconds{ 0.0 };
    NoteLength noteLength{ NoteLength::Quarter };

    //==============================================================================
    BeatTimePoint(double b, double t, NoteLength nl)
        : beat(b)
        , timeInSeconds(t)
        , noteLength(nl)
    {}

    BeatTimePoint(double b, double t)
        : beat(b)
        , timeInSeconds(t)
    {}

    // Copy
    BeatTimePoint(const BeatTimePoint& other)
        : beat(other.beat)
        , timeInSeconds(other.timeInSeconds)
        , noteLength(other.noteLength)
    {}

    // Move
    BeatTimePoint(BeatTimePoint&& other) noexcept
        : beat(std::exchange(other.beat, 0.0))
        , timeInSeconds(std::exchange(other.timeInSeconds, 0.0))
        , noteLength(std::exchange(other.noteLength, NoteLength::Quarter))
    {}

    //==============================================================================
    TimeSignature toTimeSignature(int beatsPerBar, int ticksPerBeat = 960) const
    {
        int totalTicks = static_cast<int>(std::round(beat * ticksPerBeat));
        int totalBeats = totalTicks / ticksPerBeat;

        int bar = totalBeats / beatsPerBar;
        int beatInBar = totalBeats % beatsPerBar;
        int tick = totalTicks % ticksPerBeat;

        return TimeSignature(bar, beatInBar, tick);
    }

    juce::String getFormattedTimeSignature(int beatsPerBar, bool showTick, int ticksPerBeat = 960) const
    {
        const auto time_signature = toTimeSignature(beatsPerBar, ticksPerBeat);

        if (showTick)
        {
            return juce::String::formatted("%d|%d|%03d", time_signature.bar + 1, time_signature.beat + 1, time_signature.tick);
        }

        return juce::String::formatted("%d|%d", time_signature.bar + 1, time_signature.beat + 1);
    }

private:
    //==============================================================================
    JUCE_LEAK_DETECTOR(BeatTimePoint)
};

//==============================================================================
using BeatTimePointList = std::vector<BeatTimePoint>;

}
}
