namespace cctn
{
namespace song
{

//==============================================================================
class TempoTrackHeader
    : public cctn::song::TrackHeaderBase
{
public:
    explicit TempoTrackHeader(TempoTrack& owner)
        : ownerComponent(owner)
    {
        // Grid Size
        labelGridSize = std::make_unique<juce::Label>();
        labelGridSize->setText("Tempo: ", juce::dontSendNotification);
        labelGridSize->setJustificationType(juce::Justification::centredRight);
        addAndMakeVisible(labelGridSize.get());
    }

    ~TempoTrackHeader() override
    {
    }

private:
    //==============================================================================
    void paint(juce::Graphics& g) override
    {}

    void resized() override
    {
        auto rect_area = getLocalBounds();

        const auto width_label = rect_area.getWidth() * 0.6f;

        labelGridSize->setBounds(rect_area);
    }

    //==============================================================================
    TempoTrack& ownerComponent;

    std::unique_ptr<juce::Label> labelGridSize;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TempoTrackHeader)
};

//==============================================================================
class TempoTrackLane
    : public cctn::song::TrackLaneBase<cctn::song::SongDocument>
{
public:
    TempoTrackLane()
    {
    }

    ~TempoTrackLane() override
    {
    }

private:
    //==============================================================================
    void paint(juce::Graphics& g) override
    {
        juce::Graphics::ScopedSaveState save_state(g);

        g.fillAll(kColourWallpaper);

        drawTempoEvents(g);
    }

    //==============================================================================
    void updateContent(const cctn::song::SongDocument& content) override
    {
        currentTempoTrack = content.getTempoTrack();
    }

    //==============================================================================
    void drawTempoEvents(juce::Graphics& g)
    {
        juce::Graphics::ScopedSaveState save_state(g);

        const auto rect_area = getLocalBounds();

        const auto precise_beat_and_time_array = currentTempoTrack;
        const auto range_visible_time_in_ticks = getViewRangeInTicks();

        g.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 10, 0));

        for (const auto& tempo_event : currentTempoTrack.getEvents())
        {
            if (tempo_event.getEventType() == cctn::song::SongDocument::TempoEvent::TempoEventType::kTempo ||
                tempo_event.getEventType() == cctn::song::SongDocument::TempoEvent::TempoEventType::kBoth)
            {
                const auto tempo_text = " " + juce::String(tempo_event.getTempo());

                const auto position_x = ticksToPositionX(tempo_event.getTick(), getViewRangeInTicks(), rect_area.getX(), rect_area.getRight());

                g.setColour(juce::Colours::white);
                g.drawVerticalLine(position_x, 0.0f, getHeight());
                g.drawText(tempo_text, position_x + 2, rect_area.getY() + 2, 60, 20, juce::Justification::centredLeft);
            }
        }
    }

    cctn::song::SongDocument::TempoTrack currentTempoTrack{};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TempoTrackLane)
};

//==============================================================================
TempoTrack::TempoTrack(cctn::song::ITrackDataAccessDelegate& trackAccessDelegate)
    : trackAccessDelegate(trackAccessDelegate)
{
    headerComponent = std::make_unique<TempoTrackHeader>(*this);
    addAndMakeVisible(headerComponent.get());

    laneComponent = std::make_unique<TempoTrackLane>();
    addAndMakeVisible(laneComponent.get());

    // Register auto layout
    TrackComponentBase::registerHeaderComponent(headerComponent.get());
    TrackComponentBase::registerLaneComponent(laneComponent.get());
}

TempoTrack::~TempoTrack()
{
    TrackComponentBase::registerHeaderComponent(nullptr);
    TrackComponentBase::registerLaneComponent(nullptr);

    headerComponent.reset();
    laneComponent.reset();
}

//==============================================================================
void TempoTrack::handleAsyncUpdate()
{
    repaint();
}

//==============================================================================
void TempoTrack::triggerUpdateContent()
{
    if (trackAccessDelegate.getSongDocumentEditor().has_value() &&
        trackAccessDelegate.getSongDocumentEditor().value()->getCurrentDocument().has_value())
    {
        const auto& song_document = *trackAccessDelegate.getSongDocumentEditor().value()->getCurrentDocument().value();
        laneComponent->updateContent(song_document);
    }

    repaint();
}

void TempoTrack::triggerUpdateVisibleRange()
{
    juce::Range<double> range_visible;

    if (trackAccessDelegate.getVisibleRangeInTicks().has_value())
    {
        range_visible = trackAccessDelegate.getVisibleRangeInTicks().value();
    }

    laneComponent->setViewRangeInTicks(range_visible);

    repaint();
}

}
}
