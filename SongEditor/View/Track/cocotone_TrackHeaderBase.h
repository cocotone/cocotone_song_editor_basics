#pragma once

namespace cctn
{
namespace song
{

//==============================================================================
class TrackHeaderBase
    : public juce::Component
{
public:
    //==============================================================================
    virtual ~TrackHeaderBase() = default;

private:
    //==============================================================================

    JUCE_LEAK_DETECTOR(TrackHeaderBase)
};

}
}
