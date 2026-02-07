/**
@defgroup Editor Editor
\image html editor_logo.png

Editor is a tool for run-time world observation and manipulation.

It's designed to provide a platform for many specialized features.

## Technical Implementation
### Editor
- It's created for each player by **editor core** (SCR_EditorManagerCore).
- Locally it's controlled by **editor manager** (SCR_EditorManagerEntity).
- Various **modes** handle specialized functionality (SCR_EditorModeEntity).
- **Editor components** are responsible for individual features (SCR_BaseEditorComponent).

### Editable Entities
- Various world entities like vehicles or props can be registered for use in the editor.
- **Core of editable entities** (SCR_EditableEntityCore) holds a list of all entities.
- Individual entities are identified by a **component** (SCR_EditableEntityComponent).

*/