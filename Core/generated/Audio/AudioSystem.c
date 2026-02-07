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

	//! Play *.wav file.
	static proto AudioHandle PlaySound(string path);
	//! Preload audio file(*.acp).
	static proto bool PlayEventInitialize(string filename);
	static proto AudioHandle PlayEvent(string filename, string eventname, vector transf[], array<string> names, array<float> values);
	static proto bool IsSoundPlayed(AudioHandle handle);
	static proto void TerminateSound(AudioHandle handle);
	//! Set transformation for given audio handle, return FALSE if audio handle is not valid.
	static proto bool SetSoundTransformation(AudioHandle handle, vector transf[]);
	//! Return -1.0 for the inaudible event, otherwise return distance from passed position to listener.
	static proto float IsAudible(string filepath, string eventName, vector position);
	//! Return value current value of a variable.
	static proto float GetVariableValue(string varname, string filename);
	//------------------------------------------------------------------------------------------------
	static proto float OutputVolume();
	//------------------------------------------------------------------------------------------------
	static proto bool SetMasterVolume(int id, float volume);
	static proto float GetMasterVolume(int id);
	//------------------------------------------------------------------------------------------------
	static proto bool CreateOutputState(string filename);
	static proto int GetOutpuStateSignalIdx(int outStateIdx, string signal);
	static proto void SetOutputStateSignal(int outStateIdx, int idx, float value);
}

/*!
\}
*/
