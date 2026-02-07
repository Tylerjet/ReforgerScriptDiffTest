// Descriptive action showing that somebody is already loading supplies to truck in Campaign
class SCR_CampaignLoadingSuppliesUserAction : ScriptedUserAction
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
		outName = "#AR-Campaign_SupplyLoadInProgress-UC";
		return true;
	}
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		string playerName = GetGame().GetPlayerManager().GetPlayerName(m_SuppliesComponent.GetLoadingPlayer());
		SetCannotPerformReason(playerName);
		return false;
	}
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return false;
		
		if(m_SuppliesComponent.GetLoadingPlayer()!=playerController.GetPlayerId() && m_SuppliesComponent.GetLoadingPlayer()!=0)
			return true;
		
		return false;
	}
};