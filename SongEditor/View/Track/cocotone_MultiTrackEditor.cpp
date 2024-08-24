namespace cctn
{
namespace song
{

//==============================================================================
MultiTrackEditor::MultiTrackEditor()
{
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

    // Add listener
    scrollBarHorizontal->addListener(this);
}

MultiTrackEditor::~MultiTrackEditor()
{
    // Remove listener
    scrollBarHorizontal->removeListener(this);
}

//==============================================================================
void MultiTrackEditor::setDocumentEditor(std::shared_ptr<cctn::song::SongDocumentEditor> documentEditor)
{
    std::lock_guard lock(mutex);

    if (!documentEditorForPreviewPtr.expired())
    {
        if (documentEditorForPreviewPtr.lock().get() != documentEditor.get())
        {
            documentEditorForPreviewPtr.lock()->removeChangeListener(this);
            documentEditorForPreviewPtr.reset();
        }
    }

    documentEditorForPreviewPtr = documentEditor;

    if (!documentEditorForPreviewPtr.expired())
    {
        documentEditorForPreviewPtr.lock()->addChangeListener(this);
    }

    updateContent();

    repaint();
}

//==============================================================================
void MultiTrackEditor::setPlayingPositionInSeconds(double positionInSeconds)
{
}

void MultiTrackEditor::setCurrentPositionInfo(const juce::AudioPlayHead::PositionInfo& positionInfo)
{
}

//==============================================================================
void MultiTrackEditor::paint(juce::Graphics& g)
{
    juce::Graphics::ScopedSaveState save_state(g);

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

    scrollBarHorizontal->setBounds(rect_area.removeFromBottom(height_scroll_bar).withTrimmedLeft(width_track_header));
}

//==============================================================================
void MultiTrackEditor::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (!documentEditorForPreviewPtr.expired())
    {
        if (source == documentEditorForPreviewPtr.lock().get())
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
    if (!documentEditorForPreviewPtr.expired())
    {
        return documentEditorForPreviewPtr.lock().get();
    }

    return std::nullopt;
}

std::optional<juce::Range<double>> MultiTrackEditor::getVisibleRangeInTicks()
{
    return scrollBarHorizontal->getCurrentRange();
}

//==============================================================================
void MultiTrackEditor::updateContent()
{
    if (!documentEditorForPreviewPtr.expired())
    {
        if (documentEditorForPreviewPtr.lock()->getCurrentDocument().has_value())
        {
            const auto& song_document_editor = *documentEditorForPreviewPtr.lock();
            const auto& song_document = *documentEditorForPreviewPtr.lock()->getCurrentDocument().value();

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


