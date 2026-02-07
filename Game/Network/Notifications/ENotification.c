/*!
Notification IDs used by SCR_NotificationsComponent.
*/
enum ENotification
{
	UNKNOWN = 0, ///< Default value to make sure this value is set.
	
	//PLAYER
	PLAYER_JOINED = 100, ///< A player joined - (Param1 = PlayerID)
	PLAYER_LEFT = 101, ///< A player left - (Param1 = PlayerID)
	PLAYER_KICKED = 102, ///< A player was kicked - (Param1 = PlayerID)
	PLAYER_BANNED = 103, ///< A player was banned - (Param1 = PlayerID, Duration)
	PLAYER_JOINED_FACTION = 104, ///A player joined a faction - (Param1 = PlayerID, Param2 = factionIndex)
	PLAYER_DIED = 105, ///< Player died - (Param1 = PlayerID)
	PLAYER_KILLED_PLAYER = 106, ///< Player killed a player - (Param1 = (killer)PlayerID, Param2 = (Killed Player)PlayerID)
	AI_KILLED_PLAYER = 107, ///< AI killed a Player - (Param1 = (killer)EditibleEntityID, Param2 = (killed player)PlayerID)
	PLAYER_LOADOUT_SAVED = 108, ///< Player changed their loadout
	POSSESSED_AI_DIED = 110, ///> Shown to GM only when possessed NPC died
	POSSESSED_AI_KILLED_PLAYER = 111, ///> Shown to GM only when possessed NPC killed a player
	PLAYER_KILLED_POSSESSED_AI = 112, ///> Shown to GM only when player killed a possessed NPC 
	AI_KILLED_POSSESSED_AI = 113, ///> Shown to GM only when player killed a possessed NPC 
	POSSESSED_AI_KILLED_POSSESSED_AI = 114, ///> Shown to GM only when possessed NPC killed a player
	PLAYER_BANNED_NO_DURATION = 115, ///< A player was banned for the session - (Param1 = PlayerID)
	
	//RIGHTS
	EDITOR_EDITOR_RIGHTS_ASSIGNED = 200, ///< Player got rights assigned - (Param1 = GameMasterID, Param2 = TargetPlayerID))
	EDITOR_EDITOR_RIGHTS_REMOVED = 201, ///< Player got rights revoked - (Param1 = GameMasterID, Param2 = TargetPlayerID)
	EDITOR_ADMIN_RIGHTS_ASSIGNED = 202, ///< Player got rights assigned - (Param1 = GameMasterID, Param2 = TargetPlayerID)
	EDITOR_ADMIN_RIGHTS_REMOVED = 203, ///< Player got rights revoked - (Param1 = GameMasterID, Param2 = TargetPlayerID)
	EDITOR_PHOTOMODE_RIGHTS_ASSIGNED = 204, ///< Player got rights assigned - (Param1 = GameMasterID, Param2 = TargetPlayerID)
	EDITOR_PHOTOMODE_RIGHTS_REMOVED = 205, ///< Player got rights revoked - (Param1 = GameMasterID, Param2 = TargetPlayerID)
	EDITOR_ENABLE_PROP_BUDGET = 206,
	EDITOR_DISABLE_PROP_BUDGET = 207,
	EDITOR_ENABLE_AI_BUDGET = 208,
	EDITOR_DISABLE_AI_BUDGET = 209,
	EDITOR_ENABLE_VEHICLE_BUDGET = 210,
	EDITOR_DISABLE_VEHICLE_BUDGET = 211,
	EDITOR_ENABLE_SYSTEM_BUDGET = 212,
	EDITOR_DISABLE_SYSTEM_BUDGET = 213,
	
	//PING
	EDITOR_PING_GM = 400, ///< GM pings - (Param1 = GameMasterID, Param2 = -1, Param3 positionX, Param4 positionY, Param4 positionZ)
	EDITOR_PING_GM_TARGET_ENTITY = 401, ///< GM pings at target entity - (Param1 = GameMasterID, Param2 = EditableEntityID)
	EDITOR_PING_GM_TARGET_PLAYER = 402, ///< GM pings at target player - (Param1 = GameMasterID, Param2 = TargetPlayerID)
	EDITOR_PING_PLAYER = 403, ///< Player pings - (Param1 = PlayerID, Param2 = -1, Param3 positionX, Param4 positionY, Param4 positionZ)
	EDITOR_PING_PLAYER_TARGET_ENTITY = 404, ///< Player pings at target entity - (Param1 = PlayerID, Param2 = EditableEntityID)
	EDITOR_PING_PLAYER_TARGET_PLAYER = 405, ///< Player pings at target player - (Param1 = PlayerID, Param2 = TargetPlayerID)
	EDITOR_PING_NO_GM_TO_PING = 406, ///< No GM to ping to notification	
	EDITOR_GM_ONLY_PING = 407, ///< GM only ping - (Param1 = GameMasterID, Param2 = -1, Param3 positionX, Param4 positionY, Param4 positionZ)
	EDITOR_GM_ONLY_PING_TARGET_ENTITY = 408, ///< GM only ping at target entity - (Param1 = GameMasterID, Param2 = EditableEntityID)
	EDITOR_GM_ONLY_PING_TARGET_PLAYER = 409, ///< GM only ping at target player - (Param1 = GameMasterID, Param2 = TargetPlayerID)
	EDITOR_GM_ONLY_PING_LIMITED_RIGHTS = 410, ///< Tries to ping GM only ping but has limited editor rights
	
	//EDITOR NOTIFICATIONS
	EDITOR_CANNOT_OPEN = 500, ///< Opening the editor is disabled or no modes are available
	EDITOR_CANNOT_CLOSE = 501, ///< Closing the editor is disabled
	EDITOR_TRANSFORMING_LOST_ACCESS = 502, ///< Transformed entity is no longer available - (Param1 = EditableEntityID)
	EDITOR_TRANSFORMING_INCORRECT_POSITION = 503, ///< Attempting to move entity to incorrect position
	EDITOR_TRANSFORMING_INCORRECT_TARGET = 504, ///< Attempting to move entity inside incompatible entity
	EDITOR_PLACING_BUDGET_MAX = 505,///< Max budget reached
	EDITOR_PLACING_CANNOT_AS_PLAYER = 506, ///< Attempting to place non-character entity as a player
	EDITOR_TRANSFORMING_FAIL = 507, ///< General error when server fails to transform the entity
	EDITOR_FACTION_NO_SPAWNS = 508, ///< Called if all spawnpoints are removed
	//EDITOR_FACTION_NO_TASKS = 509, ///< Called if all objectives are removed
	EDITOR_SESSION_SAVE_SUCCESS = 510, ///< Editor session saved
	EDITOR_SESSION_SAVE_FAIL = 511, ///< Editor session save was not successful for whatever reason
	EDITOR_SESSION_LOAD_SUCCESS = 512, ///< Editor session loaded
	EDITOR_SESSION_LOAD_FAIL = 513, ///< Editor session load was not successful for whatever reason
	EDITOR_SAVED_CAMERA_POSITION = 514, ///< When GM saved camera position (Param1 = camera int)
	EDITOR_LOADED_CAMERA_POSITION = 515, ///< When GM loaded camera position (Param1 = camera int)
	EDITOR_TASK_PLACED = 516,  ///< Objective was placed (Param1 = Task Replication ID, FactionIndex)
	EDITOR_TASK_COMPLETED = 517, ///< Objective was Completed (Param1 = Task Replication ID, FactionIndex)
	EDITOR_TASK_FAILED = 518, ///< Objective was Failed (Param1 = Task Replication ID, FactionIndex)
	EDITOR_TASK_DELETED = 519, ///< Objective was Deleted by GM (Param1 = Task Replication ID, FactionIndex)
	EDITOR_TASK_CANCELED = 520, ///< Objective was Canceled (Param1 = Task Replication ID, FactionIndex)
	EDITOR_GM_TELEPORTED_PLAYER = 521, ///< When the player gets teleported a notification will be send to that player (Param1 = GM teleported the player, player that was teleported (for GM to go to position))
	EDITOR_PLACING_BUDGET_MAX_FOR_VEHICLE_OCCUPANTS = 522,///< Max budget reached when trying to place occupants in vehicle
	
	//AI
	//EDITOR_AI_GROUP_ELIMINATED = 600, ///< An AI group was eliminated - (Param1 = EditableEntityID)
	
	//GLOBAL ATTRIBUTES
	EDITOR_ATTRIBUTES_FACTION_LOADOUT_CHANGED = 700, ///< GM changes faction loadout - (For now disabled as misses specific NotificationDisplay class)
	EDITOR_ATTRIBUTES_RESPAWN_ENABLED = 701, ///< GM enabled respawn - (Param1 = GameMasterID)
	EDITOR_ATTRIBUTES_RESPAWN_DISABLED = 702, ///< GM disabled respawn - (Param1 = GameMasterID)
	EDITOR_ATTRIBUTES_DATE_CHANGED = 703, ///< GM change the game date - (Param1 = GM, Param2 = day, Param3 = month, Param4 = year)
	EDITOR_ATTRIBUTES_TIME_CHANGED = 704, ///< GM change the game time  (Param1 = GM, Param2 = hours1:, Param3 = hours2, Param4 = minute1, Param5 = minute2
	EDITOR_ATTRIBUTES_WEATHER_CHANGED = 705, ///< GM change the game weather
	EDITOR_ATTRIBUTES_GM_BUDGET_CHANGED = 706, ///< GM change other GM editor budget - (Param1 = GameMasterID)
	EDITOR_ATTRIBUTES_FACTION_CHANGED = 707, ///< A GM enabling/disabled a faction - (Param1: GM)
	EDITOR_ATTRIBUTES_WIND_CHANGED = 709, ///< GM changed wind Speed and/or Direction - (Param1: GM)
	EDITOR_ATTRIBUTES_WIND_DEFAULT = 710, ///< GM set wind speed to be default instead of overriden - (Param1: GM)
	EDITOR_ATTRIBUTES_WEATHER_AUTO = 711, ///< Weather set to auto - (Param1: GM)
	EDITOR_ATTRIBUTES_DAY_DURATION_CHANGED = 712, ///< GM changed day duration - (Param1 = GM, Param2 = NewDuration)
	EDITOR_ATTRIBUTES_RESPAWN_TIME_CHANGED = 713, ///< GM changed respawn time - (Param1 = GM id, Param2 = NewTime)
	EDITOR_ATTRIBUTES_DAY_ADVANCE_ENABLED = 714, ///< If GM enables time advancement - (Param1: GM)
	EDITOR_ATTRIBUTES_DAY_ADVANCE_DISABLED = 715,///< If GM Disables time advancement - (Param1: GM)
	EDITOR_ATTRIBUTES_ENABLE_RESPAWN_ON_PLAYER = 719, //Called when GM enableds spawning on Radio operators (Param1 = GM)
	EDITOR_ATTRIBUTES_DISABLE_RESPAWN_ON_PLAYER = 720, //Called when GM disales spawning on Radio operators (Param1 = GM)
	EDITOR_ATTRIBUTES_FACTION_CHANGED_NO_GM = 721, //Called when GM places a spawnpoint but the faction was not enabled
	EDITOR_ATTRIBUTES_ENABLED_AMBIENT_MUSIC = 724, ///< When the GM enables server wide ambient music (Param1 = GM that enables it)
	EDITOR_ATTRIBUTES_DISABLED_AMBIENT_MUSIC = 725, ///< When the GM disables server wide ambient music (Param1 = GM that disables it)	EDITOR_GM_ENABLED_AMBIENT_MUSIC = 522, ///< When the GM enables server wide ambient music (Param1 = GM that enables it)
	//NOT IN GAME \/
	EDITOR_ATTRIBUTES_SERVER_NAME_CHANGED = 716, ///< GM change the server name
	EDITOR_ATTRIBUTES_SERVER_PASSWORD_CHANGED = 717, ///< GM change the server password
	EDITOR_ATTRIBUTES_SERVER_PLAYER_COUNT_CHANGED = 718, ///< GM change the server max player count - (Param1 = newPlayerCount)
	EDITOR_ATTRIBUTES_UNMUTE_MUSIC = 726, ///< When the GM enables server wide music (Param1 = GM that enables it) (Not in game)
	EDITOR_ATTRIBUTES_MUTE_MUSIC = 727, ///< When the GM disables server wide music (Param1 = GM that disables it) (Not in game)
	//NOT IN GAME /\
	EDITOR_CHANGED_KILLFEED_TYPE = 728, ///< Changed the killfeed type (Param1 = GM who changed setting, Param2 = The new KillfeedType, Param3 = bool isReciveType)
	EDITOR_CHANGED_KILLFEED_RECEIVE_TYPE = 729, ///< Changed killfeed receive type Param1 = GM who changed setting, Param2 = The new KillfeedReceiveType, Param3 = bool isReciveType))
	
	//GM
	EDITOR_PLAYER_BECAME_GM = 800, ///<Player become GM - (Param1 = PlayerID)
	EDITOR_GM_LEFT = 801, ///<Player become GM - (Param1 = PlayerID)
	EDITOR_PLAYER_NO_LONGER_GM = 802, ///<Player no longer GM - (Param1 = PlayerID)
	
	//CAMPAIGN
	BUILD_COST_MULTI_CHANGED = 900, ///<Cost multiplier to build compositions by players (not editor) 
	BUILD_REFUND_MULTI_CHANGED = 901,///<Refund multiplier for compositions by players (not editor)
	SUPPLY_TRUCK_LOADING_PLAYER = 902, /// Shows to truck occupants when vehicle is being loaded - (Param1 = player id, Param2 = amount)
	SUPPLY_TRUCK_UNLOADING_PLAYER = 903, /// Shows to truck occupants when vehicle is being unloaded - (Param1 = player id, Param2 = amount)
	SUPPLY_TRUCK_LOADING_PLAYER_STOPPED = 904, /// Shows to truck occupants when unloading player cancels interaction - (Param1 = player id)
	SUPPLY_TRUCK_UNLOADING_PLAYER_STOPPED = 905, /// Shows to truck occupants when unloading player cancels interaction  - (Param1 = player id)
	SUPPLY_TRUCK_LOADING_PLAYER_FINISHED = 906, /// Shows to truck occupants when loading player finishes interaction - (Param1 = player id)
	SUPPLY_TRUCK_UNLOADING_PLAYER_FINISHED = 907, /// Shows to truck occupants when unloading player finishes interaction - (Param1 = player id)
	
	//VOTING
	VOTING_EDITOR_IN_START = 1000, ///< Voting started to become GM (Param1 = player id)
	VOTING_EDITOR_IN_SUCCEED = 1001, ///< Voting succeeded to become GM (Param1 = player id)
	VOTING_EDITOR_IN_FAIL = 1002, ///< Voting failed to become GM (Param1 = player id)
	VOTING_EDITOR_OUT_START = 1003, ///< Voting started to remove GM (Param1 = player id)
	VOTING_EDITOR_OUT_SUCCEED = 1004, ///< Voting succeeded to remove GM (Param1 = player id)
	VOTING_EDITOR_OUT_FAIL = 1005, ///< Voting failed to remove GM (Param1 = player id)
	VOTING_KICK_START = 1006, ///< Voting started to kick player (Param1 = player id)
	VOTING_KICK_SUCCEED = 1007, ///< Voting succeeded to kick player (Param1 = player id)
	VOTING_KICK_FAIL = 1008, ///< Voting failed to kick player (Param1 = player id)
	
	//GROUPS 
	GROUPS_PLAYER_JOINED = 1101, ///< Player joined the group
	GROUPS_PLAYER_LEFT = 1102, ///< Player has left the group
	GROUPS_PLAYER_PROMOTED_LEADER = 1103, ///< Player has been promoted to group leader
	
	//OTHERS
	ACTION_ON_COOLDOWN = 1200, ///< Action is on cooldown and cannot be executed
};

