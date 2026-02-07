class ExhaustBTRClass: ExhaustEffectBaseClass
{
};

class ExhaustBTR : ExhaustEffectBase
{
	void ExhaustBTR(IEntitySource src, IEntity parent)
	{
		SetPathToPTC("{FC98EA459BD98A6C}Particles/Vehicle/Vehicle_smoke_BTR_exhaust_black_01.ptc");
	}
	
	override void EOnInit(IEntity owner)
	{
		SetEventMask(EntityEvent.FRAME);
	}
	
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		
	}
};