namespace cctn
{
namespace song
{

//==============================================================================
MusicalTimePreviewTrack::MusicalTimePreviewTrack()
{
}

MusicalTimePreviewTrack::~MusicalTimePreviewTrack()
{
}

//==============================================================================
void MusicalTimePreviewTrack::paint(juce::Graphics& g)
{
    juce::Graphics::ScopedSaveState save_state(g);

    g.fillAll(kColourWallpaper);

    drawMusicalTimeGrid(g);
}

void MusicalTimePreviewTrack::resized()
{
}

//==============================================================================
void MusicalTimePreviewTrack::handleAsyncUpdate()
{
    repaint();
}

//==============================================================================
void MusicalTimePreviewTrack::updateContent(const cctn::song::SongDocumentEditor& songDocumentEditor)
{
    currentBeatTimePoints = songDocumentEditor.getEditorContext().currentBeatTimePoints;

    currentAbsoluteTicksRelatedBeatTimePoints.clear();
    if (songDocumentEditor.getCurrentDocument().has_value())
    {
        for (const auto& beat_time_point : currentBeatTimePoints)
        {
            const auto absolute_ticks = cctn::song::SongDocument::Calculator::barToTick(*songDocumentEditor.getCurrentDocument().value(), beat_time_point.musicalTime);
            currentAbsoluteTicksRelatedBeatTimePoints.push_back(absolute_ticks);
        }
    }

    repaint();
}

//==============================================================================
void MusicalTimePreviewTrack::drawMusicalTimeGrid(juce::Graphics& g)
{
    juce::Graphics::ScopedSaveState save_state(g);

    const auto rect_area = getLocalBounds();

    const auto precise_beat_and_time_array = currentBeatTimePoints;
    const auto range_visible_time_in_ticks = getViewRangeInTicks();

    // Set clipping mask
    //g.reduceClipRegion(rectBeatRulerArea);

    g.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 10, 0));

    juce::String last_beat_signature = "";
    for (int beat_time_idx = 0; beat_time_idx < currentAbsoluteTicksRelatedBeatTimePoints.size(); beat_time_idx++)
    {
        const auto& time_in_ticks = currentAbsoluteTicksRelatedBeatTimePoints[beat_time_idx];
        const auto& beat_time_point = currentBeatTimePoints[beat_time_idx];

        const auto signature_text = " " +
            juce::String(beat_time_point.musicalTime.bar) + ":" +
            juce::String(beat_time_point.musicalTime.beat);

        if (signature_text != last_beat_signature)
        {
            const auto position_x = ticksToPositionX(time_in_ticks, getViewRangeInTicks(), rect_area.getX(), rect_area.getRight());

            g.setColour(juce::Colours::white);
            g.drawVerticalLine(position_x, 0.0f, getHeight());
            g.drawText(signature_text, position_x + 2, rect_area.getY() + 2, 60, 20, juce::Justification::centredLeft);
            last_beat_signature = signature_text;
        }
    }
}

}
}
