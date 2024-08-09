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
    struct TimeSignature
    {
        int numerator;
        int denominator;

        JUCE_LEAK_DETECTOR(TimeSignature)
    };

    //==============================================================================
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
    SongDocument()
        : ticksPerQuarterNote(480) 
    {}

    ~SongDocument()
    {}

    //==============================================================================
    void setMetadata(const juce::String& title, const juce::String& artist)
    {
        metadata.title = title;
        metadata.artist = artist;
        metadata.created = juce::Time::getCurrentTime();
        metadata.lastModified = metadata.created;
    }

    void addTempoEvent(int64_t tick, TempoEvent::TempoEventType type, double tempo, int numerator = 4, int denominator = 4)
    {
        tempoTrack.addEvent(TempoEvent(tick, type, tempo, numerator, denominator));
    }

    void addNote(const Note& note)
    {
        notes.add(note);
    }

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
    // Convert tick to absolute time
    double tickToAbsoluteTime(int64_t targetTick) const
    {
        double absoluteTime = 0.0;
        int64_t currentTick = 0;
        double currentTempo = 120.0; // Default tempo

        for (const auto& event : tempoTrack.getEvents())
        {
            if (event.getTick() >= targetTick)
            {
                // Calculate time for remaining ticks at current tempo
                double secondsPerTick = 60.0 / (currentTempo * ticksPerQuarterNote);
                absoluteTime += (targetTick - currentTick) * secondsPerTick;
                return absoluteTime;
            }

            if (event.getEventType() == TempoEvent::TempoEventType::kTempo ||
                event.getEventType() == TempoEvent::TempoEventType::kBoth)
            {
                // Calculate time up to this tempo change
                double secondsPerTick = 60.0 / (currentTempo * ticksPerQuarterNote);
                absoluteTime += (event.getTick() - currentTick) * secondsPerTick;

                // Update current position and tempo
                currentTick = event.getTick();
                currentTempo = event.getTempo();
            }
        }

        // If target tick is beyond all tempo changes
        double secondsPerTick = 60.0 / (currentTempo * ticksPerQuarterNote);
        absoluteTime += (targetTick - currentTick) * secondsPerTick;

        return absoluteTime;
    }

    // Convert tick to bar
    MusicalTime tickToBar(int64_t tick) const
    {
        MusicalTime result{ 1, 1, 0 }; // Start from bar 1, beat 1, tick 0
        int64_t accumulatedTicks = 0;
        int currentBeat = 0;

        for (const auto& event : tempoTrack.getEvents())
        {
            if (event.getTick() > tick)
                break;

            if (event.getEventType() == TempoEvent::TempoEventType::kTimeSignature ||
                event.getEventType() == TempoEvent::TempoEventType::kBoth)
            {
                auto [numerator, denominator] = event.getTimeSignature();
                int64_t ticksPerBar = numerator * ticksPerQuarterNote * 4 / denominator;

                while (accumulatedTicks + ticksPerBar <= tick)
                {
                    accumulatedTicks += ticksPerBar;
                    result.bar++;
                }

                if (accumulatedTicks == tick)
                    return result;

                int64_t remainingTicks = tick - accumulatedTicks;
                int64_t ticksPerBeat = ticksPerBar / numerator;
                result.beat = (remainingTicks / ticksPerBeat) + 1;
                result.tick = remainingTicks % ticksPerBeat;
                return result;
            }
        }

        // If no time signature change found, assume 4/4
        int64_t ticksPerBar = 4 * ticksPerQuarterNote;
        result.bar += tick / ticksPerBar;
        int64_t remainingTicks = tick % ticksPerBar;
        result.beat = (remainingTicks / ticksPerQuarterNote) + 1;
        result.tick = remainingTicks % ticksPerQuarterNote;

        return result;
    }

    // Convert bar to tick
    int64_t barToTick(const MusicalTime& musicalTime) const
    {
        int64_t result = 0;
        int currentBar = 1;
        int currentBeat = 1;
        int currentTick = 0;

        for (const auto& event : tempoTrack.getEvents())
        {
            if (currentBar > musicalTime.bar ||
                (currentBar == musicalTime.bar && currentBeat > musicalTime.beat))
                break;

            if (event.getEventType() == TempoEvent::TempoEventType::kTimeSignature ||
                event.getEventType() == TempoEvent::TempoEventType::kBoth)
            {
                auto [numerator, denominator] = event.getTimeSignature();
                int64_t ticksPerBar = numerator * ticksPerQuarterNote * 4 / denominator;

                while (currentBar < musicalTime.bar)
                {
                    result += ticksPerBar;
                    currentBar++;
                }

                if (currentBar == musicalTime.bar)
                {
                    int64_t ticksPerBeat = ticksPerBar / numerator;
                    result += (musicalTime.beat - 1) * ticksPerBeat;
                    result += musicalTime.tick;
                    return result;
                }
            }
        }

        // If no time signature change found or if the bar is beyond all changes, assume 4/4
        int64_t ticksPerBar = 4 * ticksPerQuarterNote;
        result += (musicalTime.bar - currentBar) * ticksPerBar;
        result += (musicalTime.beat - 1) * ticksPerQuarterNote;
        result += musicalTime.tick;

        return result;
    }

private:
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
    Metadata metadata;
    int ticksPerQuarterNote;
    TempoTrack tempoTrack;
    juce::Array<Note> notes;

    JUCE_LEAK_DETECTOR(SongDocument)
};

}  // namespace song
}  // namespace cctn
