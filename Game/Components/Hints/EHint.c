/*!
Hint types.
When a hint is shown, its type, when defined, is stored persistently.
After that the hint won't be shown anymore.

Assign specific value to each entry!
Otherwise it may be shifted when new entries are added in front of it,
resulting in unintended type change in existing hints.

It's because values are saved as numbers in SCR_HintSettings.
*/
enum EHint
{
	UNDEFINED = 0, ///< Hint type will not be stored persistently if it has default value
	
	EDITOR_TOGGLE					= 100,
	
	EDITOR_MODE_EDIT				= 101,
	EDITOR_MODE_ADMIN				= 102,
	EDITOR_MODE_PHOTO_LIMITED		= 103,
	EDITOR_MODE_PHOTO_UNLIMITED		= 118,
	
	EDITOR_SELECT_ENTITY			= 104,
	EDITOR_SELECT_FACTION			= 105,
	EDITOR_SELECT_GROUP				= 106,
	
	EDITOR_PLACE					= 107,
	EDITOR_PLACE_CHARACTER			= 108,
	EDITOR_PLACE_VEHICLE			= 109,
	EDITOR_PLACE_WAYPOINT			= 123,
	EDITOR_PLACE_PLAYER				= 124,
	EDITOR_PLACE_TASK				= 110,
	
	EDITOR_ATTRIBUTES_GLOBAL		= 111,
	EDITOR_ATTRIBUTES_ENTITY		= 112,
	EDITOR_ATTRIBUTES_CAMERA		= 119,
	EDITOR_PHOTO_SCENE_PROPERTIES	= 126,
	
	EDITOR_CONTENT_BROWSER			= 113,
	EDITOR_MAP						= 114,
	EDITOR_TRANSFORM				= 115,
	EDITOR_EXTEND_SLOT				= 116,
	EDITOR_LAYER					= 117,
	EDITOR_CONTEXT_MENU				= 120,
	EDITOR_BUDGET					= 121,
	EDITOR_INTERACTION_VEHICLE		= 122,
	EDITOR_NOTIFICATION_EDITOR_ONLY	= 125,
	//								= 127
	
	MANUAL_CAMERA_MOVE				= 200,
	MANUAL_CAMERA_LIGHT				= 201,
	MANUAL_CAMERA_ATTACH			= 202,
	MANUAL_CAMERA_FOCUS				= 203,
	MANUAL_CAMERA_SAVE				= 204,
	MANUAL_CAMERA_ZOOM				= 205,
	MANUAL_CAMERA_ADJUST_SPEED		= 206,
	MANUAL_CAMERA_ROLL				= 207,
	
	GAME_MODE_EDITOR_NO_GM			= 300,
	GAME_MODE_EDITOR_PLAYER_LIST	= 301,
	
	//~ Guide
	GUIDE_EDITOR_INTRO				= 400,
	GUIDE_EDITOR_ACTIONBAR			= 401,
	GUIDE_EDITOR_ENTITIES			= 402,
	GUIDE_EDITOR_EXITING			= 404,
	GUIDE_PHOTO_INTRO				= 405,
};