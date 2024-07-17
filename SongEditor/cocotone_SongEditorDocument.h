#pragma once

namespace cctn
{
namespace song
{

//==============================================================================
struct PianoRollNote;
struct PianoRollPreviewData;
struct QueryForFindPianoRollNote;
class IPianoRollPreviewDataSource;

using SongEditorDocumentNote = PianoRollNote;
using SongEditorDocumentData = PianoRollPreviewData;

//==============================================================================
class SongEditorDocument
    : public cctn::song::IPianoRollPreviewDataSource
    , public juce::ChangeBroadcaster
{
public:
    //==============================================================================
    SongEditorDocument();
    ~SongEditorDocument() override;

    //==============================================================================
    // cctn::song::IPianoRollPreviewDataSource
    std::optional<cctn::song::PianoRollPreviewData> getPianoRollPreviewData() override;

    //==============================================================================
    // voicevox specified format
    juce::var createScoreJson() const;
    juce::String createScoreJsonString() const;

    //==============================================================================
    void serialize() const;
    void deserialize();

    //==============================================================================
    std::optional<cctn::song::SongEditorDocumentNote> findNote(const cctn::song::QueryForFindPianoRollNote& query);
    void selectNote(const cctn::song::QueryForFindPianoRollNote& query);

    // CRUD operation
    void createNote(const cctn::song::QueryForAddPianoRollNote& query);
    void readNote(const cctn::song::QueryForAddPianoRollNote& query) {};
    void updateNote(const cctn::song::QueryForAddPianoRollNote& query) {};
    void deleteNoteSingle(const cctn::song::QueryForFindPianoRollNote& query);

    //==============================================================================
    static double calculateDocumentDuration(const cctn::song::PianoRollPreviewData& data, double minimumDuration = 0.05);
    static cctn::song::PianoRollNote createSilenceNote(double startTimeInSeconds, double endTimeInSeconds);
    static cctn::song::PianoRollPreviewData makeSilenceFilledScore(const cctn::song::PianoRollPreviewData& data, double documentDuration);

private:
    //==============================================================================
    std::unique_ptr<SongEditorDocumentData> documentData;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SongEditorDocument)
};

}
}
