/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup Effects
* @{
*/

class BaseEffectComponentClass: GameComponentClass
{
};

class BaseEffectComponent: GameComponent
{
	proto void SetParticleParam(int emitter, EmitterParam param, void value);
	proto void GetParticleParam(int emitter, EmitterParam param, out void value);
	proto int GetParticleEmittors(out string emitters[]);
	//! Returns the particle entity associated with this effect component.
	proto external IEntity GetParticleEntity();
	proto external SignalsManagerComponent GetSignalsManager();
};

/** @}*/
