#pragma once

#include "../cocotone_SongDocument.h"
#include <juce_core/juce_core.h>

namespace cctn
{
namespace song
{

static SongDocument createTestSongDocument()
{
    SongDocument document;

    // Set metadata
    document.setMetadata(
        juce::CharPointer_UTF8("\xe3\x82\xb5\xe3\x83\xb3\xe3\x83\x97\xe3\x83\xab \xe3\x82\xbd\xe3\x83\xb3\xe3\x82\xb0"),
        juce::CharPointer_UTF8("\xe3\x82\xb5\xe3\x83\xb3\xe3\x83\x97\xe3\x83\xab \xe3\x82\xa2\xe3\x83\xbc\xe3\x83\x86\xe3\x82\xa3\xe3\x82\xb9\xe3\x83\x88")
    );

    // Add tempo events
    int64_t ticks_accumulated = 0;
    int64_t ticks_per_bar = 0;

    // Start: 4/4 time signature, 120 BPM
    document.addTempoEvent(0, cctn::song::SongDocument::TempoEvent::TempoEventType::kBoth, 4, 4, 120.0);

    // ticks per bar: 1920
    ticks_per_bar = 1920;
    ticks_accumulated += ticks_per_bar * 1;
    document.addTempoEvent(ticks_accumulated, cctn::song::SongDocument::TempoEvent::TempoEventType::kBoth, 4, 4, 160.0);

    ticks_accumulated += ticks_per_bar * 1;
    document.addTempoEvent(ticks_accumulated, cctn::song::SongDocument::TempoEvent::TempoEventType::kBoth, 4, 4, 120.0);

    // Bar 3: Change tempo to 140 BPM
    ticks_accumulated += ticks_per_bar * 1;
    document.addTempoEvent(ticks_accumulated, cctn::song::SongDocument::TempoEvent::TempoEventType::kTempo, 4, 4, 140.0);

    // Bar 5: Change to 3/4 time signature
    ticks_accumulated += ticks_per_bar * 1;
    document.addTempoEvent(ticks_accumulated, cctn::song::SongDocument::TempoEvent::TempoEventType::kTimeSignature, 3, 4, 120.0);

    // ticks per bar: 1440
    ticks_per_bar = 1440;

    // Bar 8: Change to 4/4 time signature
    ticks_accumulated += ticks_per_bar * 3;
    document.addTempoEvent(ticks_accumulated, cctn::song::SongDocument::TempoEvent::TempoEventType::kTimeSignature, 4, 4, 120.0);

    // ticks per bar: 1920
    ticks_per_bar = 1920;

    // Bar 13: Change to 8/8 time signature and 94 BPM
    ticks_accumulated += ticks_per_bar * 5;
    document.addTempoEvent(ticks_accumulated, cctn::song::SongDocument::TempoEvent::TempoEventType::kBoth, 8, 8, 94.0);

    // ticks per bar: 1920
    ticks_per_bar = 1920;

    // Bar 15: Change to 4/4 time signature and 160 BPM
    ticks_accumulated += ticks_per_bar * 2;
    document.addTempoEvent(ticks_accumulated, cctn::song::SongDocument::TempoEvent::TempoEventType::kBoth, 4, 4, 160);

    // Helper function to create a note
    const cctn::song::SongDocument::NoteDuration note_duration_quarter = cctn::song::SongDocument::DataFactory::convertNoteLengthToDuration(document, cctn::song::NoteLength::Quarter);
    const cctn::song::SongDocument::NoteDuration note_duration_half = cctn::song::SongDocument::DataFactory::convertNoteLengthToDuration(document, cctn::song::NoteLength::Half);
    const cctn::song::SongDocument::NoteDuration note_duration_whole = cctn::song::SongDocument::DataFactory::convertNoteLengthToDuration(document, cctn::song::NoteLength::Whole);
    const cctn::song::SongDocument::NoteDuration note_duration_dotted_quarter = cctn::song::SongDocument::DataFactory::convertNoteLengthToDuration(document, cctn::song::NoteLength::DottedQuarter);
    const cctn::song::SongDocument::NoteDuration note_duration_dotted_half = cctn::song::SongDocument::DataFactory::convertNoteLengthToDuration(document, cctn::song::NoteLength::DottedHalf);

    // Add notes for 16 bars
    // arguments: int id, int bar, int beat, int tick, int durationBars, int durationBeats, int durationTicks, int noteNumber, int velocity, const juce::String& lyric
    std::vector<cctn::song::SongDocument::Note> notes = {
        cctn::song::SongDocument::DataFactory::makeNote(document, {1, 1, 0}, note_duration_quarter, 60, 100, juce::CharPointer_UTF8("\xe3\x83\x89")),
        cctn::song::SongDocument::DataFactory::makeNote(document, {1, 2, 0}, note_duration_quarter, 62, 95, juce::CharPointer_UTF8("\xe3\x83\xac")),
        cctn::song::SongDocument::DataFactory::makeNote(document, {1, 3, 0}, note_duration_quarter, 64, 98, juce::CharPointer_UTF8("\xe3\x83\x9f")),
        cctn::song::SongDocument::DataFactory::makeNote(document, {1, 4, 0}, note_duration_quarter, 65, 90, juce::CharPointer_UTF8("\xe3\x83\x95\xe3\x82\xa1")),

        cctn::song::SongDocument::DataFactory::makeNote(document, {2, 1, 0}, note_duration_half, 67, 100, juce::CharPointer_UTF8("\xe3\x82\xbd")),
        cctn::song::SongDocument::DataFactory::makeNote(document, {2, 3, 0}, note_duration_half, 69, 95, juce::CharPointer_UTF8("\xe3\x83\xa9")),

        cctn::song::SongDocument::DataFactory::makeNote(document, {3, 1, 0}, note_duration_whole, 71, 98, juce::CharPointer_UTF8("\xe3\x82\xb7")),

        cctn::song::SongDocument::DataFactory::makeNote(document, {4, 1, 0}, note_duration_whole, 72, 100, juce::CharPointer_UTF8("\xe3\x83\x89")),

        cctn::song::SongDocument::DataFactory::makeNote(document, {5, 1, 0}, note_duration_quarter, 71, 90, juce::CharPointer_UTF8("\xe3\x82\xb7")),
        cctn::song::SongDocument::DataFactory::makeNote(document, {5, 2, 0}, note_duration_quarter, 69, 95, juce::CharPointer_UTF8("\xe3\x83\xa9")),
        cctn::song::SongDocument::DataFactory::makeNote(document, {5, 3, 0}, note_duration_quarter, 67, 98, juce::CharPointer_UTF8("\xe3\x82\xbd")),

        cctn::song::SongDocument::DataFactory::makeNote(document, {6, 1, 0}, note_duration_dotted_half, 65, 100, juce::CharPointer_UTF8("\xe3\x83\x95\xe3\x82\xa1")),

        cctn::song::SongDocument::DataFactory::makeNote(document, {7, 1, 0}, note_duration_half, 64, 95, juce::CharPointer_UTF8("\xe3\x83\x9f")),
        cctn::song::SongDocument::DataFactory::makeNote(document, {7, 3, 0}, note_duration_quarter, 62, 90, juce::CharPointer_UTF8("\xe3\x83\xac")),

        cctn::song::SongDocument::DataFactory::makeNote(document, {8, 1, 0}, note_duration_dotted_half, 60, 100, juce::CharPointer_UTF8("\xe3\x83\x89")),

        cctn::song::SongDocument::DataFactory::makeNote(document, {9, 1, 0}, note_duration_dotted_quarter, 62, 95, juce::CharPointer_UTF8("\xe3\x83\xac")),
        cctn::song::SongDocument::DataFactory::makeNote(document, {9, 3, 0}, note_duration_dotted_quarter, 64, 98, juce::CharPointer_UTF8("\xe3\x83\x9f")),

        cctn::song::SongDocument::DataFactory::makeNote(document, {10, 2, 0}, note_duration_dotted_quarter, 65, 90, juce::CharPointer_UTF8("\xe3\x83\x95\xe3\x82\xa1")),

        cctn::song::SongDocument::DataFactory::makeNote(document, {11, 1, 0}, note_duration_dotted_half, 67, 100, juce::CharPointer_UTF8("\xe3\x82\xbd")),

        cctn::song::SongDocument::DataFactory::makeNote(document, {12, 1, 0}, note_duration_dotted_half, 69, 95, juce::CharPointer_UTF8("\xe3\x83\xa9")),

        cctn::song::SongDocument::DataFactory::makeNote(document, {13, 1, 0}, note_duration_quarter, 71, 98, juce::CharPointer_UTF8("\xe3\x82\xb7")),
        cctn::song::SongDocument::DataFactory::makeNote(document, {13, 3, 0}, note_duration_quarter, 72, 100, juce::CharPointer_UTF8("\xe3\x83\x89")),
        cctn::song::SongDocument::DataFactory::makeNote(document, {13, 5, 0}, note_duration_quarter, 74, 95, juce::CharPointer_UTF8("\xe3\x83\xac")),
        cctn::song::SongDocument::DataFactory::makeNote(document, {13, 7, 0}, note_duration_quarter, 76, 90, juce::CharPointer_UTF8("\xe3\x83\x9f")),

        cctn::song::SongDocument::DataFactory::makeNote(document, {14, 1, 0}, note_duration_whole, 77, 100, juce::CharPointer_UTF8("\xe3\x83\x95\xe3\x82\xa1")),

        cctn::song::SongDocument::DataFactory::makeNote(document, {15, 1, 0}, note_duration_half, 76, 95, juce::CharPointer_UTF8("\xe3\x83\x9f")),
        cctn::song::SongDocument::DataFactory::makeNote(document, {15, 3, 0}, note_duration_half, 74, 98, juce::CharPointer_UTF8("\xe3\x83\xac")),

        cctn::song::SongDocument::DataFactory::makeNote(document, {16, 1, 0}, note_duration_whole + note_duration_whole, 72, 100, juce::CharPointer_UTF8("\xe3\x83\x89"))
    };

    // Add all notes to the document
    for (const auto& note : notes) {
        document.addNote(note);
    }

    return document;
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