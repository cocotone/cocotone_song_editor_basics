#pragma once

namespace cctn
{
namespace song
{

//==============================================================================
class IPositionInfoProvider;
class IAudioThumbnailProvider;
class SongDocumentEditor;
class SongEditorOperation;

namespace view 
{
class MultiTrackEditor;
class PianoRollEditor;
}

//==============================================================================
class SongEditor final
    : public juce::Component
    , private juce::Timer
    , public juce::ChangeListener
{
public:
    //==============================================================================
    explicit SongEditor();
    ~SongEditor() override;

    //==============================================================================
    void registerPositionInfoProvider(IPositionInfoProvider* provider);
    void unregisterPositionInfoProvider(IPositionInfoProvider* provider);

    //==============================================================================
    void registerAudioThumbnailProvider(IAudioThumbnailProvider* provider);
    void unregisterAudioThumbnailProvider(IAudioThumbnailProvider* provider);

    //==============================================================================
    void registerSongDocumentEditor(std::shared_ptr<cctn::song::SongDocumentEditor> documentEditor);
    void unregisterSongDocumentEditor(std::shared_ptr<cctn::song::SongDocumentEditor> documentEditor);

private:
    //==============================================================================
    void paint(juce::Graphics& g) override;
    void resized() override;

    // juce::Timer
    void timerCallback() override;

    // juce::ChangeListener
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    //==============================================================================
    void initialUpdate();

    //==============================================================================
    std::unique_ptr<cctn::song::view::MultiTrackEditor> multiTrackEditor;
    juce::Rectangle<int> rectMultiTrackEditor;

    std::unique_ptr<cctn::song::view::PianoRollEditor> pianoRollEditor;
    juce::Rectangle<int> rectPianoRollEditor;

    std::weak_ptr<cctn::song::SongDocumentEditor> songDocumentEditorPtr;
    std::shared_ptr<cctn::song::SongEditorOperation> songEditorOperation;

    IPositionInfoProvider* positionInfoProviderPtr;

    mutable std::mutex mutex;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SongEditor)
};

}
}
