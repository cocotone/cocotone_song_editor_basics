#pragma once

namespace cctn
{
namespace song
{

//==============================================================================
class TrackComponentBase
    : public juce::Component
    , public juce::AsyncUpdater
{
public:
    //==============================================================================
    virtual ~TrackComponentBase() = default;

    //==============================================================================
    virtual void triggerUpdateContent() = 0;
    virtual void triggerUpdateVisibleRange() = 0;

    //==============================================================================
    void setHeaderRatio(float ratio)
    {
        headerRatio = ratio;
        triggerAsyncUpdate();
    }

    const float getHeaderRatio() const 
    { 
        return headerRatio;
    }

    //==============================================================================
    void registerHeaderComponent(juce::Component* header)
    {
        headerComponent = header;
    }

    void registerLaneComponent(juce::Component* lane)
    {
        laneComponent = lane;
    }

private:
    //==============================================================================
    void paint(juce::Graphics& g) override
    {
        juce::Graphics::ScopedSaveState save_state(g);
    }

    void resized() override
    {
        auto rect_area = getLocalBounds();

        const auto width_header = rect_area.getWidth() * headerRatio;

        if (headerComponent != nullptr)
        {
            headerComponent->setBounds(rect_area.removeFromLeft(width_header));
        }

        if (laneComponent != nullptr)
        {
            laneComponent->setBounds(rect_area);
        }
    }

    //==============================================================================
    juce::Range<double> currentViewRangeInTicks;
    float headerRatio{ 0.125f };

    juce::Component* headerComponent{ nullptr };
    juce::Component* laneComponent{ nullptr };

    JUCE_LEAK_DETECTOR(TrackComponentBase)
};

}
}
