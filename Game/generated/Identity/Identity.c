/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup Identity
* @{
*/

class Identity: ScriptAndConfig
{
	//! Get Character name
	proto external string GetName();
	//! Get Charactger alias
	proto external string GetAlias();
	//! Get Character surname
	proto external string GetSurname();
	//! Get Character Name "Alias" Surname in on string
	proto external string GetFullName();
	//! Get Char head prefab
	proto external ResourceName GetHead();
	//! Get Character body prefab
	proto external ResourceName GetBody();
	//! Get character voice type ID
	proto external int GetVoiceID();
	//! Get Character voice pitch
	proto external int GetPitch();
	// <ID, pitch>
	proto external MeshConfig GetMeshConfig();
};

/** @}*/
