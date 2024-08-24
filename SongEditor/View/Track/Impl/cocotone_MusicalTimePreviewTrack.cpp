namespace cctn
{
namespace song
{

//==============================================================================
class MusicalTimePreviewTrackHeader
    : public cctn::song::TrackHeaderBase
    , private juce::Value::Listener
{
public:
    explicit MusicalTimePreviewTrackHeader(MusicalTimePreviewTrack& owner)
        : ownerComponent(owner)
    {
        // Grid Size
        labelGridSize = std::make_unique<juce::Label>();
        labelGridSize->setText("Beat  Grid Size: ", juce::dontSendNotification);
        labelGridSize->setJustificationType(juce::Justification::centredRight);
        addAndMakeVisible(labelGridSize.get());

        comboboxGridSize = std::make_unique<juce::ComboBox>();
        comboboxGridSize->onChange =
            [safe_this = juce::Component::SafePointer(this)]() {
            if (safe_this.getComponent() == nullptr)
            {
                return;
            }

            const auto item_idx = safe_this->comboboxGridSize->getSelectedItemIndex();
            if (safe_this->mapIndexToGridSize.count(item_idx) > 0)
            {
                safe_this->valueGridSize = (int)safe_this->mapIndexToGridSize[item_idx];
            }
            };
        addAndMakeVisible(comboboxGridSize.get());

        populateComboBoxWithGridSize(*comboboxGridSize.get(), mapIndexToGridSize);

        valueGridSize.addListener(this);

        initialUpdate();
    }

    ~MusicalTimePreviewTrackHeader() override
    {
        valueGridSize.removeListener(this);
    }

    //==============================================================================
    cctn::song::NoteLength getCurrenGridSize() const
    {
        return (cctn::song::NoteLength)(int)valueGridSize.getValue();
    }

private:
    //==============================================================================
    void paint(juce::Graphics& g) override
    {}

    void resized() override
    {
        auto rect_area = getLocalBounds();

        const auto width_label = rect_area.getWidth() * 0.6f;

        labelGridSize->setBounds(rect_area.removeFromLeft(width_label));
        comboboxGridSize->setBounds(rect_area);
    }

    //==============================================================================
    void valueChanged(juce::Value& value) override
    {
        if (value.refersToSameSourceAs(valueGridSize))
        {
            ownerComponent.triggerUpdateContent();
        }
    }

    //==============================================================================
    void initialUpdate()
    {
        valueGridSize = (int)cctn::song::NoteLength::Quarter;
    }

    static void populateComboBoxWithGridSize(juce::ComboBox& comboBox, std::map<int, cctn::song::NoteLength>& mapIndexToGridSize)
    {
        struct NoteLengthItem
        {
            cctn::song::NoteLength noteLength;
            const juce::String name;
        };

        /**
        enum class NoteLength
        {
            Whole,           // Whole note
            Half,            // Half note
            Quarter,         // Quarter note
            Eighth,          // Eighth note
            Sixteenth,       // Sixteenth note
            //ThirtySecond,    // Thirty-second note
            //SixtyFourth,     // Sixty-fourth note
            //Triplet,         // Quarter note triplet
            //EighthTriplet,   // Eighth note triplet
            //SixteenthTriplet,// Sixteenth note triplet
            //DottedHalf,      // Dotted half note
            //DottedQuarter,   // Dotted quarter note
            //DottedEighth,    // Dotted eighth note
            //DottedSixteenth  // Dotted sixteenth note
        };
        */
        const NoteLengthItem items[] =
        {
            { NoteLength::Whole, "1/1" },
            { NoteLength::Half, "1/2" },
            { NoteLength::Quarter, "1/4" },
            { NoteLength::Eighth, "1/8" },
            { NoteLength::Sixteenth, "1/16" },
            //{ NoteLength::ThirtySecond, "1/32" },
            //{ NoteLength::SixtyFourth, "1/64" },
            //{ NoteLength::Triplet, "1/4T" },
            //{ NoteLength::EighthTriplet, "1/8T" },
            //{ NoteLength::SixteenthTriplet, "1/16T" },
            //{ NoteLength::DottedHalf, "1/2." },
            //{ NoteLength::DottedQuarter, "1/4." },
            //{ NoteLength::DottedEighth, "1/8." },
            //{ NoteLength::DottedSixteenth, "1/16." }
        };

        comboBox.clear(juce::dontSendNotification);
        mapIndexToGridSize.clear();

        for (int item_idx = 0; item_idx < std::size(items); item_idx++)
        {
            comboBox.addItem(items[item_idx].name, item_idx + 1);
            mapIndexToGridSize[item_idx] = items[item_idx].noteLength;
        }

        comboBox.setSelectedItemIndex((int)cctn::song::NoteLength::Quarter, juce::dontSendNotification);
    }

    //==============================================================================
    MusicalTimePreviewTrack& ownerComponent;

    std::unique_ptr<juce::Label> labelGridSize;
    std::unique_ptr<juce::ComboBox> comboboxGridSize;
    std::map<int, cctn::song::NoteLength> mapIndexToGridSize;
    juce::Value valueGridSize;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MusicalTimePreviewTrackHeader)
};

//==============================================================================
class MusicalTimePreviewTrackLane
    : public cctn::song::TrackLaneBase<cctn::song::SongDocument::BeatTimePoints>
{
public:
    MusicalTimePreviewTrackLane()
    {
    }

    ~MusicalTimePreviewTrackLane() override
    {
    }

private:
    //==============================================================================
    void paint(juce::Graphics& g) override
    {
        juce::Graphics::ScopedSaveState save_state(g);

        g.fillAll(kColourWallpaper);

        drawMusicalTimeGrid(g);
    }

    //==============================================================================
    void updateContent(const cctn::song::SongDocument::BeatTimePoints& content, const juce::var& properties) override
    {
        currentBeatTimePoints = content;
    }

    //==============================================================================
    void drawMusicalTimeGrid(juce::Graphics& g)
    {
        juce::Graphics::ScopedSaveState save_state(g);

        const auto rect_area = getLocalBounds();

        const auto precise_beat_and_time_array = currentBeatTimePoints;
        const auto range_visible_time_in_ticks = getViewRangeInTicks();

        g.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 10, 0));

        juce::String last_beat_signature = "";
        for (const auto& beat_time_point : currentBeatTimePoints)
        {
            const auto signature_text = " " +
                juce::String(beat_time_point.musicalTime.bar) + ":" +
                juce::String(beat_time_point.musicalTime.beat);

            if (signature_text != last_beat_signature)
            {
                const auto position_x = ticksToPositionX(beat_time_point.absoluteTicks, getViewRangeInTicks(), rect_area.getX(), rect_area.getRight());

                g.setColour(juce::Colours::white);
                g.drawVerticalLine(position_x, 0.0f, getHeight());
                g.drawText(signature_text, position_x + 2, rect_area.getY() + 2, 60, 20, juce::Justification::centredLeft);
                last_beat_signature = signature_text;
            }
            else
            {
                const auto position_x = ticksToPositionX(beat_time_point.absoluteTicks, getViewRangeInTicks(), rect_area.getX(), rect_area.getRight());
                g.setColour(juce::Colours::white);
                g.drawVerticalLine(position_x, 0.0f, getHeight());
            }
        }
    }

    cctn::song::SongDocument::BeatTimePoints currentBeatTimePoints{};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MusicalTimePreviewTrackLane)
};

//==============================================================================
MusicalTimePreviewTrack::MusicalTimePreviewTrack(cctn::song::ITrackDataAccessDelegate& trackAccessDelegate)
    : trackAccessDelegate(trackAccessDelegate)
{
    headerComponent = std::make_unique<MusicalTimePreviewTrackHeader>(*this);
    addAndMakeVisible(headerComponent.get());

    laneComponent = std::make_unique<MusicalTimePreviewTrackLane>();
    addAndMakeVisible(laneComponent.get());

    // Register auto layout
    TrackComponentBase::registerHeaderComponent(headerComponent.get());
    TrackComponentBase::registerLaneComponent(laneComponent.get());
}

MusicalTimePreviewTrack::~MusicalTimePreviewTrack()
{
    TrackComponentBase::registerHeaderComponent(nullptr);
    TrackComponentBase::registerLaneComponent(nullptr);

    headerComponent.reset();
    laneComponent.reset();
}

//==============================================================================
void MusicalTimePreviewTrack::handleAsyncUpdate()
{

}

//==============================================================================
void MusicalTimePreviewTrack::triggerUpdateContent()
{
    currentBeatTimePoints.clear();
    
    if (trackAccessDelegate.getSongDocumentEditor().has_value() && 
        trackAccessDelegate.getSongDocumentEditor().value()->getCurrentDocument().has_value())
    {
        const auto header_properties = headerComponent->getProperties();

        auto grid_size = cctn::song::NoteLength::Quarter;
        if (auto* track_header = dynamic_cast<MusicalTimePreviewTrackHeader*>(headerComponent.get()))
        {
            grid_size = track_header->getCurrenGridSize();
        }

        const auto& song_document = *trackAccessDelegate.getSongDocumentEditor().value()->getCurrentDocument().value();
        currentBeatTimePoints = 
            cctn::song::SongDocument::BeatTimePointsFactory::makeBeatTimePoints(song_document, grid_size);

        laneComponent->updateContent(currentBeatTimePoints, header_properties);
    }

    repaint();
}

void MusicalTimePreviewTrack::triggerUpdateVisibleRange()
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
