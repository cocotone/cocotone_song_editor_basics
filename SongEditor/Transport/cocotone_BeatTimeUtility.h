#pragma once

namespace cctn
{
namespace song
{

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

private:
    PositionInfoExtractor() = delete;
    ~PositionInfoExtractor() = delete;
};

//==============================================================================
class BeatTimePointFactory
{
public:
#if 0
    //==============================================================================
    static BeatTimePointList generateBeatTimePointList(double bpm, int numerator, int denominator, double startInSeconds, double endInSeconds)
    {
        BeatTimePointList beat_time_array;

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
            beat_time_array.emplace_back(currentBeat, currentTime);
            currentBeat = (startInSeconds - currentTime) / secondsPerBeat + currentBeat;
            currentTime = startInSeconds;
        }

        while (currentTime <= endInSeconds)
        {
            beat_time_array.emplace_back(currentBeat, currentTime);

            currentBeat += 1.0;
            currentTime += secondsPerBeat;

            // Adjust for bar boundaries if needed
            if (std::fmod(currentBeat, beatsPerBar) == 0.0)
            {
                currentBeat = std::round(currentBeat); // Ensure we're on an exact bar
            }
        }

        // Add the final time point if it's not exactly on a beat
        if (!beat_time_array.empty() && beat_time_array.back().timeInSeconds < endInSeconds)
        {
            double finalBeat = (endInSeconds - beat_time_array.back().timeInSeconds) / secondsPerBeat + beat_time_array.back().beat;
            beat_time_array.emplace_back(finalBeat, endInSeconds);
        }

        return beat_time_array;
    }

    //==============================================================================
    // This function returns quantize friendly beat positions.
    static BeatTimePointList extractPreciseBeatPoints(double bpm, int numerator, int denominator, double startInSeconds, double endInSeconds)
    {
        BeatTimePointList beatPoints;

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
                // Ensure we're on an exact bar
                currentBeat = std::round(currentBeat);
            }
        }

        return beatPoints;
    }

    static BeatTimePointList extractPreciseBeatPoints(double bpm, int numerator, int denominator, double startInSeconds, double endInSeconds, NoteLength noteLength)
    {
        BeatTimePointList beatPoints;

        double secondsPerQuarterNote = 60.0 / bpm;
        double beatsPerBar = static_cast<double>(numerator);

        double noteLengthsPerQuarterNote = cctn::song::getNoteLengthsPerQuarterNote(noteLength);
        double secondsPerNoteLength = secondsPerQuarterNote / noteLengthsPerQuarterNote;

        // Find the first beat after or at the start time
        double firstBeat = std::ceil(startInSeconds / secondsPerNoteLength);
        double currentTime = firstBeat * secondsPerNoteLength;
        double currentBeat = firstBeat / noteLengthsPerQuarterNote;

        while (currentTime <= endInSeconds)
        {
            if (currentTime >= startInSeconds)
            {
                beatPoints.emplace_back(currentBeat, currentTime, noteLength);
            }

            currentBeat += 1.0 / noteLengthsPerQuarterNote;
            currentTime += secondsPerNoteLength;

            // Adjust for bar boundaries if needed
            if (std::fmod(currentBeat, beatsPerBar) < 1e-10)
            {
                // Ensure we're on an exact bar
                currentBeat = std::round(currentBeat);
            }
        }

        return beatPoints;
    }
#endif
    static BeatTimePointList extractPreciseBeatPoints(const cctn::song::SongDocument& document, double startInSeconds, double endInSeconds, NoteLength noteLength)
    {
        BeatTimePointList beatPoints;
        const auto& tempoTrack = document.getTempoTrack();
        const auto& events = tempoTrack.getEvents();
        int ticksPerQuarterNote = document.getTicksPerQuarterNote();

        double currentTime = 0.0;
        double currentBeat = 0.0;
        double currentTempo = 120.0; // Default tempo
        int currentNumerator = 4;
        int currentDenominator = 4;

        auto eventIt = events.begin();

        // Find the first event that affects our start time
        while (eventIt != events.end() && cctn::song::SongDocument::Calculator::tickToAbsoluteTime(document, eventIt->getTick()) <= startInSeconds)
        {
            if (eventIt->getEventType() == cctn::song::SongDocument::TempoEvent::TempoEventType::kTempo ||
                eventIt->getEventType() == cctn::song::SongDocument::TempoEvent::TempoEventType::kBoth)
            {
                currentTempo = eventIt->getTempo();
            }
            if (eventIt->getEventType() == cctn::song::SongDocument::TempoEvent::TempoEventType::kTimeSignature ||
                eventIt->getEventType() == cctn::song::SongDocument::TempoEvent::TempoEventType::kBoth)
            {
                auto [numerator, denominator] = eventIt->getTimeSignature();
                currentNumerator = numerator;
                currentDenominator = denominator;
            }
            currentTime = cctn::song::SongDocument::Calculator::tickToAbsoluteTime(document, eventIt->getTick());
            currentBeat = static_cast<double>(eventIt->getTick()) / ticksPerQuarterNote;
            ++eventIt;
        }

        double noteLengthsPerQuarterNote = cctn::song::getNoteLengthsPerQuarterNote(noteLength);
        double secondsPerQuarterNote = 60.0 / currentTempo;
        double secondsPerNoteLength = secondsPerQuarterNote / noteLengthsPerQuarterNote;

        // Adjust currentBeat and currentTime to the first beat after or at the start time
        while (currentTime < startInSeconds)
        {
            currentBeat += 1.0 / noteLengthsPerQuarterNote;
            currentTime += secondsPerNoteLength;
        }

        while (currentTime <= endInSeconds)
        {
            if (currentTime >= startInSeconds)
            {
                beatPoints.emplace_back(currentBeat, currentTime, noteLength);
            }

            currentBeat += 1.0 / noteLengthsPerQuarterNote;
            currentTime += secondsPerNoteLength;

            // Check if we've reached the next tempo or time signature change
            if (eventIt != events.end() && currentTime >= cctn::song::SongDocument::Calculator::tickToAbsoluteTime(document, eventIt->getTick()))
            {
                if (eventIt->getEventType() == cctn::song::SongDocument::TempoEvent::TempoEventType::kTempo ||
                    eventIt->getEventType() == cctn::song::SongDocument::TempoEvent::TempoEventType::kBoth)
                {
                    currentTempo = eventIt->getTempo();
                    secondsPerQuarterNote = 60.0 / currentTempo;
                    secondsPerNoteLength = secondsPerQuarterNote / noteLengthsPerQuarterNote;
                }
                if (eventIt->getEventType() == cctn::song::SongDocument::TempoEvent::TempoEventType::kTimeSignature ||
                    eventIt->getEventType() == cctn::song::SongDocument::TempoEvent::TempoEventType::kBoth)
                {
                    auto [numerator, denominator] = eventIt->getTimeSignature();
                    currentNumerator = numerator;
                    currentDenominator = denominator;
                }
                ++eventIt;
            }

            // Adjust for bar boundaries if needed
            double beatsPerBar = static_cast<double>(currentNumerator) * 4.0 / currentDenominator;
            if (std::fmod(currentBeat, beatsPerBar) < 1e-10)
            {
                currentBeat = std::round(currentBeat);
            }
        }

        return beatPoints;
    }

private:
    BeatTimePointFactory() = delete;
    ~BeatTimePointFactory() = delete;
};

}
}
