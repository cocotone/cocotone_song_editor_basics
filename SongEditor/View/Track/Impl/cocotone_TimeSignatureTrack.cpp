namespace cctn
{
namespace song
{

//==============================================================================
TimeSignatureTrack::TimeSignatureTrack()
{
}
TimeSignatureTrack::~TimeSignatureTrack()
{
}

//==============================================================================
void TimeSignatureTrack::handleAsyncUpdate()
{
    repaint();
}

//==============================================================================
void TimeSignatureTrack::updateContent(const cctn::song::SongDocumentEditor& songDocumentEditor)
{
    repaint();
}

}
}
