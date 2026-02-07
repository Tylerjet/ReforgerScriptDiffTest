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
	proto external void SetName(string name);
	//! Get Charactger alias
	proto external string GetAlias();
	proto external void SetAlias(string alias);
	//! Get Character surname
	proto external string GetSurname();
	proto external void SetSurname(string surname);
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
