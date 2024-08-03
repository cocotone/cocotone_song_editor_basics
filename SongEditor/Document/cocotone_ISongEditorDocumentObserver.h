#pragma once

namespace cctn
{
namespace song
{

//==============================================================================
class ISongEditorDocumentObserver
{
public:
    //==============================================================================
    virtual ~ISongEditorDocumentObserver() = default;

    //==============================================================================
    virtual void onDocumentDataChanged(const cctn::song::SongEditorDocumentData& data) = 0;

private:

    JUCE_LEAK_DETECTOR(ISongEditorDocumentObserver)
};

}
}
