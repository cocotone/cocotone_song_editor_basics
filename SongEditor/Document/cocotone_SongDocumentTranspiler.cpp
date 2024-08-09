namespace cctn
{
namespace song
{

namespace
{

//==============================================================================
struct ScoreNote
{
    juce::var key;
    int frame_length;
    juce::String lyric;

    JUCE_LEAK_DETECTOR(ScoreNote)
};

juce::String dumpScoreNotes(const juce::Array<ScoreNote>& scoreNotes)
{
    std::ostringstream oss;
    oss << "ScoreNotes dump:\n";
    oss << "----------------\n";

    for (int i = 0; i < scoreNotes.size(); ++i)
    {
        const auto& note = scoreNotes[i];
        oss << "Note " << i << ":\n";
        oss << "  Key: " << (note.key.isVoid() ? "null" : note.key.toString().toStdString()) << "\n";
        oss << "  Frame Length: " << note.frame_length << "\n";
        oss << "  Lyric: \"" << note.lyric.toStdString() << "\"\n";
        oss << "\n";
    }

    return oss.str();
}

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

//==============================================================================
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

static juce::var createScoreJsonFromSongDocument(const cctn::song::SongDocument& doc)
{
    const double sampleRate = 24000.0;  // 24kHz
    const int samplesPerFrame = 256;
    const double secondsPerFrame = samplesPerFrame / sampleRate;
    const int kInitialAndFinalSilence = 4;

    juce::Array<cctn::song::SongDocument::Note> sortedNotes = doc.getNotes();
    sortedNotes.sort(MusicalTmeDomainNoteComparator());

    juce::Array<ScoreNote> scoreNotes;

    // Add initial silence (4 frames)
    scoreNotes.add({ juce::var(), kInitialAndFinalSilence, "" });

    double currentTime = kInitialAndFinalSilence * secondsPerFrame;

    for (const auto& note : sortedNotes)
    {
        double startTime = calculateAbsoluteTime(doc, note.startTimeInMusicalTime);
        double endTime = startTime + calculateAbsoluteTimeForNoteEnd(doc, note.startTimeInMusicalTime, { note.duration.bars, note.duration.beats, note.duration.ticks });

        // If there is a gap of more than one frame, add silence
        if (startTime > currentTime + secondsPerFrame)
        {
            int silenceFrames = static_cast<int>(std::round((startTime - currentTime) / secondsPerFrame));
            scoreNotes.add({ juce::var(), silenceFrames, "" });
        }

        // Add note for singing
        int noteFrames = static_cast<int>(std::round((endTime - startTime) / secondsPerFrame));
        if (noteFrames > 0)
        {
            scoreNotes.add({ note.noteNumber, noteFrames, note.lyric });
        }

        currentTime = endTime;
    }

    // Add final silence (4 frames)
    scoreNotes.add({ juce::var(), kInitialAndFinalSilence, "" });

    // Create JSON object
    juce::DynamicObject::Ptr jsonRoot(new juce::DynamicObject());
    juce::Array<juce::var> jsonNotes;

    for (const auto& note : scoreNotes)
    {
        juce::DynamicObject::Ptr jsonNote(new juce::DynamicObject());
        jsonNote->setProperty("key", note.key);
        jsonNote->setProperty("frame_length", note.frame_length);
        jsonNote->setProperty("lyric", note.lyric);
        jsonNotes.add(jsonNote.get());
    }

    jsonRoot->setProperty("notes", jsonNotes);

    return juce::var(jsonRoot.get());
}

//// Helper function to convert the JSON to a string
//static juce::String createScoreJsonStringFromSongDocument(const cctn::song::SongDocument& doc)
//{
//    return juce::JSON::toString(createScoreJsonFromSongDocument(doc));
//}
} // namespace anonymous

juce::String song::SongDocumentTranspiler::VoicevoxTranspileTarget::transpile(const cctn::song::SongDocument& sourceDocument)
{
    juce::Logger::outputDebugString(sourceDocument.dumpToString());

    return juce::JSON::toString(cctn::song::createScoreJsonFromSongDocument(sourceDocument));
}

}  // namespace song
}  // namespace cctn
