`cctn::song::SongEditor` is service frontend.

`cctn::song::SongEditorDocument` is data model includes context.

`cctn::song::SongEditoryTypes.h` is collection of C++ type in this module.

The `cctn::song::SongEditorDocument` is designed to be compatible with the SongEditor user interface and QuantizeEngine. Specifically, the position at which a Note is placed is managed by the unit of beat position.

On the other hand, it is customary for synthetic speech engines to express Note data such as pronunciation timing in real time (seconds or sampling frames).
Therefore, it is necessary to implement a conversion process to make the Note data held in cctn::song::SongEditorDocument easy to handle for the synthetic speech engine. In this module, the utility class for this purpose is summarised as cctn::song::SongEditorDocumentExporter.
