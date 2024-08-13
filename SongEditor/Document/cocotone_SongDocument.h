#pragma once

namespace cctn
{
namespace song
{

//==============================================================================
class SongDocument
{
public:
    //==============================================================================
    // Forward declaration
    class DataFactory;

    //==============================================================================
    // Internal Data Types
    //==============================================================================
    struct Metadata
    {
        juce::String title;
        juce::String artist;
        juce::Time created;
        juce::Time lastModified;

        JUCE_LEAK_DETECTOR(Metadata)
    };

    //==============================================================================
    struct TimeSignature
    {
        int numerator;
        int denominator;

        JUCE_LEAK_DETECTOR(TimeSignature)
    };

    //==============================================================================
    // MusicalTime is a.k.a BarTime
    struct MusicalTime
    {
        int bar;
        int beat;
        int tick;

        JUCE_LEAK_DETECTOR(MusicalTime)
    };

    //==============================================================================
    struct NoteDuration
    {
        int bars;
        int beats;
        int ticks;

        JUCE_LEAK_DETECTOR(NoteDuration)
    };

    //==============================================================================
    struct Note
    {
    private:
        // Private constructor
        Note(int id, MusicalTime startTime, NoteDuration dur, int noteNum, int vel, const juce::String& lyr)
            : id(id)
            , startTimeInMusicalTime(startTime)
            , duration(dur)
            , noteNumber(noteNum)
            , velocity(vel)
            , lyric(lyr)
        {}

    public:
        int id;
        MusicalTime startTimeInMusicalTime;
        NoteDuration duration;
        int noteNumber;
        int velocity;
        juce::String lyric;

        // Make DataFactory a friend so it can access the private constructor
        friend class DataFactory;

        JUCE_LEAK_DETECTOR(Note)
    };

    //==============================================================================
    class TempoEvent
    {
    public:
        enum class TempoEventType
        { 
            kTimeSignature,
            kTempo, 
            kBoth
        };

        TempoEvent(int64_t tick, TempoEventType type, int numerator = 4, int denominator = 4, double tempo = 120)
            : tick(tick)
            , type(type)
            , timeSignature({ numerator, denominator })
            , tempo(tempo)
        {}

        int64_t getTick() const { return tick; }
        TempoEventType getEventType() const { return type; }
        double getTempo() const { return tempo; }
        TimeSignature getTimeSignature() const { return timeSignature; }

    private:
        int64_t tick;
        TempoEventType type;
        TimeSignature timeSignature;
        double tempo;

        JUCE_LEAK_DETECTOR(TempoEvent)
    };

    //==============================================================================
    class TempoTrack
    {
    public:
        void addEvent(const TempoEvent& event) 
        {
            events.push_back(event);
            std::sort(events.begin(), events.end(),
                [](const TempoEvent& a, const TempoEvent& b) 
                {
                    return a.getTick() < b.getTick(); 
                });
        }

        // Other methods for querying and managing tempo events

        const std::vector<TempoEvent>& getEvents() const { return events; };

    private:
        std::vector<TempoEvent> events;

        JUCE_LEAK_DETECTOR(TempoTrack)
    };

    //==============================================================================
    struct BeatTimePoint
    {
        MusicalTime musicalTime;
        double timeInSeconds{ 0.0 };

        // Copy
        BeatTimePoint(const BeatTimePoint&) = default;

        // Move
        BeatTimePoint(BeatTimePoint&&) noexcept = default;

        // Copy assign operator
        BeatTimePoint& operator=(const BeatTimePoint&) = default;

        // Move assign operator
        BeatTimePoint& operator=(BeatTimePoint&&) noexcept = default;

        JUCE_LEAK_DETECTOR(BeatTimePoint)
    };
    using BeatTimePoints = std::vector<BeatTimePoint>;

    //==============================================================================
    struct RegionWithBeatInfo
    {
        double startPositionInSeconds;
        double endPositionInSeconds;
        MusicalTime startMusicalTime;

        JUCE_LEAK_DETECTOR(RegionWithBeatInfo)
    };

    //==============================================================================
    // SongDocument
    //==============================================================================
    SongDocument();
    virtual ~SongDocument();

    //==============================================================================
    void setMetadata(const juce::String& title, const juce::String& artist);
    void addTempoEvent(int64_t tick, TempoEvent::TempoEventType type, int numerator = 4, int denominator = 4, double tempo = 120.0);
    void addNote(const Note& note);

    //==============================================================================
    // Getters
    const juce::String& getTitle() const { return metadata.title; }
    const juce::String& getArtist() const { return metadata.artist; }
    juce::Time getCreationTime() const { return metadata.created; }
    juce::Time getLastModifiedTime() const { return metadata.lastModified; }
    int getTicksPerQuarterNote() const { return ticksPerQuarterNote; }
    const TempoTrack& getTempoTrack() const { return tempoTrack; }
    const juce::Array<Note>& getNotes() const { return notes; }

    //==============================================================================
    // Get the total length of the song in ticks
    int64_t getTotalLengthInTicks() const;

    //==============================================================================
    std::string dumpToString() const;

    //==============================================================================
    juce::var toJson() const;

    //==============================================================================
    class Calculator
    {
    public:
        //==============================================================================
        // Convert tick to absolute time
        static int64_t barToTick(const cctn::song::SongDocument& document, const cctn::song::SongDocument::MusicalTime& musicalTime);
        static MusicalTime tickToBar(const cctn::song::SongDocument& document, int64_t targetTick);
        static double tickToAbsoluteTime(const cctn::song::SongDocument& document, int64_t targetTick);

        //==============================================================================
        static MusicalTime calculateNoteOffPosition(const SongDocument& document, const Note& note);
        
    private:
        //==============================================================================
        Calculator() = delete;
        ~Calculator() = delete;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Calculator)
    };

    //==============================================================================
    class BeatTimePointsFactory
    {
    public:
        //==============================================================================
        static BeatTimePoints makeBeatTimePoints(const cctn::song::SongDocument& document, NoteLength resolution);

    private:
        //==============================================================================
        BeatTimePointsFactory() = delete;
        ~BeatTimePointsFactory() = delete;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BeatTimePointsFactory)
    };

    //==============================================================================
    class DataFactory
    {
    public:
        //==============================================================================
        static Note makeNote(const cctn::song::SongDocument& document, int bar, int beat, int tick, int durationBars, int durationBeats, int durationTicks, int noteNumber, int velocity, const juce::String& lyric);
        static Note makeNote(const cctn::song::SongDocument& document, const MusicalTime& startTime, int durationBars, int durationBeats, int durationTicks, int noteNumber, int velocity, const juce::String& lyric);
        static Note makeNote(const cctn::song::SongDocument& document, const MusicalTime& startTime, const NoteDuration& noteDuration, int noteNumber, int velocity, const juce::String& lyric);

        //=========================================================================
        // Helper function to convert MusicalTime to tick (should be part of SongDocument or a utility class)
        static int64_t musicalTimeToTick(const SongDocument& document, const MusicalTime& musicalTime)
        {
            return cctn::song::SongDocument::Calculator::barToTick(document, musicalTime);
        }

        static NoteDuration convertNoteLengthToDuration(const SongDocument& document, NoteLength noteLength, const MusicalTime& startTime)
        {
            int64_t startTick = musicalTimeToTick(document, startTime);
            int ticksPerQuarterNote = document.getTicksPerQuarterNote();
            double quarterNotes = 1.0 / getNoteLengthsPerQuarterNote(noteLength);
            int64_t totalTicks = static_cast<int64_t>(std::round(quarterNotes * ticksPerQuarterNote));

            int bars = 0;
            int beats = 0;
            int ticks = 0;

            const auto& tempoTrack = document.getTempoTrack();
            const auto& events = tempoTrack.getEvents();

            int currentNumerator = 4;
            int currentDenominator = 4;

            auto eventIt = std::lower_bound(events.begin(), events.end(), startTick,
                [](const TempoEvent& event, int64_t tick) 
                {
                    return event.getTick() < tick;
                });

            if (eventIt != events.begin())
            {
                --eventIt;
                if (eventIt->getEventType() == TempoEvent::TempoEventType::kTimeSignature ||
                    eventIt->getEventType() == TempoEvent::TempoEventType::kBoth)
                {
                    const auto timeSignature = eventIt->getTimeSignature();
                    currentNumerator = timeSignature.numerator;
                    currentDenominator = timeSignature.denominator;
                }
            }

            int64_t currentTick = startTick;
            int64_t endTick = startTick + totalTicks;

            while (currentTick < endTick) {
                int64_t ticksPerBar = currentNumerator * ticksPerQuarterNote * 4 / currentDenominator;
                int64_t ticksToNextBar = ticksPerBar - (currentTick % ticksPerBar);

                if (currentTick + ticksToNextBar <= endTick) {
                    bars++;
                    currentTick += ticksToNextBar;
                }
                else {
                    int64_t remainingTicks = endTick - currentTick;
                    int64_t ticksPerBeat = ticksPerQuarterNote * 4 / currentDenominator;
                    beats += static_cast<int>(remainingTicks / ticksPerBeat);
                    ticks = static_cast<int>(remainingTicks % ticksPerBeat);
                    break;
                }

                // Update time signature if needed
                while (eventIt != events.end() && eventIt->getTick() <= currentTick)
                {
                    if (eventIt->getEventType() == TempoEvent::TempoEventType::kTimeSignature ||
                        eventIt->getEventType() == TempoEvent::TempoEventType::kBoth)
                    {
                        const auto timeSignature = eventIt->getTimeSignature();
                        currentNumerator = timeSignature.numerator;
                        currentDenominator = timeSignature.denominator;
                    }
                    ++eventIt;
                }
            }

            return NoteDuration{ bars, beats, ticks };
        }

        NoteLength approximateNoteLengthFromDuration(const SongDocument& document, const NoteDuration& duration, const MusicalTime& startTime)
        {
            int64_t startTick = musicalTimeToTick(document, startTime);
            int64_t totalTicks = 0;
            const auto& tempoTrack = document.getTempoTrack();
            const auto& events = tempoTrack.getEvents();

            int currentNumerator = 4;
            int currentDenominator = 4;
            int ticksPerQuarterNote = document.getTicksPerQuarterNote();

            auto eventIt = std::lower_bound(events.begin(), events.end(), startTick,
                [](const TempoEvent& event, int64_t tick) 
                { 
                    return event.getTick() < tick;
                });

            if (eventIt != events.begin())
            {
                --eventIt;
                if (eventIt->getEventType() == TempoEvent::TempoEventType::kTimeSignature ||
                    eventIt->getEventType() == TempoEvent::TempoEventType::kBoth)
                {
                    const auto timeSignature = eventIt->getTimeSignature();
                    currentNumerator = timeSignature.numerator;
                    currentDenominator = timeSignature.denominator;
                }
            }

            int64_t currentTick = startTick;

            // Calculate total ticks for bars
            for (int i = 0; i < duration.bars; ++i)
            {
                int64_t ticksPerBar = currentNumerator * ticksPerQuarterNote * 4 / currentDenominator;
                totalTicks += ticksPerBar;
                currentTick += ticksPerBar;

                // Update time signature if needed
                while (eventIt != events.end() && eventIt->getTick() <= currentTick)
                {
                    if (eventIt->getEventType() == TempoEvent::TempoEventType::kTimeSignature ||
                        eventIt->getEventType() == TempoEvent::TempoEventType::kBoth)
                    {
                        const auto timeSignature = eventIt->getTimeSignature();
                        currentNumerator = timeSignature.numerator;
                        currentDenominator = timeSignature.denominator;
                    }
                    ++eventIt;
                }
            }

            // Add ticks for beats and remaining ticks
            totalTicks += duration.beats * ticksPerQuarterNote * 4 / currentDenominator + duration.ticks;

            double quarterNotes = static_cast<double>(totalTicks) / ticksPerQuarterNote;

            // Find the closest matching note length (same as before)
            std::vector<NoteLength> noteLengths = {
                NoteLength::Whole,
                NoteLength::DottedQuarter,
                NoteLength::Half,
                NoteLength::Quarter,
                NoteLength::DottedEighth,
                NoteLength::Eighth,
                NoteLength::DottedSixteenth,
                NoteLength::Sixteenth,
                NoteLength::ThirtySecond,
                NoteLength::SixtyFourth,
                NoteLength::Triplet,
                NoteLength::EighthTriplet,
                NoteLength::SixteenthTriplet
            };

            NoteLength closestNoteLength = NoteLength::Quarter;
            double smallestDifference = std::numeric_limits<double>::max();

            for (const auto& note_length : noteLengths)
            {
                double difference = std::abs(quarterNotes - (1.0 / getNoteLengthsPerQuarterNote(note_length)));
                if (difference < smallestDifference)
                {
                    smallestDifference = difference;
                    closestNoteLength = note_length;
                }
            }

            return closestNoteLength;
        }

    private:
        //==============================================================================
        DataFactory() = delete;
        ~DataFactory() = delete;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DataFactory)
    };

    //==============================================================================
    struct MusicalTmeDomainNoteComparator
    {
        static int compareElements(const cctn::song::SongDocument::Note& a, const cctn::song::SongDocument::Note& b)
        {
            // Assuming startTimeInMusicalTime can be directly compared
            if (a.startTimeInMusicalTime.bar < b.startTimeInMusicalTime.bar)
                return -1;
            if (a.startTimeInMusicalTime.bar > b.startTimeInMusicalTime.bar)
                return 1;

            if (a.startTimeInMusicalTime.beat < b.startTimeInMusicalTime.beat)
                return -1;
            if (a.startTimeInMusicalTime.beat > b.startTimeInMusicalTime.beat)
                return 1;

            if (a.startTimeInMusicalTime.tick < b.startTimeInMusicalTime.tick)
                return -1;
            if (a.startTimeInMusicalTime.tick > b.startTimeInMusicalTime.tick)
                return 1;

            return 0;
        }
    };

private:
    //==============================================================================
    Metadata metadata;
    int ticksPerQuarterNote;
    TempoTrack tempoTrack;
    juce::Array<Note> notes;

    JUCE_LEAK_DETECTOR(SongDocument)
};

}  // namespace song
}  // namespace cctn
