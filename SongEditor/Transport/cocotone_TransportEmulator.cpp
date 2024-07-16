namespace cctn
{
namespace song
{

//==============================================================================
TransportEmulator::TransportEmulator()
{
}

TransportEmulator::~TransportEmulator()
{
}

//==============================================================================
void TransportEmulator::processPositionInfo(juce::AudioPlayHead::PositionInfo& positionInfo)
{
    if (!isEmulateActive)
    {
        return;
    }

    positionInfo.setIsPlaying(isPlaying);
    positionInfo.setIsRecording(isRecording);

    if (shouldResetCurrentPositionAsZero)
    {
        const auto time_in_seconds = positionInfo.getTimeInSeconds();
        positionAsZeroPositionInSeconds = time_in_seconds.orFallback(0.0);
        positionAsZeroPositionInSamples = positionInfo.getTimeInSamples().orFallback(0);
        shouldResetCurrentPositionAsZero = false;
    }

    if (isPositionOverride)
    {
        const auto time_in_seconds_in_host_device = positionInfo.getTimeInSeconds().orFallback(0.0);
        currentPositionInSeconds = time_in_seconds_in_host_device - positionAsZeroPositionInSeconds;
        positionInfo.setTimeInSeconds(juce::Optional<double>(currentPositionInSeconds * playSpeedRatio));

        const auto time_in_samples_in_host_device = positionInfo.getTimeInSamples().orFallback(0);
        currentPositionInSamples = time_in_samples_in_host_device - positionAsZeroPositionInSamples;
        positionInfo.setTimeInSamples(currentPositionInSamples * playSpeedRatio);
    }
}

//==============================================================================
std::unique_ptr<juce::PopupMenu> TransportEmulator::createMenu()
{
    std::unique_ptr<juce::PopupMenu> transport_emulator_menu(new juce::PopupMenu());

    transport_emulator_menu->addItem("Active", true, isEmulateActive, [this, current_active_value = isEmulateActive.load()]() {
        isEmulateActive = !current_active_value;
    });

    transport_emulator_menu->addSeparator();
    transport_emulator_menu->addSectionHeader("State");

    transport_emulator_menu->addItem("Playing", isEmulateActive.load(), isPlaying, [this, current_value = isPlaying]() {
        isPlaying = !current_value;
    });

    transport_emulator_menu->addItem("Recording", isEmulateActive.load(), isRecording, [this, current_value = isRecording]() {
        isRecording = !current_value;
    });

    transport_emulator_menu->addSeparator();
    transport_emulator_menu->addSectionHeader("Position");

    transport_emulator_menu->addItem("Reset position to zero", isEmulateActive.load(), false, [this]() {
        shouldResetCurrentPositionAsZero = true;
    });

    transport_emulator_menu->addSeparator();
    transport_emulator_menu->addSectionHeader("Speed Ratio");

    transport_emulator_menu->addItem("Reset play speed ratio", isEmulateActive.load(), false, [this]() {
        playSpeedRatio = 1.0;
    });

    transport_emulator_menu->addItem("2x play speed ratio", isEmulateActive.load(), false, [this]() {
        playSpeedRatio = 2.0;
    });

    transport_emulator_menu->addItem("0.5x play speed ratio", isEmulateActive.load(), false, [this]() {
        playSpeedRatio = 0.5;
    });

    return transport_emulator_menu;
}

}
}
