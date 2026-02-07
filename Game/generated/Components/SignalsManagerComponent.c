/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup Components
* @{
*/

class SignalsManagerComponentClass: GameComponentClass
{
};

class SignalsManagerComponent: GameComponent
{
	/*!
	Add or find a signal with given \param signalName.
	\return Index of the signal. Returns -1 in case of error.
	*/
	proto external int AddOrFindSignal(string signalName, float value = 0);
	/*!
	Add or find a MP signal with given \param signalName, if it does not exist init it with \param valueThreshold and \param blendSpeed.
	\param compressionFunc Compression type to use to compress the value when transfering it over the network.
	\warning This has to be performed on authority. If an attempt to register a MP signal is made on proxies an ordinary signal is
	created instead. If authority registers a signal it makes sure to synchronize to state with proxies.
	\return Index of the signal. Returns -1 in case of error.
	*/
	proto external int AddOrFindMPSignal(string signalName, float valueThreshold, float blendSpeed, float value = 0, SignalCompressionFunc compressionFunc = SignalCompressionFunc.None);
	/*!
	Add or find a signal with given \param signalName.
	\return Index of the signal. Returns -1 in case of error.
	*/
	proto external int FindSignal(string signalName);
	//! Set the signal with an index of \param index to \param value
	proto external void SetSignalValue(int index, float value);
	/*!
	Get value of the signal with index \param index
	\return Value of the signal. Returns 0.0f if the signal is not found.
	*/
	proto external float GetSignalValue(int index);
};

/** @}*/
