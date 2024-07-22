#pragma once

namespace cctn
{
namespace song
{

//==============================================================================
class PianoRollTimeRuler final
    : public juce::Component
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
    void setTimeRulerRectangle(const juce::Rectangle<int>& rectangle);
    void setTimeRulerLabelRectangle(const juce::Rectangle<int>& rectangle);
    void setBeatRulerRectangle(const juce::Rectangle<int>& rectangle);
    void setBeatRulerLabelRectangle(const juce::Rectangle<int>& rectangle);

    void updateLayout();

private:
    //==============================================================================
    void paint(juce::Graphics& g) override;
    void resized() override;

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

    // TODO: should abstract
    juce::AudioPlayHead::PositionInfo currentPositionInfo;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PianoRollTimeRuler)
};

}
}
