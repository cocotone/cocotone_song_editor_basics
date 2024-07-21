#include "cocotone_PianoRollPreviewSurface.h"
namespace cctn
{
namespace song
{

//==============================================================================
PianoRollPreviewSurface::PianoRollPreviewSurface(const PianoRollKeyboard& pianoRollKeyboard, int numVisibleOctaves)
    : pianoRollKeyboardRef(pianoRollKeyboard)
    , verticalLineIntervalInSeconds(1.0)
    , playingPositionInSeconds(0.0)
    , userInputPositionInSeconds(0.0)
    , userInputPositionInNoteNumber(-1)
    , quantizedInputRegionInSeconds(juce::Range<double>{0.0, 0.0})
    , isInputPositionInsertable(false)
    , paintScopedDocumentDataPtr(nullptr)
{
    numVisibleWhiteAndBlackKeys = 12 * numVisibleOctaves;
    numVisibleWhiteKeys = 7 * numVisibleOctaves;
    rangeVisibleKeyNoteNumbers = juce::Range<juce::uint8>(48, 48 + numVisibleWhiteAndBlackKeys);

    rangeVisibleTimeInSeconds = juce::Range<double>(0.0f, 60.0f);
}

PianoRollPreviewSurface::~PianoRollPreviewSurface()
{
    if (!documentForPreviewPtr.expired())
    {
        documentForPreviewPtr.lock()->removeChangeListener(this);
    }
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

void PianoRollPreviewSurface::setUserInputPositionInSeconds(double positionInSeconds)
{
    userInputPositionInSeconds = positionInSeconds;
    repaint();
}

//==============================================================================
void PianoRollPreviewSurface::emitMouseEvent(const juce::MouseEvent& mouseEvent, bool isExitAction)
{
    lastMousePosition = mouseEvent.getPosition();

    userInputPositionInSeconds = positionXToTime(lastMousePosition.getX(), 0, getWidth(), rangeVisibleTimeInSeconds);

    const auto note_and_velocity = pianoRollKeyboardRef.getNoteAndVelocityAtPositionForwarding(lastMousePosition.withX(pianoRollKeyboardRef.getX()).toFloat(), false);
    userInputPositionInNoteNumber = note_and_velocity.note;

    if (isExitAction)
    {
        userInputPositionInSeconds = -1.0;
        userInputPositionInNoteNumber = -1;
    }

    repaint();
}

std::optional<cctn::song::QueryForFindPianoRollNote> PianoRollPreviewSurface::getQueryForFindPianoRollNote(const juce::MouseEvent& mouseEvent)
{
    auto position = mouseEvent.getMouseDownPosition();

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
void PianoRollPreviewSurface::setDocumentForPreview(std::shared_ptr<cctn::song::SongEditorDocument> document)
{
    if (!documentForPreviewPtr.expired())
    {
        if (documentForPreviewPtr.lock().get() != document.get())
        {
            documentForPreviewPtr.lock()->removeChangeListener(this);
            documentForPreviewPtr.reset();
        }
    }

    documentForPreviewPtr = document;

    if (!documentForPreviewPtr.expired())
    {
        documentForPreviewPtr.lock()->addChangeListener(this);
    }

    repaint();
}

//==============================================================================
void PianoRollPreviewSurface::paint(juce::Graphics& g)
{
    juce::Graphics::ScopedSaveState save_state(g);

    const cctn::song::SongEditorDocumentData* document_data_to_paint = nullptr;
    if (!documentForPreviewPtr.expired() &&
        documentForPreviewPtr.lock()->getRawDocumentData().has_value())
    {
        document_data_to_paint = documentForPreviewPtr.lock()->getRawDocumentData().value();
    }

    juce::ScopedValueSetter<const cctn::song::SongEditorDocumentData*> svs(paintScopedDocumentDataPtr, document_data_to_paint);

    updateViewContext();

    g.fillAll(kColourMainBackground);

    fillGridHorizontalRows(g);
    drawGridHorizontalLines(g);
    drawGridVerticalLines(g);

    drawCurrentPreviewData(g);

    drawPlayingPositionMarker(g);

    //drawUserInputPositionMarker(g);

    drawQuantizedInputRegionRectangle(g);

    drawUserInputPositionCellRectangle(g);
}

void PianoRollPreviewSurface::resized()
{
    // For fast painitng.
    updateMapVisibleKeyNoteNumberToVerticalPositionRange();

    repaint();
}

//==============================================================================
void PianoRollPreviewSurface::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (!documentForPreviewPtr.expired())
    {
        if (source == documentForPreviewPtr.lock().get())
        {
            repaint();
        }
    }
}

//==============================================================================
void PianoRollPreviewSurface::updateViewContext()
{
    JUCE_ASSERT_MESSAGE_MANAGER_EXISTS;

    if (paintScopedDocumentDataPtr == nullptr)
    {
        return;
    }

    const auto& notes = paintScopedDocumentDataPtr->notes;

    // Initialize
    isInputPositionInsertable = true;
    for (const auto& note : notes)
    {
        if (juce::Range<float>(note.startPositionInSeconds, note.endPositionInSeconds).contains(userInputPositionInSeconds))
        {
            isInputPositionInsertable = false;
            break;
        }
    }

    // Initialize
    quantizedInputRegionInSeconds = juce::Range<double>{ 0.0f, 0.0f };
    if (!documentForPreviewPtr.expired() &&
        documentForPreviewPtr.lock()->getRawDocumentData().has_value())
    {
        const auto region_optional = documentForPreviewPtr.lock()->findNearestQuantizeRegion(userInputPositionInSeconds);
        if (region_optional.has_value())
        {
            quantizedInputRegionInSeconds =
                juce::Range<double>{ region_optional.value().startPositionInSeconds, region_optional.value().endPositionInSeconds };
        }
    }
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
        if (mapVisibleKeyNoteNumberToVerticalPositionRangeAsVerticalTopToBottom.count(note_number) > 0)
        {
            key_position_range = mapVisibleKeyNoteNumberToVerticalPositionRangeAsVerticalTopToBottom[note_number];
        }
#if 0
        key_position_range = pianoRollKeyboardRef.getPositionRangeForPianoRollGridHorizontalWidth(note_number);

        juce::Rectangle<float> rect_to_fill = juce::Rectangle<float>{
            0.0f,
            (float)getHeight() - key_position_range.getEnd(),
            (float)getWidth(),
            key_position_range.getLength()
        };
#else

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
        if (mapVisibleKeyNoteNumberToVerticalPositionRangeAsVerticalTopToBottom.count(note_number) > 0)
        {
            key_position_range = mapVisibleKeyNoteNumberToVerticalPositionRangeAsVerticalTopToBottom[note_number];
        }
#if 0
        key_position_range = pianoRollKeyboardRef.getPositionRangeForPianoRollGridHorizontalWidth(note_number);
        juce::Rectangle<float> rect_to_fill = { 0.0f, (float)getHeight() - key_position_range.getEnd(), (float)getWidth(), key_position_range.getLength() };
#else
        juce::Rectangle<float> rect_to_fill = { 0.0f, (float)key_position_range.getStart(), (float)getWidth(), key_position_range.getLength() };
#endif

        // In case of white key draw bezel
        if (!juce::MidiMessage::isMidiNoteBlack(note_number))
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

    if (paintScopedDocumentDataPtr == nullptr)
    {
        return;
    }

    const auto& notes = paintScopedDocumentDataPtr->notes;

    for (const auto& note : notes)
    {
        const auto note_draw_info = createNoteDrawInfo(note, rangeVisibleTimeInSeconds, 0, getWidth());
        
        juce::Range<float> key_position_range = juce::Range<float>{ 0.0f, 0.0f };
        if (mapVisibleKeyNoteNumberToVerticalPositionRangeAsVerticalTopToBottom.count(note_draw_info.noteNumber) > 0)
        {
            key_position_range = mapVisibleKeyNoteNumberToVerticalPositionRangeAsVerticalTopToBottom[note_draw_info.noteNumber];
        }

        juce::Rectangle<float> rect_to_fill = juce::Rectangle<float>{
            (float)note_draw_info.positionLeftX,
            (float)key_position_range.getStart(),
            (float)juce::jmax<int>(note_draw_info.positionRightX - note_draw_info.positionLeftX, 16),
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

void PianoRollPreviewSurface::drawUserInputPositionMarker(juce::Graphics& g)
{
    juce::Graphics::ScopedSaveState save_state(g);

    if (userInputPositionInSeconds > 0.0)
    {
        if (isInputPositionInsertable)
        {
            g.setColour(kColourPianoRollMousePositionInsertable);
        }
        else
        {
            g.setColour(kColourPianoRollMousePositionNotInsertable);
        }

        const auto position_x = timeToPositionX(userInputPositionInSeconds, rangeVisibleTimeInSeconds, getWidth());
        juce::Rectangle<int> rect_marker = juce::Rectangle<int>{ position_x, 0, 2, getHeight() };

        g.fillRect(rect_marker);
    }
}

void PianoRollPreviewSurface::drawQuantizedInputRegionRectangle(juce::Graphics& g)
{
    juce::Graphics::ScopedSaveState save_state(g);

    if (quantizedInputRegionInSeconds.getLength() > 0.0)
    {
        g.setColour(kColourPianoRollQuantizedInputRegion);

        const auto position_start_x = timeToPositionX(quantizedInputRegionInSeconds.getStart(), rangeVisibleTimeInSeconds, getWidth());
        const auto position_end_x = timeToPositionX(quantizedInputRegionInSeconds.getEnd(), rangeVisibleTimeInSeconds, getWidth());
        juce::Rectangle<int> rect_region = 
            juce::Rectangle<int>{ position_start_x, 0, position_end_x - position_start_x, getHeight() };

        g.fillRect(rect_region);
    }
}

void PianoRollPreviewSurface::drawUserInputPositionCellRectangle(juce::Graphics& g)
{
    juce::Graphics::ScopedSaveState save_state(g);

    if (quantizedInputRegionInSeconds.getLength() > 0.0 && userInputPositionInNoteNumber >= 0)
    {
        const auto position_start_x = timeToPositionX(quantizedInputRegionInSeconds.getStart(), rangeVisibleTimeInSeconds, getWidth());
        const auto position_end_x = timeToPositionX(quantizedInputRegionInSeconds.getEnd(), rangeVisibleTimeInSeconds, getWidth());

        juce::Range<float> key_position_range = juce::Range<float>{ 0.0f, 0.0f };
        if (mapVisibleKeyNoteNumberToVerticalPositionRangeAsVerticalTopToBottom.count(userInputPositionInNoteNumber) > 0)
        {
            key_position_range = mapVisibleKeyNoteNumberToVerticalPositionRangeAsVerticalTopToBottom[userInputPositionInNoteNumber];
        }

        juce::Rectangle<float> rect_to_draw = juce::Rectangle<float>{
            (float)position_start_x,
            (float)key_position_range.getStart(),
            (float)position_end_x - position_start_x,
            (float)key_position_range.getLength(),
        };

        if (isInputPositionInsertable)
        {
            g.setColour(kColourPianoRollMousePositionInsertable);
        }
        else
        {
            g.setColour(kColourPianoRollMousePositionNotInsertable);
        }

        g.drawRect(rect_to_draw, 2.0f);
    }
}

//==============================================================================
void PianoRollPreviewSurface::updateMapVisibleKeyNoteNumberToVerticalPositionRange()
{
    mapVisibleKeyNoteNumberToVerticalPositionRangeAsVerticalTopToBottom.clear();

    for (int note_number = rangeVisibleKeyNoteNumbers.getStart();
        note_number < rangeVisibleKeyNoteNumbers.getEnd();
        note_number++)
    {
        const auto key_position_range_ignore_orientation = pianoRollKeyboardRef.getPositionRangeForPianoRollGridHorizontalRow(note_number);
        
        const juce::Range<float> key_position_range_vertical = 
            juce::Range<float>{
            getHeight() - key_position_range_ignore_orientation.getEnd(),
            getHeight() - key_position_range_ignore_orientation.getStart()
        };

        mapVisibleKeyNoteNumberToVerticalPositionRangeAsVerticalTopToBottom[note_number] = key_position_range_vertical;
    }


    for (int note_number = rangeVisibleKeyNoteNumbers.getStart();
        note_number < rangeVisibleKeyNoteNumbers.getEnd();
        note_number++)
    {
        juce::Logger::outputDebugString(
            juce::String(note_number) + " : " + 
            juce::String(mapVisibleKeyNoteNumberToVerticalPositionRangeAsVerticalTopToBottom[note_number].getStart()) + " : " +
            juce::String(mapVisibleKeyNoteNumberToVerticalPositionRangeAsVerticalTopToBottom[note_number].getEnd()) + " : " +
            juce::String(mapVisibleKeyNoteNumberToVerticalPositionRangeAsVerticalTopToBottom[note_number].getLength()));
    }
}

std::optional<juce::uint8>  PianoRollPreviewSurface::findNoteNumberWithVerticalPosition(float positionY)
{
    for (const auto& element : mapVisibleKeyNoteNumberToVerticalPositionRangeAsVerticalTopToBottom)
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

PianoRollPreviewSurface::NoteDrawInfo PianoRollPreviewSurface::createNoteDrawInfo(const cctn::song::SongEditorNoteExtended& note, const juce::Range<double> visibleRangeSeconds, int positionLeft, int positionRight)
{
    NoteDrawInfo result;

    // Convert time to position X.
    const double rect_left_x =
        juce::jmap<double>(note.startPositionInSeconds, visibleRangeSeconds.getStart(), visibleRangeSeconds.getEnd(), positionLeft, positionRight);

    const double rect_right_x =
        juce::jmap<double>(note.endPositionInSeconds, visibleRangeSeconds.getStart(), visibleRangeSeconds.getEnd(), positionLeft, positionRight);

    result.positionLeftX = rect_left_x;
    result.positionRightX = rect_right_x;
    result.lyric = note.lyric;
    result.noteNumber = note.noteNumber;
    result.isSelected = note.isSelected;

    return result;
}

}
}
