#include "cocotone_SongDocumentEditor.h"
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

juce::Array<ScoreNote> convertToScoreNotes(const SongEditorDocumentData& documentData)
{
    const double sampleRate = 24000.0;  // 24kHz
    const int samplesPerFrame = 256;
    const double secondsPerFrame = samplesPerFrame / sampleRate;

    // NOTE: silence is should over 4 frames?
    const int kInitialAndFinalSilence = 4;

    juce::Array<ScoreNote> scoreNotes;

    // Add initial silence (4 frames)
    scoreNotes.add({ juce::var(), kInitialAndFinalSilence, "" });

    // Start time after initial silence
    double currentTime = kInitialAndFinalSilence * secondsPerFrame;

    for (int i = 0; i < documentData.notes.size(); ++i)
    {
        const auto& note = documentData.notes[i];

        // Calculate note duration in frames
        int noteFrames = std::round((note.endPositionInSeconds - note.startPositionInSeconds) / secondsPerFrame);

        scoreNotes.add({ note.noteNumber, noteFrames, note.lyric });

        currentTime = note.endPositionInSeconds;
    }

    // Add final silence (4 frames)
    scoreNotes.add({ juce::var(), kInitialAndFinalSilence, "" });

    return scoreNotes;
}

juce::var createScoreJsonInternal(const SongEditorDocumentData& documentData)
{
    auto scoreNotes = convertToScoreNotes(documentData);

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

juce::String createScoreJsonStringInternal(const SongEditorDocumentData& documentData)
{
    return juce::JSON::toString(createScoreJsonInternal(documentData));
}

void createDoReMiScoreDocument(SongEditorDocumentData& data)
{
    // Clear any existing notes
    data.notes.clear();

    // Define the note durations and gaps
    double noteDuration = 0.5;  // Each note lasts 0.5 seconds
    double noteGap = 0.01;      // Small gap between notes

    // Define the melody using MIDI note numbers
    // C4 = 60, D4 = 62, E4 = 64, F4 = 65, G4 = 67, A4 = 69, B4 = 71, C5 = 72
    int melody[] = { 60, 62, 64, 65, 67, 69, 71, 72, 72, 71, 69, 67, 65, 64, 62, 60 };

    // Define the lyrics
    const char* lyrics[] = {
        "\xe3\x83\x89", "\xe3\x83\xac", "\xe3\x83\x9f", "\xe3\x83\x95\xe3\x82\xa1",
        "\xe3\x82\xbd", "\xe3\x83\xa9", "\xe3\x82\xb7", "\xe3\x83\x89",
        "\xe3\x83\x89", "\xe3\x82\xb7", "\xe3\x83\xa9", "\xe3\x82\xbd",
        "\xe3\x83\x95\xe3\x82\xa1", "\xe3\x83\x9f", "\xe3\x83\xac", "\xe3\x83\x89"
    };

    double startTime = 0.0;

    for (int i = 0; i < 16; ++i)
    {
        data.notes.add({
            startTime,
            startTime + noteDuration,
            melody[i],
            juce::CharPointer_UTF8(lyrics[i]),
            ""  // No extra phoneme
            });

        startTime += noteDuration + noteGap;
    }
}

//==============================================================================
class TimeDomainNoteSorter
{
public:
    static int compareElements(const SongEditorNoteBasic& first, const SongEditorNoteBasic& second)
    {
        if (first.startPositionInSeconds < second.startPositionInSeconds)
            return -1;
        if (first.startPositionInSeconds > second.startPositionInSeconds)
            return 1;
        return 0;
    }
};

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


}

//==============================================================================
SongDocumentEditor::SongDocumentEditor()
    : beatTimePointList(nullptr)
{
    editorContext = std::make_unique<cctn::song::SongDocumentEditor::EditorContext>();

    quantizeEngine = std::make_unique<cctn::song::QuantizeEngine>();

    documentData = std::make_unique<cctn::song::SongEditorDocumentData>();
}

SongDocumentEditor::~SongDocumentEditor()
{
}

//==============================================================================
void SongDocumentEditor::attachDocument(std::shared_ptr<cctn::song::SongDocument> document)
{
    documentToEdit = document;
}

void SongDocumentEditor::detachDocument()
{
}

juce::String SongDocumentEditor::debugDumpDocument() const
{
    if (documentToEdit.get() == nullptr)
    {
        return juce::String();
    }

    return juce::String(documentToEdit->dumpToString());
}

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

// Helper function to convert the JSON to a string
static juce::String createScoreJsonStringFromSongDocument(const cctn::song::SongDocument& doc)
{
    return juce::JSON::toString(createScoreJsonFromSongDocument(doc));
}

//==============================================================================
juce::var SongDocumentEditor::createScoreJson_outdated() const
{
    // Total duration of the document in seconds
    double document_duration = calculateDocumentDuration(*documentData);
    const auto filled_silence_score = makeSilenceFilledScore(*documentData, document_duration);
    return createScoreJsonInternal(filled_silence_score);
}

juce::String SongDocumentEditor::createScoreJsonString_outdated() const
{
    double document_duration = calculateDocumentDuration(*documentData);
    const auto filled_silence_score = makeSilenceFilledScore(*documentData, document_duration);
    return createScoreJsonStringInternal(filled_silence_score);
}

juce::String SongDocumentEditor::createScoreJsonString() const
{
    if (documentToEdit.get() == nullptr)
    {
        return juce::String();
    }

    juce::Logger::outputDebugString(documentToEdit->dumpToString());

    juce::Logger::outputDebugString(juce::JSON::toString(cctn::song::createScoreJsonFromSongDocument(*documentToEdit.get())));

    return juce::JSON::toString(cctn::song::createScoreJsonFromSongDocument(*documentToEdit.get()));
}

//==============================================================================
void SongDocumentEditor::serialize() const
{
}

void SongDocumentEditor::deserialize()
{
}

//==============================================================================
std::optional<cctn::song::SongEditorNoteExtended> SongDocumentEditor::findNote(const cctn::song::QueryForFindPianoRollNote& query)
{
    for (auto& note : (*documentData).notes)
    {
        if (juce::Range<double>(note.startPositionInSeconds, note.endPositionInSeconds).contains(query.timeInSeconds))
        {
            return note;
        }
    }

    return std::nullopt;
}

void SongDocumentEditor::selectNote(const cctn::song::QueryForFindPianoRollNote& query)
{
    for (auto& note : (*documentData).notes)
    {
        if (juce::Range<double>(note.startPositionInSeconds, note.endPositionInSeconds).contains(query.timeInSeconds) &&
            note.noteNumber == query.noteNumber)
        {
            note.isSelected = true;
        }
        else
        {
            note.isSelected = false;
        }
    }

    sendChangeMessage();
}

void SongDocumentEditor::createNote(const cctn::song::QueryForAddPianoRollNote& query)
{
    cctn::song::SongEditorNoteExtended new_note;

    new_note.noteNumber = query.noteNumber;
    new_note.startPositionInSeconds = query.startTimeInSeconds;
    new_note.endPositionInSeconds = query.endTimeInSeconds;
    new_note.isSelected = true;
    new_note.lyric = editorContext->currentNoteLyric.text;

    if (query.snapToQuantizeGrid)
    {
        const auto quantize_region_optional = quantizeEngine->findNearestQuantizeRegion(query.startTimeInSeconds);
        if (quantize_region_optional.has_value())
        {
            new_note.startPositionInSeconds = quantize_region_optional.value().startPositionInSeconds;
            new_note.endPositionInSeconds = quantize_region_optional.value().endPositionInSeconds;
        }

        // TODO: support offset
        const auto region_optional = quantizeEngine->findNearestQuantizeRegion(query.startTimeInSeconds);
        if (region_optional.has_value())
        {
            // Get the note values
            const auto note_end_position_in_seconds =
                calculate_note_end_time(
                    region_optional.value().startPositionInSeconds,
                    region_optional.value().endPositionInSeconds,
                    editorContext->currentGridInterval,
                    editorContext->currentNoteLength);

            new_note.startPositionInSeconds = region_optional.value().startPositionInSeconds;
            new_note.endPositionInSeconds = note_end_position_in_seconds;
        }
    }

    for (auto& note : (*documentData).notes)
    {
        if (juce::Range<double>(new_note.startPositionInSeconds, new_note.endPositionInSeconds).contains(note.startPositionInSeconds))
        {
            new_note.endPositionInSeconds = note.startPositionInSeconds;
        }
    }

    documentData->notes.add(new_note);

    sendChangeMessage();
}

void SongDocumentEditor::deleteNoteSingle(const cctn::song::QueryForFindPianoRollNote& query)
{
    cctn::song::SongEditorNoteExtended* note_to_delete = nullptr;

    for (auto& note : (*documentData).notes)
    {
        if (juce::Range<double>(note.startPositionInSeconds, note.endPositionInSeconds).contains(query.timeInSeconds))
        {
            note_to_delete = &note;
        }
    }

    if (note_to_delete != nullptr)
    {
        documentData->notes.remove(note_to_delete);
    }

    sendChangeMessage();
}

void SongDocumentEditor::updateQuantizeRegions(const juce::AudioPlayHead::PositionInfo& positionInfo)
{
    double bpm = 120.0;
    int numerator = 4;
    int denominator = 4;

    const auto tempo_and_time_signature_optional = cctn::song::PositionInfoExtractor::extractTempoAndTimeSignature(positionInfo);
    if (tempo_and_time_signature_optional.has_value())
    {
        bpm = tempo_and_time_signature_optional.value().bpm;
        numerator = tempo_and_time_signature_optional.value().numerator;
        denominator = tempo_and_time_signature_optional.value().denominator;
    }

    beatTimePointList = std::make_unique<BeatTimePointList>(BeatTimePointFactory::extractPreciseBeatPoints(bpm, numerator, denominator, 0.0, 600.0, editorContext->currentGridInterval));

    if (beatTimePointList.get() != nullptr)
    {
        quantizeEngine->updateQuantizeRegions(*beatTimePointList.get());
    }
}

std::optional<QuantizeEngine::Region> SongDocumentEditor::findNearestQuantizeRegion(double timePositionInSeconds) const
{
    return quantizeEngine->findNearestQuantizeRegion(timePositionInSeconds);
}

//==============================================================================
double SongDocumentEditor::calculateDocumentDuration(const cctn::song::SongEditorDocumentData& data, double minimumDuration)
{
    if (data.notes.isEmpty())
    {
        return minimumDuration;
    }

    double latestEndTime = 0.0;
    for (const auto& note : data.notes)
    {
        latestEndTime = std::max(latestEndTime, note.endPositionInSeconds);
    }

    return std::max(latestEndTime, minimumDuration);
}

SongEditorNoteExtended SongDocumentEditor::createSilenceNote(double startPositionInSeconds, double endPositionInSeconds)
{
    // Use -1 as noteNumber to represent silence
    SongEditorNoteExtended silence_note;
    silence_note.startPositionInSeconds = startPositionInSeconds;
    silence_note.endPositionInSeconds = endPositionInSeconds;
    silence_note.noteNumber = -1;
    silence_note.lyric = "";
    silence_note.extraPhoneme = "";
    silence_note.isSelected = false;

    return silence_note;
}

cctn::song::SongEditorDocumentData SongDocumentEditor::makeSilenceFilledScore(const cctn::song::SongEditorDocumentData& data, double documentDuration)
{
    if (data.notes.isEmpty())
    {
        // If there are no notes, add a single silence for the entire duration
        return {};
    }

    juce::Array<SongEditorNoteExtended> notes_sorted = data.notes;
    {
        // Sort notes by start time using JUCE Array's sort method with SortFunctionConverter
        TimeDomainNoteSorter note_sorter;
        notes_sorted.sort(note_sorter);
    }

    juce::Array<SongEditorNoteExtended> notes_artefact;
    {
        // Add silence at the beginning if needed
        // Ignore under 0.05 msec gap.
        // 1000 msec / 24000 Hz * 256 samples per frame * 4 frames = 0.0426 sec
        if (notes_sorted.getFirst().startPositionInSeconds > 0.05)
        {
            notes_artefact.add(createSilenceNote(0.0, notes_sorted.getFirst().startPositionInSeconds));
        }

        // Add existing notes and fill gaps
        for (int note_idx = 0; note_idx < notes_sorted.size(); note_idx++)
        {
            notes_artefact.add(notes_sorted[note_idx]);

            if (note_idx < notes_sorted.size() - 1)
            {
                const double gap_start_time = notes_sorted[note_idx].endPositionInSeconds;
                const double gap_end_time = notes_sorted[note_idx + 1].startPositionInSeconds;

                // Ignore under 0.05 msec gap.
                // 1000 msec / 24000 Hz * 256 samples per frame * 4 frames = 0.0426 sec
                if ((gap_end_time - gap_start_time) > 0.05)
                {
                    notes_artefact.add(createSilenceNote(gap_start_time, gap_end_time));
                }
            }
        }

        // Add silence at the end if needed
        if (notes_sorted.getLast().endPositionInSeconds < documentDuration)
        {
            notes_artefact.add(createSilenceNote(notes_sorted.getLast().endPositionInSeconds, documentDuration));
        }
    }

    // Return silence filled notes
    return cctn::song::SongEditorDocumentData{ notes_artefact };
}

//==============================================================================
std::optional<const cctn::song::SongEditorDocumentData*> SongDocumentEditor::getRawDocumentData() const
{
    if (documentData.get() != nullptr)
    {
        return documentData.get();
    }

    return std::nullopt;
}

}
}
