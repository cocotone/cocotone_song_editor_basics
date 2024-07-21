#pragma once

namespace cctn
{
namespace song
{

//==============================================================================
struct TimeSignature
{
public:
    //==============================================================================
    int bar{ 0 };
    int beat{ 0 };
    int tick{ 0 };

    //==============================================================================
    TimeSignature(int b, int bt, int t)
        : bar(b), beat(bt), tick(t)
    {}

private:
    //==============================================================================
    JUCE_LEAK_DETECTOR(TimeSignature)
};

//==============================================================================
struct TempoAndTimeSignature
{
public:
    //==============================================================================
    double bpm;
    int numerator;
    int denominator;

    //==============================================================================
    TempoAndTimeSignature(double b, int num, int denm)
        : bpm(b), numerator(num), denominator(denm)
    {}

private:
    //==============================================================================
    JUCE_LEAK_DETECTOR(TempoAndTimeSignature)
};

//==============================================================================
struct BeatTimePoint
{
public:
    //==============================================================================
    double beat{ 0.0 };
    double timeInSeconds{ 0.0 };

    //==============================================================================
    BeatTimePoint(double b, double t)
        : beat(b), timeInSeconds(t)
    {}

    //==============================================================================
    TimeSignature toTimeSignature(int beatsPerBar, int ticksPerBeat = 960) const
    {
        int totalTicks = static_cast<int>(std::round(beat * ticksPerBeat));
        int totalBeats = totalTicks / ticksPerBeat;

        int bar = totalBeats / beatsPerBar;
        int beatInBar = totalBeats % beatsPerBar;
        int tick = totalTicks % ticksPerBeat;

        return TimeSignature(bar, beatInBar, tick);
    }

    juce::String getFormattedTimeSignature(int beatsPerBar, bool showTick, int ticksPerBeat = 960) const
    {
        const auto time_signature = toTimeSignature(beatsPerBar, ticksPerBeat);
        
        if (showTick)
        {
            return juce::String::formatted("%d|%d|%03d", time_signature.bar + 1, time_signature.beat + 1, time_signature.tick);
        }

        return juce::String::formatted("%d|%d", time_signature.bar + 1, time_signature.beat + 1);
    }

private:
    //==============================================================================
    JUCE_LEAK_DETECTOR(BeatTimePoint)
};

//==============================================================================
class PositionInfoExtractor
{
public:
    static std::optional<TempoAndTimeSignature> extractTempoAndTimeSignature(const juce::AudioPlayHead::PositionInfo& positionInfo)
    {
        // Check if BPM is available
        if (!positionInfo.getBpm().hasValue())
        {
            return std::nullopt;
        }

        double bpm = *positionInfo.getBpm();

        // Check if time signature is available
        if (!positionInfo.getTimeSignature().hasValue())
        {
            return std::nullopt;
        }

        auto timeSignature = *positionInfo.getTimeSignature();
        int numerator = timeSignature.numerator;
        int denominator = timeSignature.denominator;

        return TempoAndTimeSignature{ bpm, numerator, denominator };
    }
};

//==============================================================================
class BeatTimeMapper
{
public:
    //==============================================================================
    static std::vector<BeatTimePoint> generateBeatTimeMap(double bpm, int numerator, int denominator, double startInSeconds, double endInSeconds)
    {
        std::vector<BeatTimePoint> beatTimeMap;

        double secondsPerBeat = 60.0 / bpm;
        double secondsPerBar = secondsPerBeat * numerator;
        double beatsPerBar = static_cast<double>(numerator);

        // Calculate the starting beat
        double startBeat = std::floor(startInSeconds / secondsPerBeat);
        double currentTime = startBeat * secondsPerBeat;
        double currentBeat = startBeat;

        // Add the exact start time if it doesn't fall on a beat
        if (currentTime < startInSeconds)
        {
            beatTimeMap.emplace_back(currentBeat, currentTime);
            currentBeat = (startInSeconds - currentTime) / secondsPerBeat + currentBeat;
            currentTime = startInSeconds;
        }

        while (currentTime <= endInSeconds)
        {
            beatTimeMap.emplace_back(currentBeat, currentTime);

            currentBeat += 1.0;
            currentTime += secondsPerBeat;

            // Adjust for bar boundaries if needed
            if (std::fmod(currentBeat, beatsPerBar) == 0.0)
            {
                currentBeat = std::round(currentBeat); // Ensure we're on an exact bar
            }
        }

        // Add the final time point if it's not exactly on a beat
        if (!beatTimeMap.empty() && beatTimeMap.back().timeInSeconds < endInSeconds)
        {
            double finalBeat = (endInSeconds - beatTimeMap.back().timeInSeconds) / secondsPerBeat + beatTimeMap.back().beat;
            beatTimeMap.emplace_back(finalBeat, endInSeconds);
        }

        return beatTimeMap;
    }

    //==============================================================================
    static std::vector<BeatTimePoint> extractPreciseBeatPoints(double bpm, int numerator, int denominator, double startInSeconds, double endInSeconds)
    {
        std::vector<BeatTimePoint> beatPoints;

        double secondsPerBeat = 60.0 / bpm;
        double beatsPerBar = static_cast<double>(numerator);

        // Find the first beat after or at the start time
        double firstBeat = std::ceil(startInSeconds / secondsPerBeat);
        double currentTime = firstBeat * secondsPerBeat;
        double currentBeat = firstBeat;

        while (currentTime <= endInSeconds)
        {
            if (currentTime >= startInSeconds)
            {
                beatPoints.emplace_back(currentBeat, currentTime);
            }

            currentBeat += 1.0;
            currentTime += secondsPerBeat;

            // Adjust for bar boundaries if needed
            if (std::fmod(currentBeat, beatsPerBar) == 0.0)
            {
                currentBeat = std::round(currentBeat); // Ensure we're on an exact bar
            }
        }

        return beatPoints;
    }
};

}
}
