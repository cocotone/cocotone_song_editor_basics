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
    void registerSongDocumentEditor(std::shared_ptr<cctn::song::SongDocumentEditor> documentEditor);
    void unregisterSongDocumentEditor(std::shared_ptr<cctn::song::SongDocumentEditor> documentEditor);

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
    static void populateComboBoxWithNoteLength(juce::ComboBox& comboBox, std::map<int, cctn::song::NoteLength>& mapIndexToNoteLength);
    static void populateComboBoxWithLyricMora(juce::ComboBox& comboBox, std::map<int, cctn::song::Mora>& mapIndexToMora);

    void initialUpdate();

    //==============================================================================
    std::unique_ptr<cctn::song::PianoRollKeyboard> pianoRollKeyboard;
    std::unique_ptr<cctn::song::PianoRollTimeRuler> pianoRollTimeRuler;
    std::unique_ptr<cctn::song::PianoRollPreviewSurface> pianoRollPreviewSurface;
    std::unique_ptr<cctn::song::PianoRollInteractionSurface> pianoRollInteractionSurface;
    std::unique_ptr<juce::Slider> pianoRollSliderVertical;
    std::unique_ptr<juce::ScrollBar> pianoRollScrollBarHorizontal;
    juce::Value valuePianoRollBottomKeyNumber;
    
    juce::Rectangle<int> rectInputOptions;
    
    std::unique_ptr<juce::Label> labelPianoRollGridInterval;
    std::unique_ptr<juce::ComboBox> comboboxPianoRollGridInterval;
    std::map<int, cctn::song::NoteLength> mapIndexToGridInterval;
    juce::Value valuePianoRollGridInterval;

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
    std::shared_ptr<cctn::song::SongEditorOperation> songEditorOperation;

    IPositionInfoProvider* positionInfoProviderPtr;

    mutable std::mutex mutex;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SongEditor)
};

}
}
