
class SCR_CampaignChangePreviewUserAction : ScriptedUserAction
{
	protected SCR_SiteSlotEntity m_SlotEnt;
	protected SCR_CampaignSuppliesComponent m_SuppliesComponent;
	protected IEntity m_SuppliesProvider;
	protected SCR_CampaignFaction m_Faction;
	protected IEntity m_Owner;
	protected SCR_CampaignBuildingControllerComponent m_BuildingControllerComponent;
	protected SCR_CampaignBuildingComponent m_BuildingComponent;
	protected SCR_CampaignBuildingClientTrigger m_Trigger;
	protected ref array<ref SCR_CampaignSlotComposition> m_aSlotData = new array<ref SCR_CampaignSlotComposition>();
	//------------------------------------------------------------------------------------------------
	//! Translates action ID to asset ID (its index in asset list)
	protected int GetUsableActionID()
	{
		return (GetActionID() / 2) - 2;
	}
	
	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent) 
	{
		m_Owner = pOwnerEntity;
	}
	//------------------------------------------------------------------------------------------------
	void SlotArrayInit() 
	{		
		m_BuildingControllerComponent = SCR_CampaignBuildingControllerComponent.Cast(m_Owner.FindComponent(SCR_CampaignBuildingControllerComponent));
		if (!m_BuildingControllerComponent)
			return;
		
		m_SlotEnt = m_BuildingControllerComponent.GetUsedSlot();
		if (!m_SlotEnt)
			return;
		
		m_SuppliesProvider = m_BuildingControllerComponent.GetSuppliesProvider();
		if (!m_SuppliesProvider)
    		return;
		
		m_Trigger = m_BuildingControllerComponent.GetTrigger();
		if (!m_Trigger)
			return;
		
		// Check if it's a base and get a faction
		SCR_CampaignBase base = SCR_CampaignBase.Cast(m_SuppliesProvider);
		if (base)
		{
			m_Faction = SCR_CampaignFaction.Cast(base.GetOwningFaction());
		}
		else
		{
			// it's a truck
			FactionAffiliationComponent factionAffiliationComponent = FactionAffiliationComponent.Cast(m_SuppliesProvider.FindComponent(FactionAffiliationComponent));
			if (!factionAffiliationComponent)
				return;
	
			m_Faction = SCR_CampaignFaction.Cast(factionAffiliationComponent.GetDefaultAffiliatedFaction()); 
		}
	
		if (!m_Faction)
			return;
		
		ResourceName slotResName = m_SlotEnt.GetPrefabData().GetPrefabName();
		m_aSlotData = m_Faction.GetSlotResource(GetCompositionType(slotResName));
		
		IEntity player = SCR_PlayerController.GetLocalControlledEntity();
		m_BuildingComponent = SCR_CampaignBuildingComponent.Cast(player.FindComponent(SCR_CampaignBuildingComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{		
		if (!m_BuildingComponent || !m_BuildingControllerComponent)
			return;
		
		if (!m_BuildingControllerComponent.GetPreviewEntity() || !pOwnerEntity)
			return;
		
		Color iconColor;
		
		if (m_Faction)
			iconColor = m_Faction.GetFactionColor();

		// remove current preview
		m_BuildingComponent.RemovePreviewEntity(m_BuildingControllerComponent);
		m_BuildingComponent.SpawnNewPreview(m_aSlotData[GetUsableActionID()],m_SlotEnt, m_SuppliesProvider, m_BuildingControllerComponent, iconColor);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{				
		if (m_Trigger.IsToBeBuilt())
		{
			SetCannotPerformReason("#AR-Campaign_Action_BuildBlocked-UC");
			return false;
		}
		
		return true;		
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		//first time player interact with this, decide what kind of the interaction source it is. Vehicle or the base?
		if (!m_Faction)
			SlotArrayInit();
		
		// Don't show this user action in case slot is used by a composition 
		if (m_SlotEnt && m_SlotEnt.IsOccupied())
			return false;
		
		// Don't show the user action in case there isn't enough suitable compositions for this type of the slot.
		if (GetUsableActionID() >= m_aSlotData.Count())
			return false;
	
		// Don't show the composition on the list if it's same as currently spawned one.
		if (m_aSlotData[GetUsableActionID()] == m_BuildingControllerComponent.GetUsedData())	
			return false;
		
		// Don't show this if the slot is used by Service Composition 
		if (m_BuildingControllerComponent.GetUsedData().IsServiceComposition())
			return false;
		
		return true;
	}
		
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		ActionNameParams[0] = m_aSlotData[GetUsableActionID()].GetCompositionName();
		ActionNameParams[1] = string.ToString(m_aSlotData[GetUsableActionID()].GetPrice());
		outName = "#AR-Campaign_Action_ChangePreview-UC";
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool HasLocalEffectOnlyScript()
	{
		return true;
	}
		
	//------------------------------------------------------------------------------------------------
	// Compare resource name with the defined slots and if match is found, return SlotTypesEnum
	private SCR_ESlotTypesEnum GetCompositionType(ResourceName resName)
	{
		SCR_CampaignFactionManager factionManager = SCR_CampaignFactionManager.GetInstance();
		for (int i = 0; i <= SCR_ESlotTypesEnum.CheckpointLarge; i++ )
		{
			if (resName == factionManager.GetSlotsResource(i))
				return i;
		}
		
		return -1;
	}
};
