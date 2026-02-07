//! Action to unload supplies from a Supply truck in Campaign
class SCR_CampaignLoadSuppliesUserAction : ScriptedUserAction
{
	[Attribute(defvalue: "1000", uiwidget: UIWidgets.Slider, desc: "Supplies to load", "100 1000 100")]
	protected int m_iSuppliesToLoad;
	
	// Member variables
	protected SCR_CampaignSuppliesComponent m_SuppliesComponent;
	protected SCR_CampaignBase m_Base;
	protected IEntity m_Box;
	protected int m_iCanLoadSuppliesResult = SCR_CampaignSuppliesInteractionFeedback.DO_NOT_SHOW;
	protected float m_fNextConditionCheck;
	
	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_Box = pOwnerEntity;
		m_SuppliesComponent = SCR_CampaignSuppliesComponent.Cast(m_Box.FindComponent(SCR_CampaignSuppliesComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActionStart(IEntity pUserEntity)
	{
		// Find local player controller
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return;
		
		// Find campaign network component to send RPC to server
		SCR_CampaignNetworkComponent campaignNetworkComponent = SCR_CampaignNetworkComponent.Cast(playerController.FindComponent(SCR_CampaignNetworkComponent));
		if (!campaignNetworkComponent)
			return;
		
		// Find the truck's supplies component ID
		RplId suppliesComponentID = Replication.FindId(m_SuppliesComponent);
		
		// Broadcast to other players that truck is already being loaded
		if (suppliesComponentID != RplId.Invalid())
			campaignNetworkComponent.StartLoading(suppliesComponentID, m_iSuppliesToLoad);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActionCanceled(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		// Find local player controller
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return;
		
		// Find campaign network component to send RPC to server
		SCR_CampaignNetworkComponent campaignNetworkComponent = SCR_CampaignNetworkComponent.Cast(playerController.FindComponent(SCR_CampaignNetworkComponent));
		if (!campaignNetworkComponent)
			return;
		
		// Find the truck's supplies component ID
		RplId suppliesComponentID = Replication.FindId(m_SuppliesComponent);
		
		// Broadcast to other players that truck is no longer being loaded
		if (suppliesComponentID != RplId.Invalid())
			campaignNetworkComponent.StopLoading(suppliesComponentID);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		switch (m_iCanLoadSuppliesResult)
		{			
			case SCR_CampaignSuppliesInteractionFeedback.FULL:
			{
				SetCannotPerformReason("#AR-Campaign_Action_SupplyTruckFull-UC");
				//SetCannotPerformReason("FULL");
				break;
			}
			
			case SCR_CampaignSuppliesInteractionFeedback.BASE_ENEMY:
			{
				SetCannotPerformReason("#AR-Campaign_Action_WrongBase-UC");
				//SetCannotPerformReason("ENEMY BASE");
				break;
			}
			
			case SCR_CampaignSuppliesInteractionFeedback.BASE_EMPTY:
			{
				SetCannotPerformReason("#AR-Campaign_Action_BaseDepleted-UC");
				//SetCannotPerformReason("BASE DEPLETED");
				break;
			}
		}
		
		if (m_iCanLoadSuppliesResult == SCR_CampaignSuppliesInteractionFeedback.POSSIBLE)
			return true;
		else
			return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (Replication.Time() >= m_fNextConditionCheck)
		{
			m_iCanLoadSuppliesResult = CanLoadSupplies(user);
			m_fNextConditionCheck += 250;
		}
		
		if (m_iCanLoadSuppliesResult == SCR_CampaignSuppliesInteractionFeedback.DO_NOT_SHOW)
			return false;
		else
			return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{
		if (!m_Base)
			return;
		
		// Find local player controller
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return;
		
		// Find campaign network component to send RPC to server
		SCR_CampaignNetworkComponent campaignNetworkComponent = SCR_CampaignNetworkComponent.Cast(playerController.FindComponent(SCR_CampaignNetworkComponent));
		if (!campaignNetworkComponent)
			return;
		
		// Find the truck's supplies component ID
		RplId suppliesComponentID = Replication.FindId(m_SuppliesComponent);
		
		if (suppliesComponentID != -1)
		{
			int suppliesToLoad;
			
			if (m_iSuppliesToLoad != m_SuppliesComponent.GetSuppliesMax())
				suppliesToLoad = m_iSuppliesToLoad;
			else
				suppliesToLoad = Math.Min(m_SuppliesComponent.GetSuppliesMax() - m_SuppliesComponent.GetSupplies(), m_Base.GetSupplies());
			
			campaignNetworkComponent.LoadSupplies(suppliesComponentID, pUserEntity, m_Base, suppliesToLoad);
		}
	}

	//------------------------------------------------------------------------------------------------
	override bool HasLocalEffectOnlyScript()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		if (!m_Base)
			return false;
		
		outName = "#AR-Campaign_Action_LoadSupplies-UC";
		
		if (m_iSuppliesToLoad == m_SuppliesComponent.GetSuppliesMax())
		{
			int canFitInTruck = m_SuppliesComponent.GetSuppliesMax() - m_SuppliesComponent.GetSupplies();
			int suppliesInBase = m_Base.GetSupplies();
			ActionNameParams[0] = Math.Min(canFitInTruck, suppliesInBase).ToString();
		}
		else
		{
			ActionNameParams[0] = m_iSuppliesToLoad.ToString();
			outName = "#AR-Campaign_Action_LoadSuppliesPartial-UC";
		}
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check the availability of this user action
	//! \param player Player trying to unload supplies
	int CanLoadSupplies(IEntity player)
	{
		SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();
		
		if (!campaign || !player || !m_SuppliesComponent)
			return SCR_CampaignSuppliesInteractionFeedback.DO_NOT_SHOW;
		
		m_Base = campaign.GetBasePlayerPresence();
		
		if (!m_Base) 
			return SCR_CampaignSuppliesInteractionFeedback.DO_NOT_SHOW;
		
		Faction playerFaction = SCR_RespawnSystemComponent.GetLocalPlayerFaction();
		
		if (!playerFaction)
			return SCR_CampaignSuppliesInteractionFeedback.DO_NOT_SHOW;
		
		SCR_CampaignDeliveryPoint depotDeliveryPoint = m_Base.GetBaseService(ECampaignServicePointType.SUPPLY_DEPOT);
		if (!depotDeliveryPoint)
			return SCR_CampaignSuppliesInteractionFeedback.DO_NOT_SHOW;
		
		SCR_CampaignSuppliesComponent baseSuppliesComponent = SCR_CampaignSuppliesComponent.Cast(depotDeliveryPoint.FindComponent(SCR_CampaignSuppliesComponent));
		if (!baseSuppliesComponent)	
			return SCR_CampaignSuppliesInteractionFeedback.DO_NOT_SHOW;
		
		if (vector.DistanceSq(m_Box.GetOrigin(), depotDeliveryPoint.GetOrigin()) > Math.Pow(baseSuppliesComponent.GetOperationalRadius(), 2))
			return SCR_CampaignSuppliesInteractionFeedback.DO_NOT_SHOW;
		
		// Player can load supplies only in bases owned by his faction
		if (m_Base.GetOwningFaction() != playerFaction)
			return SCR_CampaignSuppliesInteractionFeedback.BASE_ENEMY;
		
		int suppliesToLoad;
		int canFitInTruck = m_SuppliesComponent.GetSuppliesMax() - m_SuppliesComponent.GetSupplies();
		bool isMainLoadAction = m_iSuppliesToLoad == m_SuppliesComponent.GetSuppliesMax();
		
		if (!isMainLoadAction)
			suppliesToLoad = m_iSuppliesToLoad;
		else
			suppliesToLoad = canFitInTruck;
		
		// Hide preset batches equal to loadable supplies so they don't duplicate the "load full" functionality
		// If the supplies amount is less than the preset batch, don't show action (unless the truck is completely empty, then inform on the "load full" action)
		if (suppliesToLoad > canFitInTruck)
			return SCR_CampaignSuppliesInteractionFeedback.DO_NOT_SHOW;
		else
		{
			if (suppliesToLoad == canFitInTruck && !isMainLoadAction)
				return SCR_CampaignSuppliesInteractionFeedback.DO_NOT_SHOW;
			else
				if (canFitInTruck == 0 && isMainLoadAction)
					return SCR_CampaignSuppliesInteractionFeedback.DO_NOT_SHOW;
		}
		
		int suppliesInBase = m_Base.GetSupplies();
		
		// Hide batches larger than or equal to supplies stored in the base
		if (suppliesInBase != 0)
		{
			if (suppliesInBase > m_iSuppliesToLoad)
				return SCR_CampaignSuppliesInteractionFeedback.POSSIBLE;
			else
			{
				if (isMainLoadAction)
					return SCR_CampaignSuppliesInteractionFeedback.POSSIBLE;
				else
					return SCR_CampaignSuppliesInteractionFeedback.DO_NOT_SHOW;
			}
		}
		else
			if (isMainLoadAction)
				return SCR_CampaignSuppliesInteractionFeedback.BASE_EMPTY;
			else
				return SCR_CampaignSuppliesInteractionFeedback.DO_NOT_SHOW;
	}
};

enum SCR_CampaignSuppliesInteractionFeedback
{
	DO_NOT_SHOW = 0,
	POSSIBLE = 1,
	EMPTY = 2,
	FULL = 3,
	BASE_ENEMY = 4,
	BASE_FULL = 5,
	BASE_EMPTY = 6
};