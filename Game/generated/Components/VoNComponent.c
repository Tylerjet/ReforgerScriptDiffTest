/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup Components
* @{
*/

class VoNComponentClass: GameComponentClass
{
};

class VoNComponent: GameComponent
{
	//! Starts/stop capturing
	proto external void SetCapture(bool isCapturing);
	//! Sets communication method used.
	proto external void SetCommMethod(ECommMethod type);
	proto external ECommMethod GetCommMethod();
	//! Sets the radio component used for VoN, when SquadRadio is selected
	proto external void SetTransmitRadio(BaseRadioComponent radio);
	proto external BaseRadioComponent GetTransmitRadio();
	//! Transfer AI sound message through VoN.
	proto external void SoundEventPriority(string eventname, array<float> values, int priority, bool ignoreQueue = false);
	
	// callbacks
	
	/*!
	Event triggered when VoN is capturing
	\param radio Radio component used. Null for direct.
	*/
	event protected void OnCapture(BaseRadioComponent radio);
	/*!
	Event called when VoNComponent receives
	\param playerId Player id from lobby
	\param radio Receiving radio
	\param frequency Frequency in kHz, on which the transmission came from
	\param quality Value in range <0,1> of how close to transmitter the radio is (linear scale)
	\param transceiverIdx Index of the radio transceiver (physical channel if you want)
	*/
	event protected void OnReceive(int playerId, BaseRadioComponent radio, int frequency, float quality, int transceiverIdx);
};

/** @}*/
