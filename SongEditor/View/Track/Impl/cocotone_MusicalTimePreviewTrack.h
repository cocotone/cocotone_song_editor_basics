#pragma once

namespace cctn
{
namespace song
{

class MusicalTimePreviewTrackHeader;

//==============================================================================
class MusicalTimePreviewTrack
    : public cctn::song::TrackComponentBase
{
public:
    //==============================================================================
    explicit MusicalTimePreviewTrack(cctn::song::ITrackDataAccessDelegate& trackAccessDelegate);
    virtual ~MusicalTimePreviewTrack() override;

private:
    //==============================================================================
    void handleAsyncUpdate() override;

    //==============================================================================
    void triggerUpdateContent() override;
    void triggerUpdateVisibleRange() override;

    //==============================================================================
    cctn::song::SongDocument::BeatTimePoints currentBeatTimePoints{};

    //==============================================================================
    cctn::song::ITrackDataAccessDelegate& trackAccessDelegate;

    std::unique_ptr<cctn::song::TrackHeaderBase> headerComponent;
    std::unique_ptr<cctn::song::TrackLaneBase<cctn::song::SongDocument::BeatTimePoints>> laneComponent;

    friend MusicalTimePreviewTrackHeader;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MusicalTimePreviewTrack)
};

}
}
