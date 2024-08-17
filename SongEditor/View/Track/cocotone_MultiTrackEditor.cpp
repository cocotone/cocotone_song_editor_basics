namespace cctn
{
namespace song
{

//==============================================================================
MultiTrackEditor::MultiTrackEditor()
{
    timeSignatureTrack = std::make_unique<cctn::song::TimeSignatureTrack>();
    addAndMakeVisible(timeSignatureTrack.get());

    musicalTimePreviewTrack = std::make_unique<cctn::song::MusicalTimePreviewTrack>();
    addAndMakeVisible(musicalTimePreviewTrack.get());

    tempoTrack = std::make_unique<cctn::song::TempoTrack>();
    addAndMakeVisible(tempoTrack.get());

    absoluteTimePreviewTrack = std::make_unique<cctn::song::AbsoluteTimePreviewTrack>();
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
    auto randomInt = juce::Random::getSystemRandom().nextInt64();
    g.fillAll(juce::Colour(randomInt).withAlpha(1.0f));
}

void MultiTrackEditor::resized()
{
    auto rect_area = getLocalBounds();

    scrollBarHorizontal->setBounds(rect_area.removeFromBottom(20));

    timeSignatureTrack->setBounds(rect_area.removeFromTop(20));
    musicalTimePreviewTrack->setBounds(rect_area.removeFromTop(20));
    tempoTrack->setBounds(rect_area.removeFromTop(20));
    absoluteTimePreviewTrack->setBounds(rect_area.removeFromTop(20));
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
    musicalTimePreviewTrack->setViewRangeInTicks(scrollBarHorizontal->getCurrentRange());
    absoluteTimePreviewTrack->setViewRangeInTicks(scrollBarHorizontal->getCurrentRange());
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

            musicalTimePreviewTrack->setViewRangeInTicks(scrollBarHorizontal->getCurrentRange());
            musicalTimePreviewTrack->updateContent(song_document_editor);

            absoluteTimePreviewTrack->setViewRangeInTicks(scrollBarHorizontal->getCurrentRange());
            absoluteTimePreviewTrack->updateContent(song_document_editor);
        }
    }
}

}
}


