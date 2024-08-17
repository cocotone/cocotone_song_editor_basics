#pragma once

namespace cctn
{
namespace song
{

//==============================================================================
class AbsoluteTimePreviewTrack
    : public cctn::song::TrackComponentBase
{
public:
    //==============================================================================
    AbsoluteTimePreviewTrack();
    virtual ~AbsoluteTimePreviewTrack() override;

private:
    //==============================================================================
    void handleAsyncUpdate() override;

    //==============================================================================
    void updateContent(const cctn::song::SongDocumentEditor& songDocumentEditor) override;

    //==============================================================================
    cctn::song::SongDocument::BeatTimePoints currentBeatTimePoints{};

    //==============================================================================
    std::unique_ptr<cctn::song::TrackHeaderBase> headerComponent;
    std::unique_ptr<cctn::song::TrackLaneBase<cctn::song::SongDocument::BeatTimePoints>> laneComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AbsoluteTimePreviewTrack)
};

}
}
