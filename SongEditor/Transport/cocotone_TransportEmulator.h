#pragma once

namespace cctn
{
namespace song
{

//==============================================================================
class TransportEmulator final
{
public:
    //==============================================================================
    TransportEmulator();
    ~TransportEmulator();

    //==============================================================================
    void setActive(bool value) { isEmulateActive = value; };
    bool isActive() const { return isEmulateActive; };

    //==============================================================================
    void processPositionInfo(juce::AudioPlayHead::PositionInfo& positionInfo);
    std::unique_ptr<juce::PopupMenu> createMenu();

private:
    //==============================================================================
    std::atomic<bool> isEmulateActive{ false };
    std::atomic<bool> shouldResetCurrentPositionAsZero{ false };

    double positionAsZeroPositionInSeconds{ 0.0 };
    double currentPositionInSeconds{ 0.0 };

    juce::int64 positionAsZeroPositionInSamples{ 0 };
    juce::int64 currentPositionInSamples{ 0 };

    double playSpeedRatio{ 1.0 };
    bool isPlaying{ false };
    bool isRecording{ false };
    bool isPositionOverride{ true };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TransportEmulator)
};

}
}
