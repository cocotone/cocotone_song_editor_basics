namespace cctn
{
namespace song
{

//==============================================================================
PianoRollTimeRuler::PianoRollTimeRuler()
    :rangeVisibleTimeInSeconds(juce::Range<double>(0.0f, 60.0))
    , verticalLineIntervalInSeconds(1.0)
    , playingPositionInSeconds(0.0)
{
}

PianoRollTimeRuler::~PianoRollTimeRuler()
{
}

//==============================================================================
void PianoRollTimeRuler::setVisibleRangeTimeInSeconds(juce::Range<double> newRange)
{
    rangeVisibleTimeInSeconds = newRange;
    repaint();
}

void PianoRollTimeRuler::setGridVerticalLineIntervaleInSeconds(double intervalInSeconds)
{
    verticalLineIntervalInSeconds = intervalInSeconds;
    repaint();
}

void PianoRollTimeRuler::setPlayingPositionInSeconds(double positionInSeconds)
{
    playingPositionInSeconds = positionInSeconds;
    repaint();
}

void PianoRollTimeRuler::setTimeRulerRectangle(const juce::Rectangle<int>& rectangle)
{
    rectTimeRulerArea = rectangle;
    updateLayout();
}

void PianoRollTimeRuler::setRulerNameRectangle(const juce::Rectangle<int>& rectangle)
{
    rectRulerNameArea = rectangle;
    updateLayout();
}

void PianoRollTimeRuler::updateLayout()
{
    repaint();
}

//==============================================================================
void PianoRollTimeRuler::paint(juce::Graphics& g)
{
    juce::Graphics::ScopedSaveState save_state(g);

    g.fillAll(kColourWallpaper);

    g.setColour(kColourMainBackground);
    g.fillRect(rectTimeRulerArea);

    drawGridVerticalLines(g);
    drawPlayingPositionMarker(g);
}

void PianoRollTimeRuler::resized()
{
    repaint();
}

//==============================================================================
void PianoRollTimeRuler::drawGridVerticalLines(juce::Graphics& g)
{
    juce::Graphics::ScopedSaveState save_state(g);

    const auto vertical_line_positions = createVerticalLinePositions(rangeVisibleTimeInSeconds, verticalLineIntervalInSeconds, rectTimeRulerArea.getX(), rectTimeRulerArea.getRight());

    // Set clipping mask
    g.reduceClipRegion(rectTimeRulerArea);

    g.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 10, 0));

    for (const auto& position_with_time_info : vertical_line_positions)
    {
        g.setColour(juce::Colours::white);
        g.drawVerticalLine(position_with_time_info.positionX, 0.0f, getHeight());
        g.drawText(juce::String(position_with_time_info.timeInSeconds, 1), position_with_time_info.positionX + 2, 0.0f, 60, 20, juce::Justification::centredLeft);
    }
}

void PianoRollTimeRuler::drawPlayingPositionMarker(juce::Graphics& g)
{
    juce::Graphics::ScopedSaveState save_state(g);

    const auto position_x = timeToPositionX(playingPositionInSeconds, rangeVisibleTimeInSeconds, rectTimeRulerArea.getX(), rectTimeRulerArea.getRight());

    juce::Rectangle<int> rect_marker =
        juce::Rectangle<int>{ position_x, 0, 2, getHeight() };

    // Set clipping mask
    g.reduceClipRegion(rectTimeRulerArea);

    g.setColour(juce::Colours::yellow);
    g.fillRect(rect_marker);
}

//==============================================================================
juce::Array<PianoRollTimeRuler::PositionWithTimeInfo> PianoRollTimeRuler::createVerticalLinePositions(const juce::Range<double> visibleRangeSeconds, double timeUnitSeconds, int position_left, int position_right)
{
    juce::Array<PositionWithTimeInfo> result;

    std::vector<double> time_secnds_vertical_lines;
    {
        // Find the smallest multiple of 10 within the range
        double min_units = static_cast<double>(std::ceil(visibleRangeSeconds.getStart() / timeUnitSeconds)) * (double)timeUnitSeconds;

        // Find the largest multiple of 10 within the range
        double max_units = static_cast<double>(std::floor(visibleRangeSeconds.getEnd() / timeUnitSeconds)) * (double)timeUnitSeconds;

        // List up multiples of 10 within the range
        for (double value = min_units; value <= max_units; value += (double)timeUnitSeconds)
        {
            time_secnds_vertical_lines.push_back((double)value);
        }
    }

    if (time_secnds_vertical_lines.size() > 0)
    {
        for (const auto& time_seconds_vertical_line : time_secnds_vertical_lines)
        {
            // Convert time to position X.
            const double position_x =
                juce::jmap<double>(time_seconds_vertical_line, visibleRangeSeconds.getStart(), visibleRangeSeconds.getEnd(), position_left, position_right);

            PositionWithTimeInfo position_with_time_info;
            position_with_time_info.positionX = position_x;
            position_with_time_info.timeInSeconds = time_seconds_vertical_line;

            result.add(position_with_time_info);
        }
    }

    return result;
}

}
}
