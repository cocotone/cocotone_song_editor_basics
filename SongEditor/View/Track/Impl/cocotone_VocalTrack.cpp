namespace cctn
{
namespace song
{

//==============================================================================
class VocalTrackHeader
    : public cctn::song::TrackHeaderBase
{
public:
    explicit VocalTrackHeader(VocalTrack& owner)
        : ownerComponent(owner)
    {
        // Grid Size
        labelGridSize = std::make_unique<juce::Label>();
        labelGridSize->setText("Vocal: ", juce::dontSendNotification);
        labelGridSize->setJustificationType(juce::Justification::centredRight);
        addAndMakeVisible(labelGridSize.get());
    }

    ~VocalTrackHeader() override
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
    VocalTrack& ownerComponent;

    std::unique_ptr<juce::Label> labelGridSize;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VocalTrackHeader)
};

//==============================================================================
class VocalTrackLane
    : public cctn::song::TrackLaneBase<cctn::song::SongDocument>
{
public:
    struct Region
    {
        juce::Range<juce::int64> rangeInTicks;
        juce::String lyrics;

        JUCE_LEAK_DETECTOR(Region)
    };

    VocalTrackLane()
    {
    }

    ~VocalTrackLane() override
    {
    }

private:
    //==============================================================================
    void paint(juce::Graphics& g) override
    {
        juce::Graphics::ScopedSaveState save_state(g);

        g.fillAll(kColourWallpaper);

        drawEvents(g);
    }

    //==============================================================================
    void updateContent(const cctn::song::SongDocument& content, const juce::var& properties) override
    {
        noteRegionsInTicks.clear();
        
        juce::int64 ticks_of_region_start = INT64_MAX;
        juce::int64 ticks_of_region_end = 0;
        juce::String lyrics_of_region = "";

        auto current_notes = content.getNotes();
        cctn::song::SongDocument::MusicalTmeDomainNoteComparator sorter;
        current_notes.sort(sorter);

        for (const auto& note : current_notes)
        {
            const auto ticks_note_start = cctn::song::SongDocument::Calculator::barToTick(content, note.startTimeInMusicalTime);

            const auto musical_time_note_end = cctn::song::SongDocument::Calculator::calculateNoteOffPosition(content, note);
            const auto ticks_note_end = cctn::song::SongDocument::Calculator::barToTick(content, musical_time_note_end);

            ticks_of_region_start = juce::jmin<juce::int64>(ticks_of_region_start, ticks_note_start);
            ticks_of_region_end = juce::jmax<juce::int64>(ticks_of_region_end, ticks_note_end);

            lyrics_of_region += note.lyric;
        }

        const auto new_region = Region {
            juce::Range<juce::int64>(ticks_of_region_start, ticks_of_region_end),
            lyrics_of_region
        };

        noteRegionsInTicks.add(new_region);
    }

    //==============================================================================
    void drawEvents(juce::Graphics& g)
    {
        juce::Graphics::ScopedSaveState save_state(g);

        const auto rect_area = getLocalBounds();

        const auto& range_visible_time_in_ticks = getViewRangeInTicks();

        juce::int64 ticks_of_region_start = 0;
        juce::int64 ticks_of_region_end = 0;

        for (const auto& region : noteRegionsInTicks)
        {
            const auto position_start = ticksToPositionX(region.rangeInTicks.getStart(), getViewRangeInTicks(), rect_area.getX(), rect_area.getRight());
            const auto position_end = ticksToPositionX(region.rangeInTicks.getEnd(), getViewRangeInTicks(), rect_area.getX(), rect_area.getRight());
            const auto rect_region = juce::Rectangle<int>{ position_start, rect_area.getY(), position_end - position_start, rect_area.getHeight() };

            g.setColour(juce::Colours::darkcyan);
            g.fillRect(rect_region);

            g.setColour(juce::Colours::white);
            g.setFont(juce::Font().withHeight(rect_area.getHeight() * 0.45f).withExtraKerningFactor(0.5f));
            g.drawText(region.lyrics, position_start + 2, rect_area.getY() + 2, position_end - position_start, rect_area.getHeight(), juce::Justification::centredLeft);

            g.setColour(juce::Colours::white);
            g.drawRect(rect_region, 1.0f);
        }
    }

    juce::Array<Region> noteRegionsInTicks;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VocalTrackLane)
};

//==============================================================================
VocalTrack::VocalTrack(cctn::song::ITrackDataAccessDelegate& trackAccessDelegate)
    : trackAccessDelegate(trackAccessDelegate)
{
    headerComponent = std::make_unique<VocalTrackHeader>(*this);
    addAndMakeVisible(headerComponent.get());

    laneComponent = std::make_unique<VocalTrackLane>();
    addAndMakeVisible(laneComponent.get());

    // Register auto layout
    TrackComponentBase::registerHeaderComponent(headerComponent.get());
    TrackComponentBase::registerLaneComponent(laneComponent.get());
}

VocalTrack::~VocalTrack()
{
    TrackComponentBase::registerHeaderComponent(nullptr);
    TrackComponentBase::registerLaneComponent(nullptr);

    headerComponent.reset();
    laneComponent.reset();
}

//==============================================================================
void VocalTrack::handleAsyncUpdate()
{
    repaint();
}

//==============================================================================
void VocalTrack::triggerUpdateContent()
{
    if (trackAccessDelegate.getSongDocumentEditor().has_value() &&
        trackAccessDelegate.getSongDocumentEditor().value()->getCurrentDocument().has_value())
    {
        const auto header_properties = headerComponent->getProperties();
        const auto& song_document = *trackAccessDelegate.getSongDocumentEditor().value()->getCurrentDocument().value();
        laneComponent->updateContent(song_document, header_properties);
    }

    repaint();
}

void VocalTrack::triggerUpdateVisibleRange()
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
