#pragma once

namespace cctn
{
namespace song
{

class SongDocument;
class MutliTrackEditor;
class PianoRollEditor;

//==============================================================================
class SongEditorOperation final
{
public:
    //==============================================================================
    SongEditorOperation();
    ~SongEditorOperation();

    //==============================================================================
    static std::unique_ptr<cctn::song::SongDocument> makeDefaultSongDocument();

    //==============================================================================
    bool testNoteExist(const cctn::song::QueryForFindPianoRollNote& query);
    void selectNote(const cctn::song::QueryForFindPianoRollNote& query);
    void addNote(const cctn::song::QueryForAddPianoRollNote& query);
    void deleteNoteSingle(const cctn::song::QueryForFindPianoRollNote& query);

private:
    //==============================================================================
    void attachDocument(std::shared_ptr<cctn::song::SongDocumentEditor> documentToAttach);
    void detachDocument();

    friend class SongEditor;
    friend class MutliTrackEditor;
    friend class PianoRollEditor;

    mutable std::mutex mutex;
    std::weak_ptr<cctn::song::SongDocumentEditor> attachedDocument;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SongEditorOperation)
};

}
}
