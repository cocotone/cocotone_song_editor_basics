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
    Triplet,         // Quarter note triplet
    EighthTriplet,   // Eighth note triplet
    SixteenthTriplet,// Sixteenth note triplet
    DottedQuarter,   // Dotted quarter note
    DottedEighth,    // Dotted eighth note
    DottedSixteenth  // Dotted sixteenth note
};

inline double getNoteValue(NoteLength noteLength)
{
    switch (noteLength) {
    case NoteLength::Whole: return 1.0;
    case NoteLength::Half: return 2.0;
    case NoteLength::Quarter: return 4.0;
    case NoteLength::Eighth: return 8.0;
    case NoteLength::Sixteenth: return 16.0;
    case NoteLength::ThirtySecond: return 32.0;
    case NoteLength::SixtyFourth: return 64.0;
    case NoteLength::Triplet: return 6.0;         // Quarter note triplet (3 in the space of 2)
    case NoteLength::EighthTriplet: return 12.0;  // Eighth note triplet (3 in the space of 2 eighth notes)
    case NoteLength::SixteenthTriplet: return 24.0; // Sixteenth note triplet (3 in the space of 2 sixteenth notes)
    case NoteLength::DottedQuarter: return 4.0 / 1.5;
    case NoteLength::DottedEighth: return 8.0 / 1.5;
    case NoteLength::DottedSixteenth: return 16.0 / 1.5;
    default: throw std::invalid_argument("Invalid note length");
    }
}

inline double getNoteLengthsPerQuarterNote(NoteLength noteLength)
{
    switch (noteLength)
    {
    case NoteLength::Whole: return 0.25;
    case NoteLength::Half: return 0.5;
    case NoteLength::Quarter: return 1.0;
    case NoteLength::Eighth: return 2.0;
    case NoteLength::Sixteenth: return 4.0;
    case NoteLength::ThirtySecond: return 8.0;
    case NoteLength::SixtyFourth: return 16.0;
    case NoteLength::Triplet: return 3.0;
    case NoteLength::EighthTriplet: return 6.0;
    case NoteLength::SixteenthTriplet: return 12.0;
    case NoteLength::DottedQuarter: return 2.0 / 3.0;
    case NoteLength::DottedEighth: return 4.0 / 3.0;
    case NoteLength::DottedSixteenth: return 8.0 / 3.0;
    default: throw std::invalid_argument("Invalid note length");
    }
}

//==============================================================================
using MoraKana = juce::String;
using Mora = MoraKana;

//==============================================================================
struct NoteLyric
{
    juce::String text;  // UTF-8 encode.
    std::vector<juce::String> phonemes;  // UTF-8 encode.

    JUCE_LEAK_DETECTOR(NoteLyric)
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
