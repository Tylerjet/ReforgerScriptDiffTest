[ComponentEditorProps(category: "GameScripted/Sound", description: "Component is active only in campaign mode")]
class SCR_CampaignSoundComponentClass: SoundComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignSoundComponent : SoundComponent
{	
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(IEntity owner)
	{
		super.OnInit(owner);
		
		// Disable dynamic simulation if not campaign game mode
		SCR_GameModeCampaignMP gameMode = SCR_GameModeCampaignMP.Cast(GetGame().GetGameMode());
		if (!gameMode)
			EnableDynamicSimulation(false);		
	}

	//------------------------------------------------------------------------------------------------
	void SCR_CampaignSoundComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		SetScriptedMethodsCall(true);	
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_CampaignSoundComponent()
	{
	}
};