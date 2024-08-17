#pragma once

namespace cctn
{
namespace song
{

//==============================================================================
class TrackComponentBase
    : public juce::Component
    , public juce::AsyncUpdater
{
public:
    //==============================================================================
    virtual ~TrackComponentBase() = default;

    //==============================================================================
    virtual void updateContent(const cctn::song::SongDocumentEditor& songDocumentEditor) = 0;

    //==============================================================================
    void setViewRangeInTicks(const juce::Range<double> viewRangeInTicks)
    { 
        currentViewRangeInTicks = viewRangeInTicks;
        triggerAsyncUpdate();
    }
    const juce::Range<double>& getViewRangeInTicks() const { return currentViewRangeInTicks; }

private:
    //==============================================================================
    juce::Range<double> currentViewRangeInTicks;

    JUCE_LEAK_DETECTOR(TrackComponentBase)
};

}
}
