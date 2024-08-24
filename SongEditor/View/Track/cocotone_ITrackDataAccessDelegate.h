#pragma once

namespace cctn
{
namespace song
{

class ITrackDataAccessDelegate
{
public:
    //==============================================================================
    virtual ~ITrackDataAccessDelegate() = default;

    //==============================================================================
    virtual std::optional<cctn::song::SongDocumentEditor*> getSongDocumentEditor() = 0;
    virtual std::optional<juce::Range<double>> getVisibleRangeInTicks() = 0;

private:

    JUCE_LEAK_DETECTOR(ITrackDataAccessDelegate)
};

}
}
