#include "cocotone_SongEditor.h"


namespace cctn
{
namespace song
{

namespace
{
constexpr int kNumVisibleOctaves = 1;
}

//==============================================================================
SongEditor::SongEditor()
    : positionInfoProviderPtr(nullptr)
    , songEditorDocumentPtr({})
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
        songEditorDocumentPtr.lock()->addChangeListener(this);
        songEditorOperation->attachDocument(songEditorDocumentPtr.lock());

        pianoRollPreviewSurface->setDocumentForPreview(document);
    }
}

void SongEditor::unregisterSongEditorDocument(std::shared_ptr<cctn::song::SongEditorDocument> document)
{
    if (songEditorDocumentPtr.lock().get() == document.get())
    {
        songEditorOperation->detachDocument();
        songEditorDocumentPtr.lock()->removeChangeListener(this);

        songEditorDocumentPtr.reset();

        pianoRollPreviewSurface->setDocumentForPreview(nullptr);
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

    const auto piano_roll_main_top = 48;
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
        {
            const auto rect_beat_ruler_content =
                rect_piano_roll_time_ruler
                .withTop(rect_piano_roll_time_ruler.getHeight() * 0.5f)
                .withLeft(rect_piano_roll_preview.getX())
                .withRight(rect_piano_roll_preview.getRight());
            pianoRollTimeRuler->setBeatRulerRectangle(rect_beat_ruler_content);

            const auto rect_beat_ruler_label =
                rect_piano_roll_time_ruler
                .withTop(rect_piano_roll_time_ruler.getHeight() * 0.5f)
                .withRight(rect_piano_roll_preview.getX());
            pianoRollTimeRuler->setBeatRulerLabelRectangle(rect_beat_ruler_label);
        }

        {
            const auto rect_time_ruler_content =
                rect_piano_roll_time_ruler
                .withBottom(rect_piano_roll_time_ruler.getHeight() * 0.5f)
                .withLeft(rect_piano_roll_preview.getX())
                .withRight(rect_piano_roll_preview.getRight());
            pianoRollTimeRuler->setTimeRulerRectangle(rect_time_ruler_content);

            const auto rect_time_ruler_label =
                rect_piano_roll_time_ruler
                .withBottom(rect_piano_roll_time_ruler.getHeight() * 0.5f)
                .withRight(rect_piano_roll_preview.getX());
            pianoRollTimeRuler->setTimeRulerLabelRectangle(rect_time_ruler_label);
        }
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
            pianoRollPreviewSurface->setCurrentPositionInfo(position_info_optional.value());

            pianoRollTimeRuler->setPlayingPositionInSeconds(current_position_in_seconds);
            pianoRollTimeRuler->setCurrentPositionInfo(position_info_optional.value());

            if (!songEditorDocumentPtr.expired())
            {
                songEditorDocumentPtr.lock()->updateQuantizeRegions(position_info_optional.value());
            }
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
        }
    }
}

}
}
