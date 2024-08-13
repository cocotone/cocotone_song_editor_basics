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
    struct Region
    {
        double startPositionInSeconds;
        double endPositionInSeconds;

        JUCE_LEAK_DETECTOR(Region)
    };

    //==============================================================================
    QuantizeEngine();
    ~QuantizeEngine();

    //==============================================================================
    void updateQuantizeRegions(const cctn::song::SongDocument::BeatTimePoints& beatTimePoints);
    std::optional<Region> findNearestQuantizeRegion(double timePositionInSeconds) const;

private:
    //==============================================================================
    juce::Array<Region> quantizeRegions;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(QuantizeEngine)
};

}
}
