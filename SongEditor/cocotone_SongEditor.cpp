namespace cctn
{
namespace song
{

namespace
{
constexpr int kNumVisibleOctaves = 2;
}

//==============================================================================
SongEditor::SongEditor()
    : positionInfoProviderPtr(nullptr)
    , songDocumentEditorPtr({})
{
    songEditorOperation = std::make_shared<cctn::song::SongEditorOperation>();

    pianoRollEventDispatcher = std::make_unique<cctn::song::PianoRollEventDispatcher>(songEditorOperation);

    pianoRollKeyboard = std::make_unique<cctn::song::PianoRollKeyboard>(kNumVisibleOctaves);
    addAndMakeVisible(pianoRollKeyboard.get());

    pianoRollPreviewSurface = std::make_unique<cctn::song::PianoRollPreviewSurface>(*pianoRollKeyboard, kNumVisibleOctaves);
    addAndMakeVisible(pianoRollPreviewSurface.get());

    pianoRollInteractionSurface = 
        std::make_unique<cctn::song::PianoRollInteractionSurface>(*pianoRollPreviewSurface.get(), *pianoRollEventDispatcher.get());
    addAndMakeVisible(pianoRollInteractionSurface.get());

    const int num_visible_keys = pianoRollKeyboard->getVisibleKeySize();

    pianoRollSliderVertical = std::make_unique<juce::Slider>(
        juce::Slider::SliderStyle::LinearVertical, 
        juce::Slider::TextEntryBoxPosition::NoTextBox);
    pianoRollSliderVertical->getValueObject().referTo(valuePianoRollBottomKeyNumber);
    pianoRollSliderVertical->setRange(0.0, 127.0 - num_visible_keys, 1.0);
    pianoRollSliderVertical->setColour(
        juce::Slider::ColourIds::trackColourId, 
        pianoRollSliderVertical->findColour(juce::Slider::ColourIds::backgroundColourId));
    addAndMakeVisible(pianoRollSliderVertical.get());

    pianoRollScrollBarHorizontal = std::make_unique<juce::ScrollBar>(false);
    pianoRollScrollBarHorizontal->setRangeLimits(juce::Range<double>{0.0, 600.0}, juce::dontSendNotification);
    pianoRollScrollBarHorizontal->setCurrentRange(juce::Range<double>{0.0, 6.0}, juce::dontSendNotification);
    addAndMakeVisible(pianoRollScrollBarHorizontal.get());

    pianoRollTimeRuler = std::make_unique<PianoRollTimeRuler>();
    addAndMakeVisible(pianoRollTimeRuler.get());

    // Grid Size
    labelPianoRollGridSize = std::make_unique<juce::Label>();
    labelPianoRollGridSize->setText("Grid Size: ", juce::dontSendNotification);
    labelPianoRollGridSize->setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(labelPianoRollGridSize.get());

    comboboxPianoRollGridSize = std::make_unique<juce::ComboBox>();
    comboboxPianoRollGridSize->onChange =
        [safe_this = juce::Component::SafePointer(this)]() {
        if (safe_this.getComponent() == nullptr)
        {
            return;
        }

        const auto item_idx = safe_this->comboboxPianoRollGridSize->getSelectedItemIndex();
        if (safe_this->mapIndexToGridSize.count(item_idx) > 0)
        {
            safe_this->valuePianoRollGridSize = (int)safe_this->mapIndexToGridSize[item_idx];
        }
        };
    addAndMakeVisible(comboboxPianoRollGridSize.get());

    populateComboBoxWithGridSize(*comboboxPianoRollGridSize.get(), mapIndexToGridSize);

    // Note Length
    labelInputNoteLength = std::make_unique<juce::Label>();
    labelInputNoteLength->setText("Note Length: ", juce::dontSendNotification);
    labelInputNoteLength->setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(labelInputNoteLength.get());

    comboboxInputNoteLength = std::make_unique<juce::ComboBox>();
    comboboxInputNoteLength->onChange = 
        [safe_this = juce::Component::SafePointer(this)]() {
        if (safe_this.getComponent() == nullptr)
        {
            return;
        }

        const auto item_idx = safe_this->comboboxInputNoteLength->getSelectedItemIndex();
        if (safe_this->mapIndexToNoteLength.count(item_idx) > 0)
        {
            safe_this->valuePianoRollInputNoteLength = (int)safe_this->mapIndexToNoteLength[item_idx];
        }
        };
    addAndMakeVisible(comboboxInputNoteLength.get());

    populateComboBoxWithNoteLength(*comboboxInputNoteLength.get(), mapIndexToNoteLength);

    // Lyric Mora
    labelInputMora = std::make_unique<juce::Label>();
    labelInputMora->setText("Lyric Mora: ", juce::dontSendNotification);
    labelInputMora->setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(labelInputMora.get());

    comboboxInputMora = std::make_unique<juce::ComboBox>();
    comboboxInputMora->onChange =
        [safe_this = juce::Component::SafePointer(this)]() {
        if (safe_this.getComponent() == nullptr)
        {
            return;
        }

        const auto item_idx = safe_this->comboboxInputMora->getSelectedItemIndex();
        if (safe_this->mapIndexToMora.count(item_idx) > 0)
        {
            safe_this->valuePianoRollInputMora = safe_this->mapIndexToMora[item_idx];
        }
        };
    addAndMakeVisible(comboboxInputMora.get());

    populateComboBoxWithLyricMora(*comboboxInputMora.get(), mapIndexToMora);


    // Add listener
    pianoRollScrollBarHorizontal->addListener(this);

    valuePianoRollBottomKeyNumber.addListener(this);
    valuePianoRollGridSize.addListener(this);
    valuePianoRollInputNoteLength.addListener(this);
    valuePianoRollInputMora.addListener(this);

    // Set initial state.
    valuePianoRollBottomKeyNumber.setValue(55);

    pianoRollPreviewSurface->setVisibleRangeTimeInSeconds(juce::Range<double>{0.0, 6.0});
    pianoRollPreviewSurface->setGridVerticalLineIntervaleInSeconds(1.0);

    pianoRollTimeRuler->setVisibleRangeTimeInSeconds(juce::Range<double>{0.0, 6.0});
    pianoRollTimeRuler->setGridVerticalLineIntervaleInSeconds(0.500);

    // Trigger Initial Update.
    initialUpdate();

    startTimerHz(30);
}

SongEditor::~SongEditor()
{
    stopTimer();

    valuePianoRollBottomKeyNumber.removeListener(this);
    valuePianoRollGridSize.removeListener(this);
    valuePianoRollInputNoteLength.removeListener(this);
    valuePianoRollInputMora.removeListener(this);

    pianoRollScrollBarHorizontal->removeListener(this);

    pianoRollSliderVertical.reset();
    pianoRollScrollBarHorizontal.reset();
    pianoRollPreviewSurface.reset();
    pianoRollKeyboard.reset();
}

//==============================================================================
void SongEditor::registerPositionInfoProvider(IPositionInfoProvider* provider)
{
    std::unique_lock lock(mutex);

    positionInfoProviderPtr = provider;
}

void SongEditor::unregisterPositionInfoProvider(IPositionInfoProvider* provider)
{
    std::unique_lock lock(mutex);

    if (positionInfoProviderPtr == provider)
    {
        positionInfoProviderPtr = nullptr;
    }
}

//==============================================================================
void SongEditor::registerSongDocumentEditor(std::shared_ptr<cctn::song::SongDocumentEditor> documentEditor)
{
    if (songDocumentEditorPtr.lock().get() != documentEditor.get())
    {
        songDocumentEditorPtr = documentEditor;
        songDocumentEditorPtr.lock()->addChangeListener(this);
        songEditorOperation->attachDocument(songDocumentEditorPtr.lock());

        pianoRollPreviewSurface->setDocumentForPreview(documentEditor);
        pianoRollTimeRuler->setDocumentForPreview(documentEditor);

        valuePianoRollGridSize = (int)songDocumentEditorPtr.lock()->getEditorContext().currentGridSize;

        valuePianoRollInputNoteLength = (int)songDocumentEditorPtr.lock()->getEditorContext().currentNoteLength;

        valuePianoRollInputMora = songDocumentEditorPtr.lock()->getEditorContext().currentNoteLyric.text;
    }
}

void SongEditor::unregisterSongDocumentEditor(std::shared_ptr<cctn::song::SongDocumentEditor> documentEditor)
{
    if (songDocumentEditorPtr.lock().get() == documentEditor.get())
    {
        songEditorOperation->detachDocument();
        songDocumentEditorPtr.lock()->removeChangeListener(this);

        songDocumentEditorPtr.reset();

        pianoRollPreviewSurface->setDocumentForPreview(nullptr);
        pianoRollTimeRuler->setDocumentForPreview(nullptr);
    }
}

//==============================================================================
void SongEditor::paint(juce::Graphics& g)
{
    g.fillAll(kColourWallpaper);
}

void SongEditor::resized()
{
    auto rect_area = getLocalBounds();

    rectInputOptions = rect_area.removeFromTop(32);

    const auto width_piano_roll_keyboard = 80;
    const auto width_piano_roll_scrollbar = 24;
    const auto height_piano_roll_time_ruler = 48;
    const auto piano_roll_time_ruler_top = rectInputOptions.getBottom();
    const auto piano_roll_main_top = piano_roll_time_ruler_top + height_piano_roll_time_ruler;
    const auto piano_roll_main_bottom = getHeight() - width_piano_roll_scrollbar;

    labelPianoRollGridSize->setBounds(rectInputOptions.removeFromLeft(160).reduced(4));
    comboboxPianoRollGridSize->setBounds(rectInputOptions.removeFromLeft(160).reduced(4));

    labelInputNoteLength->setBounds(rectInputOptions.removeFromLeft(160).reduced(4));
    comboboxInputNoteLength->setBounds(rectInputOptions.removeFromLeft(160).reduced(4));

    labelInputMora->setBounds(rectInputOptions.removeFromLeft(160).reduced(4));
    comboboxInputMora->setBounds(rectInputOptions.removeFromLeft(160).reduced(4));

    const auto rect_piano_roll_keyboard = 
        juce::Rectangle<int>{ 0, 
        (int)piano_roll_main_top, 
        width_piano_roll_keyboard, 
        piano_roll_main_bottom  - piano_roll_main_top };
    
    const auto rect_piano_roll_preview =
        juce::Rectangle<int>{ width_piano_roll_keyboard, 
        (int)piano_roll_main_top,
        getWidth() - width_piano_roll_keyboard - width_piano_roll_scrollbar,
        piano_roll_main_bottom - piano_roll_main_top};
    
    const auto rect_piano_roll_time_ruler =
        juce::Rectangle<int>{ 0,
        piano_roll_time_ruler_top,
        getWidth(),
        piano_roll_main_top - piano_roll_time_ruler_top };
    
    const auto rect_piano_roll_scrollbar_vertical =
        juce::Rectangle<int>{ getWidth() - width_piano_roll_scrollbar,
        (int)piano_roll_main_top,
        width_piano_roll_scrollbar,
        piano_roll_main_bottom - piano_roll_main_top };
    
    const auto rect_piano_roll_scrollbar_horizontal = 
        juce::Rectangle<int>{ width_piano_roll_keyboard,
        piano_roll_main_bottom,
        getWidth() - width_piano_roll_keyboard - width_piano_roll_scrollbar,
        width_piano_roll_scrollbar };

    pianoRollPreviewSurface->setBounds(rect_piano_roll_preview);
    pianoRollInteractionSurface->setBounds(rect_piano_roll_preview);

    pianoRollKeyboard->setBounds(rect_piano_roll_keyboard);
    pianoRollSliderVertical->setBounds(rect_piano_roll_scrollbar_vertical);
    pianoRollScrollBarHorizontal->setBounds(rect_piano_roll_scrollbar_horizontal);

    {
        PianoRollTimeRuler::LayoutSource layout_source;
        layout_source.labelLeft = 0;
        layout_source.labelRight = rect_piano_roll_preview.getX();
        layout_source.rulerLeft = rect_piano_roll_preview.getX();
        layout_source.rulerRight = rect_piano_roll_preview.getRight();
        pianoRollTimeRuler->setLayoutSource(layout_source);
    }
    pianoRollTimeRuler->setBounds(rect_piano_roll_time_ruler);
}

//==============================================================================
void SongEditor::valueChanged(juce::Value& value)
{
    if (value.refersToSameSourceAs(valuePianoRollBottomKeyNumber))
    {
        pianoRollKeyboard->setVisibleBottomKeyNoteNumber((int)valuePianoRollBottomKeyNumber.getValue());
        pianoRollPreviewSurface->setVisibleBottomKeyNoteNumber((int)valuePianoRollBottomKeyNumber.getValue());
    }
    else if (value.refersToSameSourceAs(valuePianoRollInputNoteLength))
    {
        comboboxInputNoteLength->setSelectedItemIndex((int)valuePianoRollInputNoteLength.getValue(), juce::dontSendNotification);
        songDocumentEditorPtr.lock()->getEditorContext().currentNoteLength = (cctn::song::NoteLength)(int)valuePianoRollInputNoteLength.getValue();
        songDocumentEditorPtr.lock()->updateEditorContext();
    }
    else if (value.refersToSameSourceAs(valuePianoRollGridSize))
    {
        comboboxPianoRollGridSize->setSelectedItemIndex((int)valuePianoRollGridSize.getValue(), juce::dontSendNotification);
        pianoRollPreviewSurface->setDrawingGridInterval((cctn::song::NoteLength)(int)valuePianoRollGridSize.getValue());

        songDocumentEditorPtr.lock()->getEditorContext().currentGridSize = (cctn::song::NoteLength)(int)valuePianoRollGridSize.getValue();
        songDocumentEditorPtr.lock()->updateEditorContext();
    }
    else if (value.refersToSameSourceAs(valuePianoRollInputMora))
    {
        comboboxInputMora->setText(valuePianoRollInputMora.getValue(), juce::dontSendNotification);

        songDocumentEditorPtr.lock()->getEditorContext().currentNoteLyric.text = valuePianoRollInputMora.getValue();
        songDocumentEditorPtr.lock()->updateEditorContext();
    }
}

//==============================================================================
void SongEditor::scrollBarMoved(juce::ScrollBar* scrollBarThatHasMoved, double newRangeStart)
{
    if (scrollBarThatHasMoved == pianoRollScrollBarHorizontal.get())
    {
        const auto current_range = pianoRollScrollBarHorizontal->getCurrentRange();
        pianoRollPreviewSurface->setVisibleRangeTimeInSeconds(current_range);
        pianoRollTimeRuler->setVisibleRangeTimeInSeconds(current_range);
    }
}

//==============================================================================
void SongEditor::timerCallback()
{
    std::unique_lock lock(mutex);

    if (positionInfoProviderPtr != nullptr)
    {
        const auto position_info_optional = positionInfoProviderPtr->getPositionInfo();
        if (position_info_optional.has_value())
        {
            const double current_position_in_seconds = position_info_optional.value().getTimeInSeconds().orFallback(0.0);
            
            pianoRollPreviewSurface->setPlayingPositionInSeconds(current_position_in_seconds);
            pianoRollPreviewSurface->setCurrentPositionInfo(position_info_optional.value());

            pianoRollTimeRuler->setPlayingPositionInSeconds(current_position_in_seconds);
            pianoRollTimeRuler->setCurrentPositionInfo(position_info_optional.value());
        }
    }
}

//==============================================================================
void SongEditor::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (!songDocumentEditorPtr.expired())
    {
        if (source == songDocumentEditorPtr.lock().get())
        {
            songDocumentEditorPtr.lock()->updateEditorContext();
        }
    }
}

//==============================================================================
void SongEditor::populateComboBoxWithGridSize(juce::ComboBox& comboBox, std::map<int, cctn::song::NoteLength>& mapIndexToNoteLength)
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
        ThirtySecond,    // Thirty-second note
        SixtyFourth,     // Sixty-fourth note
        Triplet,         // Quarter note triplet
        EighthTriplet,   // Eighth note triplet
        SixteenthTriplet,// Sixteenth note triplet
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
        { NoteLength::ThirtySecond, "1/32" },
        { NoteLength::SixtyFourth, "1/64" },
        { NoteLength::Triplet, "1/4T" },
        { NoteLength::EighthTriplet, "1/8T" },
        { NoteLength::SixteenthTriplet, "1/16T" },
        //{ NoteLength::DottedHalf, "1/2." },
        //{ NoteLength::DottedQuarter, "1/4." },
        //{ NoteLength::DottedEighth, "1/8." },
        //{ NoteLength::DottedSixteenth, "1/16." }
    };

    comboBox.clear(juce::dontSendNotification);
    mapIndexToNoteLength.clear();

    for (int item_idx = 0; item_idx < std::size(items); item_idx++)
    {
        comboBox.addItem(items[item_idx].name, item_idx + 1);
        mapIndexToNoteLength[item_idx] = items[item_idx].noteLength;
    }

    comboBox.setSelectedItemIndex((int)cctn::song::NoteLength::Quarter, juce::dontSendNotification);
}

void SongEditor::populateComboBoxWithNoteLength(juce::ComboBox& comboBox, std::map<int, cctn::song::NoteLength>& mapIndexToNoteLength)
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
        ThirtySecond,    // Thirty-second note
        SixtyFourth,     // Sixty-fourth note
        Triplet,         // Quarter note triplet
        EighthTriplet,   // Eighth note triplet
        SixteenthTriplet,// Sixteenth note triplet
        DottedHalf,      // Dotted half note
        DottedQuarter,   // Dotted quarter note
        DottedEighth,    // Dotted eighth note
        DottedSixteenth  // Dotted sixteenth note
    };
    */
    const NoteLengthItem items[] =
    {
        { NoteLength::Whole, "1/1" },
        { NoteLength::Half, "1/2" },
        { NoteLength::Quarter, "1/4" },
        { NoteLength::Eighth, "1/8" },
        { NoteLength::Sixteenth, "1/16" },
        { NoteLength::ThirtySecond, "1/32" },
        { NoteLength::SixtyFourth, "1/64" },
        { NoteLength::Triplet, "1/4T" },
        { NoteLength::EighthTriplet, "1/8T" },
        { NoteLength::SixteenthTriplet, "1/16T" },
        { NoteLength::DottedHalf, "1/2." },
        { NoteLength::DottedQuarter, "1/4." },
        { NoteLength::DottedEighth, "1/8." },
        { NoteLength::DottedSixteenth, "1/16." }
    };

    comboBox.clear(juce::dontSendNotification);
    mapIndexToNoteLength.clear();

    for (int item_idx = 0; item_idx < std::size(items); item_idx++)
    {
        comboBox.addItem(items[item_idx].name, item_idx + 1);
        mapIndexToNoteLength[item_idx] = items[item_idx].noteLength;
    }

    comboBox.setSelectedItemIndex((int)cctn::song::NoteLength::Quarter, juce::dontSendNotification);
}

void SongEditor::populateComboBoxWithLyricMora(juce::ComboBox& comboBox, std::map<int, cctn::song::Mora>& mapIndexToMora)
{
    const auto mora_kanas = StaticMoraKana().getMoraKanas();

    comboBox.clear(juce::dontSendNotification);
    mapIndexToMora.clear();

    for (int item_idx = 0; item_idx < std::size(mora_kanas); item_idx++)
    {
        comboBox.addItem(mora_kanas[item_idx], item_idx + 1);
        mapIndexToMora[item_idx] = mora_kanas[item_idx];
    }

    comboBox.setSelectedItemIndex(0, juce::dontSendNotification);
}

void SongEditor::initialUpdate()
{
    valuePianoRollInputNoteLength = (int)cctn::song::NoteLength::Quarter;
    valuePianoRollGridSize = (int)cctn::song::NoteLength::Sixteenth;
}

}
}
