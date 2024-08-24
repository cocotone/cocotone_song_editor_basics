namespace cctn
{
namespace song
{

//==============================================================================
QuantizeEngine::QuantizeEngine()
{
}

QuantizeEngine::~QuantizeEngine()
{
}

//==============================================================================
void QuantizeEngine::updateQuantizeRegions(const cctn::song::SongDocument::BeatTimePoints& beatTimePoints)
{
    quantizeRegions.clear();

    if (beatTimePoints.size() < 2)
    {
        return;
    }

    for (size_t i = 0; i < beatTimePoints.size() - 1; ++i)
    {
        cctn::song::SongDocument::RegionWithBeatInfo region;
        region.startPositionInSeconds = beatTimePoints[i].absoluteTimeInSeconds;
        region.endPositionInSeconds = beatTimePoints[i + 1].absoluteTimeInSeconds;
        region.startMusicalTime = beatTimePoints[i].musicalTime;
        quantizeRegions.add(region);
    }
}

std::optional<cctn::song::SongDocument::RegionWithBeatInfo> QuantizeEngine::findNearestQuantizeRegion(double timePositionInSeconds) const
{
    if (quantizeRegions.isEmpty() || timePositionInSeconds < 0.0)
    {
        return std::nullopt;
    }

    // Find the first region whose end is greater than the given time
    auto it = std::lower_bound(quantizeRegions.begin(), quantizeRegions.end(), timePositionInSeconds,
        [](const cctn::song::SongDocument::RegionWithBeatInfo& region, double time) {
            return region.endPositionInSeconds <= time;
        });

    if (it == quantizeRegions.begin())
    {
        // If the time is before or at the start of the first region
        return quantizeRegions.getFirst();
    }
    else if (it == quantizeRegions.end())
    {
        // If the time is after the end of the last region
        return quantizeRegions.getLast();
    }
    else
    {
        // Return the region that contains the time or the previous one
        return (timePositionInSeconds < it->startPositionInSeconds) ? *(--it) : *it;
    }
}

}
}
