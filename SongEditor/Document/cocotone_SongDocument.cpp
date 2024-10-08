#include "cocotone_SongDocument.h"
namespace cctn
{
namespace song
{

//==============================================================================
SongDocument::SongDocument()
    : ticksPerQuarterNote(480)
    , minimumTotalLengthInTicks(ticksPerQuarterNote * 4 * 128)
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

void SongDocument::addTempoEvent(int64_t tick, TempoEvent::TempoEventType type, int numerator, int denominator, double tempo)
{
    tempoTrack.addEvent(TempoEvent(tick, type, numerator, denominator, tempo));
}

//==============================================================================
void SongDocument::addNote(const Note& note)
{
    notes.add(note);
}

void SongDocument::removeNote(const Note* note)
{
    notes.remove(note);
}

//==============================================================================
int64_t SongDocument::getTotalLengthInTicks() const
{
    if (notes.isEmpty())
    {
        // If there are no notes, return the last tempo event tick or minimumTotalLengthInTicks
        return std::max<int64_t>(minimumTotalLengthInTicks, tempoTrack.getEvents().back().getTick());;
    }

    // Find the last note's end position
    int64_t lastNoteTick = 0;
    for (const auto& note : notes)
    {
        const auto noteOffPosition =  Calculator::calculateNoteOffPosition(*this, note);
        const int64_t noteTick = Calculator::barToTick(*this, note.startTimeInMusicalTime);
        const int64_t noteEndTick = Calculator::barToTick(*this, noteOffPosition);
        lastNoteTick = std::max(lastNoteTick, noteEndTick);
    }

    // Check if there's a tempo event after the last note
    const int64_t lastTempoEventTick = tempoTrack.getEvents().empty() ? 0 : tempoTrack.getEvents().back().getTick();

    // Return the maximum of last note end and last tempo event
    return std::max<int64_t>(minimumTotalLengthInTicks, std::max<int64_t>(lastNoteTick, lastTempoEventTick));
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
        case TempoEvent::TempoEventType::kTimeSignature:
        {
            const auto timeSignature = event.getTimeSignature();
            oss << "    Time Signature Change: " << timeSignature.numerator << "/" << timeSignature.denominator << "\n";
        }
        break;
        case TempoEvent::TempoEventType::kTempo:
            oss << "    Tempo Change: " << event.getTempo() << " BPM\n";
            break;
        case TempoEvent::TempoEventType::kBoth:
            oss << "    Tempo Change: " << event.getTempo() << " BPM\n";
            {
                const auto timeSignature = event.getTimeSignature();
                oss << "    Time Signature Change: " << timeSignature.numerator << "/" << timeSignature.denominator << "\n";
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
        case cctn::song::SongDocument::TempoEvent::TempoEventType::kTimeSignature:
            tempoEvent->setProperty("type", "kTimeSignature");
            {
                const auto time_signature = event.getTimeSignature();
                juce::DynamicObject* timeSignature = new juce::DynamicObject();
                timeSignature->setProperty("numerator", time_signature.numerator);
                timeSignature->setProperty("denominator", time_signature.denominator);
                tempoEvent->setProperty("timeSignature", timeSignature);
            }
            break;
        case cctn::song::SongDocument::TempoEvent::TempoEventType::kTempo:
            tempoEvent->setProperty("type", "kTempo");
            tempoEvent->setProperty("tempo", event.getTempo());
            break;
        case cctn::song::SongDocument::TempoEvent::TempoEventType::kBoth:
            tempoEvent->setProperty("type", "kBoth");
            tempoEvent->setProperty("tempo", event.getTempo());
            {
                const auto time_signature = event.getTimeSignature();
                juce::DynamicObject* timeSignature = new juce::DynamicObject();
                timeSignature->setProperty("numerator", time_signature.numerator);
                timeSignature->setProperty("denominator", time_signature.denominator);
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
            const auto timeSignature = event.getTimeSignature();
            currentNumerator = timeSignature.numerator;
            currentDenominator = timeSignature.denominator;
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
            const auto timeSignature = event.getTimeSignature();
            currentNumerator = timeSignature.numerator;
            currentDenominator = timeSignature.denominator;

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

int64_t SongDocument::Calculator::absoluteTimeToTick(const cctn::song::SongDocument& document, double targetTime)
{
    if (targetTime <= 0.0)
        return 0;

    int64_t resultTick = 0;
    double currentTime = 0.0;
    double currentTempo = 120.0; // Default tempo
    int64_t lastEventTick = 0;

    for (const auto& event : document.tempoTrack.getEvents())
    {
        if (event.getEventType() == TempoEvent::TempoEventType::kTempo ||
            event.getEventType() == TempoEvent::TempoEventType::kBoth)
        {
            double secondsPerTick = 60.0 / (currentTempo * document.ticksPerQuarterNote);
            double eventTime = currentTime + (event.getTick() - lastEventTick) * secondsPerTick;

            if (eventTime >= targetTime)
            {
                // Target time is within this tempo section
                resultTick = lastEventTick + static_cast<int64_t>((targetTime - currentTime) / secondsPerTick);
                return resultTick;
            }

            // Update current position, time, and tempo
            currentTime = eventTime;
            lastEventTick = event.getTick();
            currentTempo = event.getTempo();
        }
    }

    // If target time is beyond all tempo changes
    double secondsPerTick = 60.0 / (currentTempo * document.ticksPerQuarterNote);
    resultTick = lastEventTick + static_cast<int64_t>((targetTime - currentTime) / secondsPerTick);

    return resultTick;
}

int64_t SongDocument::Calculator::noteLengthToTicks(const cctn::song::SongDocument& document, const NoteLength resolution)
{
    double noteLengthsPerQuarterNote = cctn::song::getNoteLengthsPerQuarterNote(resolution);
    int64_t ticksPerNoteLength = static_cast<int64_t>(document.ticksPerQuarterNote / noteLengthsPerQuarterNote);

    return ticksPerNoteLength;
}

//==============================================================================
SongDocument::MusicalTime SongDocument::Calculator::calculateNoteOffPosition(const SongDocument& document, const Note& note)
{
    // Calculate the tick position for the start of the note
    int64_t tickOnPosition = barToTick(document, note.startTimeInMusicalTime);

    // Calculate duration in ticks
    int durationInTicks = note.duration.ticks;

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
                const auto timeSignature = eventIt->getTimeSignature();
                currentNumerator = timeSignature.numerator;
                currentDenominator = timeSignature.denominator;
            }
            ++eventIt;
        }

        // Convert current tick to MusicalTime
        const MusicalTime musicalTime = cctn::song::SongDocument::Calculator::tickToBar(document, currentTick);

        // Add BeatTimePoint
        beatPoints.push_back({ currentTick, musicalTime, currentTime });

        // Move to next beat point
        currentTick += ticksPerNoteLength;
        currentTime += secondsPerNoteLength;

        // Convert current tick to MusicalTime
        MusicalTime next_musical_time = cctn::song::SongDocument::Calculator::tickToBar(document, currentTick);

        // In case of bar is move to next bar and not equal to first beat of bar.
        if (next_musical_time.bar != musicalTime.bar && next_musical_time.beat != 1)
        {
            next_musical_time.beat = 1;
            next_musical_time.tick = 0;

            currentTick = cctn::song::SongDocument::Calculator::barToTick(document, next_musical_time);
            currentTime = cctn::song::SongDocument::Calculator::tickToAbsoluteTime(document, currentTick);
        }

        // Check if we've reached the end of the song
        if (eventIt == events.end() && currentTick >= document.getTotalLengthInTicks())
        {
            break;
        }
    }

    // Add tail BeatTimePoint
    {
        const MusicalTime musicalTime = cctn::song::SongDocument::Calculator::tickToBar(document, currentTick);
        beatPoints.push_back({ currentTick, musicalTime, currentTime });
    }

    return beatPoints;
}

//==============================================================================
namespace
{
    // Static variable to generate unique IDs
    static int nextId = 1;
}

SongDocument::Note SongDocument::DataFactory::makeNote(const cctn::song::SongDocument& document, const MusicalTime& startTime, const NoteDuration& noteDuration, int noteNumber, int velocity, const juce::String& lyric)
{
    return Note(nextId++, startTime, noteDuration, noteNumber, velocity, lyric);
}

SongDocument::NoteDuration SongDocument::DataFactory::convertNoteLengthToDuration(const SongDocument& document, NoteLength noteLength)
{
    int ticksPerQuarterNote = document.getTicksPerQuarterNote();
    double quarterNotes = 1.0 / getNoteLengthsPerQuarterNote(noteLength);
    int totalTicks = std::round(quarterNotes * ticksPerQuarterNote);

    return NoteDuration{ totalTicks };
}

}  // namespace song
}  // namespace cctn
