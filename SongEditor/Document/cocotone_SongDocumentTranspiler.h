#pragma once

namespace cctn
{
namespace song
{

//==============================================================================
template <typename ArtefactDataType>
class ITranspileTarget
{
public:
    //==============================================================================
    virtual ~ITranspileTarget() {}
    virtual ArtefactDataType transpile(const cctn::song::SongDocument& sourceDocument) = 0;

private:
    //==============================================================================
    JUCE_LEAK_DETECTOR(ITranspileTarget<ArtefactDataType>)
};

//==============================================================================
class SongDocumentTranspiler
{
public:
    //==============================================================================

private:
    //==============================================================================
    SongDocumentTranspiler() = delete;
    ~SongDocumentTranspiler() = delete;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SongDocumentTranspiler)
};

}  // namespace song
}  // namespace cctn
