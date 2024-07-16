#include "cocotone_SongEditorCommand.h"
namespace cctn
{
namespace song
{

//==============================================================================
SongEditorEventBridge::SongEditorEventBridge()
{
}

SongEditorEventBridge::~SongEditorEventBridge()
{
    attachedDocument.reset();
}

//==============================================================================
void SongEditorEventBridge::attachDocument(std::shared_ptr<cctn::song::SongEditorDocument> documentToAttach)
{
    std::lock_guard lock(mutex);

    attachedDocument = documentToAttach;
}

void SongEditorEventBridge::detachDocument()
{
    std::lock_guard lock(mutex);

    attachedDocument.reset();
}

//==============================================================================
bool SongEditorEventBridge::testNoteExist(const cctn::song::QueryForFindPianoRollNote& query)
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

void SongEditorEventBridge::selectNote(const cctn::song::QueryForFindPianoRollNote& query)
{
    std::lock_guard lock(mutex);

    if (attachedDocument.expired())
    {
        return;
    }

    attachedDocument.lock()->selectNote(query);
}

void SongEditorEventBridge::addNote(const cctn::song::QueryForAddPianoRollNote& query)
{
    std::lock_guard lock(mutex);

    if (attachedDocument.expired())
    {
        return;
    }

    attachedDocument.lock()->addNote(query);
}

void SongEditorEventBridge::deleteNoteSingle(const cctn::song::QueryForFindPianoRollNote& query)
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