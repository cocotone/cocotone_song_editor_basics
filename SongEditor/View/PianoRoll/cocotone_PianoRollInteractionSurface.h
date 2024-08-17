#pragma once

namespace cctn
{
namespace song
{

//==============================================================================
class PianoRollInteractionSurface final
    : public juce::Component
{
public:
    //==============================================================================
    explicit PianoRollInteractionSurface(cctn::song::PianoRollPreviewSurface& previewSurface, cctn::song::PianoRollEventDispatcher& eventDispatcher);
    ~PianoRollInteractionSurface() override;

private:
    //==============================================================================
    void paint(juce::Graphics& g) override;
    void resized() override;

    //==============================================================================
    // juce::MouseListener
    void mouseMove(const juce::MouseEvent& mouseEvent) override;
    void mouseEnter(const juce::MouseEvent& mouseEvent) override;
    void mouseExit(const juce::MouseEvent& mouseEvent) override;
    void mouseDown(const juce::MouseEvent& mouseEvent) override;
    void mouseDrag(const juce::MouseEvent& mouseEvent) override;
    void mouseUp(const juce::MouseEvent& mouseEvent) override;

    //==============================================================================
    cctn::song::PianoRollPreviewSurface& pianoRollPreviewSurface;
    cctn::song::PianoRollEventDispatcher& pianoRollEventDispatcher;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PianoRollInteractionSurface)
};

}
}
