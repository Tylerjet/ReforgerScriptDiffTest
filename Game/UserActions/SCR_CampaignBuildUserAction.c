//------------------------------------------------------------------------------------------------
class SCR_CampaignBuildUserAction : ScriptedUserAction
{
	protected SCR_CampaignBase m_Base;
	protected SCR_SiteSlotEntity m_SlotEnt;	
	protected SCR_CampaignBuildingControllerComponent m_BuildingController;
	protected SCR_CampaignSuppliesComponent m_SuppliesComponent;
	protected IEntity m_SuppliesProvider;
	protected SCR_CampaignBuildingTrigger m_Trigger;
	protected float m_fSearchDistance = 5;
	
	private const string SOUND_BUILD = "SOUND_BUILD";
		
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{				
		if (!m_Base && !m_SuppliesComponent || !m_SlotEnt)
			return;
		
		// get the prefab resource of current preview composition
		ResourceName resName = m_BuildingController.GetResource();	
		SCR_CampaignFaction owningFaction;		
		
		// action was executed on vehicle
		if (m_SuppliesComponent)
		{			
			FactionAffiliationComponent factionAffiliationComponent = FactionAffiliationComponent.Cast(m_SuppliesProvider.FindComponent(FactionAffiliationComponent));
			if (!factionAffiliationComponent)
			    return;
			
			owningFaction = SCR_CampaignFaction.Cast(factionAffiliationComponent.GetDefaultAffiliatedFaction());
		}
			
		// action was executed on base
		if (m_Base)
		{
			owningFaction = m_Base.GetOwningFaction();
		};

		if (!owningFaction)
			return;
		
		// get the index
		int compIndex = owningFaction.GetAvailableSlotResources().Find(resName);
		if (compIndex == -1)
			return;	
		
		// get composition value
		int compValue = m_BuildingController.GetPrice();
		if (compValue == -1)
			return;	
					
		if (!HaveResourcesToBuild(compValue))
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
			campaignNetworkComponent.BuildVehicle(m_SlotEnt.GetID(), m_SuppliesComponent, compIndex, compValue, m_BuildingController.GetAngle());
		
		// action was executed on base
		else if (m_Base)
			campaignNetworkComponent.BuildBase(m_SlotEnt.GetID(), m_Base, compIndex, compValue, m_BuildingController.GetAngle());
		
		SCR_UISoundEntity.SoundEvent(SOUND_BUILD);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		if (!m_Base && !m_SuppliesComponent || !m_BuildingController || m_BuildingController.GetUsedData() == null)
			return false;
				
		int compValue = m_BuildingController.GetPrice();
		
		if (!HaveResourcesToBuild(compValue))
		{
			SetCannotPerformReason(" " + GetAvailableResources().ToString() + " ");
			return false;
		}
		
		if (m_Trigger.IsBlocked())
		{
			SetCannotPerformReason("#AR-Campaign_Action_BuildBlocked-UC");
			return false;
		}
				
		return true;		
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (!m_BuildingController && GetOwner())
			Initialize(GetOwner());
		
		if (!m_SlotEnt)
			return false;
		
		// Don't show this user action in case slot is used by a composition 
		if (m_SlotEnt && m_SlotEnt.IsOccupied())
			return false;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	// Check if player has any resources available to build
	protected bool HaveResourcesToBuild(int compValue)
	{
		if (!compValue && !m_Base)
			return false;
		
		if (m_SuppliesComponent)
		{
			if (compValue > m_SuppliesComponent.GetSupplies())
			return false;
		}
		
		if (m_Base)
		{
			if (compValue > m_Base.GetSupplies())
			return false;
		}

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	// Return ammouont of supply currently available for building
	protected int GetAvailableResources()
	{
		int suppliesAvailable;
		if (m_SuppliesComponent)
		{
			suppliesAvailable = m_SuppliesComponent.GetSupplies();
		}
		
		if (m_Base)
		{
			suppliesAvailable = m_Base.GetSupplies();
		}
		
		return suppliesAvailable;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{	
		if (!m_BuildingController || !m_SlotEnt)
			return false;
		
		// get composition value
		int compValue = m_BuildingController.GetPrice();
		
		ActionNameParams[0] = m_BuildingController.GetCompositionName();
		ActionNameParams[1] = compValue.ToString();
		ActionNameParams[2] = GetAvailableResources().ToString();
		
		if (HaveResourcesToBuild(compValue))
		{
			outName = ("#AR-Campaign_Action_Build-UC");
		}
		else
		{
			outName = ("#AR-Campaign_Action_BuildNoResources-UC");	             
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool HasLocalEffectOnlyScript()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	void Initialize(notnull IEntity pOwnerEntity)
	{
		m_BuildingController = SCR_CampaignBuildingControllerComponent.Cast(pOwnerEntity.FindComponent(SCR_CampaignBuildingControllerComponent));
		if (!m_BuildingController)
			return; 
		
		m_SlotEnt = m_BuildingController.GetUsedSlot();
		
		m_SuppliesProvider = m_BuildingController.GetSuppliesProvider();
		if (!m_SuppliesProvider)
    		return;
		
		m_Trigger = m_BuildingController.GetTrigger();
		if (!m_Trigger)
			return;
		
		// Register supplies provider
		m_Base = SCR_CampaignBase.Cast(m_SuppliesProvider);
		if (m_Base)
		    return;
		
		SlotManagerComponent slotManager = SlotManagerComponent.Cast(m_SuppliesProvider.FindComponent(SlotManagerComponent));
		
		// It's a supply truck, register
		if (slotManager)
		{
			array<EntitySlotInfo> slots = {};
			slotManager.GetSlotInfos(slots);
			
			foreach (EntitySlotInfo slot: slots)
			{
				if (!slot)
					continue;
				
				IEntity truckBed = slot.GetAttachedEntity();
				
				if (!truckBed)
					continue;
				
				SCR_CampaignSuppliesComponent comp = SCR_CampaignSuppliesComponent.Cast(truckBed.FindComponent(SCR_CampaignSuppliesComponent));
				if (comp)
					m_SuppliesComponent = comp;
			}
		}
	}
};
