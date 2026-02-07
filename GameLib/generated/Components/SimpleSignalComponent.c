/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components
\{
*/

class SimpleSignalComponent: GenericComponent
{
	proto external int FindSignalIndex(IEntity owner, string signalName);
	proto external float GetSignalValue(int signalIndex);
	proto external void SetSignalValue(int signalIndex, float signalValue);
}

/*!
\}
*/
