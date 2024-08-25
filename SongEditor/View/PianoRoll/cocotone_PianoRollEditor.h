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
class IAudioThumbnailProvider;
class SongDocumentEditor;
class SongEditorOperation;

namespace view
{

//==============================================================================
class PianoRollEditor final
    : public juce::Component
    , private juce::Value::Listener
    , private juce::ScrollBar::Listener
    , public juce::ChangeListener
{
public:
    //==============================================================================
    explicit PianoRollEditor();
    ~PianoRollEditor() override;

    //==============================================================================
    void registerAudioThumbnailProvider(IAudioThumbnailProvider* provider);
    void unregisterAudioThumbnailProvider(IAudioThumbnailProvider* provider);

    //==============================================================================
    void registerSongDocumentEditor(std::shared_ptr<cctn::song::SongDocumentEditor> documentEditor);
    void unregisterSongDocumentEditor(std::shared_ptr<cctn::song::SongDocumentEditor> documentEditor);

    //==============================================================================
    void setPlayingPositionInSeconds(double positionInSeconds);
    void setCurrentPositionInfo(const juce::AudioPlayHead::PositionInfo& positionInfo);

private:
    //==============================================================================
    void paint(juce::Graphics& g) override;
    void paintOverChildren(juce::Graphics& g) override;
    void resized() override;

    // juce::Value::Listener
    void valueChanged(juce::Value& value) override;

    // juce::ScrollBar::Listener
    void scrollBarMoved(juce::ScrollBar* scrollBarThatHasMoved, double newRangeStart) override;

    // juce::ChangeListener
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    //==============================================================================
    static void populateComboBoxWithGridSize(juce::ComboBox& comboBox, std::map<int, cctn::song::NoteLength>& mapIndexToGridSize);
    static void populateComboBoxWithNoteLength(juce::ComboBox& comboBox, std::map<int, cctn::song::NoteLength>& mapIndexToNoteLength);
    static void populateComboBoxWithLyricMora(juce::ComboBox& comboBox, std::map<int, cctn::song::Mora>& mapIndexToMora);

    //==============================================================================
    void initialUpdate();

    //==============================================================================
    std::unique_ptr<cctn::song::PianoRollKeyboard> pianoRollKeyboard;
    std::unique_ptr<cctn::song::PianoRollTimeRuler> pianoRollTimeRuler;
    std::unique_ptr<cctn::song::PianoRollPreviewSurface> pianoRollPreviewSurface;
    std::unique_ptr<cctn::song::PianoRollInteractionSurface> pianoRollInteractionSurface;
    juce::Value valuePianoRollBottomKeyNumber;

    std::unique_ptr<juce::Slider> pianoRollSliderVertical;
    std::unique_ptr<juce::ScrollBar> pianoRollScrollBarHorizontal;
    std::unique_ptr<juce::ToggleButton> buttonFollowPlayingPosition;
    juce::Value valueFollowPlayingPosition;

    juce::Rectangle<int> rectInputOptions;

    std::unique_ptr<juce::Label> labelPianoRollGridSize;
    std::unique_ptr<juce::ComboBox> comboboxPianoRollGridSize;
    std::map<int, cctn::song::NoteLength> mapIndexToGridSize;
    juce::Value valuePianoRollGridSize;

    std::unique_ptr<juce::Label> labelInputNoteLength;
    std::unique_ptr<juce::ComboBox> comboboxInputNoteLength;
    std::map<int, cctn::song::NoteLength> mapIndexToNoteLength;
    juce::Value valuePianoRollInputNoteLength;

    std::unique_ptr<juce::Label> labelInputMora;
    std::unique_ptr<juce::ComboBox> comboboxInputMora;
    std::map<int, cctn::song::Mora> mapIndexToMora;
    juce::Value valuePianoRollInputMora;

    std::unique_ptr<cctn::song::PianoRollEventDispatcher> pianoRollEventDispatcher;

    std::weak_ptr<cctn::song::SongDocumentEditor> songDocumentEditorPtr;
    std::shared_ptr<cctn::song::SongEditorOperation> songEditorOperationApi;

    mutable std::mutex mutex;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PianoRollEditor)
};

}
}
}
