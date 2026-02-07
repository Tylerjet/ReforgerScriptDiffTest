/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components
\{
*/

class VoNComponentClass: GameComponentClass
{
}

class VoNComponent: GameComponent
{
	/*!
	Request to starts/stop audio capturing.
	\return if the request suceeded
	*/
	proto external bool SetCapture(bool isCapturing);
	//! Sets communication method used.
	proto external void SetCommMethod(ECommMethod type);
	proto external ECommMethod GetCommMethod();
	//! Sets the radio component used for VoN, when SquadRadio is selected
	proto external void SetTransmitRadio(BaseTransceiver transceiver);
	proto external BaseTransceiver GetTransmitRadio();
	//! Transfer AI sound message through VoN.
	proto external void SoundEventPriority(string eventname, array<float> values, int priority, bool ignoreQueue = false);

	// callbacks

	/*!
	Event triggered when VoN is capturing
	\param radio Radio component used. Null for direct.
	*/
	event protected void OnCapture(BaseTransceiver transmitter);
	/*!
	Event called when VoNComponent receives
	\param playerId Senders PlayerId matching player in the PlayerManager
	\param receiver Receiving transceiver
	\param frequency Frequency in kHz, on which the transmission came from. Can be different from the frequency which transceiver have set. This is very questionable and could be changed in future.
	\param quality Value in range <0,1> describing quality of the transmission
	*/
	event protected void OnReceive(int playerId, BaseTransceiver receiver, int frequency, float quality);
}

/*!
\}
*/
