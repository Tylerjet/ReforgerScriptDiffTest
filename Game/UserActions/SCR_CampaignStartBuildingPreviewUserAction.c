//------------------------------------------------------------------------------------------------
class SCR_CampaignStartBuildingPreviewUserAction : ScriptedUserAction
{
	// Member variables
	protected SCR_CampaignBase m_Base;
	protected SCR_CampaignSuppliesComponent m_SuppliesComponent;
	protected SCR_CampaignBuildingComponent m_BuildingComponent;
	
	// array of slots
	private ref array<SCR_SiteSlotEntity> m_aSlotEntities = new array<SCR_SiteSlotEntity>();	
			
	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent) 
	{		
		if (pOwnerEntity)
			m_SuppliesComponent = SCR_CampaignSuppliesComponent.Cast(pOwnerEntity.FindComponent(SCR_CampaignSuppliesComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool ProcessTracedEntity(IEntity ent)
	{
		m_Base = SCR_CampaignBase.Cast(ent);
		
		if (m_Base)
		{
			IEntity player = SCR_PlayerController.GetLocalControlledEntity();
			m_BuildingComponent = SCR_CampaignBuildingComponent.Cast(player.FindComponent(SCR_CampaignBuildingComponent));
			
			if (m_BuildingComponent)
			{
				// Get slots around the base
				m_BuildingComponent.GetSlotsNearby(m_Base, m_BuildingComponent.GetBuildingRadius());
				m_aSlotEntities = m_BuildingComponent.GetSlots();
			}
			
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
		if (!GetOwner())
			return;
		
		BaseWorld world = GetGame().GetWorld();
	
		if (!world)
			return;
		
		vector origin = GetOwner().GetOrigin();
		
		// Register parent base - find supply depot in the nearest vicinity
		world.QueryEntitiesBySphere(origin, SCR_CampaignReconfigureHQRadioUserAction.MAX_BASE_DISTANCE, ProcessTracedEntity, null, EQueryEntitiesFlags.ALL);
	}
		
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{		
		m_BuildingComponent = SCR_CampaignBuildingComponent.Cast(pUserEntity.FindComponent(SCR_CampaignBuildingComponent)); 
		if (!m_BuildingComponent)
				return;	
		
		if (m_SuppliesComponent)
		{
			m_BuildingComponent.SpawnPreviewsVehicle(pOwnerEntity);
		} 
		
		if (m_Base)
		{					
			m_BuildingComponent.SpawnPreviewsBase(m_Base, m_aSlotEntities);
		} 
		
		m_BuildingComponent.SetBuilding(true);
		SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_STARTBUILDING);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{		
		// For vehicles - it's always visible
		if (m_SuppliesComponent)
			return true;
		
		if (!m_BuildingComponent.GetSlots().IsEmpty())
			return true;
		
		SetCannotPerformReason("#AR-Campaign_Action_CannotBuild-UC");
		return false;	
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		return false;
		
		//first time player interact with this, decide what kind of the interaction source it is. Vehicle or the base?
		if (!m_SuppliesComponent && !m_Base || !m_SuppliesComponent && m_BuildingComponent && m_BuildingComponent.GetSlots().IsEmpty())
			InitializeActionOwner();
				
		// get the player faction
		Faction playerFaction = SCR_CampaignReconfigureRelayUserAction.GetPlayerFaction(user);
		if (!playerFaction)
			return false;
		
		if (m_SuppliesComponent)
		{	
			FactionAffiliationComponent factionAffiliationComponent = FactionAffiliationComponent.Cast(user.FindComponent(FactionAffiliationComponent));
			if (!factionAffiliationComponent)
				return false;
			
			Faction owningFaction = factionAffiliationComponent.GetDefaultAffiliatedFaction();
			if (!owningFaction)
				return false;
			
			return CanBeShown(owningFaction, playerFaction, user);
		}
		
		if (m_Base)
		{
			Faction owningFaction = m_Base.GetOwningFaction();
			if (!owningFaction)
				return false;
			
			return CanBeShown(owningFaction, playerFaction, user);
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		ActionNameParams[0] = string.ToString(AvailableResources());
		outName = ("#AR-Campaign_Action_ShowBuildPreview-UC");
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool HasLocalEffectOnlyScript()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	// Return how much available resources base or vehicle has 
	int AvailableResources()
	{
		if (m_SuppliesComponent)
			return m_SuppliesComponent.GetSupplies();

		if (m_Base)
			return m_Base.GetSupplies();

		return -1;
	}
	
	//------------------------------------------------------------------------------------------------
	// return true, when player faction match to source faction and there aren't any previews
	bool CanBeShown(notnull Faction owningFaction,notnull Faction playerFaction, IEntity user)
	{
		// if the side owning of the base don't match the player, he can't see the option
		if (playerFaction != owningFaction)
			return false;
		
		m_BuildingComponent = SCR_CampaignBuildingComponent.Cast(user.FindComponent(SCR_CampaignBuildingComponent)); 
		if (!m_BuildingComponent)
			return false;	
		
		// Don't show this user action if the building mode is on.
		return !m_BuildingComponent.IsBuilding();	
	}
};
