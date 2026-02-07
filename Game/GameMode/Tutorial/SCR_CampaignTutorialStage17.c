class SCR_CampaignTutorialStage17Class: SCR_BaseCampaignTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialStage17 : SCR_BaseCampaignTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_WEAPON_PICK");
		m_fWaypointHeightOffset = 0;
		m_bCheckWaypoint = false;
		IEntity gun = GetGame().GetWorld().FindEntityByName("M16");
				
		if (gun)
			m_WaypointEntity.SetOrigin(gun.GetOrigin());
		
		SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_WeaponEquip" + CreateString("#AR-WeaponMenu_DescEquip","CharacterAction") + CreateString("#AR-KeybindEditor_MultiSelection","SelectAction"), duration: -1);	
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		BaseWeaponManagerComponent weaponManager = BaseWeaponManagerComponent.Cast(m_Player.FindComponent(BaseWeaponManagerComponent));
		
		if (weaponManager)	
			return weaponManager.GetCurrent();
		else
			return true;
	}
};