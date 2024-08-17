namespace cctn
{
namespace song
{

//==============================================================================
class AbsoluteTimePreviewTrackHeader
    : public cctn::song::TrackHeaderBase
{
public:
    AbsoluteTimePreviewTrackHeader()
    {
    }

    ~AbsoluteTimePreviewTrackHeader() override
    {
    }

private:

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AbsoluteTimePreviewTrackHeader)
};

//==============================================================================
class AbsoluteTimePreviewTrackLane
    : public cctn::song::TrackLaneBase<cctn::song::SongDocument::BeatTimePoints>
{
public:
    AbsoluteTimePreviewTrackLane()
    {
    }

    ~AbsoluteTimePreviewTrackLane() override
    {
    }

private:
    //==============================================================================
    void paint(juce::Graphics& g) override
    {
        juce::Graphics::ScopedSaveState save_state(g);

        g.fillAll(kColourWallpaper);

        drawAbsoluteTimeGrid(g);
    }

    //==============================================================================
    void updateContent(const cctn::song::SongDocument::BeatTimePoints& content) override
    {
        currentBeatTimePoints = content;
    }

    //==============================================================================
    void drawAbsoluteTimeGrid(juce::Graphics& g)
    {
        juce::Graphics::ScopedSaveState save_state(g);

        const auto rect_area = getLocalBounds();

        const auto precise_beat_and_time_array = currentBeatTimePoints;
        const auto range_visible_time_in_ticks = getViewRangeInTicks();

        // Set clipping mask
        //g.reduceClipRegion(rectBeatRulerArea);

        g.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 10, 0));

        juce::String last_beat_signature = "";
        for (const auto& beat_time_point : currentBeatTimePoints)
        {
            const auto time_text = juce::String(beat_time_point.absoluteTimeInSeconds, 1);

            if (time_text != last_beat_signature)
            {
                const auto position_x = ticksToPositionX(beat_time_point.absoluteTicks, getViewRangeInTicks(), rect_area.getX(), rect_area.getRight());

                g.setColour(juce::Colours::white);
                g.drawVerticalLine(position_x, 0.0f, getHeight());
                g.drawText(time_text, position_x + 2, rect_area.getY() + 2, 60, 20, juce::Justification::centredLeft);
                last_beat_signature = time_text;
            }
        }
    }

    cctn::song::SongDocument::BeatTimePoints currentBeatTimePoints{};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AbsoluteTimePreviewTrackLane)
};

//==============================================================================
AbsoluteTimePreviewTrack::AbsoluteTimePreviewTrack()
{
    headerComponent = std::make_unique<AbsoluteTimePreviewTrackHeader>();
    addAndMakeVisible(headerComponent.get());

    laneComponent = std::make_unique<AbsoluteTimePreviewTrackLane>();
    addAndMakeVisible(laneComponent.get());

    // Register auto layout
    TrackComponentBase::registerHeaderComponent(headerComponent.get());
    TrackComponentBase::registerLaneComponent(laneComponent.get());
}

AbsoluteTimePreviewTrack::~AbsoluteTimePreviewTrack()
{
    TrackComponentBase::registerHeaderComponent(nullptr);
    TrackComponentBase::registerLaneComponent(nullptr);

    headerComponent.reset();
    laneComponent.reset();
}

//==============================================================================
void AbsoluteTimePreviewTrack::handleAsyncUpdate()
{
    laneComponent->setViewRangeInTicks(getViewRangeInTicks());
    repaint();
}

//==============================================================================
void AbsoluteTimePreviewTrack::updateContent(const cctn::song::SongDocumentEditor& songDocumentEditor)
{
    currentBeatTimePoints.clear();
    if (songDocumentEditor.getCurrentDocument().has_value())
    {
        currentBeatTimePoints =
            cctn::song::SongDocument::BeatTimePointsFactory::makeBeatTimePoints(
                *songDocumentEditor.getCurrentDocument().value(),
                cctn::song::NoteLength::Whole
            );
    }
    laneComponent->updateContent(currentBeatTimePoints);

    repaint();
}

}
}
