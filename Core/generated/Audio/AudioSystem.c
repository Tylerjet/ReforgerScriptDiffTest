/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Audio
\{
*/

sealed class AudioSystem
{
	private void AudioSystem();
	private void ~AudioSystem();

	// Mastering voices categories.
	static const int SFX = 0;
	static const int Music = 1;
	static const int VoiceChat = 2;
	static const int Dialog = 3;
	static const int UI = 4;

	static const int DefaultOutputState = 0;
	static const int BV_None = 0;
	static const int BV_Sphere = 1;
	static const int BV_Box = 2;
	static const int BV_Cylinder = 3;

	//! Play *.wav file.
	static proto AudioHandle PlaySound(string path);
	//! Preload audio file(*.acp).
	static proto bool PlayEventInitialize(string filename);
	static proto AudioHandle PlayEvent(string filename, string eventname, vector transf[], array<string> names = null, array<float> values = null);
	static proto bool IsSoundPlayed(AudioHandle handle);
	static proto void TerminateSound(AudioHandle handle);
	//! Set transformation for given audio handle, return FALSE if audio handle is not valid.
	static proto bool SetSoundTransformation(AudioHandle handle, vector transf[]);
	//! Return -1.0 for the inaudible event, otherwise return distance from passed position to listener.
	static proto float IsAudible(string filepath, string eventName, vector position);
	/*
	Allow update bounding volume for existing audio source.
		BV_Sphere	: param0 = radius
		BV_Box		: param0 = x, param1 = y, param2 = z
		BV_Cylinde	: param0 = radius, param1 = height
	*/
	static proto bool SetBoundingVolumeParams(AudioHandle handle, int volumeType, float params0, float param1, float param2);
	//------------------------------------------------------------------------------------------------
	static proto float OutputVolume();
	//------------------------------------------------------------------------------------------------
	static proto bool SetMasterVolume(int id, float volume);
	static proto float GetMasterVolume(int id);
	//------------------------------------------------------------------------------------------------
	static proto bool CreateOutputState(string filename);
	static proto int GetOutpuStateSignalIdx(int outStateIdx, string signal);
	static proto void SetOutputStateSignal(int outStateIdx, int idx, float value);
	//! Returns ID of a variable by Name.
	static proto int GetVariableIDByName(string varName, string filename);
	//! Returns Name of a variable by ID.
	static proto string GetVariableNameByID(int ID, string filename);
	//! Returns current value of a variable by Name.
	static proto float GetVariableValue(string varName, string filename);
	//! Returns current value of a variable by ID.
	static proto float GetVariableByID(int ID, string filename);
	//! Returns false if variable does not exist.
	static proto bool SetVariableByName(string varName, float value, string filename);
	//! Returns false if variable does not exist.
	static proto bool SetVariableByID(int ID, float value, string filename);
	//! Resets all applicable values to 0 or default value
	static proto void ResetVariables();
}

/*!
\}
*/
