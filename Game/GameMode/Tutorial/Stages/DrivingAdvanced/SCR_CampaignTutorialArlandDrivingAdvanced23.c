[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandDrivingAdvanced23Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandDrivingAdvanced23 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_fDuration = 10;
		StopVehicleSmoke();
		
		SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage()));
		m_TutorialComponent.SetStagesComplete(5, true);	
		if (!m_TutorialComponent.GetVoiceSystem().IsPlaying())
			PlaySoundSystem("End", true);
		else
			GetGame().GetCallqueue().CallLater(PlaySoundSystem, 1000, false, "End", true);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void StopVehicleSmoke()
	{
		Vehicle car = m_TutorialComponent.GetHummer();
		if (!car)
			return;
		
		array <HitZone> hitzones = {};
		SCR_DamageManagerComponent damageManager = SCR_DamageManagerComponent.GetDamageManager(car);
		if (!damageManager)
			return;
		
		damageManager.GetAllHitZonesInHierarchy(hitzones);

		SCR_FlammableHitZone flammableHitZone;
		foreach (HitZone hitZone : hitzones)
		{
			flammableHitZone = SCR_FlammableHitZone.Cast(hitZone);
			if (flammableHitZone)
				flammableHitZone.SetFireState(SCR_EBurningState.NONE);
		}
	}
};