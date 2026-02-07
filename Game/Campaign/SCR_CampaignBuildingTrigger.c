//------------------------------------------------------------------------------------------------
class SCR_CampaignBuildingTriggerClass: ScriptedGameTriggerEntityClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignBuildingTrigger : ScriptedGameTriggerEntity
{
	//------------------------------------------------------------------------------------------------
	bool CanBlockPreview(notnull IEntity element)
	{
		Vehicle veh = Vehicle.Cast(element);
		if (veh)	
			return true;
		
		// If it's something else like a gear, weapon etc, remove it from array of blocking elements.
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	AIAgent FindAiAgent(IEntity entity)
	{		
		AIControlComponent AIControlComp = AIControlComponent.Cast(entity.FindComponent(AIControlComponent));
		if (!AIControlComp)
			return null;
				
		return AIControlComp.GetControlAIAgent();
	}
			
	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity ent)
	{
		SetEventMask(EntityEvent.FRAME);
	}
};
