/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup System
\{
*/

class SndSystem: BaseSystem
{
	//! Current the sound world - may be nullptr if SndWorld is not present in the world.
	proto external SndWorld GetSndWorld();
	//! Returns scripted module by class.
	proto external SndBaseModule FindModule(typename type);
}

/*!
\}
*/
