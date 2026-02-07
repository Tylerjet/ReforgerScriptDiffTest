/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup Components
* @{
*/

class BaseRadioComponentClass: GameComponentClass
{
};

class BaseRadioComponent: GameComponent
{
	/*!
	\return Entity this component is atached to.
	*/
	proto external IEntity GetOwner();
	/*!
	Sets maximum transmitting range of radio.
	Has effect only on owner (server).
	\param range: Distance in meters.
	*/
	proto external void SetRange(float range);
	/*!
	Gets maximum transmitting range of radio.
	\return Distance in meters.
	*/
	proto external float GetRange();
	/*!
	Frequency resolution for all transceivers.
	\return Frequency in KHz.
	*/
	proto external int GetFrequencyResolution();
	/*!
	Sets the active transciever
	\param[in] transvIdx Transciever index
	*/
	proto external void SetActiveTransceiver(int transvIdx);
	/*!
	\return Index of active the transciever
	*/
	proto external int GetActiveTransceiver();
	/*!
	Sets frequency on active transceiver
	\param[in] freq Frequency in KHz.
	*/
	proto external void SetFrequency(int freq);
	/*!
	Returns frequency of active transceiver
	\return Frequency in KHz.
	*/
	proto external int GetFrequency();
	/*!
	Returns maximum tunable frequency of the radio
	\return Frequency in KHz.
	*/
	proto external int GetMinFrequency();
	/*!
	Returns minimum tunable frequency of the radio
	\return Frequency in KHz.
	*/
	proto external int GetMaxFrequency();
	proto external void SetBusy(bool isBusy, int transvIdx = 0);
	// TODO: Rename to IsTransceiverBusy
	proto external bool IsChannelBusy(int transvIdx = 0);
	/*!
	Set the key used to encrypt outgoing transmission
	*/
	proto external void SetEncryptionKey(string key);
	/*!
	\return Key used to encrypt outgoing transmission
	*/
	proto external string GetEncryptionKey();
	// TODO: Rename to SetPowered
	proto external void TogglePower(bool power);
	proto external bool IsPowered();
	proto external void Transmit(BaseRadioMessage msg);
};

/** @}*/
