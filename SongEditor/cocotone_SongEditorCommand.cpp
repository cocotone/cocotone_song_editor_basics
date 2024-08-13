namespace cctn
{
namespace song
{

//==============================================================================
SongEditorOperation::SongEditorOperation()
{
}

SongEditorOperation::~SongEditorOperation()
{
    attachedDocument.reset();
}

//==============================================================================
std::unique_ptr<cctn::song::SongDocument> SongEditorOperation::makeDefaultSongDocument()
{
    auto document = std::make_unique<cctn::song::SongDocument>();

    // Set metadata
    document->setMetadata(
        juce::CharPointer_UTF8("\xe3\x82\xbd\xe3\x83\xb3\xe3\x82\xb0"),
        juce::CharPointer_UTF8("\xe3\x82\xa2\xe3\x83\xbc\xe3\x83\x86\xe3\x82\xa3\xe3\x82\xb9\xe3\x83\x88")
    );

    // Start: 4/4 time signature, 120 BPM
    document->addTempoEvent(0, cctn::song::SongDocument::TempoEvent::TempoEventType::kBoth, 4, 4, 120.0);

    return std::move(document);
}

//==============================================================================
void SongEditorOperation::attachDocument(std::shared_ptr<cctn::song::SongDocumentEditor> documentToAttach)
{
    std::lock_guard lock(mutex);

    attachedDocument = documentToAttach;
}

void SongEditorOperation::detachDocument()
{
    std::lock_guard lock(mutex);

    attachedDocument.reset();
}

//==============================================================================
bool SongEditorOperation::testNoteExist(const cctn::song::QueryForFindPianoRollNote& query)
{
    std::lock_guard lock(mutex);

    if (attachedDocument.expired())
    {
        return false;
    }

    const auto note_optional = attachedDocument.lock()->findNote(query);
    if (note_optional.has_value())
    {
        return true;
    }
    return false;
}

void SongEditorOperation::selectNote(const cctn::song::QueryForFindPianoRollNote& query)
{
    std::lock_guard lock(mutex);

    if (attachedDocument.expired())
    {
        return;
    }

    attachedDocument.lock()->selectNote(query);
}

void SongEditorOperation::addNote(const cctn::song::QueryForAddPianoRollNote& query)
{
    std::lock_guard lock(mutex);

    if (attachedDocument.expired())
    {
        return;
    }

    attachedDocument.lock()->createNote(query);
}

void SongEditorOperation::deleteNoteSingle(const cctn::song::QueryForFindPianoRollNote& query)
{
    std::lock_guard lock(mutex);

    if (attachedDocument.expired())
    {
        return;
    }

    attachedDocument.lock()->deleteNoteSingle(query);
}

}
}