/*******************************************************************************
 The block below describes the properties of this module, and is read by
 the Projucer to automatically generate project code that uses it.
 For details about the syntax and how to create or use a module, see the
 JUCE Module Format.md file.


 BEGIN_JUCE_MODULE_DECLARATION

  ID:                 cocotone_song_editor_basics
  vendor:             COCOTONE
  version:            0.0.1
  name:               JUCE extended classes for singing synthesizer.
  description:        Classes for creating singing synthesizer application with JUCE framework.
  website:            https://github.com/cocotone/cocotone_song_editor_basics
  license:            BSD 3-Clause License
  minimumCppStandard: 17

  dependencies:       juce_core juce_audio_basics juce_audio_utils juce_gui_basics

 END_JUCE_MODULE_DECLARATION

*******************************************************************************/


#pragma once
#define COCOTONE_SONG_EDITOR_BASICS_H_INCLUDED

#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_gui_basics/juce_gui_basics.h>

//==============================================================================

#include "SongEditor/cocotone_SongEditorTypes.h"

#include "SongEditor/cocotone_IPositionInfoProvider.h"
#include "SongEditor/Document/cocotone_SongDocument.h"
#include "SongEditor/Transport/cocotone_BeatTimeUtility.h"
#include "SongEditor/Quantize/cocotone_QuantizeEngine.h"
#include "SongEditor/Document/cocotone_ISongEditorDocumentObserver.h"
#include "SongEditor/Document/cocotone_SongEditorDataModel.h"
#include "SongEditor/Document/cocotone_SongEditorDocument.h"
#include "SongEditor/cocotone_SongEditorCommand.h"
#include "SongEditor/cocotone_SongEditor.h"
#include "SongEditor/Transport/cocotone_TransportEmulator.h"
