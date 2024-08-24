#pragma once

namespace cctn
{
namespace song
{

class AbsoluteTimePreviewTrackHeader;

//==============================================================================
class AbsoluteTimePreviewTrack
    : public cctn::song::TrackComponentBase
{
public:
    //==============================================================================
    explicit AbsoluteTimePreviewTrack(cctn::song::ITrackDataAccessDelegate& trackAccessDelegate);
    virtual ~AbsoluteTimePreviewTrack() override;

private:
    //==============================================================================
    void handleAsyncUpdate() override;

    //==============================================================================
    void triggerUpdateContent() override;
    void triggerUpdateVisibleRange() override;

    //==============================================================================
    cctn::song::ITrackDataAccessDelegate& trackAccessDelegate;

    std::unique_ptr<cctn::song::TrackHeaderBase> headerComponent;
    std::unique_ptr<cctn::song::TrackLaneBase<cctn::song::SongDocument>> laneComponent;

    friend AbsoluteTimePreviewTrackHeader;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AbsoluteTimePreviewTrack)
};

}
}
