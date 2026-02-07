class SCR_CampaignTutorialStage19Class: SCR_BaseCampaignTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialStage19 : SCR_BaseCampaignTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_MagazineLoading"+ CreateString("#AR-Keybind_Reload","CharacterReload"), duration: -1);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		BaseWeaponManagerComponent weaponManager = BaseWeaponManagerComponent.Cast(m_Player.FindComponent(BaseWeaponManagerComponent));
		
		if (!weaponManager)
			return false;

		BaseWeaponComponent weaponComponent = weaponManager.GetCurrent();
		
		if (!weaponComponent)
			return false;
		
		BaseMagazineComponent mag = weaponComponent.GetCurrentMagazine();
		
		if (mag)
			return (mag.GetAmmoCount() != 0);
		else
			return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override void Reset()
	{
		m_TutorialComponent.StageReset_RifleRespawn();
	}
};