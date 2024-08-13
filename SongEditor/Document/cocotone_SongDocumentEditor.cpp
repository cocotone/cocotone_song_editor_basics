namespace cctn
{
namespace song
{

//==============================================================================
SongDocumentEditor::SongDocumentEditor()
{
    editorContext = std::make_unique<cctn::song::SongDocumentEditor::EditorContext>();

    quantizeEngine = std::make_unique<cctn::song::QuantizeEngine>();
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
std::optional<cctn::song::SongDocument::Note> SongDocumentEditor::findNote(const cctn::song::QueryForFindPianoRollNote& query)
{
    if (documentToEdit.get() == nullptr)
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
    if (documentToEdit.get() == nullptr)
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
    if (documentToEdit.get() == nullptr)
    {
        return;
    }

    if (query.snapToQuantizeGrid)
    {
        const auto quantize_region_optional = quantizeEngine->findNearestQuantizeRegion(query.startTimeInSeconds);
        if (quantize_region_optional.has_value())
        {
            const auto& start_time = quantize_region_optional.value().startMusicalTime;

            const auto note_duration = 
                cctn::song::SongDocument::DataFactory::convertNoteLengthToDuration(*documentToEdit.get(), editorContext->currentNoteLength, start_time);

            const auto new_note =
                cctn::song::SongDocument::DataFactory::makeNote(
                    *documentToEdit.get(),
                    start_time,
                    note_duration,
                    query.noteNumber, 100,
                    editorContext->currentNoteLyric.text);

            documentToEdit->addNote(new_note);

            editorContext->currentSelectedNoteId = new_note.id;
        }
    }

    sendChangeMessage();
}

void SongDocumentEditor::deleteNoteSingle(const cctn::song::QueryForFindPianoRollNote& query)
{
    if (documentToEdit.get() == nullptr)
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

std::optional<cctn::song::SongDocument::RegionWithBeatInfo> SongDocumentEditor::findNearestQuantizeRegion(double timePositionInSeconds) const
{
    return quantizeEngine->findNearestQuantizeRegion(timePositionInSeconds);
}

//==============================================================================
void SongDocumentEditor::updateEditorContext()
{
    if (documentToEdit.get() == nullptr)
    {
        editorContext->currentBeatTimePoints = {};
        return;
    }

    editorContext->currentBeatTimePoints = cctn::song::SongDocument::BeatTimePointsFactory::makeBeatTimePoints(*documentToEdit.get(), editorContext->currentGridInterval);
    quantizeEngine->updateQuantizeRegions(editorContext->currentBeatTimePoints);
}

}
}
