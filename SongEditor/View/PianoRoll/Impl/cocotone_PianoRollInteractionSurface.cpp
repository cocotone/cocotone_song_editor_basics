namespace cctn
{
namespace song
{

//==============================================================================
PianoRollInteractionSurface::PianoRollInteractionSurface(
    cctn::song::PianoRollPreviewSurface& previewSurface, cctn::song::PianoRollEventDispatcher& eventDispatcher)
    : pianoRollPreviewSurface(previewSurface)
    , pianoRollEventDispatcher(eventDispatcher)
{

}

PianoRollInteractionSurface::~PianoRollInteractionSurface()
{

}

//==============================================================================
void PianoRollInteractionSurface::paint(juce::Graphics& g)
{

}

void PianoRollInteractionSurface::resized()
{

}

//==============================================================================
void PianoRollInteractionSurface::mouseMove(const juce::MouseEvent& mouseEvent)
{
    pianoRollPreviewSurface.emitMouseEvent(mouseEvent, false);
}

void PianoRollInteractionSurface::mouseEnter(const juce::MouseEvent& mouseEvent)
{
    pianoRollPreviewSurface.emitMouseEvent(mouseEvent, false);
}

void PianoRollInteractionSurface::mouseExit(const juce::MouseEvent& mouseEvent)
{
    pianoRollPreviewSurface.emitMouseEvent(mouseEvent, true);
}

void PianoRollInteractionSurface::mouseDown(const juce::MouseEvent& mouseEvent)
{

}

void PianoRollInteractionSurface::mouseDrag(const juce::MouseEvent& mouseEvent)
{
}

void PianoRollInteractionSurface::mouseUp(const juce::MouseEvent& mouseEvent)
{
    const auto query_optional = pianoRollPreviewSurface.getQueryForFindPianoRollNote(mouseEvent);
    if (query_optional.has_value())
    {
        const auto query = query_optional.value();
        const bool is_note_exist = pianoRollEventDispatcher.emitTestNoteExist(query);
        if (is_note_exist)
        {
            if (mouseEvent.mods.isRightButtonDown())
            {
                pianoRollEventDispatcher.emitDeleteNoteSingle(query);
            }
            else
            {
                pianoRollEventDispatcher.emitSelectNote(query);
            }
        }
        else
        {
            if (mouseEvent.mods.isLeftButtonDown())
            {
                cctn::song::QueryForAddPianoRollNote query_for_add;
                query_for_add.startTimeInSeconds = query.timeInSeconds;
                query_for_add.endTimeInSeconds = query.timeInSeconds + 0.500;
                query_for_add.noteNumber = query.noteNumber;
                query_for_add.snapToQuantizeGrid = true;
                pianoRollEventDispatcher.emitAddNote(query_for_add);

                cctn::song::QueryForFindPianoRollNote query_for_select;
                query_for_select.noteNumber = query_for_add.noteNumber;
                query_for_select.timeInSeconds = query_for_add.startTimeInSeconds;
                pianoRollEventDispatcher.emitSelectNote(query_for_select);
            }
        }
    }
}

}
}
