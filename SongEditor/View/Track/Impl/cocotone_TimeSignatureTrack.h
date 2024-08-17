#pragma once

namespace cctn
{
namespace song
{

//==============================================================================
class TimeSignatureTrack
    : public cctn::song::TrackComponentBase
{
public:
    //==============================================================================
    TimeSignatureTrack();
    virtual ~TimeSignatureTrack() override;

private:
    //==============================================================================
    void handleAsyncUpdate() override;

    //==============================================================================
    void triggerUpdateContent() override;
    void triggerUpdateVisibleRange() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TimeSignatureTrack)
};

}
}
