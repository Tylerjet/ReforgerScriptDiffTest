/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup Effects
* @{
*/

class MultiEffectComponentClass: ScriptComponentClass
{
};

class MultiEffectComponent: ScriptComponent
{
	proto external bool HasActiveParticles();
	proto external void UpdateBatch(IEntity owner, float timeSlice);
	proto external void ReserveEffects(int count);
	
	// callbacks
	
	event void UpdateEffect(ParticleEffectHandle effect, IEntity owner, float timeSlice, int index);
};

/** @}*/
