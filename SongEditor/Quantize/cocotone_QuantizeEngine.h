#pragma once

namespace cctn
{
namespace song
{

//==============================================================================
class QuantizeEngine final
{
public:
    //==============================================================================
    QuantizeEngine();
    ~QuantizeEngine();

    //==============================================================================
    void updateQuantizeRegions(const cctn::song::SongDocument::BeatTimePoints& beatTimePoints);
    std::optional<cctn::song::SongDocument::RegionWithBeatInfo> findNearestQuantizeRegion(double timePositionInSeconds) const;

private:
    //==============================================================================
    juce::Array<cctn::song::SongDocument::RegionWithBeatInfo> quantizeRegions;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(QuantizeEngine)
};

}
}
