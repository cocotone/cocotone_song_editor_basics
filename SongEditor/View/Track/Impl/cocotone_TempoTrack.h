#pragma once

namespace cctn
{
namespace song
{

//==============================================================================
class TempoTrack
    : public cctn::song::TrackComponentBase
{
public:
    //==============================================================================
    TempoTrack();
    virtual ~TempoTrack() override;

private:
    //==============================================================================
    void handleAsyncUpdate() override;

    //==============================================================================
    void updateContent(const cctn::song::SongDocumentEditor& songDocumentEditor) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TempoTrack)
};

}
}
