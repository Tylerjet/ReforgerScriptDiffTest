/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup Entities
* @{
*/

class MusicManagerClass: GenericEntityClass
{
};

class MusicManager: GenericEntity
{
	//! Returns if the music manager is muted (wont play any music) or not
	proto external bool IsMuted();
	proto external bool IsAmbientMusicAuthorizedByServer();
	//! Get index for signal name. Return -1 if signal not found.
	proto external int GetSignalIndex(string name);
	//! Set signal value by 'index'. Index is obtained by GetSignalIndex method.
	proto external void SetSignalValue(int index, float value);
	//! Set signal value by 'name'.
	proto external void SetSignalValueStr(string signal, float value);
};

/** @}*/
