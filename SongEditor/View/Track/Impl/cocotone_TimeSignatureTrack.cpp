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
void TimeSignatureTrack::paint(juce::Graphics& g)
{
    auto randomInt = juce::Random::getSystemRandom().nextInt64();
    g.fillAll(juce::Colour(randomInt).withAlpha(1.0f));
}

void TimeSignatureTrack::resized()
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
