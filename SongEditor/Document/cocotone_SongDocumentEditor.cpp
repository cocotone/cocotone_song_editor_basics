#include "cocotone_SongDocumentEditor.h"
namespace cctn
{
namespace song
{

//==============================================================================
SongDocumentEditor::SongDocumentEditor()
{
    editorContext = std::make_unique<cctn::song::SongDocumentEditor::EditorContext>();

    quantizeEngine = std::make_unique<cctn::song::QuantizeEngine>();

#if 0
    documentData = std::make_unique<cctn::song::SongEditorDocumentData>();
#endif
}

SongDocumentEditor::~SongDocumentEditor()
{
}

//==============================================================================
void SongDocumentEditor::attachDocument(std::shared_ptr<cctn::song::SongDocument> document)
{
    documentToEdit = document;

    updateEditorContext();
}

void SongDocumentEditor::detachDocument()
{
    documentToEdit.reset();

    updateEditorContext();
}

std::optional<const cctn::song::SongDocument*> SongDocumentEditor::getCurrentDocument() const
{
    if (documentToEdit.get() != nullptr)
    {
        return documentToEdit.get();
    }

    return std::nullopt;
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
void SongDocumentEditor::serialize() const
{
}

void SongDocumentEditor::deserialize()
{
}

//==============================================================================
std::optional<cctn::song::SongDocument::Note> SongDocumentEditor::findNote(const cctn::song::QueryForFindPianoRollNote& query)
{
    if (documentToEdit.get() != nullptr)
    {
        return std::nullopt;
    }

#if 0
    for (auto& note : (*documentToEdit).getNotes())
    {
        if (juce::Range<double>(note.startPositionInSeconds, note.endPositionInSeconds).contains(query.timeInSeconds))
        {
            return note;
        }
    }
#endif

    return std::nullopt;
}

void SongDocumentEditor::selectNote(const cctn::song::QueryForFindPianoRollNote& query)
{
    if (documentToEdit.get() != nullptr)
    {
        return;
    }

#if 0
    for (auto& note : (*documentToEdit).getNotes())
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
#endif

    sendChangeMessage();
}

void SongDocumentEditor::createNote(const cctn::song::QueryForAddPianoRollNote& query)
{
    if (documentToEdit.get() != nullptr)
    {
        return;
    }

    cctn::song::SongDocument::Note new_note;

#if 0
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
#endif

    sendChangeMessage();
}

void SongDocumentEditor::deleteNoteSingle(const cctn::song::QueryForFindPianoRollNote& query)
{
    if (documentToEdit.get() != nullptr)
    {
        return;
    }

    cctn::song::SongDocument::Note* note_to_delete = nullptr;

#if 0
    for (auto& note : (*documentToEdit).getNotes())
    {
        if (juce::Range<double>(note.startPositionInSeconds, note.endPositionInSeconds).contains(query.timeInSeconds))
        {
            note_to_delete = &note;
        }
    }

    if (note_to_delete != nullptr)
    {
        documentToEdit->getNotes().remove(note_to_delete);
    }
#endif

    sendChangeMessage();
}

#if 0
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
#endif

void SongDocumentEditor::updateQuantizeRegions(const juce::AudioPlayHead::PositionInfo& positionInfo)
{
    return;

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

    if (documentToEdit.get() == nullptr)
    {
        return;
    }

    //beatTimePointList = std::make_unique<BeatTimePointList>(BeatTimePointFactory::extractPreciseBeatPoints(bpm, numerator, denominator, 0.0, 600.0, editorContext->currentGridInterval));
    editorContext->currentBeatTimePointList = cctn::song::BeatTimePointFactory::extractPreciseBeatPoints(*documentToEdit.get(), 0.0, 600.0, editorContext->currentGridInterval);
    quantizeEngine->updateQuantizeRegions(editorContext->currentBeatTimePointList);
}

std::optional<QuantizeEngine::Region> SongDocumentEditor::findNearestQuantizeRegion(double timePositionInSeconds) const
{
    return quantizeEngine->findNearestQuantizeRegion(timePositionInSeconds);
}

//==============================================================================
void SongDocumentEditor::updateEditorContext()
{
    if (documentToEdit.get() == nullptr)
    {
        editorContext->currentBeatTimePointList = {};
        return;
    }

    editorContext->currentBeatTimePointList = cctn::song::BeatTimePointFactory::extractPreciseBeatPoints(*documentToEdit.get(), 0.0, 600.0, editorContext->currentGridInterval);
    quantizeEngine->updateQuantizeRegions(editorContext->currentBeatTimePointList);
}

#if 0
//==============================================================================
std::optional<const cctn::song::SongEditorDocumentData*> SongDocumentEditor::getRawDocumentData() const
{
    if (documentData.get() != nullptr)
    {
        return documentData.get();
    }

    return std::nullopt;
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

    return juce::JSON::toString(cctn::song::createScoreJsonFromSongDocument(*documentToEdit.get()));
}
#endif

}
}
