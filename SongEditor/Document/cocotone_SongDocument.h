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
    static double calculateAbsoluteTime(const cctn::song::SongDocument& doc, const cctn::song::SongDocument::MusicalTime& time)
    {
        double absoluteTime = 0.0;
        int64_t currentTick = 0;
        double currentTempo = 120.0; // Default tempo
        int currentNumerator = 4, currentDenominator = 4; // Assume 4/4 time signature as default

        for (const auto& event : doc.getTempoTrack().getEvents())
        {
            int64_t targetTick = doc.barToTick(time);
            if (event.getTick() > targetTick)
            {
                // Goal time reached
                double tickDuration = 60.0 / (currentTempo * doc.getTicksPerQuarterNote());
                absoluteTime += (targetTick - currentTick) * tickDuration;
                return absoluteTime;
            }

            // Calculate the time until this event
            double tickDuration = 60.0 / (currentTempo * doc.getTicksPerQuarterNote());
            absoluteTime += (event.getTick() - currentTick) * tickDuration;

            // Update the current position and tempo
            currentTick = event.getTick();
            if (event.getEventType() == cctn::song::SongDocument::TempoEvent::TempoEventType::kTempo ||
                event.getEventType() == cctn::song::SongDocument::TempoEvent::TempoEventType::kBoth)
            {
                currentTempo = event.getTempo();
            }
            if (event.getEventType() == cctn::song::SongDocument::TempoEvent::TempoEventType::kTimeSignature ||
                event.getEventType() == cctn::song::SongDocument::TempoEvent::TempoEventType::kBoth)
            {
                auto [numerator, denominator] = event.getTimeSignature();
                currentNumerator = numerator;
                currentDenominator = denominator;
            }
        }

        // Calculate the time since the last event
        int64_t targetTick = doc.barToTick(time);
        double tickDuration = 60.0 / (currentTempo * doc.getTicksPerQuarterNote());
        absoluteTime += (targetTick - currentTick) * tickDuration;

        return absoluteTime;
    }

    // New overload for calculating with adding note duration
    static double calculateAbsoluteTimeForNoteEnd(const cctn::song::SongDocument& doc, const cctn::song::SongDocument::MusicalTime& startTime, const cctn::song::SongDocument::NoteDuration& duration)
    {
        // Calculate end time
        cctn::song::SongDocument::MusicalTime endTime;
        endTime.bar = startTime.bar + duration.bars;
        endTime.beat = startTime.beat + duration.beats;
        endTime.tick = startTime.tick + duration.ticks;

        // Normalize end time
        int ticksPerBeat = doc.getTicksPerQuarterNote();
        int beatsPerBar = 4; // Assume 4/4 time signature as default

        // Adjust for overflow in ticks
        if (endTime.tick >= ticksPerBeat)
        {
            endTime.beat += endTime.tick / ticksPerBeat;
            endTime.tick %= ticksPerBeat;
        }

        // Adjust for overflow in beats
        // This part might need to be adjusted based on how your SongDocument handles time signatures
        if (endTime.beat > beatsPerBar)
        {
            endTime.bar += (endTime.beat - 1) / beatsPerBar;
            endTime.beat = ((endTime.beat - 1) % beatsPerBar) + 1;
        }

        // Calculate absolute times for start and end
        double startAbsoluteTime = calculateAbsoluteTime(doc, startTime);
        double endAbsoluteTime = calculateAbsoluteTime(doc, endTime);

        // Return the difference
        return endAbsoluteTime - startAbsoluteTime;
    }

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
        if (musicalTime.bar < 1 || musicalTime.beat < 1 || musicalTime.tick < 0)
        {
            // Invalid musical time
            jassertfalse;
            return 0;
        }

        int64_t tick = 0;
        int currentBar = 1;
        int currentBeat = 1;
        int currentNumerator = 4;
        int currentDenominator = 4;
        int ticksPerQuarterNote = getTicksPerQuarterNote();

        for (const auto& event : tempoTrack.getEvents())
        {
            if (currentBar > musicalTime.bar ||
                (currentBar == musicalTime.bar && currentBeat > musicalTime.beat))
            {
                break;
            }

            if (event.getEventType() == TempoEvent::TempoEventType::kTimeSignature ||
                event.getEventType() == TempoEvent::TempoEventType::kBoth)
            {
                auto [numerator, denominator] = event.getTimeSignature();
                int64_t ticksPerBar = numerator * ticksPerQuarterNote * 4 / denominator;

                while (currentBar < musicalTime.bar)
                {
                    tick += ticksPerBar;
                    currentBar++;
                }

                currentNumerator = numerator;
                currentDenominator = denominator;
            }
        }

        // Add ticks for remaining beats and ticks
        int64_t ticksPerBeat = (int64_t)currentNumerator * ticksPerQuarterNote * 4 / (currentDenominator * currentNumerator);
        tick += (int64_t)(musicalTime.beat - 1) * ticksPerBeat;
        tick += musicalTime.tick;

        return tick;
    }

    //==============================================================================
    std::string dumpToString() const
    {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2);

        // Metadata
        oss << "Song Document Dump:\n";
        oss << "Title: " << metadata.title << "\n";
        oss << "Artist: " << metadata.artist << "\n";
        oss << "Created: " << metadata.created.toISO8601(true) << "\n";
        oss << "Last Modified: " << metadata.lastModified.toISO8601(true) << "\n";
        oss << "Ticks Per Quarter Note: " << ticksPerQuarterNote << "\n\n";

        // Tempo Track
        oss << "Tempo Track:\n";
        for (const auto& event : tempoTrack.getEvents())
        {
            MusicalTime mt = tickToBar(event.getTick());
            oss << "  Bar " << mt.bar << ", Beat " << mt.beat << ", Tick " << mt.tick << " (Tick " << event.getTick() << "):\n";

            switch (event.getEventType())
            {
            case TempoEvent::TempoEventType::kTempo:
                oss << "    Tempo Change: " << event.getTempo() << " BPM\n";
                break;
            case TempoEvent::TempoEventType::kTimeSignature:
            {
                auto [numerator, denominator] = event.getTimeSignature();
                oss << "    Time Signature Change: " << numerator << "/" << denominator << "\n";
            }
            break;
            case TempoEvent::TempoEventType::kBoth:
                oss << "    Tempo Change: " << event.getTempo() << " BPM\n";
                {
                    auto [numerator, denominator] = event.getTimeSignature();
                    oss << "    Time Signature Change: " << numerator << "/" << denominator << "\n";
                }
                break;
            }
        }
        oss << "\n";

        // Notes
        oss << "Notes:\n";
        for (const auto& note : notes)
        {
            oss << "  Note ID " << note.id << ":\n";
            oss << "    Start: Bar " << note.startTimeInMusicalTime.bar
                << ", Beat " << note.startTimeInMusicalTime.beat
                << ", Tick " << note.startTimeInMusicalTime.tick << "\n";
            oss << "    Duration: "
                << note.duration.bars << " bars, "
                << note.duration.beats << " beats, "
                << note.duration.ticks << " ticks\n";
            oss << "    Note Number: " << note.noteNumber << "\n";
            oss << "    Velocity: " << note.velocity << "\n";
            oss << "    Lyric: " << note.lyric << "\n";

            double startTime = tickToAbsoluteTime(barToTick(note.startTimeInMusicalTime));
            oss << "    Absolute Start Time: " << startTime << " seconds\n";

            double endTime = calculateAbsoluteTimeForNoteEnd(*this, note.startTimeInMusicalTime, note.duration);
            oss << "    Absolute End Time: " << endTime << " seconds\n";
        }

        return oss.str();
    }

    //==============================================================================
    juce::var toJson() const
    {
        const cctn::song::SongDocument& doc = *this;

        juce::DynamicObject* jsonDoc = new juce::DynamicObject();

        // Metadata
        juce::DynamicObject* metadata = new juce::DynamicObject();
        metadata->setProperty("title", doc.getTitle());
        metadata->setProperty("artist", doc.getArtist());
        metadata->setProperty("created", doc.getCreationTime().toISO8601(true));
        metadata->setProperty("lastModified", doc.getLastModifiedTime().toISO8601(true));
        jsonDoc->setProperty("metadata", metadata);

        // Ticks per quarter note
        jsonDoc->setProperty("ticksPerQuarterNote", doc.getTicksPerQuarterNote());

        // Tempo Track
        juce::Array<juce::var> tempoTrack;
        for (const auto& event : doc.getTempoTrack().getEvents())
        {
            juce::DynamicObject* tempoEvent = new juce::DynamicObject();
            tempoEvent->setProperty("tick", event.getTick());

            switch (event.getEventType())
            {
            case cctn::song::SongDocument::TempoEvent::TempoEventType::kTempo:
                tempoEvent->setProperty("type", "kTempo");
                tempoEvent->setProperty("tempo", event.getTempo());
                break;
            case cctn::song::SongDocument::TempoEvent::TempoEventType::kTimeSignature:
                tempoEvent->setProperty("type", "kTimeSignature");
                {
                    auto [numerator, denominator] = event.getTimeSignature();
                    juce::DynamicObject* timeSignature = new juce::DynamicObject();
                    timeSignature->setProperty("numerator", numerator);
                    timeSignature->setProperty("denominator", denominator);
                    tempoEvent->setProperty("timeSignature", timeSignature);
                }
                break;
            case cctn::song::SongDocument::TempoEvent::TempoEventType::kBoth:
                tempoEvent->setProperty("type", "kBoth");
                tempoEvent->setProperty("tempo", event.getTempo());
                {
                    auto [numerator, denominator] = event.getTimeSignature();
                    juce::DynamicObject* timeSignature = new juce::DynamicObject();
                    timeSignature->setProperty("numerator", numerator);
                    timeSignature->setProperty("denominator", denominator);
                    tempoEvent->setProperty("timeSignature", timeSignature);
                }
                break;
            }

            tempoTrack.add(tempoEvent);
        }
        jsonDoc->setProperty("tempoTrack", tempoTrack);

        // Notes
        juce::Array<juce::var> notes;
        for (const auto& note : doc.getNotes())
        {
            juce::DynamicObject* jsonNote = new juce::DynamicObject();
            jsonNote->setProperty("id", note.id);

            juce::DynamicObject* startTime = new juce::DynamicObject();
            startTime->setProperty("bar", note.startTimeInMusicalTime.bar);
            startTime->setProperty("beat", note.startTimeInMusicalTime.beat);
            startTime->setProperty("tick", note.startTimeInMusicalTime.tick);
            jsonNote->setProperty("startTimeInMusicalTime", startTime);

            juce::DynamicObject* duration = new juce::DynamicObject();
            duration->setProperty("bars", note.duration.bars);
            duration->setProperty("beats", note.duration.beats);
            duration->setProperty("ticks", note.duration.ticks);
            jsonNote->setProperty("duration", duration);

            jsonNote->setProperty("noteNumber", note.noteNumber);
            jsonNote->setProperty("velocity", note.velocity);
            jsonNote->setProperty("lyric", note.lyric);

            notes.add(jsonNote);
        }
        jsonDoc->setProperty("notes", notes);

        return jsonDoc;
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
