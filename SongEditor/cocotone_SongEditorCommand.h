#pragma once

namespace cctn
{
namespace song
{

//==============================================================================
class SongEditorDocument;

class SongEditorEventBridge
    : public cctn::song::IPianoRollEventTarget
{
public:
    //==============================================================================
    SongEditorEventBridge();
    ~SongEditorEventBridge() override;

    //==============================================================================
    void attachDocument(std::shared_ptr<cctn::song::SongEditorDocument> documentToAttach);
    void detachDocument();

private:
    //==============================================================================
    bool testNoteExist(const cctn::song::QueryForFindPianoRollNote& query) override;
    void selectNote(const cctn::song::QueryForFindPianoRollNote& query) override;
    void addNote(const cctn::song::QueryForAddPianoRollNote& query) override;
    void deleteNoteSingle(const cctn::song::QueryForFindPianoRollNote& query) override;

    mutable std::mutex mutex;
    std::weak_ptr<cctn::song::SongEditorDocument> attachedDocument;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SongEditorEventBridge)
};

}
}
