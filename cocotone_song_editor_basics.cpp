#include "cocotone_song_editor_basics.h"

//==============================================================================
#include "SongEditor/Lyric/cocotone_MoraKana.h"

#include "SongEditor/View/Track/cocotone_ITrackDataAccessDelegate.h"
#include "SongEditor/View/Track/cocotone_TrackComponentBase.h"
#include "SongEditor/View/Track/cocotone_TrackHeaderBase.h"
#include "SongEditor/View/Track/cocotone_TrackLaneBase.h"
#include "SongEditor/View/Track/Impl/cocotone_TimeSignatureTrack.h"
#include "SongEditor/View/Track/Impl/cocotone_TempoTrack.h"
#include "SongEditor/View/Track/Impl/cocotone_MusicalTimePreviewTrack.h"
#include "SongEditor/View/Track/Impl/cocotone_AbsoluteTimePreviewTrack.h"
#include "SongEditor/View/Track/Impl/cocotone_VocalTrack.h"
#include "SongEditor/View/Track/Impl/cocotone_BackingTrack.h"
#include "SongEditor/View/Track/cocotone_MultiTrackEditor.h"

#include "SongEditor/View/PianoRoll/cocotone_PianoRollColours.h"
#include "SongEditor/View/PianoRoll/cocotone_PianoRollPositionUtility.h"
#include "SongEditor/View/PianoRoll/cocotone_PianoRollKeyboard.h"
#include "SongEditor/View/PianoRoll/cocotone_PianoRollTimeRuler.h"
#include "SongEditor/View/PianoRoll/cocotone_PianoRollEventDispatcher.h"
#include "SongEditor/View/PianoRoll/cocotone_PianoRollPreviewSurface.h"
#include "SongEditor/View/PianoRoll/cocotone_PianoRollInteractionSurface.h"

#include "SongEditor/Quantize/cocotone_QuantizeEngine.h"

#include "SongEditor/cocotone_SongEditorCommand.h"

//==============================================================================
#include "SongEditor/Lyric/cocotone_MoraKana.cpp"

#include "SongEditor/View/Track/Impl/cocotone_TimeSignatureTrack.cpp"
#include "SongEditor/View/Track/Impl/cocotone_TempoTrack.cpp"
#include "SongEditor/View/Track/Impl/cocotone_MusicalTimePreviewTrack.cpp"
#include "SongEditor/View/Track/Impl/cocotone_AbsoluteTimePreviewTrack.cpp"
#include "SongEditor/View/Track/Impl/cocotone_VocalTrack.cpp"
#include "SongEditor/View/Track/Impl/cocotone_BackingTrack.cpp"
#include "SongEditor/View/Track/cocotone_MultiTrackEditor.cpp"

#include "SongEditor/View/PianoRoll/cocotone_PianoRollKeyboard.cpp"
#include "SongEditor/View/PianoRoll/cocotone_PianoRollTimeRuler.cpp"
#include "SongEditor/View/PianoRoll/cocotone_PianoRollEventDispatcher.cpp"
#include "SongEditor/View/PianoRoll/cocotone_PianoRollPreviewSurface.cpp"
#include "SongEditor/View/PianoRoll/cocotone_PianoRollInteractionSurface.cpp"

#include "SongEditor/Transport/cocotone_TransportEmulator.cpp"

#include "SongEditor/Quantize/cocotone_QuantizeEngine.cpp"

#include "SongEditor/Document/cocotone_SongDocument.cpp"
#include "SongEditor/Document/cocotone_SongDocumentEditor.cpp"
#include "SongEditor/Document/cocotone_SongDocumentTranspiler.cpp"

#include "SongEditor/cocotone_SongEditorCommand.cpp"
#include "SongEditor/cocotone_SongEditor.cpp"
