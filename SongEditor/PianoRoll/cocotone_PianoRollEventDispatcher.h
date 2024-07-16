#pragma once

namespace cctn
{
namespace song
{

//==============================================================================
class IPianoRollEventTarget
{
public:
    //==============================================================================
    virtual ~IPianoRollEventTarget() = default;

    //==============================================================================
    virtual bool testNoteExist(const cctn::song::QueryForFindPianoRollNote& query) = 0;
    virtual void selectNote(const cctn::song::QueryForFindPianoRollNote& query) = 0;
    virtual void addNote(const cctn::song::QueryForAddPianoRollNote& query) = 0;
    virtual void deleteNoteSingle(const cctn::song::QueryForFindPianoRollNote& query) = 0;

private:
    //==============================================================================

    JUCE_LEAK_DETECTOR(IPianoRollEventTarget)
};

//==============================================================================
class PianoRollEventDispatcher final
{
public:
    //==============================================================================
    explicit PianoRollEventDispatcher(std::shared_ptr<cctn::song::IPianoRollEventTarget> eventTarget);
    ~PianoRollEventDispatcher();

    //==============================================================================
    bool emitTestNoteExist(const cctn::song::QueryForFindPianoRollNote& query);
    void emitSelectNote(const cctn::song::QueryForFindPianoRollNote& query);
    void emitAddNote(const cctn::song::QueryForAddPianoRollNote& query);
    void emitDeleteNoteSingle(const cctn::song::QueryForFindPianoRollNote& query);

private:
    //==============================================================================
    std::weak_ptr<cctn::song::IPianoRollEventTarget> pianoRollEventTarget;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PianoRollEventDispatcher)
};

}
}
