#pragma once

namespace cctn
{
namespace song
{

class QuantizeEngine;

//==============================================================================
class SongDocumentEditor
    : public juce::ChangeBroadcaster
{
public:
    //==============================================================================
    SongDocumentEditor();
    ~SongDocumentEditor() override;

    //==============================================================================
    void attachDocument(std::shared_ptr<cctn::song::SongDocument> document);
    void detachDocument();
    std::optional<const cctn::song::SongDocument*> getCurrentDocument() const;
    juce::String debugDumpDocument() const;

    //==============================================================================
    std::optional<cctn::song::SongDocument::Note> findNote(const cctn::song::QueryForFindPianoRollNote& query);
    void selectNote(const cctn::song::QueryForFindPianoRollNote& query);

    // CRUD operation
    void createNote(const cctn::song::QueryForAddPianoRollNote& query);
    void readNote(const cctn::song::QueryForAddPianoRollNote& query) {};
    void updateNote(const cctn::song::QueryForAddPianoRollNote& query) {};
    void deleteNoteSingle(const cctn::song::QueryForFindPianoRollNote& query);

    //==============================================================================
    std::optional<cctn::song::SongDocument::RegionWithBeatInfo> findNearestQuantizeRegion(double timePositionInSeconds) const;

    //==============================================================================
    class EditorContext
    {
    public:
        cctn::song::NoteLength currentGridInterval{ cctn::song::NoteLength::Quarter };
        cctn::song::NoteLength currentNoteLength{ cctn::song::NoteLength::Quarter };
        cctn::song::NoteLyric currentNoteLyric{ juce::CharPointer_UTF8("\xe3\x83\xa9") }; // ra
        cctn::song::SongDocument::BeatTimePoints currentBeatTimePoints{};
        int currentSelectedNoteId{ -1 };

    private:
        JUCE_LEAK_DETECTOR(EditorContext)
    };
    void updateEditorContext();
    EditorContext& getEditorContext() const { return *editorContext.get(); };

private:
    //==============================================================================
    std::shared_ptr<cctn::song::SongDocument> documentToEdit;
    std::unique_ptr<cctn::song::QuantizeEngine> quantizeEngine;
    std::unique_ptr<EditorContext> editorContext;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SongDocumentEditor)
};

}
}
