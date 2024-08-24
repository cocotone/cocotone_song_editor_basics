namespace cctn
{
namespace song
{

//==============================================================================
class AbsoluteTimePreviewTrackHeader
    : public cctn::song::TrackHeaderBase
    , private juce::Value::Listener
{
public:
    explicit AbsoluteTimePreviewTrackHeader(AbsoluteTimePreviewTrack& owner)
        : ownerComponent(owner)
    {
        // Grid Size
        labelGridSize = std::make_unique<juce::Label>();
        labelGridSize->setText("Time  Grid Size: ", juce::dontSendNotification);
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
                safe_this->valueGridSize = (double)safe_this->mapIndexToGridSize[item_idx];
            }
            };
        addAndMakeVisible(comboboxGridSize.get());

        populateComboBoxWithGridSize(*comboboxGridSize.get(), mapIndexToGridSize);

        valueGridSize.addListener(this);

        initialUpdate();
    }

    ~AbsoluteTimePreviewTrackHeader() override
    {
        valueGridSize.removeListener(this);
    }

    //==============================================================================
    virtual juce::var getProperties()
    { 
        juce::DynamicObject::Ptr properties = new juce::DynamicObject();
        properties->setProperty("seconds", (double)valueGridSize.getValue());
        return properties;
    };

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
        const auto item_idx = comboboxGridSize->getSelectedItemIndex();
        if (mapIndexToGridSize.count(item_idx) > 0)
        {
            valueGridSize = (double)mapIndexToGridSize[item_idx];
        }
        else
        {
            valueGridSize = (double)0.5;
        }
    }

    static void populateComboBoxWithGridSize(juce::ComboBox& comboBox, std::map<int, double>& mapIndexToGridSize)
    {
        struct GridSizeItem
        {
            double seconds;
            const juce::String name;
        };

        const GridSizeItem items[] =
        {
            { 1.0,  "1.0 [seconds]" },
            { 0.5,  "0.5 [seconds]" },
            { 0.25, "0.25 [seconds]" },
            { 0.1,  "0.1 [seconds]" },
        };

        comboBox.clear(juce::dontSendNotification);
        mapIndexToGridSize.clear();

        for (int item_idx = 0; item_idx < std::size(items); item_idx++)
        {
            comboBox.addItem(items[item_idx].name, item_idx + 1);
            mapIndexToGridSize[item_idx] = items[item_idx].seconds;
        }

        comboBox.setSelectedItemIndex(1, juce::dontSendNotification);
    }


    //==============================================================================
    AbsoluteTimePreviewTrack& ownerComponent;

    std::unique_ptr<juce::Label> labelGridSize;
    std::unique_ptr<juce::ComboBox> comboboxGridSize;
    std::map<int, double> mapIndexToGridSize;
    juce::Value valueGridSize;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AbsoluteTimePreviewTrackHeader)
};

//==============================================================================
class AbsoluteTimePreviewTrackLane
    : public cctn::song::TrackLaneBase<cctn::song::SongDocument>
{
public:
    //==============================================================================
    struct TicksWithTimeInSeconds
    {
        juce::int64 absoluteTicks;
        double absoluteTimeInSeconds;

        JUCE_LEAK_DETECTOR(TicksWithTimeInSeconds)
    };

    //==============================================================================
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
    void updateContent(const cctn::song::SongDocument& content, const juce::var& properties) override
    {
        currentTicksWithTime.clear();

        const auto ticks_document_end = content.getTotalLengthInTicks();
        const auto grid_size_in_seconds = (double)properties.getProperty("seconds", 0.5);
        jassert(grid_size_in_seconds > 0.0);

        const auto absolute_time_end_in_seconds = cctn::song::SongDocument::Calculator::tickToAbsoluteTime(content, ticks_document_end);
        for (double seconds = 0.0; seconds < absolute_time_end_in_seconds; )
        {
            const auto ticks = cctn::song::SongDocument::Calculator::absoluteTimeToTick(content, seconds);

            currentTicksWithTime.add(TicksWithTimeInSeconds{ ticks, seconds });

            seconds += (double)properties.getProperty("seconds", grid_size_in_seconds);
        }
    }

    //==============================================================================
    void drawAbsoluteTimeGrid(juce::Graphics& g)
    {
        juce::Graphics::ScopedSaveState save_state(g);

        const auto rect_area = getLocalBounds();

        const auto range_visible_time_in_ticks = getViewRangeInTicks();

        g.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 10, 0));

        juce::String last_beat_signature = "";
        for (const auto& ticks_with_time : currentTicksWithTime)
        {
            const auto time_text = juce::String(ticks_with_time.absoluteTimeInSeconds, 1);

            if (time_text != last_beat_signature)
            {
                const auto position_x = ticksToPositionX(ticks_with_time.absoluteTicks, getViewRangeInTicks(), rect_area.getX(), rect_area.getRight());

                g.setColour(juce::Colours::white);
                g.drawVerticalLine(position_x, 0.0f, getHeight());
                g.drawText(time_text, position_x + 2, rect_area.getY() + 2, 60, 20, juce::Justification::centredLeft);
                last_beat_signature = time_text;
            }
        }
    }

    juce::Array<TicksWithTimeInSeconds> currentTicksWithTime{};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AbsoluteTimePreviewTrackLane)
};

//==============================================================================
AbsoluteTimePreviewTrack::AbsoluteTimePreviewTrack(cctn::song::ITrackDataAccessDelegate& delegate)
    : trackAccessDelegate(delegate)
{
    headerComponent = std::make_unique<AbsoluteTimePreviewTrackHeader>(*this);
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
    
    repaint();
}

//==============================================================================
void AbsoluteTimePreviewTrack::triggerUpdateContent()
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

void AbsoluteTimePreviewTrack::triggerUpdateVisibleRange()
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
