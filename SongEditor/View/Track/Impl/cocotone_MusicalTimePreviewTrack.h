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
    void paint(juce::Graphics& g) override;
    void resized() override;

    //==============================================================================
    void handleAsyncUpdate() override;

    //==============================================================================
    void updateContent(const cctn::song::SongDocumentEditor& songDocumentEditor) override;

    //==============================================================================
    void drawMusicalTimeGrid(juce::Graphics& g);

    //==============================================================================
    cctn::song::SongDocument::BeatTimePoints currentBeatTimePoints{};
    std::vector<int64_t> currentAbsoluteTicksRelatedBeatTimePoints{};

    juce::AudioPlayHead::PositionInfo currentPositionInfo;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MusicalTimePreviewTrack)
};

}
}
