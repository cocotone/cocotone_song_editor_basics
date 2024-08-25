namespace cctn
{
namespace song
{

//==============================================================================
SongEditor::SongEditor()
    : positionInfoProviderPtr(nullptr)
    , songDocumentEditorPtr({})
{
    songEditorOperation = std::make_shared<cctn::song::SongEditorOperation>();

    multiTrackEditor = std::make_unique<cctn::song::view::MultiTrackEditor>();
    addAndMakeVisible(multiTrackEditor.get());

    pianoRollEditor = std::make_unique<cctn::song::view::PianoRollEditor>();
    addAndMakeVisible(pianoRollEditor.get());

    // Set initial state.

    // Trigger Initial Update.
    initialUpdate();

    startTimerHz(30);
}

SongEditor::~SongEditor()
{
    stopTimer();

    pianoRollEditor.reset();
    multiTrackEditor.reset();
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
void SongEditor::registerAudioThumbnailProvider(IAudioThumbnailProvider* provider)
{
    // Bypass function
    pianoRollEditor->registerAudioThumbnailProvider(provider);
}

void SongEditor::unregisterAudioThumbnailProvider(IAudioThumbnailProvider* provider)
{
    // Bypass function
    pianoRollEditor->unregisterAudioThumbnailProvider(provider);
}

//==============================================================================
void SongEditor::registerSongDocumentEditor(std::shared_ptr<cctn::song::SongDocumentEditor> documentEditor)
{
    if (songDocumentEditorPtr.lock().get() != documentEditor.get())
    {
        songDocumentEditorPtr = documentEditor;
        songDocumentEditorPtr.lock()->addChangeListener(this);
        songEditorOperation->attachDocument(songDocumentEditorPtr.lock());

        pianoRollEditor->registerSongDocumentEditor(documentEditor);
        multiTrackEditor->setDocumentEditor(documentEditor);
    }
}

void SongEditor::unregisterSongDocumentEditor(std::shared_ptr<cctn::song::SongDocumentEditor> documentEditor)
{
    if (songDocumentEditorPtr.lock().get() == documentEditor.get())
    {
        songEditorOperation->detachDocument();
        songDocumentEditorPtr.lock()->removeChangeListener(this);

        songDocumentEditorPtr.reset();

        pianoRollEditor->unregisterSongDocumentEditor(documentEditor);
        multiTrackEditor->setDocumentEditor(nullptr);
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

    rectMultiTrackEditor = rect_area.removeFromTop(240);
    multiTrackEditor->setBounds(rectMultiTrackEditor.reduced(4));

    rect_area.removeFromTop(2);

    rectPianoRollEditor = rect_area;
    pianoRollEditor->setBounds(rectPianoRollEditor.reduced(4));
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
            
            multiTrackEditor->setPlayingPositionInSeconds(current_position_in_seconds);
            multiTrackEditor->setCurrentPositionInfo(position_info_optional.value());

            pianoRollEditor->setPlayingPositionInSeconds(current_position_in_seconds);
            pianoRollEditor->setCurrentPositionInfo(position_info_optional.value());
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
void SongEditor::initialUpdate()
{
}

}
}
