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

    //==============================================================================
    void setTimeRulerRectangle(const juce::Rectangle<int>& rectangle);
    void setRulerNameRectangle(const juce::Rectangle<int>& rectangle);

    void updateLayout();

private:
    //==============================================================================
    void paint(juce::Graphics& g) override;
    void resized() override;

    //==============================================================================
    void drawGridVerticalLines(juce::Graphics& g);
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

    //==============================================================================
    juce::Rectangle<int> rectTimeRulerArea;
    juce::Rectangle<int> rectRulerNameArea;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PianoRollTimeRuler)
};

}
}
