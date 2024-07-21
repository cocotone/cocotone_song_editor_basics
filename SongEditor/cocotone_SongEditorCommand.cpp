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
void SongEditorOperation::attachDocument(std::shared_ptr<cctn::song::SongEditorDocument> documentToAttach)
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