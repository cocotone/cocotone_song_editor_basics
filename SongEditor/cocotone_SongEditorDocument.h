#pragma once

namespace cctn
{
namespace song
{

//==============================================================================
class SongEditorDocument
    : public juce::ChangeBroadcaster
{
public:
    //==============================================================================
    SongEditorDocument();
    ~SongEditorDocument() override;

    //==============================================================================
    // voicevox specified format
    juce::var createScoreJson() const;
    juce::String createScoreJsonString() const;

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
    static double calculateDocumentDuration(const cctn::song::SongEditorDocumentData& data, double minimumDuration = 0.05);
    static cctn::song::SongEditorNoteExtended createSilenceNote(double startPositionInSeconds, double endPositionInSeconds);
    static cctn::song::SongEditorDocumentData makeSilenceFilledScore(const cctn::song::SongEditorDocumentData& data, double documentDuration);

    //==============================================================================
    std::optional<const cctn::song::SongEditorDocumentData*> getRawDocumentData() const;

    //==============================================================================
    class Listener
    {
    public:
        virtual ~Listener() = default;

        virtual void onDataChanged() = 0;

    private:

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Listener)
    };

private:
    //==============================================================================
    std::unique_ptr<cctn::song::SongEditorDocumentData> documentData;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SongEditorDocument)
};

}
}
