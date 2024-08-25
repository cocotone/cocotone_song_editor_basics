namespace cctn
{
namespace song
{
namespace view
{

//==============================================================================
MultiTrackEditor::MultiTrackEditor()
    : playingPositionInSeconds(0.0)
    , playingPositionInTicks(0)
{
    songEditorOperationApi = std::make_shared<cctn::song::SongEditorOperation>();

    timeSignatureTrack = std::make_unique<cctn::song::TimeSignatureTrack>(*this);
    addAndMakeVisible(timeSignatureTrack.get());

    musicalTimePreviewTrack = std::make_unique<cctn::song::MusicalTimePreviewTrack>(*this);
    addAndMakeVisible(musicalTimePreviewTrack.get());

    tempoTrack = std::make_unique<cctn::song::TempoTrack>(*this);
    addAndMakeVisible(tempoTrack.get());

    absoluteTimePreviewTrack = std::make_unique<cctn::song::AbsoluteTimePreviewTrack>(*this);
    addAndMakeVisible(absoluteTimePreviewTrack.get());

    vocalTrack = std::make_unique<cctn::song::VocalTrack>(*this);
    addAndMakeVisible(vocalTrack.get());

    scrollBarHorizontal = std::make_unique<juce::ScrollBar>(false);
    scrollBarHorizontal->setRangeLimits(juce::Range<double>{0.0, 600.0}, juce::dontSendNotification);
    scrollBarHorizontal->setCurrentRange(juce::Range<double>{0.0, 6.0}, juce::dontSendNotification);
    addAndMakeVisible(scrollBarHorizontal.get());

    buttonFollowPlayingPosition = std::make_unique<juce::ToggleButton>();
    buttonFollowPlayingPosition->setButtonText("Follow Position");
    buttonFollowPlayingPosition->onStateChange =
        [safe_this = juce::Component::SafePointer(this)]() {
        if (safe_this.getComponent() == nullptr)
        {
            return;
        }

        safe_this->valueFollowPlayingPosition = (bool)safe_this->buttonFollowPlayingPosition->getToggleState();
        };
    addAndMakeVisible(buttonFollowPlayingPosition.get());

    // Add listener
    scrollBarHorizontal->addListener(this);
    valueFollowPlayingPosition.addListener(this);

    // Initial update
    initialUpdate();
}

MultiTrackEditor::~MultiTrackEditor()
{
    // Remove listener
    scrollBarHorizontal->removeListener(this);
    valueFollowPlayingPosition.removeListener(this);
}

//==============================================================================
void MultiTrackEditor::setDocumentEditor(std::shared_ptr<cctn::song::SongDocumentEditor> documentEditor)
{
    std::lock_guard lock(mutex);

    if (!songDocumentEditorPtr.expired())
    {
        if (songDocumentEditorPtr.lock().get() != documentEditor.get())
        {
            songDocumentEditorPtr.lock()->removeChangeListener(this);
            songDocumentEditorPtr.reset();
        }
    }

    songDocumentEditorPtr = documentEditor;

    songEditorOperationApi->attachDocument(documentEditor);

    if (!songDocumentEditorPtr.expired())
    {
        songDocumentEditorPtr.lock()->addChangeListener(this);
    }

    updateContent();

    repaint();
}

//==============================================================================
void MultiTrackEditor::setPlayingPositionInSeconds(double positionInSeconds)
{
    playingPositionInSeconds = positionInSeconds;
    repaint();
}

void MultiTrackEditor::setCurrentPositionInfo(const juce::AudioPlayHead::PositionInfo&)
{
}

//==============================================================================
void MultiTrackEditor::paint(juce::Graphics& g)
{
    juce::Graphics::ScopedSaveState save_state(g);

    const cctn::song::SongDocument* document_to_paint = nullptr;
    if (!songDocumentEditorPtr.expired() &&
        songDocumentEditorPtr.lock()->getCurrentDocument().has_value())
    {
        document_to_paint = songDocumentEditorPtr.lock()->getCurrentDocument().value();
    }

    juce::ScopedValueSetter<const cctn::song::SongDocument*> svs_1(scopedSongDocumentPtrToPaint, document_to_paint);

    updateViewContext();

    g.fillAll(kColourMainBackground);
}

void MultiTrackEditor::paintOverChildren(juce::Graphics& g)
{
    juce::Graphics::ScopedSaveState save_state(g);

    g.setColour(juce::Colours::darkgrey);
    g.drawRect(timeSignatureTrack->getBounds(), 1);
    g.drawRect(musicalTimePreviewTrack->getBounds(), 1);
    g.drawRect(tempoTrack->getBounds(), 1);
    g.drawRect(absoluteTimePreviewTrack->getBounds(), 1);
    g.drawRect(vocalTrack->getBounds(), 1);

    drawPlayingPositionMarker(g);

    g.setColour(juce::Colours::grey);
    g.drawRect(getLocalBounds(), 2);
}

void MultiTrackEditor::resized()
{
    auto rect_area = getLocalBounds();

    const auto ratio_track_header = 0.125f;
    const auto width_track_header = rect_area.getWidth() * ratio_track_header;

    const auto height_scroll_bar = 20;
    const auto height_track = (rect_area.getHeight() - height_scroll_bar) / 5;

    timeSignatureTrack->setHeaderRatio(ratio_track_header);
    timeSignatureTrack->setBounds(rect_area.removeFromTop(height_track));

    musicalTimePreviewTrack->setHeaderRatio(ratio_track_header);
    musicalTimePreviewTrack->setBounds(rect_area.removeFromTop(height_track));

    tempoTrack->setHeaderRatio(ratio_track_header);
    tempoTrack->setBounds(rect_area.removeFromTop(height_track));

    absoluteTimePreviewTrack->setHeaderRatio(ratio_track_header);
    absoluteTimePreviewTrack->setBounds(rect_area.removeFromTop(height_track));

    vocalTrack->setHeaderRatio(ratio_track_header);
    vocalTrack->setBounds(rect_area.removeFromTop(height_track));

    auto rect_footer = rect_area.removeFromBottom(height_scroll_bar).withTrimmedBottom(2);
    buttonFollowPlayingPosition->setBounds(rect_footer.removeFromLeft(width_track_header));
    scrollBarHorizontal->setBounds(rect_footer);

    rectTrackLane = juce::Rectangle<int>{
        (int)width_track_header,
        0,
        getLocalBounds().getWidth() - (int)width_track_header,
        getLocalBounds().getHeight() - (int)height_scroll_bar
    };
}

//==============================================================================
void MultiTrackEditor::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (!songDocumentEditorPtr.expired())
    {
        if (source == songDocumentEditorPtr.lock().get())
        {
            updateContent();

            repaint();
        }
    }
}

//==============================================================================
void MultiTrackEditor::scrollBarMoved(juce::ScrollBar* scrollBarThatHasMoved, double newRangeStart)
{
    timeSignatureTrack->triggerUpdateVisibleRange();
    musicalTimePreviewTrack->triggerUpdateVisibleRange();
    tempoTrack->triggerUpdateVisibleRange();
    absoluteTimePreviewTrack->triggerUpdateVisibleRange();
    vocalTrack->triggerUpdateVisibleRange();
}

//==============================================================================
void MultiTrackEditor::valueChanged(juce::Value& value)
{
}

//==============================================================================
std::optional<cctn::song::SongDocumentEditor*> MultiTrackEditor::getSongDocumentEditor()
{
    if (!songDocumentEditorPtr.expired())
    {
        return songDocumentEditorPtr.lock().get();
    }

    return std::nullopt;
}

std::optional<juce::Range<double>> MultiTrackEditor::getVisibleRangeInTicks()
{
    return scrollBarHorizontal->getCurrentRange();
}

//==============================================================================
void MultiTrackEditor::initialUpdate()
{
    valueFollowPlayingPosition = (bool)buttonFollowPlayingPosition->getToggleState();
}

//==============================================================================
void MultiTrackEditor::updateViewContext()
{
    JUCE_ASSERT_MESSAGE_MANAGER_EXISTS;

    if (scopedSongDocumentPtrToPaint == nullptr)
    {
        return;
    }

    // Update playing position in ticks
    {
        const cctn::song::SongDocument& document = *scopedSongDocumentPtrToPaint;
        playingPositionInTicks = cctn::song::SongDocument::Calculator::absoluteTimeToTick(document, playingPositionInSeconds);
    }

    if ((bool)valueFollowPlayingPosition.getValue())
    {
        if (!scrollBarHorizontal->getCurrentRange().contains(playingPositionInTicks))
        {
            auto new_range = scrollBarHorizontal->getCurrentRange().movedToStartAt(playingPositionInTicks);
            scrollBarHorizontal->setCurrentRange(new_range);
        }
    }
}

void MultiTrackEditor::drawPlayingPositionMarker(juce::Graphics& g)
{
    juce::Graphics::ScopedSaveState save_state(g);

    const auto range_visible_time_in_ticks = scrollBarHorizontal->getCurrentRange();
    const auto position_x = ticksToPositionX(playingPositionInTicks, range_visible_time_in_ticks, rectTrackLane.getX(), rectTrackLane.getRight());

    juce::Rectangle<int> rect_marker =
        juce::Rectangle<int>{ position_x, 0, 2, getHeight() };

    // Set clipping mask
    g.reduceClipRegion(rectTrackLane);

    g.setColour(juce::Colours::yellow);
    g.fillRect(rect_marker);
}

//==============================================================================
void MultiTrackEditor::updateContent()
{
    if (!songDocumentEditorPtr.expired())
    {
        if (songDocumentEditorPtr.lock()->getCurrentDocument().has_value())
        {
            const auto& song_document_editor = *songDocumentEditorPtr.lock();
            const auto& song_document = *songDocumentEditorPtr.lock()->getCurrentDocument().value();

            const auto ticks_tail = song_document.getTotalLengthInTicks();
            const auto ticks_per_4bars = song_document.getTicksPerQuarterNote() * 4 * 4;
            const auto current_range = scrollBarHorizontal->getCurrentRange();

            scrollBarHorizontal->setRangeLimits(juce::Range<double>{0.0, (double)ticks_tail}, juce::dontSendNotification);
            scrollBarHorizontal->setCurrentRange(current_range.withLength(ticks_per_4bars), juce::dontSendNotification);

            timeSignatureTrack->triggerUpdateContent();
            musicalTimePreviewTrack->triggerUpdateContent();
            tempoTrack->triggerUpdateContent();
            absoluteTimePreviewTrack->triggerUpdateContent();
            vocalTrack->triggerUpdateContent();

            timeSignatureTrack->triggerUpdateVisibleRange();
            musicalTimePreviewTrack->triggerUpdateVisibleRange();
            tempoTrack->triggerUpdateVisibleRange();
            absoluteTimePreviewTrack->triggerUpdateVisibleRange();
            vocalTrack->triggerUpdateVisibleRange();
        }
    }
}

}
}
}
