// Descriptive action showing that somebody is  unloading supplies to truck in Campaign
class SCR_CampaignUnloadingSuppliesUserAction : ScriptedUserAction
{
	//Member variables 
	protected SCR_CampaignSuppliesComponent m_SuppliesComponent;
	
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_SuppliesComponent = SCR_CampaignSuppliesComponent.Cast(pOwnerEntity.FindComponent(SCR_CampaignSuppliesComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		outName = "#AR-Campaign_SupplyUnloadInProgress-UC";
		return true;
	}
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		string playerName = GetGame().GetPlayerManager().GetPlayerName(m_SuppliesComponent.GetLoadingPlayer(true));
		SetCannotPerformReason(playerName);
		return false;
	}
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return false;
		
		if(m_SuppliesComponent.GetLoadingPlayer(true)!=playerController.GetPlayerId() && m_SuppliesComponent.GetLoadingPlayer(true)!=0)
			return true;
		
		return false;
	}
};