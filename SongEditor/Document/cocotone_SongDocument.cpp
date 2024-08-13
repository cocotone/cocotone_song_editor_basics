#include "cocotone_SongDocument.h"
namespace cctn
{
namespace song
{

//==============================================================================
SongDocument::SongDocument()
    : ticksPerQuarterNote(480)
{
}

SongDocument::~SongDocument()
{
}

//==============================================================================
void SongDocument::setMetadata(const juce::String& title, const juce::String& artist)
{
    metadata.title = title;
    metadata.artist = artist;
    metadata.created = juce::Time::getCurrentTime();
    metadata.lastModified = metadata.created;
}

void SongDocument::addTempoEvent(int64_t tick, TempoEvent::TempoEventType type, double tempo, int numerator, int denominator)
{
    tempoTrack.addEvent(TempoEvent(tick, type, tempo, numerator, denominator));
}

void SongDocument::addNote(const Note& note)
{
    notes.add(note);
}

//==============================================================================
int64_t SongDocument::getTotalLengthInTicks() const
{
    if (notes.isEmpty())
    {
        // If there are no notes, return the last tempo event tick or 0
        return tempoTrack.getEvents().empty() ? 0 : tempoTrack.getEvents().back().getTick();
    }

    // Find the last note's end position
    int64_t lastNoteTick = 0;
    for (const auto& note : notes)
    {
        const auto noteOffPosition =  Calculator::calculateNoteOffPosition(*this, note);
        int64_t noteTick = Calculator::barToTick(*this, note.startTimeInMusicalTime);
        int64_t noteEndTick = Calculator::barToTick(*this, noteOffPosition);
        lastNoteTick = std::max(lastNoteTick, noteEndTick);
    }

    // Check if there's a tempo event after the last note
    int64_t lastTempoEventTick = tempoTrack.getEvents().empty() ? 0 : tempoTrack.getEvents().back().getTick();

    // Return the maximum of last note end and last tempo event
    return std::max(lastNoteTick, lastTempoEventTick);
}

//==============================================================================
std::string SongDocument::dumpToString() const
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
        MusicalTime mt = Calculator::tickToBar(*this, event.getTick());
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

        const auto musical_time_note_on = note.startTimeInMusicalTime;
        int64_t tickOnPosition = Calculator::barToTick(*this, musical_time_note_on);
        oss << "    Absolute Tick On Position: " << tickOnPosition << " ticks\n";

        const auto musical_time_note_off = cctn::song::SongDocument::Calculator::calculateNoteOffPosition(*this, note);
        int64_t tickOffPosition = Calculator::barToTick(*this, musical_time_note_off);
        oss << "    Absolute Tick Off Position: " << tickOffPosition << " ticks\n";

        double startTime = Calculator::tickToAbsoluteTime(*this, Calculator::barToTick(*this, note.startTimeInMusicalTime));
        oss << "    Absolute Start Time: " << startTime << " seconds\n";

        double endTime = Calculator::tickToAbsoluteTime(*this, Calculator::barToTick(*this, musical_time_note_off));
        oss << "    Absolute End Time: " << endTime << " seconds\n";
    }

    return oss.str();
}

//==============================================================================
// SongDocument::Calculator
//==============================================================================
juce::var SongDocument::toJson() const
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
        tempoEvent->setProperty("measurePosition", Calculator::tickToBar(*this, event.getTick()).bar - 1);

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

        // Add absolute tick position for note on
        jsonNote->setProperty("absoluteTickOn", cctn::song::SongDocument::Calculator::barToTick(*this, note.startTimeInMusicalTime));

        // Add absolute tick position for note off
        const auto musical_time_note_off = cctn::song::SongDocument::Calculator::calculateNoteOffPosition(*this, note);
        jsonNote->setProperty("absoluteTickOff", cctn::song::SongDocument::Calculator::barToTick(*this, musical_time_note_off));

        jsonNote->setProperty("noteNumber", note.noteNumber);
        jsonNote->setProperty("velocity", note.velocity);
        jsonNote->setProperty("lyric", note.lyric);

        notes.add(jsonNote);
    }
    jsonDoc->setProperty("notes", notes);

    return jsonDoc;
}

//==============================================================================
int64_t SongDocument::Calculator::barToTick(const cctn::song::SongDocument& document, const MusicalTime& musicalTime)
{
    if (musicalTime.bar < 1 || musicalTime.beat < 1 || musicalTime.tick < 0)
    {
        // Invalid musical time
        jassertfalse;
        return 0;
    }

    int64_t accumulatedTicks = 0;
    int currentBar = 1;
    int currentBeat = 1;
    int currentNumerator = 4;
    int currentDenominator = 4;
    int ticksPerQuarterNote = document.getTicksPerQuarterNote();

    const auto& events = document.getTempoTrack().getEvents();
    auto eventIt = events.begin();

    while (eventIt != events.end() &&
        (currentBar < musicalTime.bar ||
            (currentBar == musicalTime.bar && currentBeat <= musicalTime.beat)))
    {
        const auto& event = *eventIt;

        // Calculate ticks up to this event or musicalTime, whichever comes first
        int64_t ticksToNextPoint = event.getTick() - accumulatedTicks;
        int64_t ticksPerBar = currentNumerator * ticksPerQuarterNote * 4 / currentDenominator;
        int64_t ticksPerBeat = ticksPerBar / currentNumerator;

        while (currentBar < musicalTime.bar && ticksToNextPoint >= ticksPerBar)
        {
            accumulatedTicks += ticksPerBar;
            ticksToNextPoint -= ticksPerBar;
            currentBar++;
        }

        while ((currentBar < musicalTime.bar ||
            (currentBar == musicalTime.bar && currentBeat < musicalTime.beat))
            && ticksToNextPoint >= ticksPerBeat)
        {
            accumulatedTicks += ticksPerBeat;
            ticksToNextPoint -= ticksPerBeat;
            currentBeat++;
            if (currentBeat > currentNumerator)
            {
                currentBar++;
                currentBeat = 1;
            }
        }

        // If we've reached musicalTime, add remaining ticks and exit
        if (currentBar == musicalTime.bar && currentBeat == musicalTime.beat)
        {
            accumulatedTicks += musicalTime.tick;
            return accumulatedTicks;
        }

        // Otherwise, add ticks to this event and update time signature
        accumulatedTicks += ticksToNextPoint;

        if (event.getEventType() == TempoEvent::TempoEventType::kTimeSignature ||
            event.getEventType() == TempoEvent::TempoEventType::kBoth)
        {
            auto [numerator, denominator] = event.getTimeSignature();
            currentNumerator = numerator;
            currentDenominator = denominator;
        }

        ++eventIt;
    }

    // If we've exited the loop, it means we've passed all events
    // Calculate remaining ticks to musicalTime
    int64_t ticksPerBar = currentNumerator * ticksPerQuarterNote * 4 / currentDenominator;
    int64_t ticksPerBeat = ticksPerBar / currentNumerator;

    accumulatedTicks += (musicalTime.bar - currentBar) * ticksPerBar;
    accumulatedTicks += (musicalTime.beat - currentBeat) * ticksPerBeat;
    accumulatedTicks += musicalTime.tick;

    return accumulatedTicks;
}

SongDocument::MusicalTime SongDocument::Calculator::tickToBar(const cctn::song::SongDocument& document, int64_t targetTick)
{
    MusicalTime result{ 1, 1, 0 }; // Start from bar 1, beat 1, tick 0
    int64_t accumulatedTicks = 0;
    int currentNumerator = 4;
    int currentDenominator = 4;

    for (const auto& event : document.tempoTrack.getEvents())
    {
        if (accumulatedTicks >= targetTick)
            break;

        int64_t ticksToEvent = event.getTick() - accumulatedTicks;
        int64_t ticksToProcess = std::min(ticksToEvent, targetTick - accumulatedTicks);

        int64_t ticksPerBar = currentNumerator * document.ticksPerQuarterNote * 4 / currentDenominator;
        int64_t ticksPerBeat = ticksPerBar / currentNumerator;

        // Process full bars
        while (ticksToProcess >= ticksPerBar)
        {
            result.bar++;
            ticksToProcess -= ticksPerBar;
            accumulatedTicks += ticksPerBar;
        }

        // Process full beats
        while (ticksToProcess >= ticksPerBeat)
        {
            result.beat++;
            ticksToProcess -= ticksPerBeat;
            accumulatedTicks += ticksPerBeat;
            if (result.beat > currentNumerator)
            {
                result.bar++;
                result.beat = 1;
            }
        }

        // Add remaining ticks
        result.tick = ticksToProcess;
        accumulatedTicks += ticksToProcess;

        // If we've reached or passed the target tick, we're done
        if (accumulatedTicks >= targetTick)
            break;

        // Update time signature if this event changes it
        if (event.getEventType() == TempoEvent::TempoEventType::kTimeSignature ||
            event.getEventType() == TempoEvent::TempoEventType::kBoth)
        {
            auto [numerator, denominator] = event.getTimeSignature();
            currentNumerator = numerator;
            currentDenominator = denominator;

            // Reset beat and tick for the new time signature
            result.beat = 1;
            result.tick = 0;
        }
    }

    // If we've exited the loop and haven't reached the target tick,
    // process any remaining ticks using the last known time signature
    if (accumulatedTicks < targetTick)
    {
        int64_t remainingTicks = targetTick - accumulatedTicks;
        int64_t ticksPerBar = currentNumerator * document.ticksPerQuarterNote * 4 / currentDenominator;
        int64_t ticksPerBeat = ticksPerBar / currentNumerator;

        // Process full bars
        result.bar += remainingTicks / ticksPerBar;
        remainingTicks %= ticksPerBar;

        // Process full beats
        result.beat += remainingTicks / ticksPerBeat;
        remainingTicks %= ticksPerBeat;

        // Add remaining ticks
        result.tick = remainingTicks;

        // Normalize beat if it exceeds the number of beats in a bar
        if (result.beat > currentNumerator)
        {
            result.bar += (result.beat - 1) / currentNumerator;
            result.beat = ((result.beat - 1) % currentNumerator) + 1;
        }
    }

    return result;
}

double SongDocument::Calculator::tickToAbsoluteTime(const cctn::song::SongDocument& document, int64_t targetTick)
{
    double absoluteTime = 0.0;
    int64_t currentTick = 0;
    double currentTempo = 120.0; // Default tempo

    for (const auto& event : document.tempoTrack.getEvents())
    {
        if (event.getTick() >= targetTick)
        {
            // Calculate time for remaining ticks at current tempo
            double secondsPerTick = 60.0 / (currentTempo * document.ticksPerQuarterNote);
            absoluteTime += (targetTick - currentTick) * secondsPerTick;
            return absoluteTime;
        }

        if (event.getEventType() == TempoEvent::TempoEventType::kTempo ||
            event.getEventType() == TempoEvent::TempoEventType::kBoth)
        {
            // Calculate time up to this tempo change
            double secondsPerTick = 60.0 / (currentTempo * document.ticksPerQuarterNote);
            absoluteTime += (event.getTick() - currentTick) * secondsPerTick;

            // Update current position and tempo
            currentTick = event.getTick();
            currentTempo = event.getTempo();
        }
    }

    // If target tick is beyond all tempo changes
    double secondsPerTick = 60.0 / (currentTempo * document.ticksPerQuarterNote);
    absoluteTime += (targetTick - currentTick) * secondsPerTick;

    return absoluteTime;
}

//==============================================================================
SongDocument::MusicalTime SongDocument::Calculator::calculateNoteOffPosition(const SongDocument& document, const Note& note)
{
    // Calculate the tick position for the start of the note
    int64_t tickOnPosition = barToTick(document, note.startTimeInMusicalTime);

    // Find the tempo and time signature at the note start position
    const auto& events = document.getTempoTrack().getEvents();
    auto eventIt = std::lower_bound(events.begin(), events.end(), tickOnPosition,
        [](const TempoEvent& event, int64_t tick) {
            return event.getTick() <= tick;
        });

    double currentTempo = 120.0; // Default tempo
    int currentNumerator = 4;
    int currentDenominator = 4;

    // If we found an event before or at the start position, update tempo and time signature
    if (eventIt != events.begin()) {
        --eventIt;
        if (eventIt->getEventType() == TempoEvent::TempoEventType::kTempo ||
            eventIt->getEventType() == TempoEvent::TempoEventType::kBoth) {
            currentTempo = eventIt->getTempo();
        }
        if (eventIt->getEventType() == TempoEvent::TempoEventType::kTimeSignature ||
            eventIt->getEventType() == TempoEvent::TempoEventType::kBoth) {
            auto [numerator, denominator] = eventIt->getTimeSignature();
            currentNumerator = numerator;
            currentDenominator = denominator;
        }
    }

    // Calculate duration in ticks
    int ticksPerQuarterNote = document.getTicksPerQuarterNote();
    int64_t durationInTicks = 0;
    durationInTicks += note.duration.bars * currentNumerator * ticksPerQuarterNote * 4 / currentDenominator;
    durationInTicks += note.duration.beats * ticksPerQuarterNote * 4 / currentDenominator;
    durationInTicks += note.duration.ticks;

    // Calculate the tick-off position
    int64_t tickOffPosition = tickOnPosition + durationInTicks;

    // Convert tick-off position back to MusicalTime
    MusicalTime noteOffMusicalTime = tickToBar(document, tickOffPosition);

    return noteOffMusicalTime;
}

SongDocument::BeatTimePoints SongDocument::BeatTimePointsFactory::makeBeatTimePoints(const cctn::song::SongDocument& document, NoteLength resolution)
{
    BeatTimePoints beatPoints;
    const auto& tempoTrack = document.getTempoTrack();
    const auto& events = tempoTrack.getEvents();
    int ticksPerQuarterNote = document.getTicksPerQuarterNote();

    double currentTime = 0.0;
    int64_t currentTick = 0;
    double currentTempo = 120.0; // Default tempo
    int currentNumerator = 4;
    int currentDenominator = 4;

    auto eventIt = events.begin();

    double noteLengthsPerQuarterNote = cctn::song::getNoteLengthsPerQuarterNote(resolution);
    double secondsPerQuarterNote = 60.0 / currentTempo;
    double secondsPerNoteLength = secondsPerQuarterNote / noteLengthsPerQuarterNote;
    int64_t ticksPerNoteLength = static_cast<int64_t>(ticksPerQuarterNote / noteLengthsPerQuarterNote);

    while (true)
    {
        // Check if we've reached the next tempo or time signature change
        while (eventIt != events.end() && currentTick >= eventIt->getTick())
        {
            if (eventIt->getEventType() == TempoEvent::TempoEventType::kTempo ||
                eventIt->getEventType() == TempoEvent::TempoEventType::kBoth)
            {
                currentTempo = eventIt->getTempo();
                secondsPerQuarterNote = 60.0 / currentTempo;
                secondsPerNoteLength = secondsPerQuarterNote / noteLengthsPerQuarterNote;
            }
            if (eventIt->getEventType() == TempoEvent::TempoEventType::kTimeSignature ||
                eventIt->getEventType() == TempoEvent::TempoEventType::kBoth)
            {
                auto [numerator, denominator] = eventIt->getTimeSignature();
                currentNumerator = numerator;
                currentDenominator = denominator;
            }
            ++eventIt;
        }

        // Convert current tick to MusicalTime
        const MusicalTime musicalTime = cctn::song::SongDocument::Calculator::tickToBar(document, currentTick);

        // Add BeatTimePoint
        beatPoints.push_back({ musicalTime, currentTime });

        // Move to next beat point
        currentTick += ticksPerNoteLength;
        currentTime += secondsPerNoteLength;

        // Check if we've reached the end of the song
        if (eventIt == events.end() && currentTick >= document.getTotalLengthInTicks())
        {
            break;
        }
    }

    // Add tail BeatTimePoint
    {
        const MusicalTime musicalTime = cctn::song::SongDocument::Calculator::tickToBar(document, currentTick);
        beatPoints.push_back({ musicalTime, currentTime });
    }

    return beatPoints;
}

}  // namespace song
}  // namespace cctn
