#include "scripts/Game/config.c"
//! Action to unload supplies from a Supply truck in Campaign
class SCR_CampaignUnloadSuppliesUserAction : ScriptedUserAction
{
	[Attribute(defvalue: "1000", uiwidget: UIWidgets.Slider, desc: "Supplies to unload", "100 1000 100")]
	protected int m_iSuppliesToUnload;
	
	// Member variables
	protected SCR_CampaignSuppliesComponent m_SuppliesComponent;
	protected SCR_CampaignMilitaryBaseComponent m_Base;
	protected IEntity m_Box;
	protected int m_iCanUnloadSuppliesResult = SCR_CampaignSuppliesInteractionFeedback.DO_NOT_SHOW;
	#ifndef AR_CAMPAIGN_TIMESTAMP
	protected float m_fNextConditionCheck;
	#else
	protected WorldTimestamp m_fNextConditionCheck;
	#endif
	
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
		
		// Find conflict network component to send RPC to server
		SCR_CampaignNetworkComponent campaignNetworkComponent = SCR_CampaignNetworkComponent.Cast(playerController.FindComponent(SCR_CampaignNetworkComponent));
		if (!campaignNetworkComponent)
			return;
		
		// Find the truck's supplies component ID
		RplId suppliesComponentID = Replication.FindId(m_SuppliesComponent);
		
		// Broadcast to other players that truck is already being unloaded
		if (suppliesComponentID != RplId.Invalid())
			campaignNetworkComponent.StartLoading(suppliesComponentID, m_iSuppliesToUnload, true);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActionCanceled(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		// Find local player controller
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return;
		
		// Find conflict network component to send RPC to server
		SCR_CampaignNetworkComponent campaignNetworkComponent = SCR_CampaignNetworkComponent.Cast(playerController.FindComponent(SCR_CampaignNetworkComponent));
		if (!campaignNetworkComponent)
			return;
		
		// Find the truck's supplies component ID
		RplId suppliesComponentID = Replication.FindId(m_SuppliesComponent);
		
		// Broadcast to other players that truck is no longer being unloaded
		if (suppliesComponentID != RplId.Invalid())
			campaignNetworkComponent.StopLoading(suppliesComponentID, true);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		switch (m_iCanUnloadSuppliesResult)
		{
			case SCR_CampaignSuppliesInteractionFeedback.EMPTY:
			{
				SetCannotPerformReason("#AR-Campaign_Action_SupplyTruckEmpty-UC");
				//SetCannotPerformReason("EMPTY");
				break;
			}
			
			case SCR_CampaignSuppliesInteractionFeedback.BASE_ENEMY:
			{
				SetCannotPerformReason("#AR-Campaign_Action_WrongBase-UC");
				//SetCannotPerformReason("ENEMY BASE");
				break;
			}
			
			case SCR_CampaignSuppliesInteractionFeedback.BASE_FULL:
			{
				SetCannotPerformReason("#AR-Campaign_Action_BaseFull-UC");
				//SetCannotPerformReason("STORAGE FULL");
				break;
			}
		}
		
		if (m_iCanUnloadSuppliesResult == SCR_CampaignSuppliesInteractionFeedback.POSSIBLE)
			return true;
		else
			return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		#ifndef AR_CAMPAIGN_TIMESTAMP
		if (Replication.Time() >= m_fNextConditionCheck)
		#else
		ChimeraWorld world = GetGame().GetWorld();
		if (world.GetServerTimestamp().GreaterEqual(m_fNextConditionCheck))
		#endif
		{
			m_iCanUnloadSuppliesResult = CanUnloadSupplies(user);
			#ifndef AR_CAMPAIGN_TIMESTAMP
			m_fNextConditionCheck += 250;
			#else
			m_fNextConditionCheck = m_fNextConditionCheck.PlusMilliseconds(250);
			#endif
		}
		
		if (m_iCanUnloadSuppliesResult == SCR_CampaignSuppliesInteractionFeedback.DO_NOT_SHOW)
			return false;
		else
			return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{
		// Find local player controller
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return;
		
		// Find conflict network component to send RPC to server
		SCR_CampaignNetworkComponent campaignNetworkComponent = SCR_CampaignNetworkComponent.Cast(playerController.FindComponent(SCR_CampaignNetworkComponent));
		if (!campaignNetworkComponent)
			return;
		
		// Find the truck's supplies component ID
		RplId suppliesComponentID = Replication.FindId(m_SuppliesComponent);
		
		if (suppliesComponentID != -1)
		{
			int suppliesToUnload;
			int suppliesCarried = m_SuppliesComponent.GetSupplies();
			
			if (m_iSuppliesToUnload != m_SuppliesComponent.GetSuppliesMax())
				suppliesToUnload = m_iSuppliesToUnload;
			else
				suppliesToUnload = suppliesCarried;
			
			campaignNetworkComponent.UnloadSupplies(suppliesComponentID, pUserEntity, m_Base, suppliesToUnload);
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
		outName = "#AR-Campaign_Action_UnloadSupplies-UC";
		
		if (m_iSuppliesToUnload == m_SuppliesComponent.GetSuppliesMax())
		{
			int carried = m_SuppliesComponent.GetSupplies();
			int canFitInBase = m_Base.GetSuppliesMax() - m_Base.GetSupplies();
			
			// Show how much can be stored in the base from the truck
			// If the base storage is full, show how much the truck is carrying
			if (carried > canFitInBase && canFitInBase == 0)
				ActionNameParams[0] = string.ToString(carried);
			else
				ActionNameParams[0] = string.ToString(Math.Min(carried, canFitInBase));
			
			if (carried > canFitInBase && canFitInBase != 0)
				outName = "#AR-Campaign_Action_UnloadSuppliesPartial-UC" + " (" + "#AR-Campaign_Action_BaseFull-UC" + ")";
		}
		else
		{
			ActionNameParams[0] = string.ToString(m_iSuppliesToUnload);
			outName = "#AR-Campaign_Action_UnloadSuppliesPartial-UC";
		}
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check the availability of this user action
	//! \param player Player trying to unload supplies
	int CanUnloadSupplies(IEntity player)
	{
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		
		if (!campaign || !player || !m_SuppliesComponent)
			return SCR_CampaignSuppliesInteractionFeedback.DO_NOT_SHOW;
		
		SCR_CampaignFeedbackComponent feedbackComponent = SCR_CampaignFeedbackComponent.GetInstance();
		
		if (!feedbackComponent)
			return SCR_CampaignSuppliesInteractionFeedback.DO_NOT_SHOW;
		
		m_Base = feedbackComponent.GetBaseWithPlayer();
		
		if (!m_Base)
			return SCR_CampaignSuppliesInteractionFeedback.DO_NOT_SHOW;
		
		Faction playerFaction = SCR_FactionManager.SGetLocalPlayerFaction();
		
		if (!playerFaction)
			return SCR_CampaignSuppliesInteractionFeedback.DO_NOT_SHOW;
		
		SCR_CampaignSuppliesComponent baseSuppliesComponent = SCR_CampaignSuppliesComponent.Cast(m_Base.GetOwner().FindComponent(SCR_CampaignSuppliesComponent));
		if (!baseSuppliesComponent)	
			return SCR_CampaignSuppliesInteractionFeedback.DO_NOT_SHOW;
		
		IEntity truck = SCR_EntityHelper.GetMainParent(GetOwner(), true);
		
		if (truck)
		{
			DamageManagerComponent damageManager = DamageManagerComponent.Cast(truck.FindComponent(DamageManagerComponent));
			
			// No action if the truck is destroyed
			if (damageManager.GetState() == EDamageState.DESTROYED)
				return SCR_CampaignSuppliesInteractionFeedback.DO_NOT_SHOW;
		}
		
		if (vector.DistanceSq(m_Box.GetOrigin(), m_Base.GetOwner().GetOrigin()) > Math.Pow(baseSuppliesComponent.GetOperationalRadius(), 2))
		{
			SCR_ServicePointComponent service = m_Base.GetServiceByType(SCR_EServicePointType.SUPPLY_DEPOT);
			if (!service)
				return SCR_CampaignSuppliesInteractionFeedback.DO_NOT_SHOW;
			
			if (vector.DistanceSq(m_Box.GetOrigin(), service.GetOwner().GetOrigin()) > Math.Pow(baseSuppliesComponent.GetOperationalRadius(), 2))
				return SCR_CampaignSuppliesInteractionFeedback.DO_NOT_SHOW;
		}
		
		// Player can unload supplies only in bases owned by his faction
		if (m_Base.GetFaction() != playerFaction)
			return SCR_CampaignSuppliesInteractionFeedback.BASE_ENEMY;
		
		int suppliesToUnload;
		int suppliesCarried = m_SuppliesComponent.GetSupplies();
		bool isMainUnloadAction = m_iSuppliesToUnload == m_SuppliesComponent.GetSuppliesMax();
		
		if (!isMainUnloadAction)
			suppliesToUnload = m_iSuppliesToUnload;
		else
			suppliesToUnload = suppliesCarried;
		
		// Hide preset batches equal to the remaining supplies so they don't duplicate the "unload all" functionality
		// If the supplies amount is less than the preset batch, don't show action (unless the truck is completely empty, then inform on the "unload all" action)
		if (suppliesToUnload > suppliesCarried)
			return SCR_CampaignSuppliesInteractionFeedback.DO_NOT_SHOW;
		else
		{
			if (suppliesCarried == m_iSuppliesToUnload && !isMainUnloadAction)
				return SCR_CampaignSuppliesInteractionFeedback.DO_NOT_SHOW;
			else
				if (suppliesCarried == 0 && isMainUnloadAction)
					return SCR_CampaignSuppliesInteractionFeedback.DO_NOT_SHOW;
		}
		
		int canFitInBase = m_Base.GetSuppliesMax() - m_Base.GetSupplies();
		
		// Hide batches larger than or equal to supplies that can fit in the base
		if (canFitInBase != 0)
		{
			if (canFitInBase > m_iSuppliesToUnload)
				return SCR_CampaignSuppliesInteractionFeedback.POSSIBLE;
			else
			{
				if (isMainUnloadAction)
					return SCR_CampaignSuppliesInteractionFeedback.POSSIBLE;
				else
					return SCR_CampaignSuppliesInteractionFeedback.DO_NOT_SHOW;
			}
		}
		else
			if (isMainUnloadAction)
				return SCR_CampaignSuppliesInteractionFeedback.BASE_FULL;
			else
				return SCR_CampaignSuppliesInteractionFeedback.DO_NOT_SHOW;
	}
};