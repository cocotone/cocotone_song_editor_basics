#pragma once

namespace cctn
{
namespace song
{

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
    juce::String debugDumpDocument() const;

    //==============================================================================
    void serialize() const;
    void deserialize();

    //==============================================================================
    std::optional<cctn::song::SongEditorNoteExtended> findNote(const cctn::song::QueryForFindPianoRollNote& query);
    void selectNote(const cctn::song::QueryForFindPianoRollNote& query);

    // CRUD operation
    void createNote(const cctn::song::QueryForAddPianoRollNote& query);
    void readNote(const cctn::song::QueryForAddPianoRollNote& query) {};
    void updateNote(const cctn::song::QueryForAddPianoRollNote& query) {};
    void deleteNoteSingle(const cctn::song::QueryForFindPianoRollNote& query);

    //==============================================================================
    void updateQuantizeRegions(const juce::AudioPlayHead::PositionInfo& positionInfo);
    std::optional<cctn::song::QuantizeEngine::Region> findNearestQuantizeRegion(double timePositionInSeconds) const;

    //==============================================================================
    std::optional<const cctn::song::SongEditorDocumentData*> getRawDocumentData() const;

    //==============================================================================
    class EditorContext
    {
    public:
        cctn::song::NoteLength currentGridInterval{ cctn::song::NoteLength::Quarter };
        cctn::song::NoteLength currentNoteLength{ cctn::song::NoteLength::Quarter };
        cctn::song::NoteLyric currentNoteLyric{ juce::CharPointer_UTF8("\xe3\x83\xa9") }; // ra

    private:
        JUCE_LEAK_DETECTOR(EditorContext)
    };
    EditorContext& getEditorContext() const { return *editorContext.get(); };

private:
    //==============================================================================
    std::shared_ptr<cctn::song::SongDocument> documentToEdit;
    std::unique_ptr<cctn::song::SongEditorDocumentData> documentData;
    std::unique_ptr<cctn::song::QuantizeEngine> quantizeEngine;
    std::unique_ptr<cctn::song::BeatTimePointList> beatTimePointList;
    std::unique_ptr<EditorContext> editorContext;

#if 0
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
