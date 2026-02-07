//------------------------------------------------------------------------------------------------
class SCR_TutorialBuildingStartUserAction : SCR_CampaignBuildingStartUserAction
{
	protected bool m_bCanShow;
	
	//------------------------------------------------------------------------------------------------
	void SetCanShow(bool canShow)
	{
		m_bCanShow = canShow;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (!m_bCanShow)
			return false;
		
		return super.CanBeShownScript(user);
	}
}
