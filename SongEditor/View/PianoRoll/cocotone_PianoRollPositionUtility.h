#pragma once

namespace cctn
{
namespace song
{

int timeToPositionX(double timeInSeconds, const juce::Range<double>& visibleRangeInSeconds, double width)
{
    if (width == 0.0)
    {
        return 0.0;
    }
    const int position_x = juce::jmap<double>(timeInSeconds, visibleRangeInSeconds.getStart(), visibleRangeInSeconds.getEnd(), 0, width);
    return position_x;
}

int timeToPositionX(double timeInSeconds, const juce::Range<double>& visibleRangeInSeconds, double position_left, double position_right)
{
    if (position_right - position_left == 0.0)
    {
        return 0.0;
    }
    const int position_x = juce::jmap<double>(timeInSeconds, visibleRangeInSeconds.getStart(), visibleRangeInSeconds.getEnd(), position_left, position_right);
    return position_x;
}

double positionXToTime(double positionX, double position_left, double position_right, const juce::Range<double>& visibleRangeInSeconds)
{
    if (position_right - position_left == 0.0)
    {
        return 0.0;
    }
    const double time = juce::jmap<double>(positionX, position_left, position_right, visibleRangeInSeconds.getStart(), visibleRangeInSeconds.getEnd());
    return time;
}

int ticksToPositionX(double timeInTicks, const juce::Range<double>& visibleRangeInTicks, double position_left, double position_right)
{
    if (visibleRangeInTicks.getLength() == 0.0)
    {
        return 0.0;
    }

    if (position_right - position_left == 0.0)
    {
        return 0.0;
    }

    const int position_x = juce::jmap<double>(timeInTicks, visibleRangeInTicks.getStart(), visibleRangeInTicks.getEnd(), position_left, position_right);
    return position_x;
}

}
}

