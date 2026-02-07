/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components
\{
*/

class SndComponent: GenericComponent
{
	proto external void SetCallbackMask(SndComponentCallbacks mask);
	/* Component owner. */
	proto external IEntity GetOwner();
	/* Beginning of sound event scope. */
	proto external int SoundEventBegin(string eventName);
	/* Sets scope parameters and plays the event. */
	proto external int SoundEventEnd(int eventBeginId);
	/* Deprecated - will be replaced. */
	proto external AudioHandle PlayStr(string name);
	/* Sets signal value by name. */
	proto external void SetSignalValueStr(string signal, float value);
	/* Set transformation of all sounds played by this component */
	proto external void SetTransformation(vector transf[]);
	/* Update playback of triggered sounds */
	proto external void UpdateTrigger();
	/* Terminate the sound associated with the given handle */
	proto external void Terminate(AudioHandle handle);

	// callbacks

	/* Called when the component is audible. Audibility is determined as max from all events in acps. */
	event void Update(float dt);
	event void OnPostInit();
	event void OnDelete();
}

/*!
\}
*/
