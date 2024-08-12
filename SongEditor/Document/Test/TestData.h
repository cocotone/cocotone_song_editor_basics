#pragma once

#include "../cocotone_SongDocument.h"
#include <juce_core/juce_core.h>

namespace cctn
{
namespace song
{

static SongDocument createTestSongDocument()
{
    SongDocument doc;

    // Set metadata
    doc.setMetadata(
        juce::CharPointer_UTF8("\xe3\x82\xb5\xe3\x83\xb3\xe3\x83\x97\xe3\x83\xab \xe3\x82\xbd\xe3\x83\xb3\xe3\x82\xb0"),
        juce::CharPointer_UTF8("\xe3\x82\xb5\xe3\x83\xb3\xe3\x83\x97\xe3\x83\xab \xe3\x82\xa2\xe3\x83\xbc\xe3\x83\x86\xe3\x82\xa3\xe3\x82\xb9\xe3\x83\x88")
    );

    // Add tempo events
    int64_t ticks_accumulated = 0;
    int64_t ticks_per_bar = 0;

    // Start: 4/4 time signature, 120 BPM
    doc.addTempoEvent(0, cctn::song::SongDocument::TempoEvent::TempoEventType::kBoth, 120.0, 4, 4);

    // ticks per bar: 1920
    ticks_per_bar = 1920;
    ticks_accumulated += ticks_per_bar * 1;
    doc.addTempoEvent(ticks_accumulated, cctn::song::SongDocument::TempoEvent::TempoEventType::kBoth, 160.0, 4, 4);

    ticks_accumulated += ticks_per_bar * 1;
    doc.addTempoEvent(ticks_accumulated, cctn::song::SongDocument::TempoEvent::TempoEventType::kBoth, 120.0, 4, 4);

    // Bar 3: Change tempo to 140 BPM
    ticks_accumulated += ticks_per_bar * 1;
    doc.addTempoEvent(ticks_accumulated, cctn::song::SongDocument::TempoEvent::TempoEventType::kTempo, 140.0);

    // Bar 5: Change to 3/4 time signature
    ticks_accumulated += ticks_per_bar * 1;
    doc.addTempoEvent(ticks_accumulated, cctn::song::SongDocument::TempoEvent::TempoEventType::kTimeSignature, 120.0, 3, 4);

    // ticks per bar: 1440
    ticks_per_bar = 1440;

    // Bar 8: Change to 4/4 time signature
    ticks_accumulated += ticks_per_bar * 3;
    doc.addTempoEvent(ticks_accumulated, cctn::song::SongDocument::TempoEvent::TempoEventType::kTimeSignature, 120.0, 4, 4);

    // ticks per bar: 1920
    ticks_per_bar = 1920;

    // Bar 13: Change to 8/8 time signature and 94 BPM
    ticks_accumulated += ticks_per_bar * 5;
    doc.addTempoEvent(ticks_accumulated, cctn::song::SongDocument::TempoEvent::TempoEventType::kBoth, 94.0, 8, 8);

    // ticks per bar: 1920
    ticks_per_bar = 1920;

    // Bar 15: Change to 4/4 time signature and 160 BPM
    ticks_accumulated += ticks_per_bar * 2;
    doc.addTempoEvent(ticks_accumulated, cctn::song::SongDocument::TempoEvent::TempoEventType::kBoth, 160, 4, 4);

    // Helper function to create a note
    auto createNote = [](int id, int bar, int beat, int tick, int durationBars, int durationBeats, int durationTicks,
        int noteNumber, int velocity, const juce::String& lyric) {
            return cctn::song::SongDocument::Note{
                id,
                {bar, beat, tick},
                {durationBars, durationBeats, durationTicks},
                noteNumber,
                velocity,
                lyric
            };
        };

    // Add notes for 16 bars
    // arguments: int id, int bar, int beat, int tick, int durationBars, int durationBeats, int durationTicks, int noteNumber, int velocity, const juce::String& lyric
    std::vector<cctn::song::SongDocument::Note> notes = {
        createNote(1, 1, 1, 0, 0, 1, 0, 60, 100, juce::CharPointer_UTF8("\xe3\x83\x89")),
        createNote(2, 1, 2, 0, 0, 1, 0, 62, 95, juce::CharPointer_UTF8("\xe3\x83\xac")),
        createNote(3, 1, 3, 0, 0, 1, 0, 64, 98, juce::CharPointer_UTF8("\xe3\x83\x9f")),
        createNote(4, 1, 4, 0, 0, 1, 0, 65, 90, juce::CharPointer_UTF8("\xe3\x83\x95\xe3\x82\xa1")),

        createNote(5, 2, 1, 0, 0, 2, 0, 67, 100, juce::CharPointer_UTF8("\xe3\x82\xbd")),
        createNote(6, 2, 3, 0, 0, 2, 0, 69, 95, juce::CharPointer_UTF8("\xe3\x83\xa9")),

        createNote(7, 3, 1, 0, 0, 4, 0, 71, 98, juce::CharPointer_UTF8("\xe3\x82\xb7")),

        createNote(8, 4, 1, 0, 0, 4, 0, 72, 100, juce::CharPointer_UTF8("\xe3\x83\x89")),

        createNote(9, 5, 1, 0, 0, 1, 0, 71, 90, juce::CharPointer_UTF8("\xe3\x82\xb7")),
        createNote(10, 5, 2, 0, 0, 1, 0, 69, 95, juce::CharPointer_UTF8("\xe3\x83\xa9")),
        createNote(11, 5, 3, 0, 0, 1, 0, 67, 98, juce::CharPointer_UTF8("\xe3\x82\xbd")),

        createNote(12, 6, 1, 0, 0, 3, 0, 65, 100, juce::CharPointer_UTF8("\xe3\x83\x95\xe3\x82\xa1")),

        createNote(13, 7, 1, 0, 0, 2, 0, 64, 95, juce::CharPointer_UTF8("\xe3\x83\x9f")),
        createNote(14, 7, 3, 0, 0, 1, 0, 62, 90, juce::CharPointer_UTF8("\xe3\x83\xac")),

        createNote(15, 8, 1, 0, 0, 3, 0, 60, 100, juce::CharPointer_UTF8("\xe3\x83\x89")),

        createNote(16, 9, 1, 0, 0, 1, 240, 62, 95, juce::CharPointer_UTF8("\xe3\x83\xac")),
        createNote(17, 9, 3, 0, 0, 1, 240, 64, 98, juce::CharPointer_UTF8("\xe3\x83\x9f")),

        createNote(18, 10, 2, 0, 0, 1, 240, 65, 90, juce::CharPointer_UTF8("\xe3\x83\x95\xe3\x82\xa1")),

        createNote(19, 11, 1, 0, 0, 3, 0, 67, 100, juce::CharPointer_UTF8("\xe3\x82\xbd")),

        createNote(20, 12, 1, 0, 0, 3, 0, 69, 95, juce::CharPointer_UTF8("\xe3\x83\xa9")),

        createNote(21, 13, 1, 0, 0, 2, 0, 71, 98, juce::CharPointer_UTF8("\xe3\x82\xb7")),
        createNote(22, 13, 3, 0, 0, 2, 0, 72, 100, juce::CharPointer_UTF8("\xe3\x83\x89")),
        createNote(23, 13, 5, 0, 0, 2, 0, 74, 95, juce::CharPointer_UTF8("\xe3\x83\xac")),
        createNote(24, 13, 7, 0, 0, 2, 0, 76, 90, juce::CharPointer_UTF8("\xe3\x83\x9f")),

        createNote(25, 14, 1, 0, 0, 8, 0, 77, 100, juce::CharPointer_UTF8("\xe3\x83\x95\xe3\x82\xa1")),

        createNote(26, 15, 1, 0, 0, 2, 0, 76, 95, juce::CharPointer_UTF8("\xe3\x83\x9f")),
        createNote(27, 15, 3, 0, 0, 2, 0, 74, 98, juce::CharPointer_UTF8("\xe3\x83\xac")),

        createNote(28, 16, 1, 0, 2, 0, 0, 72, 100, juce::CharPointer_UTF8("\xe3\x83\x89"))
    };

    // Add all notes to the document
    for (const auto& note : notes) {
        doc.addNote(note);
    }

    return doc;
}

}  // namespace song
}  // namespace cctn

#if 0
// Example usage
int main()
{
    cctn::song::SongDocument testDoc = cctn::song::createTestSongDocument();

    // You can now use testDoc for testing or further processing
    // For example:
    std::cout << "Test song title: " << testDoc.getTitle() << std::endl;
    std::cout << "Number of notes: " << testDoc.getNotes().size() << std::endl;

    return 0;
}
#endif