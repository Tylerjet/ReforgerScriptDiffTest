[EntityEditorProps(insertable: false)]
class SCR_BaseTourStageClass: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_BaseTourStage : SCR_BaseCampaignTutorialArlandStage
{
	IEntity m_AntennaPos, m_ArmoryPos, m_FieldHospitalPos, m_FuelDepotPos, m_HQPos, m_LivingQuartersPos, m_LightVehDepotPos, m_HeavyVehDepotPos, m_HeliportPos;
	ref array <IEntity> m_aVisitedServices = {};
	bool m_bIsShowingHint;
	
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_bCheckWaypoint = false;
		RegisterWaypoint("TeleportRadio");
		
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		GetGame().GetCallqueue().CallLater(PlaySoundSystem, 2000, false, "BaseTour_Start", false);
		GetGame().GetCallqueue().CallLater(HintOnVoiceOver, 2000, false);
		
		m_AntennaPos = GetGame().GetWorld().FindEntityByName("WP_Tour_Antenna");
		if (m_AntennaPos)
			RegisterWaypoint(m_AntennaPos);
		
		m_ArmoryPos = GetGame().GetWorld().FindEntityByName("WP_Tour_Armory");
		if (m_ArmoryPos)
			RegisterWaypoint(m_ArmoryPos);
		
		m_FieldHospitalPos = GetGame().GetWorld().FindEntityByName("WP_Tour_FieldHospital");
		if (m_FieldHospitalPos)
			RegisterWaypoint(m_FieldHospitalPos);
		
		m_FuelDepotPos = GetGame().GetWorld().FindEntityByName("WP_Tour_FuelDepot");
		if (m_FuelDepotPos)
			RegisterWaypoint(m_FuelDepotPos);
		
		m_HQPos = GetGame().GetWorld().FindEntityByName("WP_Tour_HQ");
		if (m_HQPos)
			RegisterWaypoint(m_HQPos);
		
		m_LivingQuartersPos = GetGame().GetWorld().FindEntityByName("WP_Tour_LivingQuarters");
		if (m_LivingQuartersPos)
			RegisterWaypoint(m_LivingQuartersPos);
		
		m_LightVehDepotPos = GetGame().GetWorld().FindEntityByName("WP_Tour_LightVehDepot");
		if (m_LightVehDepotPos)
			RegisterWaypoint(m_LightVehDepotPos);
		
		m_HeavyVehDepotPos = GetGame().GetWorld().FindEntityByName("WP_Tour_HeavyVehDepot");
		if (m_HeavyVehDepotPos)
			RegisterWaypoint(m_HeavyVehDepotPos);
		
		m_HeliportPos = GetGame().GetWorld().FindEntityByName("WP_Tour_Heliport");
		if (m_HeliportPos)
			RegisterWaypoint(m_HeliportPos);
		
		GetGame().GetCallqueue().CallLater(CheckVicinity, 1000, true);
		
		IEntity base = GetGame().GetWorld().FindEntityByName("MainBaseHQ");
		if (!base)
			return;
		
		SCR_CampaignMilitaryBaseComponent baseComp = SCR_CampaignMilitaryBaseComponent.Cast(base.FindComponent(SCR_CampaignMilitaryBaseComponent));
		if (baseComp)
			baseComp.SetSupplies(0);
		
		m_TutorialComponent.SetStagesComplete(7, true);	
	}
	
	
	
	//------------------------------------------------------------------------------------------------
	protected bool IsInRange(IEntity ent, float range)
	{
		vector playerPos = m_Player.GetOrigin();
		float sqDistance = vector.DistanceSq(playerPos, ent.GetOrigin());
		
		return sqDistance < range*range;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ShowServiceHint(string serviceName)
	{
		if (m_bIsShowingHint)
			return;
		
		SCR_HintManagerComponent.ClearLatestHint();
		
		SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage(), serviceName));
		m_bIsShowingHint = true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CheckVicinity()
	{
		if (!IsInRange(m_HQPos, 100))
		{
			m_TutorialComponent.SetActiveConfig(SCR_ETutorialArlandStageMasters.HUB);
		}
		else if (IsInRange(m_AntennaPos, 8))
		{
			if (!m_aVisitedServices.Contains(m_AntennaPos))
			{
				CancelHintOnVoiceOver();
				SCR_HintManagerComponent.HideHint();
				SCR_HintManagerComponent.ClearLatestHint();
				m_aVisitedServices.Insert(m_AntennaPos);
				PlaySoundSystem("BaseTour_Antenna");	
			}
			else
			{
				if (!m_TutorialComponent.GetVoiceSystem().IsPlaying())
					ShowServiceHint("Antenna");
			}
			
			return;
		}
		else if (IsInRange(m_ArmoryPos, 8))
		{
			if (!m_aVisitedServices.Contains(m_ArmoryPos))
			{
				CancelHintOnVoiceOver();
				SCR_HintManagerComponent.HideHint();
				SCR_HintManagerComponent.ClearLatestHint();
				m_aVisitedServices.Insert(m_ArmoryPos);
				PlaySoundSystem("BaseTour_Armory", true);	
			}
			else
			{
				if (!m_TutorialComponent.GetVoiceSystem().IsPlaying())
					ShowServiceHint("Armory");
			}
			
			return;
		}
		else if (IsInRange(m_FieldHospitalPos, 14))
		{
			if (!m_aVisitedServices.Contains(m_FieldHospitalPos))
			{
				CancelHintOnVoiceOver();
				SCR_HintManagerComponent.HideHint();
				SCR_HintManagerComponent.ClearLatestHint();
				m_aVisitedServices.Insert(m_FieldHospitalPos);
				PlaySoundSystem("BaseTour_Hospital", true);	
			}
			else
			{
				if (!m_TutorialComponent.GetVoiceSystem().IsPlaying())
					ShowServiceHint("Hospital");
			}
			
			return;
		}
		else if (IsInRange(m_FuelDepotPos, 8))
		{
			if (!m_aVisitedServices.Contains(m_FuelDepotPos))
			{
				CancelHintOnVoiceOver();
				SCR_HintManagerComponent.HideHint();
				SCR_HintManagerComponent.ClearLatestHint();
				m_aVisitedServices.Insert(m_FuelDepotPos);
				PlaySoundSystem("BaseTour_Fuel", true);	
			}
			else
			{
				if (!m_TutorialComponent.GetVoiceSystem().IsPlaying())
					ShowServiceHint("Fuel");
			}
			
			return;
		}
		else if (IsInRange(m_LivingQuartersPos, 8))
		{
			if (!m_aVisitedServices.Contains(m_LivingQuartersPos))
			{
				CancelHintOnVoiceOver();
				SCR_HintManagerComponent.HideHint();
				SCR_HintManagerComponent.ClearLatestHint();
				m_aVisitedServices.Insert(m_LivingQuartersPos);
				PlaySoundSystem("BaseTour_LivingQuarters", true);	
			}
			else
			{
				if (!m_TutorialComponent.GetVoiceSystem().IsPlaying())
					ShowServiceHint("Living");
			}
			
			return;
		}
		else if (IsInRange(m_HQPos, 8))
		{
			if (!m_aVisitedServices.Contains(m_HQPos))
			{
				CancelHintOnVoiceOver();
				SCR_HintManagerComponent.HideHint();
				SCR_HintManagerComponent.ClearLatestHint();
				m_aVisitedServices.Insert(m_HQPos);
				PlaySoundSystem("BaseTour_HQTents");	
			}
			else
			{
				if (!m_TutorialComponent.GetVoiceSystem().IsPlaying())
					ShowServiceHint("HQ");
			}
			
			return;
		}
		else if (IsInRange(m_LightVehDepotPos, 8))
		{
			if (!m_aVisitedServices.Contains(m_LightVehDepotPos))
			{
				CancelHintOnVoiceOver();
				SCR_HintManagerComponent.HideHint();
				SCR_HintManagerComponent.ClearLatestHint();
				m_aVisitedServices.Insert(m_LightVehDepotPos);
				PlaySoundSystem("BaseTour_LightVeh", true);	
			}
			else
			{
				if (!m_TutorialComponent.GetVoiceSystem().IsPlaying())
					ShowServiceHint("LightVeh");
			}
			
			return;
		}
		else if (IsInRange(m_HeavyVehDepotPos, 18))
		{
			if (!m_aVisitedServices.Contains(m_HeavyVehDepotPos))
			{
				CancelHintOnVoiceOver();
				SCR_HintManagerComponent.HideHint();
				SCR_HintManagerComponent.ClearLatestHint();
				m_aVisitedServices.Insert(m_HeavyVehDepotPos);
				PlaySoundSystem("BaseTour_HeavyVeh", true);	
			}
			else
			{
				if (!m_TutorialComponent.GetVoiceSystem().IsPlaying())
					ShowServiceHint("HeavyVeh");
			}
			
			return;
		}
		else if (IsInRange(m_HeliportPos, 12))
		{
			if (!m_aVisitedServices.Contains(m_HeliportPos))
			{
				CancelHintOnVoiceOver();
				SCR_HintManagerComponent.HideHint();
				SCR_HintManagerComponent.ClearLatestHint();
				m_aVisitedServices.Insert(m_HeliportPos);
				PlaySoundSystem("BaseTour_Heli", true);	
			}
			else
			{
				if (!m_TutorialComponent.GetVoiceSystem().IsPlaying())
					ShowServiceHint("Heli");
			}
			
			return;
		}
		else
		{
			if (m_bIsShowingHint && !m_TutorialComponent.GetVoiceSystem().IsPlaying())
				SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage(), "Default"));
			
			m_bIsShowingHint = false;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_BaseTourStage()
	{
		GetGame().GetCallqueue().Remove(CheckVicinity);
	}
};