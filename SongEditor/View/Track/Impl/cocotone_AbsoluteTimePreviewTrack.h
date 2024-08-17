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
    void paint(juce::Graphics& g) override;
    void resized() override;

    //==============================================================================
    void handleAsyncUpdate() override;

    //==============================================================================
    void updateContent(const cctn::song::SongDocumentEditor& songDocumentEditor) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AbsoluteTimePreviewTrack)
};

}
}
