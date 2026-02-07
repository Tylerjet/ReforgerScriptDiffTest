[EntityEditorProps(category: "GameScripted/Campaign", description: "Trigger entity for campaign.", color: "0 0 255 255")]
class SCR_CampaignTriggerEntityClass: ScriptedGameTriggerEntityClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTriggerEntity : ScriptedGameTriggerEntity
{
	//***************//
	//MEMBER METHODS//
	//***************//
	
	//------------------------------------------------------------------------------------------------
	protected void GetChimeraCharacters(array<IEntity> entityList, out array<ChimeraCharacter> characterList)
	{
		if (!characterList || !entityList)
			return;
		
		characterList.Clear();
		foreach (IEntity entity : entityList)
		{
			if (!entity)
				return;
			auto castEntity = ChimeraCharacter.Cast(entity);
			if (castEntity)
				characterList.Insert(castEntity);
		}
	}
	
	//****************//
	//OVERRIDE METHODS//
	//****************//
	
	//------------------------------------------------------------------------------------------------
	//! Override this method in inherited class to define a new filter.
	override bool ScriptedEntityFilterForQuery(IEntity ent)
	{
		return DefaultEntityFilterForQuery(ent);
	}
	
	//------------------------------------------------------------------------------------------------
	//! callback - activation - occurs when and entity which fulfills the filter definitions enters the Trigger
	override void OnActivate(IEntity ent);
	
	//! callback - deactivation - occurs when and entity which was activated (OnActivate) leaves the Trigger
	override void OnDeactivate(IEntity ent);
	
	//------------------------------------------------------------------------------------------------
	protected IEntity FindParentOfType(typename type)
	{
		IEntity parent = GetParent();
		while (parent)
		{
			if (parent.Type() == type)
				return parent;
			parent = parent.GetParent();
		}
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		IEntity foundEntity = FindParentOfType(SCR_CampaignBase);
		if (foundEntity)
		{
			SCR_CampaignBase base = SCR_CampaignBase.Cast(foundEntity);
			if (base)
				base.RegisterTrigger(this);
		}
		
		ClearFlags(EntityFlags.ACTIVE, false);
	}
	
	//************************//
	//CONSTRUCTOR / DESTRUCTOR//
	//************************//
	
	//------------------------------------------------------------------------------------------------
	void SCR_CampaignTriggerEntity(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);
		SetFlags(EntityFlags.ACTIVE, true);
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_CampaignTriggerEntity()
	{
	}

};
