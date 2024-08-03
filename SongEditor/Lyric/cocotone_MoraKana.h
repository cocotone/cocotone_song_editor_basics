#pragma once

namespace cctn
{
namespace song
{

//==============================================================================
class StaticMoraKana final
{
public:
    //==============================================================================
    StaticMoraKana();
    ~StaticMoraKana();

    //==============================================================================
    const std::vector<MoraKana>& getMoraKanas() const;

private:
    //==============================================================================

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StaticMoraKana)
};

}
}
