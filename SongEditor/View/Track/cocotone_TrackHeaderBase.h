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

    //==============================================================================
    virtual juce::var getProperties() { return juce::var(); };

private:
    //==============================================================================

    JUCE_LEAK_DETECTOR(TrackHeaderBase)
};

}
}
