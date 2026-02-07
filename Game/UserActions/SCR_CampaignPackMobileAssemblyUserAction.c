class SCR_CampaignPackMobileAssemblyUserAction : SCR_CampaignDeployMobileAssemblyUserAction
{
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (!m_AssemblyComponent)
			return false;
		
		return m_AssemblyComponent.IsDeployed();
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{	
		PlayerController playerController = GetGame().GetPlayerController();
		
		if (!playerController)
			return;
		
		SCR_CampaignNetworkComponent campaignNetworkComponent = SCR_CampaignNetworkComponent.Cast(playerController.FindComponent(SCR_CampaignNetworkComponent));
		
		if (!campaignNetworkComponent)
			return;
		
		campaignNetworkComponent.DeployMobileAsembly(m_AssemblyComponent, false);
	}
};