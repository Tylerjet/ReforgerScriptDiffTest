//------------------------------------------------------------------------------------------------
class SCR_CampaignDisassembleCompositionUserAction : ScriptedUserAction
{
	protected SCR_CampaignBuildingControllerComponent m_BuildingController
	protected SCR_CampaignFaction m_Faction;
	protected SCR_SiteSlotEntity m_SlotEnt;
	protected ResourceName m_SlotResName;
	protected SCR_CampaignBase m_Base;
	protected SCR_CampaignSuppliesComponent m_SuppliesComponent;
	
	protected IEntity m_SuppliesProvider;
	protected ref array<ref SCR_CampaignSlotComposition> m_aSlotData = new array<ref SCR_CampaignSlotComposition>();
	
	private const string SOUND_DISASSEMBLY = "SOUND_DISASSEMBLY";
	
	//------------------------------------------------------------------------------------------------
	private void InitializeActionOwner()
	{
		m_BuildingController = SCR_CampaignBuildingControllerComponent.Cast(GetOwner().FindComponent(SCR_CampaignBuildingControllerComponent));
		if (!m_BuildingController)
			return; 
		
		m_SuppliesProvider = m_BuildingController.GetSuppliesProvider();
		if (!m_SuppliesProvider)
    		return;
		
		// Register parent base
		if (m_SuppliesProvider.Type() == SCR_CampaignBase)
		{
			m_Base = SCR_CampaignBase.Cast(m_SuppliesProvider);
			m_Faction = SCR_CampaignFaction.Cast(m_Base.GetOwningFaction());
		}
		else
		// It's a supply truck, register
		{
			FactionAffiliationComponent factionAffiliationComponent = FactionAffiliationComponent.Cast(m_SuppliesProvider.FindComponent(FactionAffiliationComponent));
			if (!factionAffiliationComponent)
				return;

			m_Faction = SCR_CampaignFaction.Cast(factionAffiliationComponent.GetDefaultAffiliatedFaction()); 
			if (!m_Faction)
				return;
			
			SlotManagerComponent slotManager = SlotManagerComponent.Cast(m_SuppliesProvider.FindComponent(SlotManagerComponent));
			
			if (slotManager)
			{
				array<EntitySlotInfo> slots = new array<EntitySlotInfo>;
				slotManager.GetSlotInfos(slots);
				
				foreach (EntitySlotInfo slot: slots)
				{
					if (!slot)
						continue;
					
					IEntity truckBed = slot.GetAttachedEntity();
					
					if (!truckBed)
						continue;
					
					m_SuppliesComponent = SCR_CampaignSuppliesComponent.Cast(truckBed.FindComponent(SCR_CampaignSuppliesComponent));
					if (m_SuppliesComponent)
						break;
				}
			}
		}
		
		m_SlotEnt = m_BuildingController.GetUsedSlot();
		
		if (!m_SlotEnt)
			return;
		
		m_SlotResName = m_SlotEnt.GetPrefabData().GetPrefabName();
		m_aSlotData = m_Faction.GetSlotResource(GetCompositionType(m_SlotResName));
	}
		
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{		
		if (!m_BuildingController || !m_SlotEnt)
			return;
				
		// Find local player controller
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return;
			
		// Find campaign network component to send RPC to server
		SCR_CampaignNetworkComponent campaignNetworkComponent = SCR_CampaignNetworkComponent.Cast(playerController.FindComponent(SCR_CampaignNetworkComponent));
		if (!campaignNetworkComponent)
			return;

		// action was executed on vehicle
		if (m_SuppliesComponent)
			campaignNetworkComponent.DisassembleCompositionVehicle(m_SlotEnt.GetID(), m_SuppliesComponent, CalculateRefundValue());
		
		// action was executed on base
		else if (m_Base)
			campaignNetworkComponent.DisassembleComposition(m_SlotEnt.GetID(), m_Base, CalculateRefundValue());

		m_SlotEnt.SetOccupant(null);
		SCR_UISoundEntity.SoundEvent(SOUND_DISASSEMBLY);
		
		// Check if the slot is used for services.
		
		SCR_GameModeCampaignMP gameMode = SCR_GameModeCampaignMP.Cast(GetGame().GetGameMode());
		if (!gameMode)
			return; 
		
		SCR_CampaignSlotComposition slotData;
		if (gameMode.GetSlotPresetResourceName(m_SlotEnt) != ResourceName.Empty)
			{
				// Get all compositions for Services
				array<ref SCR_CampaignSlotComposition> slotDataArrayServices = m_Faction.GetSlotResource(SCR_ESlotTypesEnum.Services);
				for (int y = slotDataArrayServices.Count() -1; y >= 0; y--)
				{
					if (slotDataArrayServices[y].GetResourceName() == gameMode.GetSlotPresetResourceName(m_SlotEnt))
					{
						slotData = slotDataArrayServices[y];
					}
				}
			}
			else 
			{
				//As a default we will spawn the 1st composition from the list of all available.
				slotData = m_aSlotData[0];
			}
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{			
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		// If this code runs for the first time we need to initialize the variables
		if (!m_Faction)
			InitializeActionOwner();
		
		// Don't show the user action if controller has not a parent.
		if (!m_SuppliesProvider)
			return false;
		
		// Don't show this user action in case slot is not used by a composition 
		if (m_SlotEnt && !m_SlotEnt.IsOccupied())
			return false;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		ActionNameParams[0] = string.ToString(CalculateRefundValue());
		outName = "#AR-Campaign_Action_DisassembleComposition-UC";
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
	
	//------------------------------------------------------------------------------------------------
	// Calculate return cost
	protected int CalculateRefundValue()
	{
		return m_BuildingController.GetRefundValue();
	}
};
