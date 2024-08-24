#pragma once

namespace cctn
{
namespace song
{

//==============================================================================
class MultiTrackEditor final
    : public juce::Component
    , private juce::ChangeListener
    , private juce::ScrollBar::Listener
    , private juce::Value::Listener
    , public cctn::song::ITrackDataAccessDelegate
{
public:
    //==============================================================================
    MultiTrackEditor();
    ~MultiTrackEditor() override;

    //==============================================================================
    void setDocumentEditor(std::shared_ptr<cctn::song::SongDocumentEditor> documentEditor);

    //==============================================================================
    void setPlayingPositionInSeconds(double positionInSeconds);
    void setCurrentPositionInfo(const juce::AudioPlayHead::PositionInfo& positionInfo);

private:
    //==============================================================================
    void paint(juce::Graphics& g) override;
    void paintOverChildren(juce::Graphics& g) override;
    void resized() override;

    // juce::ChangeListener
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    // juce::ScrollBar::Listener
    void scrollBarMoved(juce::ScrollBar* scrollBarThatHasMoved, double newRangeStart) override;

    // juce::Value::Listener
    void valueChanged(juce::Value& value) override;

    // cctn::song::ITrackDataAccessDelegate
    std::optional<cctn::song::SongDocumentEditor*> getSongDocumentEditor() override;
    std::optional<juce::Range<double>> getVisibleRangeInTicks() override;

    //==============================================================================
    void updateContent();

    //==============================================================================
    std::weak_ptr<cctn::song::SongDocumentEditor> documentEditorForPreviewPtr;
    const cctn::song::SongDocument* scopedSongDocumentPtrToPaint;
    cctn::song::SongDocument::BeatTimePoints currentBeatTimePoints{};

    std::unique_ptr<juce::ScrollBar> scrollBarHorizontal;

    std::unique_ptr<cctn::song::TrackComponentBase> timeSignatureTrack;
    std::unique_ptr<cctn::song::TrackComponentBase> musicalTimePreviewTrack;
    std::unique_ptr<cctn::song::TrackComponentBase> tempoTrack;
    std::unique_ptr<cctn::song::TrackComponentBase> absoluteTimePreviewTrack;
    std::unique_ptr<cctn::song::TrackComponentBase> vocalTrack;

    juce::Value valueMusicalTimePreviewGridSize;

    mutable std::mutex mutex;

    JUCE_LEAK_DETECTOR(MultiTrackEditor)
};

}
}
