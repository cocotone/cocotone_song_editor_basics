namespace cctn
{
namespace song
{

//==============================================================================
AbsoluteTimePreviewTrack::AbsoluteTimePreviewTrack()
{
}

AbsoluteTimePreviewTrack::~AbsoluteTimePreviewTrack()
{
}

//==============================================================================
void AbsoluteTimePreviewTrack::paint(juce::Graphics& g)
{
    auto randomInt = juce::Random::getSystemRandom().nextInt64();
    g.fillAll(juce::Colour(randomInt).withAlpha(1.0f));
}

void AbsoluteTimePreviewTrack::resized()
{
}

//==============================================================================
void AbsoluteTimePreviewTrack::handleAsyncUpdate()
{
    repaint();
}

//==============================================================================
void AbsoluteTimePreviewTrack::updateContent(const cctn::song::SongDocumentEditor& songDocumentEditor)
{
    repaint();
}

}
}
