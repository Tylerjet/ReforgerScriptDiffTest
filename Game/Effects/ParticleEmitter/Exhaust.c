class ExhaustEffectBaseClass: SCR_ParticleEmitterClass
{
};

class ExhaustEffectBase : SCR_ParticleEmitter
{
	void ExhaustEffectBase(IEntitySource src, IEntity parent)
	{
		// SetPathToPTC(" PUT PTC PATH HERE ");
	}
	
	override void EOnInit(IEntity owner)
	{
		SetEventMask(EntityEvent.FRAME);
	}
	
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		
	}
};