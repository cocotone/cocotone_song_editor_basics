#pragma once

namespace cctn
{
namespace song
{

//==============================================================================
template<class ContentType>
class TrackLaneBase
    : public juce::Component
{
public:
    //==============================================================================
    virtual ~TrackLaneBase() = default;

    //==============================================================================
    virtual void updateContent(const ContentType& content, const juce::var& properties) = 0;

    //==============================================================================
    void setViewRangeInTicks(const juce::Range<double> viewRangeInTicks)
    {
        currentViewRangeInTicks = viewRangeInTicks;
    }
    const juce::Range<double>& getViewRangeInTicks() const { return currentViewRangeInTicks; }

private:
    //==============================================================================
    juce::Range<double> currentViewRangeInTicks;

    //==============================================================================

    JUCE_LEAK_DETECTOR(TrackLaneBase)
};

}
}
