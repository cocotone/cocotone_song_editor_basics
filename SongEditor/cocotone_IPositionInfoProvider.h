#pragma once

namespace cctn
{
namespace song
{

//==============================================================================
class IPositionInfoProvider
{
public:
    //==============================================================================
    virtual ~IPositionInfoProvider() = default;

    //==============================================================================
    virtual std::optional<juce::AudioPlayHead::PositionInfo> getPositionInfo() = 0;

private:
    //==============================================================================

    JUCE_LEAK_DETECTOR(IPositionInfoProvider)
};

}
}
