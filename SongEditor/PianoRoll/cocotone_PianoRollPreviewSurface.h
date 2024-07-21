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
    void setInputPositionInSeconds(double positionInSeconds);

    //==============================================================================
    void emitMouseEvent(const juce::MouseEvent& mouseEvent, bool isExitAction);
    std::optional<cctn::song::QueryForFindPianoRollNote> getQueryForFindPianoRollNote(const juce::MouseEvent& mouseEvent);

    //==============================================================================
    void setDocumentForPreview(std::shared_ptr<cctn::song::SongEditorDocument> document);

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
    void drawGridVerticalLines(juce::Graphics& g);
    void drawCurrentPreviewData(juce::Graphics& g);
    void drawPlayingPositionMarker(juce::Graphics& g);
    void drawInputPositionMarker(juce::Graphics& g);

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
    static juce::Array<PositionWithTimeInfo> createVerticalLinePositions(const juce::Range<double> visibleRangeSeconds, double timeUnitSeconds, int width);
    
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
    double inputPositionInSeconds;
    bool isInputPositionInsertable;
    juce::Point<int> lastMousePosition;

    // Generate for Grid.
    const PianoRollKeyboard& pianoRollKeyboardRef;

    // For fast painitng.
    std::map<juce::uint8, juce::Range<float>> mapVisibleKeyNoteNumberToVerticalPositionRangeAsVerticalTopToBottom;

    std::weak_ptr<cctn::song::SongEditorDocument> documentForPreviewPtr;
    const cctn::song::SongEditorDocumentData* paintScopedDocumentDataPtr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PianoRollPreviewSurface)
};

}
}
