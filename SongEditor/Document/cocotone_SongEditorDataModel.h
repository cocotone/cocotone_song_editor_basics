#pragma once

namespace cctn
{
namespace song
{

class SongEditorDataModel
{
public:
    SongEditorDataModel();
    ~SongEditorDataModel();

private:

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SongEditorDataModel)
};

#if 0
class TempoMap {
public:
    void addTempoChange(double timeInSeconds, double newBPM);
    double getBPMAtTime(double timeInSeconds) const;
    // Other necessary methods
};

class TimeSignatureMap {
public:
    void addTimeSignatureChange(double timeInSeconds, int numerator, int denominator);
    std::pair<int, int> getTimeSignatureAtTime(double timeInSeconds) const;
    // Other necessary methods
};

class BeatPointList {
public:
    void updateBeatPoints(const TempoMap& tempoMap, const TimeSignatureMap& timeSignatureMap);
    double getTimeForBeat(double beat) const;
    double getBeatForTime(double timeInSeconds) const;
    // Other necessary methods
};

class Note {
public:
    Note(double beatPosition, double timeInSeconds, double duration);
    void updateTime(const BeatPointList& beatPoints);
    // Getters and setters
private:
    double m_beatPosition;
    double m_timeInSeconds;
    double m_duration;
};

class QuantizeEngine {
public:
    void updateTempoMap(const TempoMap& newTempoMap);
    void updateTimeSignatureMap(const TimeSignatureMap& newTimeSignatureMap);
    void addNote(const Note& note);
    void quantizeNotes(NoteLength quantizeLevel);
    void updateAllNotes();
    // Other necessary methods
private:
    TempoMap m_tempoMap;
    TimeSignatureMap m_timeSignatureMap;
    BeatPointList m_beatPointList;
    std::vector<Note> m_notes;
};
#endif 

}
}
