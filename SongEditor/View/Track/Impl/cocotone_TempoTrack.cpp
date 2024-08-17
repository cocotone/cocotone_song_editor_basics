namespace cctn
{
namespace song
{

//==============================================================================
TempoTrack::TempoTrack()
{
}

TempoTrack::~TempoTrack()
{
}

//==============================================================================
void TempoTrack::handleAsyncUpdate()
{
    repaint();
}

//==============================================================================
void TempoTrack::updateContent(const cctn::song::SongDocumentEditor& songDocumentEditor)
{
    repaint();
}

}
}
