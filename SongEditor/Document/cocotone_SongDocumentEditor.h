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
    void serialize() const;
    void deserialize();

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

#if 0
    //==============================================================================
    std::optional<cctn::song::SongEditorNoteExtended> findNote(const cctn::song::QueryForFindPianoRollNote& query);
    std::optional<const cctn::song::SongEditorDocumentData*> getRawDocumentData() const;
    std::unique_ptr<cctn::song::SongEditorDocumentData> documentData;

    //==============================================================================
    // voicevox specified format
    juce::var createScoreJson_outdated() const;
    juce::String createScoreJsonString_outdated() const;
    juce::String createScoreJsonString() const;

    //==============================================================================
    static double calculateDocumentDuration(const cctn::song::SongEditorDocumentData& data, double minimumDuration = 0.05);
    static cctn::song::SongEditorNoteExtended createSilenceNote(double startPositionInSeconds, double endPositionInSeconds);
    static cctn::song::SongEditorDocumentData makeSilenceFilledScore(const cctn::song::SongEditorDocumentData& data, double documentDuration);
#endif

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SongDocumentEditor)
};

}
}
