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
void TempoTrack::triggerUpdateContent()
{
    repaint();
}

void TempoTrack::triggerUpdateVisibleRange()
{
}

}
}
