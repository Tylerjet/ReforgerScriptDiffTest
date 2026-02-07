/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Effects
\{
*/

class BaseScriptEffectComponentClass: BaseEffectComponentClass
{
}

class BaseScriptEffectComponent: BaseEffectComponent
{
	proto external void SetEffectBirthRateCoeff(float newCoeff);
	proto external float GetEffectBirthRateCoeff();

	// callbacks

	/*!
	Called during EOnInit.
	\param owner Entity this component is attached to.
	*/
	event void EOnInit(IEntity owner);
	/*!
	Called during EOnFrame.
	\param owner Entity this component is attached to.
	\param timeSlice Delta time since last update.
	*/
	event void EOnFrame(IEntity owner);
}

/*!
\}
*/
