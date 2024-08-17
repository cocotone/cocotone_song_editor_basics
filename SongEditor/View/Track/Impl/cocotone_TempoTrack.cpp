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
void TempoTrack::paint(juce::Graphics& g)
{
    auto randomInt = juce::Random::getSystemRandom().nextInt64();
    g.fillAll(juce::Colour(randomInt).withAlpha(1.0f));
}

void TempoTrack::resized()
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
