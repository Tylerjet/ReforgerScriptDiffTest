//! Action to request a vehicle at a depot in Campaign
class SCR_CampaignRequestVehicleAssetUserAction : ScriptedUserAction
{
	// Member variables
	protected SCR_CampaignBase m_Base;
	protected SCR_CampaignDeliveryPoint m_Depot;
	protected int m_iCanRequestResult = SCR_CampaignAssetRequestDeniedReason.DO_NOT_SHOW;

	//------------------------------------------------------------------------------------------------
	//! Translates action ID to asset ID (its index in asset list)
	protected int GetUsableActionID()
	{
		return (GetActionID() / 2) - 1;
	}
	
	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		// Register necessary data
		IEntity parent = pOwnerEntity;
		
		while (parent && (!m_Depot || !m_Base))
		{
			switch (parent.Type())
			{
				case SCR_CampaignDeliveryPoint: {m_Depot = SCR_CampaignDeliveryPoint.Cast(parent); break;};
				case SCR_CampaignBase: {m_Base = SCR_CampaignBase.Cast(parent); break;};
			}
			
			parent = parent.GetParent();
		}
		
		if (m_Base)
			m_Base.RegisterGarageBoard(pOwnerEntity);
	} 
	
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{
		// Find local player controller
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return;
		
		if (!m_Depot.CanRequest(playerController.GetPlayerId(), GetUsableActionID()))
			return;
		
		// Find campaign network component to send RPC to server
		SCR_CampaignNetworkComponent campaignNetworkComponent = SCR_CampaignNetworkComponent.Cast(playerController.FindComponent(SCR_CampaignNetworkComponent));
		if (campaignNetworkComponent)
			campaignNetworkComponent.SpawnVehicle(m_Depot, GetUsableActionID());
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{	
		switch (m_iCanRequestResult)
		{
			case SCR_CampaignAssetRequestDeniedReason.NO_SIGNAL:
			{
				SetCannotPerformReason("#AR-Campaign_Action_NoSignal-UC");
				break;
			}
			
			case SCR_CampaignAssetRequestDeniedReason.OUT_OF_STOCK:
			{
				SetCannotPerformReason("#AR-Campaign_Action_Depleted-UC");
				break;
			}
			
			case SCR_CampaignAssetRequestDeniedReason.COOLDOWN:
			{
				SetCannotPerformReason("#AR-Campaign_Action_Cooldown-UC");
				break;
			}
			
			case SCR_CampaignAssetRequestDeniedReason.RANK_LOW:
			{
				SCR_CampaignFaction faction = SCR_CampaignFaction.Cast(SCR_RespawnSystemComponent.GetLocalPlayerFaction());
			
				if (!faction)
					return false;
				
				string minRankName = faction.GetRankNameUpperCase(SCR_CampaignFactionManager.GetInstance().GetVehicleAssetMinimumRank(GetUsableActionID()));
				SetCannotPerformReason(minRankName);
				break;
			}
		}
	
		if (m_iCanRequestResult == SCR_CampaignAssetRequestDeniedReason.POSSIBLE)
			return true;
		else
			return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{

		m_iCanRequestResult = m_Depot.CanRequest(SCR_PlayerController.GetLocalPlayerId(), GetUsableActionID());

		if (m_iCanRequestResult == SCR_CampaignAssetRequestDeniedReason.DO_NOT_SHOW)
			return false;
		else
			return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		//Print("GetActionNameScript :: " + GetUsableActionID());
		// Show proper asset name in action's UI
		//outName = ("REQUEST " + factionManager.GetVehicleAssetDisplayName(GetUsableActionID()) + " [ " + m_Depot.GetStockSize(GetUsableActionID()) + " ]");
		ActionNameParams[0] = SCR_CampaignFactionManager.GetInstance().GetVehicleAssetDisplayNameUpperCase(GetUsableActionID());
		ActionNameParams[1] = string.ToString(m_Depot.GetStockSize(GetUsableActionID()));
		outName = "#AR-Campaign_Action_RequestVehicle-UC";
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool HasLocalEffectOnlyScript()
	{
		return true;
	}
};

enum SCR_CampaignAssetRequestDeniedReason
{
	DO_NOT_SHOW = 0,
	POSSIBLE = 1,
	COOLDOWN = 2,
	RANK_LOW = 3,
	NO_SIGNAL = 4,
	OUT_OF_STOCK = 5
};
