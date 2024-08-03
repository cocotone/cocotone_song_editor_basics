#include "cocotone_SongEditorDocument.h"
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

}

//==============================================================================
SongEditorDocument::SongEditorDocument()
    : beatTimePointList(nullptr)
{
    documentContext = std::make_unique<cctn::song::SongEditorDocument::DocumentContext>();

    quantizeEngine = std::make_unique<cctn::song::QuantizeEngine>();

    documentData = std::make_unique<cctn::song::SongEditorDocumentData>();
}

SongEditorDocument::~SongEditorDocument()
{
}

//==============================================================================
juce::var SongEditorDocument::createScoreJson() const
{
    // Total duration of the document in seconds
    double document_duration = calculateDocumentDuration(*documentData);
    const auto filled_silence_score = makeSilenceFilledScore(*documentData, document_duration);
    return createScoreJsonInternal(filled_silence_score);
}

juce::String SongEditorDocument::createScoreJsonString() const
{
    double document_duration = calculateDocumentDuration(*documentData);
    const auto filled_silence_score = makeSilenceFilledScore(*documentData, document_duration);
    return createScoreJsonStringInternal(filled_silence_score);
}

//==============================================================================
void SongEditorDocument::serialize() const
{
}

void SongEditorDocument::deserialize()
{
}

//==============================================================================
std::optional<cctn::song::SongEditorNoteExtended> SongEditorDocument::findNote(const cctn::song::QueryForFindPianoRollNote& query)
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

void SongEditorDocument::selectNote(const cctn::song::QueryForFindPianoRollNote& query)
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

void SongEditorDocument::createNote(const cctn::song::QueryForAddPianoRollNote& query)
{
    cctn::song::SongEditorNoteExtended new_note;

    new_note.noteNumber = query.noteNumber;
    new_note.startPositionInSeconds = query.startTimeInSeconds;
    new_note.endPositionInSeconds = query.endTimeInSeconds;
    new_note.isSelected = true;
    new_note.lyric = documentContext->currentNoteLyric.text;

    if (query.snapToQuantizeGrid)
    {
        const auto quantize_region_optional = quantizeEngine->findNearestQuantizeRegion(query.startTimeInSeconds);
        if (quantize_region_optional.has_value())
        {
            new_note.startPositionInSeconds = quantize_region_optional.value().startPositionInSeconds;
            new_note.endPositionInSeconds = quantize_region_optional.value().endPositionInSeconds;
        }
    }

    for (auto& note : (*documentData).notes)
    {
        if (juce::Range<double>(note.startPositionInSeconds, note.endPositionInSeconds).contains(new_note.endPositionInSeconds))
        {
            new_note.endPositionInSeconds = note.startPositionInSeconds;
        }
    }

    documentData->notes.add(new_note);

    sendChangeMessage();
}

void SongEditorDocument::deleteNoteSingle(const cctn::song::QueryForFindPianoRollNote& query)
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

void SongEditorDocument::updateQuantizeRegions(const juce::AudioPlayHead::PositionInfo& positionInfo)
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

    beatTimePointList = std::make_unique<BeatTimePointList>(BeatTimePointFactory::extractPreciseBeatPoints(bpm, numerator, denominator, 0.0, 600.0, documentContext->currentGridInterval));

    if (beatTimePointList.get() != nullptr)
    {
        quantizeEngine->updateQuantizeRegions(*beatTimePointList.get());
    }
}

std::optional<QuantizeEngine::Region> SongEditorDocument::findNearestQuantizeRegion(double timePositionInSeconds) const
{
    return quantizeEngine->findNearestQuantizeRegion(timePositionInSeconds);
}

//==============================================================================
double SongEditorDocument::calculateDocumentDuration(const cctn::song::SongEditorDocumentData& data, double minimumDuration)
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

SongEditorNoteExtended SongEditorDocument::createSilenceNote(double startPositionInSeconds, double endPositionInSeconds)
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

cctn::song::SongEditorDocumentData SongEditorDocument::makeSilenceFilledScore(const cctn::song::SongEditorDocumentData& data, double documentDuration)
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
std::optional<const cctn::song::SongEditorDocumentData*> SongEditorDocument::getRawDocumentData() const
{
    if (documentData.get() != nullptr)
    {
        return documentData.get();
    }

    return std::nullopt;
}

}
}
