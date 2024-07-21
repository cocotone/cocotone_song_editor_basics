#include "cocotone_PianoRollEventDispatcher.h"
namespace cctn
{
namespace song
{

//==============================================================================
PianoRollEventDispatcher::PianoRollEventDispatcher(std::shared_ptr<cctn::song::SongEditorOperation> eventTarget)
    : pianoRollEventTarget(eventTarget)
{
}

PianoRollEventDispatcher::~PianoRollEventDispatcher()
{
}

//==============================================================================
bool PianoRollEventDispatcher::emitTestNoteExist(const cctn::song::QueryForFindPianoRollNote& query)
{
    if (pianoRollEventTarget.expired())
    {
        return false;
    }

    return pianoRollEventTarget.lock()->testNoteExist(query);
}

void PianoRollEventDispatcher::emitSelectNote(const cctn::song::QueryForFindPianoRollNote& query)
{
    if (pianoRollEventTarget.expired())
    {
        return;
    }

    pianoRollEventTarget.lock()->selectNote(query);
}

void PianoRollEventDispatcher::emitAddNote(const cctn::song::QueryForAddPianoRollNote& query)
{
    if (pianoRollEventTarget.expired())
    {
        return;
    }

    pianoRollEventTarget.lock()->addNote(query);
}

void PianoRollEventDispatcher::emitDeleteNoteSingle(const cctn::song::QueryForFindPianoRollNote& query)
{
    if (pianoRollEventTarget.expired())
    {
        return;
    }

    pianoRollEventTarget.lock()->deleteNoteSingle(query);
}

}
}
