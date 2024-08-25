#pragma once

namespace cctn
{
namespace song
{

//==============================================================================
class PianoRollTimeRuler final
    : public juce::Component
    , private juce::ChangeListener
{
public:
    //==============================================================================
    PianoRollTimeRuler();
    ~PianoRollTimeRuler() override;

    //==============================================================================
    void setVisibleRangeTimeInSeconds(juce::Range<double> newRange);
    void setGridVerticalLineIntervaleInSeconds(double intervalInSeconds);

    //==============================================================================
    void setPlayingPositionInSeconds(double positionInSeconds);
    void setCurrentPositionInfo(const juce::AudioPlayHead::PositionInfo& positionInfo);

    //==============================================================================
    void setDocumentForPreview(std::shared_ptr<cctn::song::SongDocumentEditor> documentEditor);

    //==============================================================================
    struct LayoutSource
    {
        int labelLeft{ 0 };
        int labelRight{ 0 };
        int rulerLeft{ 0 };
        int rulerRight{ 0 };

        JUCE_LEAK_DETECTOR(LayoutSource)
    };

    void setLayoutSource(const LayoutSource& layoutSource);
    void updateLayout();

private:
    //==============================================================================
    void paint(juce::Graphics& g) override;
    void resized() override;

    //==============================================================================
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    //==============================================================================
    void updateViewContext();

    //==============================================================================
    void drawTimeRulerVerticalLines(juce::Graphics& g);
    void drawBeatRulerVerticalLines(juce::Graphics& g);
    void drawPlayingPositionMarker(juce::Graphics& g);

    //==============================================================================
    struct PositionWithTimeInfo
    {
        int positionX{ 0 };
        double timeInSeconds{ 0.0 };

        JUCE_LEAK_DETECTOR(PositionWithTimeInfo)
    };
    static juce::Array<PositionWithTimeInfo> createVerticalLinePositions(const juce::Range<double> visibleRangeSeconds, double timeUnitSeconds, int position_left, int position_right);

    //==============================================================================
    juce::Range<double> rangeVisibleTimeInSeconds;
    double verticalLineIntervalInSeconds;
    double playingPositionInSeconds;

    std::unique_ptr<juce::Label> labelTimeRuler;
    std::unique_ptr<juce::Label> labelBeatRuler;

    //==============================================================================
    juce::Rectangle<int> rectTimeRulerArea;
    juce::Rectangle<int> rectTimeRulerLabelArea;
    juce::Rectangle<int> rectBeatRulerArea;
    juce::Rectangle<int> rectBeatRulerLabelArea;

    LayoutSource currentLayoutSource;

    //==============================================================================
    std::weak_ptr<cctn::song::SongDocumentEditor> documentEditorForPreviewPtr;
    const cctn::song::SongDocument* scopedSongDocumentPtrToPaint;
    cctn::song::SongDocument::BeatTimePoints currentBeatTimePoints{};

    // TODO: should abstract
    juce::AudioPlayHead::PositionInfo currentPositionInfo;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PianoRollTimeRuler)
};

}
}
