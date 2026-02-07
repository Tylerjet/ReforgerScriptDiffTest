/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Effects
\{
*/

class ParticleEffectHandle
{
	proto external IEntity GetEntity();
	proto external ResourceName GetResource();
	proto external IEntity CreateEffect(ResourceName effectResource, typename type, BaseWorld world);
	proto external void StopEmit();
	proto external void Pause();
	proto external void Continue();
	proto external void Update(float timeSlice);
	proto external bool HasActiveParticles();
	proto external void SetMaterial(GameMaterial mat);
	proto external GameMaterial GetMaterial();
}

/*!
\}
*/
