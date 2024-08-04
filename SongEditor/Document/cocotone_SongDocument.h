#pragma once

namespace cctn
{
namespace song
{

class SongDocument
{
public:
    struct TimeSignature
    {
        int numerator;
        int denominator;
    };

    struct MusicalTime
    {
        int bar;
        int beat;
        int tick;
    };

    struct TempoMapEntry
    {
        MusicalTime time;
        TimeSignature timeSignature;
        double tempo;
    };

    struct TempoChange
    {
        MusicalTime time;
        double newTempo;
    };

    struct NoteDuration
    {
        int bars;
        int beats;
        int ticks;
    };

    struct Note
    {
        int id;
        MusicalTime startTime;
        NoteDuration duration;
        int noteNumber;
        int velocity;
        juce::String lyric;
    };

    SongDocument() : ticksPerQuarterNote(480) {}

    void setMetadata(const juce::String& title, const juce::String& artist)
    {
        metadata.title = title;
        metadata.artist = artist;
        metadata.created = juce::Time::getCurrentTime();
        metadata.lastModified = metadata.created;
    }

    void addTempoMapEntry(const TempoMapEntry& entry)
    {
        tempoMap.add(entry);
    }

    void addTempoChange(const TempoChange& change)
    {
        tempoChanges.add(change);
    }

    void addNote(const Note& note)
    {
        notes.add(note);
    }

    // Getters
    const juce::String& getTitle() const { return metadata.title; }
    const juce::String& getArtist() const { return metadata.artist; }
    juce::Time getCreationTime() const { return metadata.created; }
    juce::Time getLastModifiedTime() const { return metadata.lastModified; }
    int getTicksPerQuarterNote() const { return ticksPerQuarterNote; }

    const juce::Array<TempoMapEntry>& getTempoMap() const { return tempoMap; }
    const juce::Array<TempoChange>& getTempoChanges() const { return tempoChanges; }
    const juce::Array<Note>& getNotes() const { return notes; }

    // Helper methods
    double tickToAbsoluteTime(int64_t targetTick) const
    {
        // Implementation of tick to absolute time conversion
        // This would be a complex method considering tempo changes
        // For brevity, we're not including the full implementation here
        return 0.0; // Placeholder
    }

private:
    struct Metadata
    {
        juce::String title;
        juce::String artist;
        juce::Time created;
        juce::Time lastModified;
    };

    Metadata metadata;
    int ticksPerQuarterNote;
    juce::Array<TempoMapEntry> tempoMap;
    juce::Array<TempoChange> tempoChanges;
    juce::Array<Note> notes;
};

}  // namespace song
}  // namespace cctn
