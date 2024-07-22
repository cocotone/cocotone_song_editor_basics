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
    labelTimeRuler = std::make_unique<juce::Label>();
    labelTimeRuler->setText("Time", juce::dontSendNotification);
    labelTimeRuler->setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(labelTimeRuler.get());

    labelBeatRuler = std::make_unique<juce::Label>();
    labelBeatRuler->setText("Beat", juce::dontSendNotification);
    labelBeatRuler->setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(labelBeatRuler.get());
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

void PianoRollTimeRuler::setCurrentPositionInfo(const juce::AudioPlayHead::PositionInfo& positionInfo)
{
    currentPositionInfo = positionInfo;
    repaint();
}

void PianoRollTimeRuler::setTimeRulerRectangle(const juce::Rectangle<int>& rectangle)
{
    rectTimeRulerArea = rectangle;
    updateLayout();
}

void PianoRollTimeRuler::setTimeRulerLabelRectangle(const juce::Rectangle<int>& rectangle)
{
    rectTimeRulerLabelArea = rectangle;
    updateLayout();
}

void PianoRollTimeRuler::setBeatRulerRectangle(const juce::Rectangle<int>& rectangle)
{
    rectBeatRulerArea = rectangle;
    updateLayout();
}

void PianoRollTimeRuler::setBeatRulerLabelRectangle(const juce::Rectangle<int>& rectangle)
{
    rectBeatRulerLabelArea = rectangle;
    updateLayout();
}

void PianoRollTimeRuler::updateLayout()
{
    labelTimeRuler->setBounds(rectTimeRulerLabelArea);
    labelBeatRuler->setBounds(rectBeatRulerLabelArea);

    repaint();
}

//==============================================================================
void PianoRollTimeRuler::paint(juce::Graphics& g)
{
    juce::Graphics::ScopedSaveState save_state(g);

    g.fillAll(kColourWallpaper);

    g.setColour(kColourMainBackground);
    
    //
    g.fillRect(rectTimeRulerArea);
    g.fillRect(rectTimeRulerLabelArea);
    
    //
    g.fillRect(rectBeatRulerArea);
    g.fillRect(rectBeatRulerLabelArea);

    //
    drawTimeRulerVerticalLines(g);
    drawBeatRulerVerticalLines(g);
    drawPlayingPositionMarker(g);

    //
    g.setColour(kColourGridLines);
    g.drawHorizontalLine(rectTimeRulerArea.getBottom() - 1, 0, rectTimeRulerArea.getRight());

    //
    g.setColour(kColourGridLines);
    g.drawHorizontalLine(rectBeatRulerArea.getBottom() - 1, 0, rectBeatRulerArea.getRight());
}

void PianoRollTimeRuler::resized()
{
    repaint();
}

//==============================================================================
void PianoRollTimeRuler::drawTimeRulerVerticalLines(juce::Graphics& g)
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
        g.drawText(juce::String(position_with_time_info.timeInSeconds, 1), position_with_time_info.positionX + 2, rectTimeRulerArea.getY() + 2, 60, 20, juce::Justification::centredLeft);
    }
}

void PianoRollTimeRuler::drawBeatRulerVerticalLines(juce::Graphics& g)
{
    juce::Graphics::ScopedSaveState save_state(g);

    double bpm = 120.0;
    int numerator = 4;
    int denominator = 4;

    const auto tempo_and_time_signature_optional = cctn::song::PositionInfoExtractor::extractTempoAndTimeSignature(currentPositionInfo);
    if (tempo_and_time_signature_optional.has_value())
    {
        bpm = tempo_and_time_signature_optional.value().bpm;
        numerator = tempo_and_time_signature_optional.value().numerator;
        denominator = tempo_and_time_signature_optional.value().denominator;
    }


    // NOTE: This procedure will fit to feature of tempo map track.
    const auto precise_beat_and_time_array = cctn::song::BeatTimePointFactory::extractPreciseBeatPoints(bpm, numerator, denominator, rangeVisibleTimeInSeconds.getStart(), rangeVisibleTimeInSeconds.getEnd());

    // Set clipping mask
    g.reduceClipRegion(rectBeatRulerArea);

    g.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 10, 0));

    for (const auto& beat_time_point : precise_beat_and_time_array)
    {
        const auto time_in_seconds = beat_time_point.timeInSeconds;
        beat_time_point.beat;
        const auto signature_text = beat_time_point.getFormattedTimeSignature(numerator, false);

        const auto position_x = timeToPositionX(time_in_seconds, rangeVisibleTimeInSeconds, rectBeatRulerArea.getX(), rectBeatRulerArea.getRight());

        g.setColour(juce::Colours::white);
        g.drawVerticalLine(position_x, 0.0f, getHeight());
        g.drawText(signature_text, position_x + 2, rectBeatRulerArea.getY() + 2, 60, 20, juce::Justification::centredLeft);
    }
}

void PianoRollTimeRuler::drawPlayingPositionMarker(juce::Graphics& g)
{
    juce::Graphics::ScopedSaveState save_state(g);

    const auto position_x = timeToPositionX(playingPositionInSeconds, rangeVisibleTimeInSeconds, rectTimeRulerArea.getX(), rectTimeRulerArea.getRight());

    juce::Rectangle<int> rect_marker =
        juce::Rectangle<int>{ position_x, 0, 2, getHeight() };

    // Set clipping mask
    g.reduceClipRegion(rectTimeRulerArea.getUnion(rectBeatRulerArea));

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
