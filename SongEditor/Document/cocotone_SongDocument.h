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
        int id;
        MusicalTime startTimeInMusicalTime;
        NoteDuration duration;
        int noteNumber;
        int velocity;
        juce::String lyric;

        JUCE_LEAK_DETECTOR(Note)
    };

    //==============================================================================
    class TempoEvent
    {
    public:
        enum class TempoEventType
        { 
            kTempo, 
            kTimeSignature, 
            kBoth
        };

        TempoEvent(int64_t tick, TempoEventType type, double tempo, int numerator = 4, int denominator = 4)
            : tick(tick)
            , type(type)
            , tempo(tempo)
            , numerator(numerator)
            , denominator(denominator) 
        {}

        int64_t getTick() const { return tick; }
        TempoEventType getEventType() const { return type; }
        double getTempo() const { return tempo; }
        std::pair<int, int> getTimeSignature() const { return { numerator, denominator }; }

    private:
        int64_t tick;
        TempoEventType type;
        double tempo;
        int numerator;
        int denominator;

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
                [](const TempoEvent& a, const TempoEvent& b) { return a.getTick() < b.getTick(); });
        }

        // Other methods for querying and managing tempo events

        const std::vector<TempoEvent>& getEvents() const { return events; };

    private:
        std::vector<TempoEvent> events;

        JUCE_LEAK_DETECTOR(TempoTrack)
    };

    //==============================================================================
    SongDocument();
    virtual ~SongDocument();

    //==============================================================================
    void setMetadata(const juce::String& title, const juce::String& artist);
    void addTempoEvent(int64_t tick, TempoEvent::TempoEventType type, double tempo, int numerator = 4, int denominator = 4);
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
