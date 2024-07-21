#pragma once

namespace cctn
{
namespace song
{

//==============================================================================
class PianoRollKeyboard;
class PianoRollTimeRuler;
class PianoRollPreviewSurface;
class PianoRollInteractionSurface;
class PianoRollEventDispatcher;
class IPositionInfoProvider;

//==============================================================================
class SongEditor final
    : public juce::Component
    , private juce::Value::Listener
    , private juce::ScrollBar::Listener
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
    void registerSongEditorDocument(std::shared_ptr<cctn::song::SongEditorDocument> document);
    void unregisterSongEditorDocument(std::shared_ptr<cctn::song::SongEditorDocument> document);

private:
    //==============================================================================
    void paint(juce::Graphics& g) override;
    void resized() override;

    // juce::Value::Listener
    void valueChanged(juce::Value& value) override;

    // juce::ScrollBar::Listener
    void scrollBarMoved(juce::ScrollBar* scrollBarThatHasMoved, double newRangeStart) override;

    // juce::Timer
    void timerCallback() override;

    // juce::ChangeListener
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    //==============================================================================
    std::unique_ptr<cctn::song::PianoRollKeyboard> pianoRollKeyboard;
    std::unique_ptr<cctn::song::PianoRollTimeRuler> pianoRollTimeRuler;
    std::unique_ptr<cctn::song::PianoRollPreviewSurface> pianoRollPreviewSurface;
    std::unique_ptr<cctn::song::PianoRollInteractionSurface> pianoRollInteractionSurface;
    std::unique_ptr<juce::Slider> pianoRollSliderVertical;
    std::unique_ptr<juce::ScrollBar> pianoRollScrollBarHorizontal;
    juce::Value valuePianoRollBottomKeyNumber;

    std::unique_ptr<cctn::song::PianoRollEventDispatcher> pianoRollEventDispatcher;

    std::weak_ptr<cctn::song::SongEditorDocument> songEditorDocumentPtr;
    std::shared_ptr<cctn::song::SongEditorOperation> songEditorOperation;

    IPositionInfoProvider* positionInfoProviderPtr;

    mutable std::mutex mutex;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SongEditor)
};

}
}
