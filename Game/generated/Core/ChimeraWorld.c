/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Core
\{
*/

sealed class ChimeraWorld: BaseWorld
{
	proto external MusicManager GetMusicManager();
	proto external SoundWorld GetSoundWorld();
	/*!
	Pause game time after the current frame. This will tick only specified entities in the world. This is working only in single-player.
	Only FRAME and POSTFRAME events will be fired for entities that have been registered to be updated while the game is paused.
	*/
	proto external void PauseGameTime(bool state);
	proto external bool IsGameTimePaused();
	//! Register entity to be updated while the game is paused. Deleting of entity is handled correctly on the C++ side.
	proto external void RegisterEntityToBeUpdatedWhileGameIsPaused(IEntity entity);
	//! Unregister the entity, it will not be updated anymore while the game is paused.
	proto external void UnregisterEntityToBeUpdatedWhileGameIsPaused(IEntity entity);
}

/*!
\}
*/
