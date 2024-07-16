#include "cocotone_SongEditor.h"


namespace cctn
{
namespace song
{

//==============================================================================
SongEditor::SongEditor()
    : positionInfoProviderPtr(nullptr)
    , songEditorDocumentPtr({})
{
    songEditorEventBridge = std::make_shared<cctn::song::SongEditorEventBridge>();

    pianoRollEventDispatcher = std::make_unique<cctn::song::PianoRollEventDispatcher>(songEditorEventBridge);

    pianoRollKeyboard = std::make_unique<cctn::song::PianoRollKeyboard>();
    addAndMakeVisible(pianoRollKeyboard.get());

    pianoRollPreviewSurface = std::make_unique<cctn::song::PianoRollPreviewSurface>(*pianoRollKeyboard);
    addAndMakeVisible(pianoRollPreviewSurface.get());

    pianoRollInteractionSurface = 
        std::make_unique<cctn::song::PianoRollInteractionSurface>(*pianoRollPreviewSurface.get(), *pianoRollEventDispatcher.get());
    addAndMakeVisible(pianoRollInteractionSurface.get());

    const int num_visible_keys = pianoRollKeyboard->getVisibleKeySize();

    pianoRollSliderVertical = std::make_unique<juce::Slider>(juce::Slider::SliderStyle::LinearVertical, juce::Slider::TextEntryBoxPosition::NoTextBox);
    pianoRollSliderVertical->getValueObject().referTo(valuePianoRollBottomKeyNumber);
    pianoRollSliderVertical->setRange(0.0, 127.0 - num_visible_keys, 1.0);
    pianoRollSliderVertical->setColour(juce::Slider::ColourIds::trackColourId, pianoRollSliderVertical->findColour(juce::Slider::ColourIds::backgroundColourId));
    addAndMakeVisible(pianoRollSliderVertical.get());

    pianoRollScrollBarHorizontal = std::make_unique<juce::ScrollBar>(false);
    pianoRollScrollBarHorizontal->setRangeLimits(juce::Range<double>{0.0, 600.0}, juce::dontSendNotification);
    pianoRollScrollBarHorizontal->setCurrentRange(juce::Range<double>{0.0, 6.0}, juce::dontSendNotification);
    addAndMakeVisible(pianoRollScrollBarHorizontal.get());

    pianoRollTimeRuler = std::make_unique<PianoRollTimeRuler>();
    addAndMakeVisible(pianoRollTimeRuler.get());

    pianoRollScrollBarHorizontal->addListener(this);

    valuePianoRollBottomKeyNumber.addListener(this);
    
    // Set initial state.
    valuePianoRollBottomKeyNumber.setValue(55);
    pianoRollPreviewSurface->setVisibleRangeTimeInSeconds(juce::Range<double>{0.0, 6.0});
    pianoRollPreviewSurface->setGridVerticalLineIntervaleInSeconds(1.0);
    pianoRollTimeRuler->setVisibleRangeTimeInSeconds(juce::Range<double>{0.0, 6.0});
    pianoRollTimeRuler->setGridVerticalLineIntervaleInSeconds(0.500);

    startTimerHz(30);
}

SongEditor::~SongEditor()
{
    stopTimer();

    valuePianoRollBottomKeyNumber.removeListener(this);

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
void SongEditor::registerSongEditorDocument(std::shared_ptr<cctn::song::SongEditorDocument> document)
{
    if (songEditorDocumentPtr.lock().get() != document.get())
    {
        songEditorDocumentPtr = document;

        const auto preview_data_optional = songEditorDocumentPtr.lock()->getPianoRollPreviewData();
        if (preview_data_optional.has_value())
        {
            pianoRollPreviewSurface->setPianoRollPreviewData(preview_data_optional.value());
        }

        songEditorDocumentPtr.lock()->addChangeListener(this);
        songEditorEventBridge->attachDocument(songEditorDocumentPtr.lock());
    }
}

void SongEditor::unregisterSongEditorDocument(std::shared_ptr<cctn::song::SongEditorDocument> document)
{
    if (songEditorDocumentPtr.lock().get() == document.get())
    {
        songEditorEventBridge->detachDocument();
        songEditorDocumentPtr.lock()->removeChangeListener(this);

        songEditorDocumentPtr.reset();

        pianoRollPreviewSurface->setPianoRollPreviewData({});
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

    const auto piano_roll_main_top = 24;
    const auto piano_roll_main_bottom = getHeight() - 24;

    const auto rect_piano_roll_keyboard = 
        juce::Rectangle<int>{ 0, (int)piano_roll_main_top, 80, piano_roll_main_bottom  - piano_roll_main_top };
    
    const auto rect_piano_roll_preview =
        juce::Rectangle<int>{ 80, (int)piano_roll_main_top, getWidth() - 80 - 24, piano_roll_main_bottom - piano_roll_main_top};
    
    const auto rect_piano_roll_time_ruler =
        juce::Rectangle<int>{ 0, 0, getWidth(), piano_roll_main_top };
    
    const auto rect_piano_roll_scrollbar_vertical =
        juce::Rectangle<int>{ getWidth() - 24, (int)piano_roll_main_top, 24, piano_roll_main_bottom - piano_roll_main_top };
    
    const auto rect_piano_roll_scrollbar_horizontal = 
        juce::Rectangle<int>{ 80, piano_roll_main_bottom, getWidth() - 80 - 24, 24 };

    pianoRollPreviewSurface->setBounds(rect_piano_roll_preview);
    pianoRollInteractionSurface->setBounds(rect_piano_roll_preview);

    pianoRollKeyboard->setBounds(rect_piano_roll_keyboard);
    pianoRollTimeRuler->setBounds(rect_piano_roll_time_ruler);
    pianoRollSliderVertical->setBounds(rect_piano_roll_scrollbar_vertical);
    pianoRollScrollBarHorizontal->setBounds(rect_piano_roll_scrollbar_horizontal);

    {
        const auto rect_time_ruler_content =
            rect_piano_roll_time_ruler
            .withLeft(rect_piano_roll_preview.getX())
            .withRight(rect_piano_roll_preview.getRight());
        pianoRollTimeRuler->setTimeRulerRectangle(rect_time_ruler_content);
    }
}

//==============================================================================
void SongEditor::valueChanged(juce::Value& value)
{
    if (value.refersToSameSourceAs(valuePianoRollBottomKeyNumber))
    {
        pianoRollKeyboard->setVisibleBottomKeyNoteNumber((int)valuePianoRollBottomKeyNumber.getValue());

        pianoRollPreviewSurface->setVisibleBottomKeyNoteNumber((int)valuePianoRollBottomKeyNumber.getValue());
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
            pianoRollTimeRuler->setPlayingPositionInSeconds(current_position_in_seconds);
        }
    }
}

//==============================================================================
void SongEditor::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (!songEditorDocumentPtr.expired())
    {
        if (source == songEditorDocumentPtr.lock().get())
        {
            const auto preview_data_optional = songEditorDocumentPtr.lock()->getPianoRollPreviewData();
            if (preview_data_optional.has_value())
            {
                pianoRollPreviewSurface->setPianoRollPreviewData(preview_data_optional.value());
            }
        }
    }
}

}
}