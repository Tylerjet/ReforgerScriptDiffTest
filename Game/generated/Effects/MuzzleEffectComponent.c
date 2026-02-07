/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup Effects
* @{
*/

class MuzzleEffectComponentClass: BaseEffectComponentClass
{
};

class MuzzleEffectComponent: BaseEffectComponent
{
	/*!
	Show the particle effect
	*/
	proto external void Play();
	/*!
	Returns true if the particle effect is playing
	*/
	proto external bool IsPlaying();
	/*!
	Stop the particle effect
	*/
	proto external void Stop();
	
	// callbacks
	
	/*!
	Called during EOnInit.
	\param owner Entity this component is attached to.
	*/
	event void OnInit(IEntity owner);
	//! Runs when Stop function has been called.
	event void OnStop();
	/*!
	Called during EOnFrame.
	\param owner Entity this component is attached to.
	\param timeSlice Delta time since last update.
	*/
	event void OnFrame(IEntity owner, float timeSlice);
	event void OnFired(IEntity effectEntity, BaseMuzzleComponent muzzle, IEntity projectileEntity);
};

/** @}*/
