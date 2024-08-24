#pragma once

namespace cctn
{
namespace song
{

//==============================================================================
class IAudioThumbnailProvider
{
public:
    //==============================================================================
    virtual ~IAudioThumbnailProvider() = default;

    //==============================================================================
    virtual std::optional<juce::AudioThumbnail*> getAudioThumbnail() = 0;

private:
    //==============================================================================

    JUCE_LEAK_DETECTOR(IAudioThumbnailProvider)
};

}
}
