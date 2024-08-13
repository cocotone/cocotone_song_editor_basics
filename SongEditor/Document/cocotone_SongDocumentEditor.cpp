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

    sendChangeMessage();
}

void SongDocumentEditor::detachDocument()
{
    documentToEdit.reset();

    updateEditorContext();

    sendChangeMessage();
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

    for (auto& note : (*documentToEdit).getNotes())
    {
        const auto note_on_tick = cctn::song::SongDocument::Calculator::barToTick((*documentToEdit), note.startTimeInMusicalTime);
        const auto note_on_position_in_seconds = cctn::song::SongDocument::Calculator::tickToAbsoluteTime((*documentToEdit), note_on_tick);

        const auto note_off_bar = cctn::song::SongDocument::Calculator::calculateNoteOffPosition((*documentToEdit), note);
        const auto note_off_tick = cctn::song::SongDocument::Calculator::barToTick((*documentToEdit), note_off_bar);
        const auto note_off_position_in_seconds = cctn::song::SongDocument::Calculator::tickToAbsoluteTime((*documentToEdit), note_off_tick);

        if (juce::Range<double>(note_on_position_in_seconds, note_off_position_in_seconds).contains(query.timeInSeconds))
        {
            return note;
        }
    }

    return std::nullopt;
}

void SongDocumentEditor::selectNote(const cctn::song::QueryForFindPianoRollNote& query)
{
    if (documentToEdit.get() == nullptr)
    {
        return;
    }

    for (auto& note : (*documentToEdit).getNotes())
    {
        const auto note_on_tick = cctn::song::SongDocument::Calculator::barToTick((*documentToEdit), note.startTimeInMusicalTime);
        const auto note_on_position_in_seconds = cctn::song::SongDocument::Calculator::tickToAbsoluteTime((*documentToEdit), note_on_tick);

        const auto note_off_bar = cctn::song::SongDocument::Calculator::calculateNoteOffPosition((*documentToEdit), note);
        const auto note_off_tick = cctn::song::SongDocument::Calculator::barToTick((*documentToEdit), note_off_bar);
        const auto note_off_position_in_seconds = cctn::song::SongDocument::Calculator::tickToAbsoluteTime((*documentToEdit), note_off_tick);

        if (juce::Range<double>(note_on_position_in_seconds, note_off_position_in_seconds).contains(query.timeInSeconds))
        {
            editorContext->currentSelectedNoteId = note.id;
            break;
        }
        else
        {
            editorContext->currentSelectedNoteId = -1;
        }
    }

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
                cctn::song::SongDocument::DataFactory::convertNoteLengthToDuration(*documentToEdit.get(), editorContext->currentNoteLength);

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

    const cctn::song::SongDocument::Note* note_to_delete = nullptr;

    for (const auto& note : (*documentToEdit).getNotes())
    {
        const auto note_on_tick = cctn::song::SongDocument::Calculator::barToTick((*documentToEdit), note.startTimeInMusicalTime);
        const auto note_on_position_in_seconds = cctn::song::SongDocument::Calculator::tickToAbsoluteTime((*documentToEdit), note_on_tick);

        const auto note_off_bar = cctn::song::SongDocument::Calculator::calculateNoteOffPosition((*documentToEdit), note);
        const auto note_off_tick = cctn::song::SongDocument::Calculator::barToTick((*documentToEdit), note_off_bar);
        const auto note_off_position_in_seconds = cctn::song::SongDocument::Calculator::tickToAbsoluteTime((*documentToEdit), note_off_tick);

        if (juce::Range<double>(note_on_position_in_seconds, note_off_position_in_seconds).contains(query.timeInSeconds))
        {
            note_to_delete = &note;
        }
    }

    if (note_to_delete != nullptr)
    {
        documentToEdit->removeNote(note_to_delete);
    }

    sendChangeMessage();
}

//==============================================================================
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

    editorContext->currentBeatTimePoints = cctn::song::SongDocument::BeatTimePointsFactory::makeBeatTimePoints(*documentToEdit.get(), editorContext->currentGridSize);
    quantizeEngine->updateQuantizeRegions(editorContext->currentBeatTimePoints);
}

}
}
