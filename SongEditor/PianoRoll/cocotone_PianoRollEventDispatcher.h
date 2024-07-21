#pragma once

namespace cctn
{
namespace song
{

//==============================================================================
class PianoRollEventDispatcher final
{
public:
    //==============================================================================
    explicit PianoRollEventDispatcher(std::shared_ptr<cctn::song::SongEditorOperation> eventTarget);
    ~PianoRollEventDispatcher();

    //==============================================================================
    bool emitTestNoteExist(const cctn::song::QueryForFindPianoRollNote& query);
    void emitSelectNote(const cctn::song::QueryForFindPianoRollNote& query);
    void emitAddNote(const cctn::song::QueryForAddPianoRollNote& query);
    void emitDeleteNoteSingle(const cctn::song::QueryForFindPianoRollNote& query);

private:
    //==============================================================================
    std::weak_ptr<cctn::song::SongEditorOperation> pianoRollEventTarget;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PianoRollEventDispatcher)
};

}
}
