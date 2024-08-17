#pragma once

namespace cctn
{
namespace song
{

//==============================================================================
class MusicalTimePreviewTrack
    : public cctn::song::TrackComponentBase
{
public:
    //==============================================================================
    MusicalTimePreviewTrack();
    virtual ~MusicalTimePreviewTrack() override;

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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MusicalTimePreviewTrack)
};

}
}
