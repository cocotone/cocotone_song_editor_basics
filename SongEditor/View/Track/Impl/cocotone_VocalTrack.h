#pragma once

namespace cctn
{
namespace song
{

//==============================================================================
class VocalTrack
    : public cctn::song::TrackComponentBase
{
public:
    //==============================================================================
    explicit VocalTrack(cctn::song::ITrackDataAccessDelegate& trackAccessDelegate);
    virtual ~VocalTrack() override;

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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VocalTrack)
};

}
}
