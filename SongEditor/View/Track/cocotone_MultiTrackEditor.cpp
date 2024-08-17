namespace cctn
{
namespace song
{

//==============================================================================
MultiTrackEditor::MultiTrackEditor()
{
    timeSignatureTrack = std::make_unique<cctn::song::TimeSignatureTrack>();
    addAndMakeVisible(timeSignatureTrack.get());

    musicalTimePreviewTrack = std::make_unique<cctn::song::MusicalTimePreviewTrack>(*this);
    addAndMakeVisible(musicalTimePreviewTrack.get());

    tempoTrack = std::make_unique<cctn::song::TempoTrack>();
    addAndMakeVisible(tempoTrack.get());

    absoluteTimePreviewTrack = std::make_unique<cctn::song::AbsoluteTimePreviewTrack>(*this);
    addAndMakeVisible(absoluteTimePreviewTrack.get());

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

    auto rect_area = getLocalBounds();
    auto randomInt = juce::Random::getSystemRandom().nextInt64();
    g.setColour(juce::Colour(randomInt).withAlpha(1.0f));
    g.drawRect(rect_area, 2);
}

void MultiTrackEditor::resized()
{
    auto rect_area = getLocalBounds();

    const auto ratio_track_header = 0.125f;
    const auto width_track_header = rect_area.getWidth() * ratio_track_header;

    timeSignatureTrack->setHeaderRatio(ratio_track_header);
    timeSignatureTrack->setBounds(rect_area.removeFromTop(56));

    musicalTimePreviewTrack->setHeaderRatio(ratio_track_header);
    musicalTimePreviewTrack->setBounds(rect_area.removeFromTop(40));
    
    tempoTrack->setHeaderRatio(ratio_track_header);
    tempoTrack->setBounds(rect_area.removeFromTop(56));
    
    absoluteTimePreviewTrack->setHeaderRatio(ratio_track_header);
    absoluteTimePreviewTrack->setBounds(rect_area.removeFromTop(40));

    scrollBarHorizontal->setBounds(rect_area.removeFromBottom(20).withTrimmedLeft(width_track_header));
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
    musicalTimePreviewTrack->triggerUpdateVisibleRange();
    absoluteTimePreviewTrack->triggerUpdateVisibleRange();
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

            musicalTimePreviewTrack->triggerUpdateContent();
            absoluteTimePreviewTrack->triggerUpdateContent();

            musicalTimePreviewTrack->triggerUpdateVisibleRange();
            absoluteTimePreviewTrack->triggerUpdateVisibleRange();
        }
    }
}

}
}


