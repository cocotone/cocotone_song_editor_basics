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
    std::optional<cctn::song::SongDocument::RegionInSeconds> findNearestQuantizeRegion(double timePositionInSeconds) const;

private:
    //==============================================================================
    juce::Array<cctn::song::SongDocument::RegionInSeconds> quantizeRegions;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(QuantizeEngine)
};

}
}
