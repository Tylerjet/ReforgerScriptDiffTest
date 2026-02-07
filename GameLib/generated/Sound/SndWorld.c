/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Sound
\{
*/

class SndWorldClass: GenericEntityClass
{
}

class SndWorld: GenericEntity
{
	proto void GetMapValuesAtPos(vector pos, out float sea, out float forest, out float city, out float meadow, out float coast, out float height);
	// To be removed, needed for Arma Reforger
	proto void CalculateInterirorAt(vector pos, InteriorRequestCallback callback);
}

/*!
\}
*/
