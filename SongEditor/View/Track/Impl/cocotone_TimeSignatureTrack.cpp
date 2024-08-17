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
void TimeSignatureTrack::triggerUpdateContent()
{
    repaint();
}

void TimeSignatureTrack::triggerUpdateVisibleRange()
{
}

}
}
