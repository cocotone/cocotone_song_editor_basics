#pragma once

namespace cctn
{
namespace song
{

//==============================================================================
class PianoRollPreviewSurface final
    : public juce::Component
    , private juce::ChangeListener
{
public:
    //==============================================================================
    explicit PianoRollPreviewSurface(const cctn::song::PianoRollKeyboard& pianoRollKeyboard, int numVisibleOctaves);
    ~PianoRollPreviewSurface() override;

    //==============================================================================
    void setVisibleBottomKeyNoteNumber(int noteNumber);
    void setVisibleRangeTimeInSeconds(juce::Range<double> newRange);
    void setGridVerticalLineIntervaleInSeconds(double intervalInSeconds);

    //==============================================================================
    void setPlayingPositionInSeconds(double positionInSeconds);
    void setCurrentPositionInfo(const juce::AudioPlayHead::PositionInfo& positionInfo);

    //==============================================================================
    void setUserInputPositionInSeconds(double positionInSeconds);
    void setDrawingGridInterval(cctn::song::NoteLength intervalLength);

    //==============================================================================
    void emitMouseEvent(const juce::MouseEvent& mouseEvent, bool isExitAction);
    std::optional<cctn::song::QueryForFindPianoRollNote> getQueryForFindPianoRollNote(const juce::MouseEvent& mouseEvent);

    //==============================================================================
    void setDocumentForPreview(std::shared_ptr<cctn::song::SongEditorDocument> document);

    //==============================================================================
    enum class VisibleGridVerticalType
    {
        kNone = 0,
        kTimeSeconds = 1,
        kTimeSignature = 2,
        kQuantize = 3,
    };

private:
    //==============================================================================
    void paint(juce::Graphics& g) override;
    void resized() override;

    //==============================================================================
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    //==============================================================================
    void updateViewContext();

    //==============================================================================
    void fillGridHorizontalRows(juce::Graphics& g);
    void drawGridHorizontalLines(juce::Graphics& g);
    void drawGridVerticalLinesInTimeSecondsDomain(juce::Graphics& g);
    void drawGridVerticalLinesInTimeSignatureDomain(juce::Graphics& g);
    void drawGridVerticalLinesInQuantizeDomain(juce::Graphics& g);
    void drawCurrentPreviewData(juce::Graphics& g);
    void drawPlayingPositionMarker(juce::Graphics& g);
    void drawUserInputPositionMarker(juce::Graphics& g);
    void drawQuantizedInputRegionRectangle(juce::Graphics& g);
    void drawUserInputPositionCellRectangle(juce::Graphics& g);

    //==============================================================================
    // For fast painitng.
    void updateMapVisibleKeyNoteNumberToVerticalPositionRange();
    std::optional<juce::uint8> findNoteNumberWithVerticalPosition(float positionY);

    //==============================================================================
    struct PositionWithTimeInfo
    {
        int positionX{ 0 };
        double timeInSeconds{ 0.0 };

        JUCE_LEAK_DETECTOR(PositionWithTimeInfo)
    };
    static juce::Array<PositionWithTimeInfo> createVerticalLinePositionsInTimeSecondsDomain(const juce::Range<double> visibleRangeSeconds, double timeStepInSeconds, int width);
    static juce::Array<PositionWithTimeInfo> createVerticalLinePositionsInTimeSignatureDomain(const juce::Range<double> visibleRangeSeconds, const BeatTimePointList& beatTimePoints, int width);
    
    struct NoteDrawInfo
    {
        int positionLeftX{ 0 };
        int positionRightX{ 0 };
        int noteNumber{ 0 };
        juce::String lyric{ "" };
        bool isSelected{ false };

        JUCE_LEAK_DETECTOR(NoteDrawInfo)
    };
    static NoteDrawInfo createNoteDrawInfo(const cctn::song::SongEditorNoteExtended& note, const juce::Range<double> visibleRangeSeconds, int positionLeft, int positionRight);

    //==============================================================================
    // TODO: Move state value.
    juce::uint8 numVisibleWhiteAndBlackKeys;
    juce::uint8 numVisibleWhiteKeys;
    juce::Range<juce::uint8> rangeVisibleKeyNoteNumbers;
    juce::Range<double> rangeVisibleTimeInSeconds;
    double verticalLineIntervalInSeconds;
    double playingPositionInSeconds;
    double userInputPositionInSeconds;
    int userInputPositionInNoteNumber;
    bool isInputPositionInsertable;
    juce::Point<int> lastMousePosition;
    juce::Range<double> quantizedInputRegionInSeconds;

    juce::Value visibleGridVerticalLineType;
    juce::Value drawingGirdVerticalInterval;

    // Generate for Grid.
    const PianoRollKeyboard& pianoRollKeyboardRef;

    // For fast painitng.
    std::map<juce::uint8, juce::Range<float>> mapVisibleKeyNoteNumberToVerticalPositionRangeAsVerticalTopToBottom;

    std::weak_ptr<cctn::song::SongEditorDocument> documentForPreviewPtr;
    const cctn::song::SongEditorDocumentData* paintScopedDocumentDataPtr;

    // TODO: should abstract
    juce::AudioPlayHead::PositionInfo currentPositionInfo;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PianoRollPreviewSurface)
};

}
}
