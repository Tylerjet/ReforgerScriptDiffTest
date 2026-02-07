/*
===========================================
Do not modify, this script is generated
===========================================
*/

class MusicManagerController
{
	/*!
	Requests the music controller to play ONCE the song requested
	\param song: Sound to play
	\param force: If true, it will play anyways
	\param persistent: if true, it won't get stopped by music disruptions (bullets/explosions)
	\param priority: 0 = min priority. Only one requested sound will play per frame.
	*/
	proto external void PlayOneShot(string song, bool force, bool persistent, int priority = 0);
	/*!
	Requests the music manager to mute/unmute its clients
	\param muteValue: true to mute clients, false to unmute them
	*/
	proto external void MuteClients(bool muteValue);
	/*!
	Enables ambient music specifically for current instance of the game.
	Careful! For a client to play ambient music they need to be authorized by the server + have it enabled!
	Ambient music will NOT be enabled without server authorization.
	\param muteValue: true to enable ambient music in this instance of the game, false prevent them from playing
	*/
	proto external void EnableAmbientMusic(bool value);
	/*!
	Enables/disables ambient music for all clients
	\param muteValue: true to enable ambient music, false prevent it from playing
	*/
	proto external void AuthorizeAmbientMusicForClients(bool value);
	proto external bool IsAmbientMusicEnabled();
	/*!
	Terminates the song currently playing. This ignores a tracks persistency.
	If there is no need to ignore track persistance use AddMusicDisruption.
	*/
	proto external void Terminate();
	//! Returns the music manager that owns the music controller
	proto external MusicManager GetMusicManager();
	//! Returns the music controller, return can be null. Only use this when you cannot access ChimeraWorld from the current script.
	static proto MusicManagerController GetController();
};
