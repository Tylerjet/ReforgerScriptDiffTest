[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandStageShooting15Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandStageShooting15 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_BOARDING");
		m_bCheckWaypoint = false;
		IEntity m2 = GetGame().GetWorld().FindEntityByName("TheGun");
		GetWaypoint().SetOrigin(m2.GetOrigin());
		GetGame().GetCallqueue().CallLater(DelayedPopup, 1000, false, "#AR-Tutorial_Popup_Title-UC", "#AR-Tutorial_Popup_WeaponLowering", 12, "", "", "<color rgba='226,168,79,200'><shadow mode='image' color='0,0,0' size='1' offset='1,1' opacity = '0.5'><action name = 'CharacterRaiseWeapon'/></shadow></color>", "");
		
		foreach (SCR_FiringRangeTarget target : m_TutorialComponent.GetAllTargets())
		{		
			if (target.GetSetDistance() < 100)
			{
				target.SetState(ETargetState.TARGET_DOWN);
				target.SetAutoResetTarget(false);
				target.Event_TargetChangeState.Remove(m_TutorialComponent.CountTargetHit);
			}
		}
		
		
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		PlaySoundSystem("GetInTurret", false);
		HintOnVoiceOver();
		
		m_TutorialComponent.SetWaypointMiscImage("GETIN", true);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return (m_TutorialComponent.CheckCharacterInVehicle(ECompartmentType.TURRET));
	}
};