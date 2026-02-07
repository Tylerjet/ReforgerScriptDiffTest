//------------------------------------------------------------------------------------------------
class SCR_CampaignCloseBuildingPreviewUserAction : ScriptedUserAction
{
	protected SCR_CampaignBase m_Base
	protected SCR_CampaignSuppliesComponent m_SuppliesComponent;
	protected SCR_CampaignBuildingComponent m_BuildingComponent;
	protected SCR_SiteSlotEntity m_Slot;
		
	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent) 
	{		
		m_SuppliesComponent = SCR_CampaignSuppliesComponent.Cast(GetOwner().FindComponent(SCR_CampaignSuppliesComponent));	
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool ProcessTracedEntity(IEntity ent)
	{
		m_Base = SCR_CampaignBase.Cast(ent);
		
		if (m_Base)
		{
			// Base was found, stop query
			return false;
		}
		
		// Keep looking
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	// This is temporary method due to problems with getting parent on spawned vehicle It should goes to Init!!!
	private void InitializeActionOwner()
	{
		IEntity owner = GetOwner();		
		if (!owner)
			return;
		
		BaseWorld world = owner.GetWorld();
	
		if (!world)
			return;
		
		SCR_CampaignBuildingControllerComponent buildinController = SCR_CampaignBuildingControllerComponent.Cast(owner.FindComponent(SCR_CampaignBuildingControllerComponent));
		if (buildinController)
			m_Slot = buildinController.GetUsedSlot();
		
		vector origin = owner.GetOrigin();
		
		// Register parent base - find supply depot in the nearest vicinity
		world.QueryEntitiesBySphere(origin, SCR_CampaignReconfigureHQRadioUserAction.MAX_BASE_DISTANCE, ProcessTracedEntity, null, EQueryEntitiesFlags.ALL);
	}
	
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{
		m_BuildingComponent = SCR_CampaignBuildingComponent.Cast(pUserEntity.FindComponent(SCR_CampaignBuildingComponent)); 
		if (!m_BuildingComponent)
			return;	
		
		// delete all previews
		m_BuildingComponent.RemovePreviewEntities();
		m_BuildingComponent.RemoveControllers();
		m_BuildingComponent.RemoveHandlers();
		m_BuildingComponent.SetBuilding(false);
		
		SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_CANCLELBUILDING);		
	}
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		return false;
		
		//first time player interact with this, decide what kind of the interaction source it is. Vehicle or the base?
		if (!m_SuppliesComponent && !m_Base)
			InitializeActionOwner();
		
		if (m_Slot)
			return true;
		
		if (!m_SuppliesComponent && !m_Base)
			return false;
		
		// get the player faction
		Faction playerFaction = SCR_CampaignReconfigureRelayUserAction.GetPlayerFaction(user);
		if (!playerFaction)
			return false;
		
		if (m_SuppliesComponent)
		{
			FactionAffiliationComponent factionAffiliationComponent  = FactionAffiliationComponent.Cast(user.FindComponent(FactionAffiliationComponent));
			if (!factionAffiliationComponent)
				return false;
			
			Faction owningFaction = factionAffiliationComponent.GetDefaultAffiliatedFaction();
			if (!owningFaction)	
				return false;
		
			return CanBeShown(owningFaction, playerFaction, user);
		}
		
		if (m_Base)
		{
			SCR_CampaignFaction owningFaction = m_Base.GetOwningFaction();
			if (!owningFaction)	
				return false;
			
			return CanBeShown(owningFaction, playerFaction, user);
		}
	
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		outName = ("#AR-Campaign_Action_CloseBuildPreview-UC");
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool HasLocalEffectOnlyScript()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	// return true, when player faction match to source faction and there aren't any previews
	protected bool CanBeShown(notnull Faction owningFaction, notnull Faction playerFaction, IEntity user)
	{
		// if the side owning of the base don't match the player, he can't see the option
		if (playerFaction != owningFaction)
			return false;

		m_BuildingComponent = SCR_CampaignBuildingComponent.Cast(GenericEntity.Cast(user).FindComponent(SCR_CampaignBuildingComponent)); 
		if (!m_BuildingComponent)
			return false;
		
		return m_BuildingComponent.IsBuilding();

	}
};
