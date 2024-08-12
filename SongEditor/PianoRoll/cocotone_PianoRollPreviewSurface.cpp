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
    , scopedSongDocumentPtrToPaint(nullptr)
    , visibleGridVerticalLineType(juce::var((int)VisibleGridVerticalType::kTimeSignature))
{
    numVisibleWhiteAndBlackKeys = 12 * numVisibleOctaves;
    numVisibleWhiteKeys = 7 * numVisibleOctaves;
    rangeVisibleKeyNoteNumbers = juce::Range<juce::uint8>(48, 48 + numVisibleWhiteAndBlackKeys);

    rangeVisibleTimeInSeconds = juce::Range<double>(0.0f, 60.0f);
}

PianoRollPreviewSurface::~PianoRollPreviewSurface()
{
    if (!documentEditorForPreviewPtr.expired())
    {
        documentEditorForPreviewPtr.lock()->removeChangeListener(this);
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

//==============================================================================
void PianoRollPreviewSurface::setPlayingPositionInSeconds(double positionInSeconds)
{
    playingPositionInSeconds = positionInSeconds;
    repaint();
}

void PianoRollPreviewSurface::setCurrentPositionInfo(const juce::AudioPlayHead::PositionInfo& positionInfo)
{
    currentPositionInfo = positionInfo;
    repaint();
}

//==============================================================================
void PianoRollPreviewSurface::setUserInputPositionInSeconds(double positionInSeconds)
{
    userInputPositionInSeconds = positionInSeconds;
    repaint();
}

void PianoRollPreviewSurface::setDrawingGridInterval(cctn::song::NoteLength intervalLength)
{
    drawingGirdVerticalInterval = (int)intervalLength;
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
void PianoRollPreviewSurface::setDocumentForPreview(std::shared_ptr<cctn::song::SongDocumentEditor> documentEditor)
{
    if (!documentEditorForPreviewPtr.expired())
    {
        if (documentEditorForPreviewPtr.lock().get() != documentEditor.get())
        {
            documentEditorForPreviewPtr.lock()->removeChangeListener(this);
            documentEditorForPreviewPtr.reset();
        }
    }

    documentEditorForPreviewPtr = documentEditor;

    if (!documentEditorForPreviewPtr.expired())
    {
        documentEditorForPreviewPtr.lock()->addChangeListener(this);
    }

    repaint();
}

//==============================================================================
void PianoRollPreviewSurface::paint(juce::Graphics& g)
{
    juce::Graphics::ScopedSaveState save_state(g);

#if 0
    const cctn::song::SongEditorDocumentData* document_data_to_paint = nullptr;
    if (!documentEditorForPreviewPtr.expired() &&
        documentEditorForPreviewPtr.lock()->getRawDocumentData().has_value())
    {
        document_data_to_paint = documentEditorForPreviewPtr.lock()->getRawDocumentData().value();
    }

    juce::ScopedValueSetter<const cctn::song::SongEditorDocumentData*> svs(paintScopedDocumentDataPtr, document_data_to_paint);
#endif

    const cctn::song::SongDocument* document_to_paint = nullptr;
    if (!documentEditorForPreviewPtr.expired() &&
        documentEditorForPreviewPtr.lock()->getCurrentDocument().has_value())
    {
        document_to_paint = documentEditorForPreviewPtr.lock()->getCurrentDocument().value();
    }

    juce::ScopedValueSetter<const cctn::song::SongDocument*> svs(scopedSongDocumentPtrToPaint, document_to_paint);

    updateViewContext();

    g.fillAll(kColourMainBackground);

    fillGridHorizontalRows(g);
    drawGridHorizontalLines(g);

    if ((VisibleGridVerticalType)(int)visibleGridVerticalLineType.getValue() == VisibleGridVerticalType::kNone)
    {
    }
    else if ((VisibleGridVerticalType)(int)visibleGridVerticalLineType.getValue() == VisibleGridVerticalType::kTimeSeconds)
    {
        drawGridVerticalLinesInTimeSecondsDomain(g);
    }
    else if ((VisibleGridVerticalType)(int)visibleGridVerticalLineType.getValue() == VisibleGridVerticalType::kTimeSignature)
    {
        drawGridVerticalLinesInTimeSignatureDomain(g);
    }
    else if ((VisibleGridVerticalType)(int)visibleGridVerticalLineType.getValue() == VisibleGridVerticalType::kQuantize)
    {
    }

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
    if (!documentEditorForPreviewPtr.expired())
    {
        if (source == documentEditorForPreviewPtr.lock().get())
        {
            repaint();
        }
    }
}

double calculate_note_end_time(double x_start, double x_end, NoteLength x_length, NoteLength y_length)
{
    // Calculate the duration of the X note
    double x_duration = x_end - x_start;

    // Get the note values
    double x_value = getNoteValue(x_length);
    double y_value = getNoteValue(y_length);

    // Calculate the tempo in BPM
    double tempo = 60.0 * 4.0 / (x_duration * x_value);

    // Calculate the duration of one beat (quarter note) in seconds
    double beat_duration = 60.0 / tempo;

    // Calculate the duration of the Y note
    double y_duration = beat_duration * 4.0 / y_value;

    // Calculate and return the end time of the Y note
    return x_start + y_duration;
}

//==============================================================================
void PianoRollPreviewSurface::updateViewContext()
{
    JUCE_ASSERT_MESSAGE_MANAGER_EXISTS;

    if (scopedSongDocumentPtrToPaint == nullptr)
    {
        return;
    }

    const auto& notes = scopedSongDocumentPtrToPaint->getNotes();

    // Update input position is insertable or not.
    isInputPositionInsertable = true;
    for (const auto& note : notes)
    {
        const cctn::song::SongDocument& document = *scopedSongDocumentPtrToPaint;

        const auto musical_time_note_on = note.startTimeInMusicalTime;
        const int64_t tickOnPosition = cctn::song::SongDocument::Calculator::barToTick(document, musical_time_note_on);

        const auto musical_time_note_off = cctn::song::SongDocument::Calculator::calculateNoteOffPosition(document, note);
        const int64_t tickOffPosition = cctn::song::SongDocument::Calculator::barToTick(document, musical_time_note_off);

        const double start_position_in_seconds = cctn::song::SongDocument::Calculator::tickToAbsoluteTime(document, tickOnPosition);
        const double end_position_in_seconds = cctn::song::SongDocument::Calculator::tickToAbsoluteTime(document, tickOffPosition);

        if (juce::Range<float>(start_position_in_seconds, end_position_in_seconds).contains(userInputPositionInSeconds))
        {
            isInputPositionInsertable = false;
            break;
        }
    }

    currentBeatTimePointList = documentEditorForPreviewPtr.lock()->getEditorContext().currentBeatTimePointList;

    // Update input region in seconds
    quantizedInputRegionInSeconds = juce::Range<double>{ 0.0f, 0.0f };
    if (!documentEditorForPreviewPtr.expired() &&
        documentEditorForPreviewPtr.lock()->getCurrentDocument().has_value())
    {
        const auto region_optional = documentEditorForPreviewPtr.lock()->findNearestQuantizeRegion(userInputPositionInSeconds);
        if (region_optional.has_value())
        {
            // Get the note values
            const auto note_end_position_in_seconds =
                calculate_note_end_time(
                    region_optional.value().startPositionInSeconds, 
                    region_optional.value().endPositionInSeconds, 
                    documentEditorForPreviewPtr.lock()->getEditorContext().currentGridInterval,
                    documentEditorForPreviewPtr.lock()->getEditorContext().currentNoteLength);

            quantizedInputRegionInSeconds =
                juce::Range<double>{ region_optional.value().startPositionInSeconds, note_end_position_in_seconds };
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

        juce::Rectangle<float> rect_to_fill = juce::Rectangle<float>{
            0.0f,
            (float)key_position_range.getStart(),
            (float)getWidth(),
            key_position_range.getLength()
        };

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

        juce::Rectangle<float> rect_to_fill = 
            juce::Rectangle<float>{ 0.0f, (float)key_position_range.getStart(), (float)getWidth(), key_position_range.getLength() };

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

void PianoRollPreviewSurface::drawGridVerticalLinesInTimeSecondsDomain(juce::Graphics& g)
{
    juce::Graphics::ScopedSaveState save_state(g);

    const auto vertical_line_positions = createVerticalLinePositionsInTimeSecondsDomain(rangeVisibleTimeInSeconds, verticalLineIntervalInSeconds, getWidth());

    for (const auto& position_with_time_info : vertical_line_positions)
    {
        g.setColour(juce::Colours::white);
        g.drawVerticalLine(position_with_time_info.positionX, 0.0f, getHeight());
    }
}

void PianoRollPreviewSurface::drawGridVerticalLinesInTimeSignatureDomain(juce::Graphics& g)
{
    juce::Graphics::ScopedSaveState save_state(g);

    // NOTE: Make abstract.
    double bpm = 120.0;
    int numerator = 4;
    int denominator = 4;

    const auto tempo_and_time_signature_optional = cctn::song::PositionInfoExtractor::extractTempoAndTimeSignature(currentPositionInfo);
    if (tempo_and_time_signature_optional.has_value())
    {
        bpm = tempo_and_time_signature_optional.value().bpm;
        numerator = tempo_and_time_signature_optional.value().numerator;
        denominator = tempo_and_time_signature_optional.value().denominator;
    }

    if (scopedSongDocumentPtrToPaint == nullptr)
    {
        return;
    }

    // NOTE: This procedure will fit to feature of tempo map track.
    //const auto precise_beat_and_time_array = cctn::song::BeatTimePointFactory::extractPreciseBeatPoints(bpm, numerator, denominator, rangeVisibleTimeInSeconds.getStart(), rangeVisibleTimeInSeconds.getEnd(), (cctn::song::NoteLength)(int)drawingGirdVerticalInterval.getValue());
    //const auto precise_beat_and_time_array = cctn::song::BeatTimePointFactory::extractPreciseBeatPoints(*scopedSongDocumentPtrToPaint, rangeVisibleTimeInSeconds.getStart(), rangeVisibleTimeInSeconds.getEnd(), (cctn::song::NoteLength)(int)drawingGirdVerticalInterval.getValue());
    const auto& precise_beat_and_time_array = currentBeatTimePointList;

    const auto vertical_line_positions = createVerticalLinePositionsInTimeSignatureDomain(rangeVisibleTimeInSeconds, precise_beat_and_time_array, getWidth());

    for (const auto& position_with_time_info : vertical_line_positions)
    {
        g.setColour(juce::Colours::white);
        g.drawVerticalLine(position_with_time_info.positionX, 0.0f, getHeight());
    }
}

void PianoRollPreviewSurface::drawGridVerticalLinesInQuantizeDomain(juce::Graphics& g)
{
    juce::Graphics::ScopedSaveState save_state(g);
}

void PianoRollPreviewSurface::drawCurrentPreviewData(juce::Graphics& g)
{
    juce::Graphics::ScopedSaveState save_state(g);

    if (scopedSongDocumentPtrToPaint == nullptr)
    {
        return;
    }

    const auto& notes = scopedSongDocumentPtrToPaint->getNotes();

    for (const auto& note : notes)
    {
        const auto note_draw_info = createNoteDrawInfo(*scopedSongDocumentPtrToPaint, note, rangeVisibleTimeInSeconds, 0, getWidth());
        
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

    if (quantizedInputRegionInSeconds.getLength() > 0.0 && userInputPositionInNoteNumber >= 0)
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
juce::Array<PianoRollPreviewSurface::PositionWithTimeInfo> 
PianoRollPreviewSurface::createVerticalLinePositionsInTimeSecondsDomain(const juce::Range<double> visibleRangeSeconds, double timeStepInSeconds, int width)
{
    juce::Array<PositionWithTimeInfo> positions;

    std::vector<double> time_secnds_vertical_lines;
    {
        // Find the smallest multiple of 10 within the range
        double min_units = static_cast<double>(std::ceil(visibleRangeSeconds.getStart() / timeStepInSeconds)) * (double)timeStepInSeconds;

        // Find the largest multiple of 10 within the range
        double max_units = static_cast<double>(std::floor(visibleRangeSeconds.getEnd() / timeStepInSeconds)) * (double)timeStepInSeconds;

        // List up multiples of 10 within the range
        for (double value = min_units; value <= max_units; value += (double)timeStepInSeconds)
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
            position_with_time_info.positionX = (int)position_x;
            position_with_time_info.timeInSeconds = time_seconds_vertical_line;

            positions.add(position_with_time_info);
        }
    }

    return positions;
}

juce::Array<PianoRollPreviewSurface::PositionWithTimeInfo> 
PianoRollPreviewSurface::createVerticalLinePositionsInTimeSignatureDomain(const juce::Range<double> visibleRangeSeconds, const BeatTimePointList& beatTimePoints, int width)
{
    juce::Array<PositionWithTimeInfo> positions;

    if (beatTimePoints.empty() || width <= 0)
    {
        return positions;
    }

    // Find the start and end indices in beatTimePoints that fall within the visible range
    auto start_iter = std::lower_bound(beatTimePoints.begin(), beatTimePoints.end(), visibleRangeSeconds.getStart(),
        [](const BeatTimePoint& btp, double time) 
        { 
            return btp.timeInSeconds < time;
        });

    auto end_iter = std::upper_bound(beatTimePoints.begin(), beatTimePoints.end(), visibleRangeSeconds.getEnd(),
        [](double time, const BeatTimePoint& btp)
        { 
            return time < btp.timeInSeconds;
        });

    if (start_iter == beatTimePoints.end() || end_iter == beatTimePoints.begin())
    {
        return positions;
    }

    // If the first visible beat is after the start of the visible range, include the previous beat
    if (start_iter != beatTimePoints.begin() && start_iter->timeInSeconds > visibleRangeSeconds.getStart())
    {
        --start_iter;
    }

    double visible_duration_in_seconds = visibleRangeSeconds.getLength();

    for (auto it = start_iter; it != end_iter; ++it)
    {
        if (it->timeInSeconds >= visibleRangeSeconds.getStart() && it->timeInSeconds <= visibleRangeSeconds.getEnd())
        {
            // Convert time to position X.
            const double position_x =
                juce::jmap<double>(it->timeInSeconds, visibleRangeSeconds.getStart(), visibleRangeSeconds.getEnd(), 0, width);

            PositionWithTimeInfo pos;
            pos.positionX = (int)position_x;
            pos.timeInSeconds = it->timeInSeconds;

            positions.add(pos);
        }
    }

    return positions;
}

#if 0
PianoRollPreviewSurface::NoteDrawInfo 
PianoRollPreviewSurface::createNoteDrawInfo(const cctn::song::SongEditorNoteExtended& note, const juce::Range<double> visibleRangeSeconds, int positionLeft, int positionRight)
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
#endif

PianoRollPreviewSurface::NoteDrawInfo
PianoRollPreviewSurface::createNoteDrawInfo(const cctn::song::SongDocument& document, const cctn::song::SongDocument::Note& note, const juce::Range<double> visibleRangeSeconds, int positionLeft, int positionRight)
{
    NoteDrawInfo result;
    
    const auto musical_time_note_on = note.startTimeInMusicalTime;
    const int64_t tickOnPosition = cctn::song::SongDocument::Calculator::barToTick(document, musical_time_note_on);

    const auto musical_time_note_off = cctn::song::SongDocument::Calculator::calculateNoteOffPosition(document, note);
    const int64_t tickOffPosition = cctn::song::SongDocument::Calculator::barToTick(document, musical_time_note_off);

    const double start_position_in_seconds = cctn::song::SongDocument::Calculator::tickToAbsoluteTime(document, tickOnPosition);
    const double end_position_in_seconds = cctn::song::SongDocument::Calculator::tickToAbsoluteTime(document, tickOffPosition);

    // Convert time to position X.
    const double rect_left_x =
        juce::jmap<double>(start_position_in_seconds, visibleRangeSeconds.getStart(), visibleRangeSeconds.getEnd(), positionLeft, positionRight);

    const double rect_right_x =
        juce::jmap<double>(end_position_in_seconds, visibleRangeSeconds.getStart(), visibleRangeSeconds.getEnd(), positionLeft, positionRight);

    result.positionLeftX = rect_left_x;
    result.positionRightX = rect_right_x;
    result.lyric = note.lyric;
    result.noteNumber = note.noteNumber;
    result.isSelected = false;  // TODO: add support

    return result;
}

}
}
