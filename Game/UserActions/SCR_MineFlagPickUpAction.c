//------------------------------------------------------------------------------------------------
class SCR_MineFlagPickUpAction : SCR_PickUpItemAction
{
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{
		super.PerformAction(pOwnerEntity, pUserEntity);
		
		SoundComponent soundComponent = SoundComponent.Cast(GetOwner().FindComponent(SoundComponent));
		if (!soundComponent)
			return;
		
		soundComponent.SoundEvent("SOUND_MINEFLAG_REMOVE");
	}
};