/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup System
\{
*/

sealed class WorldSystemInfo: pointer
{
	proto external WorldSystemInfo SetAbstract(bool value);
	proto external WorldSystemInfo SetLocation(ESystemLocation value);
	proto external WorldSystemInfo AddPoints(ESystemPoint points);
	proto external WorldSystemInfo AddExecuteBefore(typename otherSystem, ESystemPoint points);
	proto external WorldSystemInfo AddExecuteAfter(typename otherSystem, ESystemPoint points);
}

/*!
\}
*/
