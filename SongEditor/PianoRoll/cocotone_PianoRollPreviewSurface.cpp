#include "cocotone_PianoRollPreviewSurface.h"
namespace cctn
{
namespace song
{

//==============================================================================
PianoRollPreviewSurface::PianoRollPreviewSurface(const PianoRollKeyboard& pianoRollKeyboard)
    : pianoRollKeyboardRef(pianoRollKeyboard)
    , verticalLineIntervalInSeconds(1.0)
    , playingPositionInSeconds(0.0)
    , inputPositionInSeconds(0.0)
    , currentPreviewData({})
{
    numVisibleWhiteAndBlackKeys = 24;
    numVisibleWhiteKeys = 14;
    rangeVisibleKeyNoteNumbers = juce::Range<juce::uint8>(48, 48 + numVisibleWhiteAndBlackKeys);

    rangeVisibleTimeInSeconds = juce::Range<double>(0.0f, 60.0f);
}

PianoRollPreviewSurface::~PianoRollPreviewSurface()
{
}

//==============================================================================
void PianoRollPreviewSurface::setVisibleBottomKeyNoteNumber(int noteNumber)
{
    rangeVisibleKeyNoteNumbers = juce::Range<juce::uint8>(noteNumber, noteNumber + numVisibleWhiteAndBlackKeys);

    // For fast painitng.
    updateMapVisibleKeyNoteNumberToVerticalPositionRange();
    
    repaint();
}

void PianoRollPreviewSurface::setVisibleRangeTimeInSeconds(juce::Range<double> newRange)
{
    rangeVisibleTimeInSeconds = newRange;
    repaint();
}

void PianoRollPreviewSurface::setGridVerticalLineIntervaleInSeconds(double intervalInSeconds)
{
    verticalLineIntervalInSeconds = intervalInSeconds;
    repaint();
}

void PianoRollPreviewSurface::setPlayingPositionInSeconds(double positionInSeconds)
{
    playingPositionInSeconds = positionInSeconds;
    repaint();
}

void PianoRollPreviewSurface::setInputPositionInSeconds(double positionInSeconds)
{
    inputPositionInSeconds = positionInSeconds;
    repaint();
}

void PianoRollPreviewSurface::setPianoRollPreviewData(cctn::song::PianoRollPreviewData previewData)
{
    currentPreviewData = previewData;
    repaint();
}

//==============================================================================
void PianoRollPreviewSurface::emitMouseEvent(const juce::MouseEvent& mouseEvent, bool isExit)
{
    lastMousePosition = mouseEvent.getPosition();

    inputPositionInSeconds = positionXToTime(lastMousePosition.getX(), 0, getWidth(), rangeVisibleTimeInSeconds);

    if (isExit)
    {
        inputPositionInSeconds = -1.0;
    }

    repaint();
}

std::optional<cctn::song::QueryForFindPianoRollNote> PianoRollPreviewSurface::getQueryForFindPianoRollNote(const juce::MouseEvent& mouseEvent)
{
    auto position = mouseEvent.getPosition();

    const auto note_number_optional = findNoteNumberWithVerticalPosition(position.getY());
    const auto time = positionXToTime(position.getX(), 0, getWidth(), rangeVisibleTimeInSeconds);

    if (note_number_optional.has_value())
    {
        cctn::song::QueryForFindPianoRollNote query;
        query.noteNumber = note_number_optional.value();
        query.timeInSeconds = time;
        return query;
    }

    return std::nullopt;
}

//==============================================================================
void PianoRollPreviewSurface::paint(juce::Graphics& g)
{
    juce::Graphics::ScopedSaveState save_state(g);

    g.fillAll(kColourMainBackground);

    fillGridHorizontalRows(g);
    drawGridHorizontalLines(g);
    drawGridVerticalLines(g);

    drawCurrentPreviewData(g);

    drawPlayingPositionMarker(g);

    drawInputPositionMarker(g);
}

void PianoRollPreviewSurface::resized()
{
    // For fast painitng.
    updateMapVisibleKeyNoteNumberToVerticalPositionRange();

    repaint();
}

//==============================================================================
void PianoRollPreviewSurface::fillGridHorizontalRows(juce::Graphics& g)
{
    juce::Graphics::ScopedSaveState save_state(g);

#if 1
    g.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 10, 0));
#endif

    for (int note_number = rangeVisibleKeyNoteNumbers.getStart();
        note_number < rangeVisibleKeyNoteNumbers.getEnd();
        note_number++)
    {
        juce::Range<float> key_position_range = juce::Range<float>{ 0.0f, 0.0f };
#if 0
        key_position_range = pianoRollKeyboardRef.getPositionRangeForPianoRollGridHorizontalWidth(note_number);

        juce::Rectangle<float> rect_to_fill = juce::Rectangle<float>{
            0.0f,
            (float)getHeight() - key_position_range.getEnd(),
            (float)getWidth(),
            key_position_range.getLength()
        };
#else
        if (mapVisibleKeyNoteNumberToVerticalPositionRange.count(note_number) > 0)
        {
            key_position_range = mapVisibleKeyNoteNumberToVerticalPositionRange[note_number];
        }

        juce::Rectangle<float> rect_to_fill = juce::Rectangle<float>{
            0.0f,
            (float)key_position_range.getStart(),
            (float)getWidth(),
            key_position_range.getLength()
        };
#endif

        // In case of black key
        if (juce::MidiMessage::isMidiNoteBlack(note_number))
        {
            g.setColour(kColourGridBlackKeyBackground);
            g.fillRect(rect_to_fill);
        }
        else
        {
            g.setColour(kColourGridWhiteKeyBackground);
            g.fillRect(rect_to_fill);
        }

#if 1
        g.setColour(juce::Colours::white);
        //const auto text = juce::String(note_number);
        const auto text = juce::MidiMessage::getMidiNoteName(note_number, true, true, 3);
        g.drawText(text, rect_to_fill.withLeft(rect_to_fill.getWidth() - 40).withTrimmedRight(2), juce::Justification::centred);
#endif
    }
}

void PianoRollPreviewSurface::drawGridHorizontalLines(juce::Graphics& g)
{
    juce::Graphics::ScopedSaveState save_state(g);

    for (int note_number = rangeVisibleKeyNoteNumbers.getStart();
        note_number < rangeVisibleKeyNoteNumbers.getEnd();
        note_number++)
    {
        juce::Range<float> key_position_range = juce::Range<float>{ 0.0f, 0.0f };
#if 0
        key_position_range = pianoRollKeyboardRef.getPositionRangeForPianoRollGridHorizontalWidth(note_number);
        juce::Rectangle<float> rect_to_fill = { 0.0f, (float)getHeight() - key_position_range.getEnd(), (float)getWidth(), key_position_range.getLength() };
#else
        if (mapVisibleKeyNoteNumberToVerticalPositionRange.count(note_number) > 0)
        {
            key_position_range = mapVisibleKeyNoteNumberToVerticalPositionRange[note_number];
        }
        juce::Rectangle<float> rect_to_fill = { 0.0f, (float)key_position_range.getStart(), (float)getWidth(), key_position_range.getLength()};
#endif

        // In case of black key
        if (juce::MidiMessage::isMidiNoteBlack(note_number))
        {
            g.setColour(kColourGridBlackKeyBezel);

            const float horizontal_y_top = rect_to_fill.getY() + 1.0f;
            const float horizontal_y_bottom = rect_to_fill.getBottom() - 1.0f;
            g.drawHorizontalLine(horizontal_y_top, 0.0f, static_cast<float>(getWidth()));
            g.drawHorizontalLine(horizontal_y_bottom, 0.0f, static_cast<float>(getWidth()));
        }
        else
        {
            g.setColour(kColourGridWhiteKeyBezel);

            const float horizontal_y_top = rect_to_fill.getY();
            const float horizontal_y_bottom = rect_to_fill.getBottom();
            g.drawHorizontalLine(horizontal_y_top, 0.0f, static_cast<float>(getWidth()));
            g.drawHorizontalLine(horizontal_y_bottom, 0.0f, static_cast<float>(getWidth()));
        }
    }
}

void PianoRollPreviewSurface::drawGridVerticalLines(juce::Graphics& g)
{
    juce::Graphics::ScopedSaveState save_state(g);

    const auto vertical_line_positions = createVerticalLinePositions(rangeVisibleTimeInSeconds, verticalLineIntervalInSeconds, getWidth());

#if 0
    g.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 10, 0));
#endif

    for (const auto& position_with_time_info : vertical_line_positions)
    {
        g.setColour(juce::Colours::white);
        g.drawVerticalLine(position_with_time_info.positionX, 0.0f, getHeight());

#if 0
        g.setColour(juce::Colours::white);
        g.drawText(juce::String(position_with_time_info.timeInSeconds, 2), position_with_time_info.positionX, 0.0f, 60, 20, juce::Justification::centredLeft);
#endif
    }
}

void PianoRollPreviewSurface::drawCurrentPreviewData(juce::Graphics& g)
{
    juce::Graphics::ScopedSaveState save_state(g);

    const auto& notes = currentPreviewData.notes;

    for (const auto& note : notes)
    {
        const auto note_draw_info = createNoteDrawInfo(note, rangeVisibleTimeInSeconds, 0, getWidth());

        const auto key_position_range = pianoRollKeyboardRef.getPositionRangeForPianoRollGridHorizontalWidth(note_draw_info.noteNumber);

        juce::Rectangle<float> rect_to_fill = juce::Rectangle<float>{
            (float)note_draw_info.positionLeftX,
            (float)getHeight() - key_position_range.getEnd(),
            (float)(note_draw_info.positionRightX - note_draw_info.positionLeftX),
            (float)key_position_range.getLength(),
        };

        g.setColour(kColourGridNote);
        g.fillRect(rect_to_fill);

        if (rect_to_fill.contains(lastMousePosition.getX(), lastMousePosition.getY()))
        {
            g.setColour(kColourGridNote.brighter());
            g.fillRect(rect_to_fill);
        }

        if (note_draw_info.isSelected)
        {
            g.setColour(kColourGridNoteSelected);
            g.fillRect(rect_to_fill);
        }

        g.setColour(juce::Colours::white);
        g.drawText(note_draw_info.lyric, rect_to_fill.withTrimmedLeft(4), juce::Justification::centredLeft);

        g.setColour(kColourGridNoteBezel);
        g.drawRect(rect_to_fill, 2);
    }
}

void PianoRollPreviewSurface::drawPlayingPositionMarker(juce::Graphics& g)
{
    juce::Graphics::ScopedSaveState save_state(g);

    const auto position_x = timeToPositionX(playingPositionInSeconds, rangeVisibleTimeInSeconds, getWidth());

    juce::Rectangle<int> rect_marker =
        juce::Rectangle<int>{ position_x, 0, 2, getHeight() };

    g.setColour(juce::Colours::yellow);
    g.fillRect(rect_marker);
}

void PianoRollPreviewSurface::drawInputPositionMarker(juce::Graphics& g)
{
    juce::Graphics::ScopedSaveState save_state(g);

    if (inputPositionInSeconds > 0.0)
    {
        const auto position_x = timeToPositionX(inputPositionInSeconds, rangeVisibleTimeInSeconds, getWidth());

        juce::Rectangle<int> rect_marker =
            juce::Rectangle<int>{ position_x, 0, 2, getHeight() };

        g.setColour(kColourGridNote);
        g.fillRect(rect_marker);
    }
}

//==============================================================================
void PianoRollPreviewSurface::updateMapVisibleKeyNoteNumberToVerticalPositionRange()
{
    mapVisibleKeyNoteNumberToVerticalPositionRange.clear();

    for (int note_number = rangeVisibleKeyNoteNumbers.getStart();
        note_number < rangeVisibleKeyNoteNumbers.getEnd();
        note_number++)
    {
        const auto key_position_range_ignore_orientation = pianoRollKeyboardRef.getPositionRangeForPianoRollGridHorizontalWidth(note_number);
        
        const juce::Range<float> key_position_range_vertical = 
            juce::Range<float>{
            getHeight() - key_position_range_ignore_orientation.getEnd(),
            getHeight() - key_position_range_ignore_orientation.getStart()
        };

        mapVisibleKeyNoteNumberToVerticalPositionRange[note_number] = key_position_range_vertical;
    }
}

std::optional<juce::uint8>  PianoRollPreviewSurface::findNoteNumberWithVerticalPosition(float positionY)
{
    for (const auto& element : mapVisibleKeyNoteNumberToVerticalPositionRange)
    {
        if (element.second.contains(positionY))
        {
            return element.first;
        }
    }

    return std::nullopt;
}

//==============================================================================
juce::Array<PianoRollPreviewSurface::PositionWithTimeInfo> PianoRollPreviewSurface::createVerticalLinePositions(const juce::Range<double> visibleRangeSeconds, double timeUnitSeconds, int width)
{
    juce::Array<PositionWithTimeInfo> result;

    std::vector<double> time_secnds_vertical_lines;
    {
        // Find the smallest multiple of 10 within the range
        double min_units = static_cast<double>(std::ceil(visibleRangeSeconds.getStart() / timeUnitSeconds)) * (double)timeUnitSeconds;

        // Find the largest multiple of 10 within the range
        double max_units = static_cast<double>(std::floor(visibleRangeSeconds.getEnd() / timeUnitSeconds)) * (double)timeUnitSeconds;

        // List up multiples of 10 within the range
        for (double value = min_units; value <= max_units; value += (double)timeUnitSeconds)
        {
            time_secnds_vertical_lines.push_back((double)value);
        }
    }

    if (time_secnds_vertical_lines.size() > 0)
    {
        for (const auto& time_seconds_vertical_line : time_secnds_vertical_lines)
        {
            // Convert time to position X.
            const double position_x =
                juce::jmap<double>(time_seconds_vertical_line, visibleRangeSeconds.getStart(), visibleRangeSeconds.getEnd(), 0, width);

            PositionWithTimeInfo position_with_time_info;
            position_with_time_info.positionX = position_x;
            position_with_time_info.timeInSeconds = time_seconds_vertical_line;

            result.add(position_with_time_info);
        }
    }

    return result;
}

PianoRollPreviewSurface::NoteDrawInfo PianoRollPreviewSurface::createNoteDrawInfo(PianoRollNote note, const juce::Range<double> visibleRangeSeconds, int positionLeft, int positionRight)
{
    NoteDrawInfo result;

    // Convert time to position X.
    const double rect_left_x =
        juce::jmap<double>(note.startInSeconds, visibleRangeSeconds.getStart(), visibleRangeSeconds.getEnd(), positionLeft, positionRight);

    const double rect_right_x =
        juce::jmap<double>(note.endInSeconds, visibleRangeSeconds.getStart(), visibleRangeSeconds.getEnd(), positionLeft, positionRight);

    result.positionLeftX = rect_left_x;
    result.positionRightX = rect_right_x;
    result.lyric = note.lyric;
    result.noteNumber = note.noteNumber;
    result.isSelected = note.isSelected;

    return result;
}

}
}