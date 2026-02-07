#include "scripts/Game/config.c"
[EntityEditorProps(category: "GameScripted/Campaign", description: "Allow respawning at this vehicle in Campaign", color: "0 0 255 255")]
class SCR_CampaignMobileAssemblyComponentClass: ScriptComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignMobileAssemblyComponent : ScriptComponent
{
	static ref ScriptInvoker s_OnSpawnPointOwnerChanged = new ScriptInvoker();
	static ref ScriptInvoker s_OnUpdateRespawnCooldown = new ScriptInvoker();
	
	static const int RESPAWN_COOLDOWN = 60000;
	static const float MAX_WATER_DEPTH = 2.5;
	
	protected RplComponent m_RplComponent;
	protected MapItem m_MapItem;
	protected SCR_CampaignFaction m_Faction;
	protected SCR_CampaignFaction m_ParentFaction;
	protected bool m_bIsInRadioRange;
	protected bool m_bCooldownDone = true;
	protected RplId m_RplId = RplId.Invalid();
	protected ref array<SCR_CampaignMilitaryBaseComponent> m_aBasesInRadioRange = {};
	protected SCR_SpawnPoint m_SpawnPoint;
	protected bool m_bIsHovered;
	protected ref array<MapLink> m_aMapLinks = {};
	
	[RplProp(onRplName: "OnParentFactionIDSet")]
	protected int m_iParentFaction = SCR_CampaignMilitaryBaseComponent.INVALID_FACTION_INDEX;
	[RplProp(onRplName: "OnFactionChanged")]
	protected int m_iFaction = SCR_CampaignMilitaryBaseComponent.INVALID_FACTION_INDEX;
	[RplProp()]
	#ifndef AR_CAMPAIGN_TIMESTAMP
	protected float m_fRespawnAvailableSince = float.MAX;
	#else
	protected WorldTimestamp m_fRespawnAvailableSince;
	#endif
	[RplProp()]
	protected int m_iBasesCoveredByMHQOnly;
	[RplProp(onRplName: "OnDeployChanged")]
	protected bool m_bIsDeployed;
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		GetGame().GetCallqueue().CallLater(AssignSpawnpoint, 1000, false);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		
		if (!campaign)
			return;
		
		super.OnPostInit(owner);
		SetEventMask(owner, EntityEvent.INIT);
		
		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		
		GetGame().GetCallqueue().CallLater(UpdateRadioCoverage, 1000, true);
		
		if (IsProxy())
		{
			campaign.GetInstance().GetOnFactionAssignedLocalPlayer().Insert(OnParentFactionIDSet);
			campaign.GetInstance().GetOnFactionAssignedLocalPlayer().Insert(OnDeployChanged);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		super.OnDelete(owner);
		
		GetGame().GetCallqueue().Remove(UpdateRadioCoverage);
		GetGame().GetCallqueue().Remove(UpdateRespawnCooldown);	
		
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		
		if (!campaign)
			return;
		
		campaign.GetInstance().GetOnFactionAssignedLocalPlayer().Remove(OnParentFactionIDSet);
		campaign.GetInstance().GetOnFactionAssignedLocalPlayer().Remove(OnDeployChanged);
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateBasesInRadioRange()
	{
		array<SCR_MilitaryBaseComponent> bases = {};
		SCR_MilitaryBaseManager.GetInstance().GetBases(bases);
		
		float radioRange = GetRadioRange();
		radioRange = radioRange * radioRange;	// We're checking square distance
		vector truckPosition = GetOwner().GetOrigin();
		SCR_CampaignMilitaryBaseComponent campaignBase;
		
		m_aBasesInRadioRange = {};
		
		foreach (SCR_MilitaryBaseComponent base : bases)
		{
			campaignBase = SCR_CampaignMilitaryBaseComponent.Cast(base);
			
			if (!campaignBase || !campaignBase.IsInitialized())
				continue;
			
			if (campaignBase.IsHQ() && campaignBase.GetFaction() != m_ParentFaction)
				continue;
			
			if (vector.DistanceSqXZ(truckPosition, campaignBase.GetOwner().GetOrigin()) < radioRange)
				m_aBasesInRadioRange.Insert(campaignBase);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	bool CanReachByRadio(notnull SCR_CampaignMilitaryBaseComponent base)
	{
		return m_aBasesInRadioRange.Contains(base);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void AssignSpawnpoint()
	{
		//getting spawn point entity from prefab
		IEntity child = GetOwner().GetChildren();
		while(child)
		{
			if (child.Type() == SCR_SpawnPoint)
			{
				m_SpawnPoint = SCR_SpawnPoint.Cast(child);
				GetGame().GetCallqueue().Remove(AssignSpawnpoint);
				m_SpawnPoint.SetOrigin(m_SpawnPoint.GetOrigin() + vector.Up)
			}
				
			child = child.GetSibling();
		}
		
		SCR_MapDescriptorComponent descriptor = SCR_MapDescriptorComponent.Cast(GetOwner().FindComponent(SCR_MapDescriptorComponent));
		
		if (descriptor)
		{
			m_MapItem = descriptor.Item();
			
			if (m_MapItem)
				m_MapItem.SetDisplayName("#AR-Vehicle_MobileAssembly_Name");
		}
		
		if (m_Faction && m_SpawnPoint.GetFactionKey() != m_Faction.GetFactionKey())
			OnFactionChanged();
	}
	
	//------------------------------------------------------------------------------------------------
	void OnIconHovered(SCR_CampaignMapUIBase icon, bool hovering)
	{
		m_bIsHovered = hovering;
		for (int i = m_aMapLinks.Count() - 1; i >= 0; i--)
		{
			if (!m_aMapLinks[i])
				continue;
			
			ColorMapLink(m_aMapLinks[i]);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void ColorMapLink(notnull MapLink link)
	{
		if (!m_ParentFaction)
			return;
		
		MapLinkProps props = link.GetMapLinkProps();
		
		if (!props)
			return;
		
		SCR_CampaignMilitaryBaseComponent hq = m_ParentFaction.GetMainBase();
		
		if (!hq)
			return;
		
		props.SetLineWidth(hq.GetLineWidth());
		
		Color c = props.GetLineColor();
		
		if (!c)
			return;
		
		SCR_GraphLinesData linesData = hq.GetGraphLinesData();
		
		if (m_bIsHovered)
		{
			// Highlight only bases in range
			MapItem otherEnd = link.Target();
			
			if (otherEnd == m_MapItem)
				otherEnd = link.Owner();
			
			IEntity otherEndOwner = otherEnd.Entity();
			
			if (!otherEndOwner)
				return;
			
			SCR_CampaignMilitaryBaseComponent base = SCR_CampaignMilitaryBaseComponent.Cast(otherEndOwner.FindComponent(SCR_CampaignMilitaryBaseComponent));
			
			if (!base)
				return;
			
			if (!m_aBasesInRadioRange.Contains(base))
				return;
			
			c.SetA(linesData.GetHighlightedAlpha());	
		}
		else
		{
			c.SetA(linesData.GetDefaultAlpha());
		}
		
		props.SetLineColor(c);
	}
	
	//------------------------------------------------------------------------------------------------	
	void AddMapLink(notnull MapLink link)
	{
		m_aMapLinks.Insert(link);
	}
	
	//------------------------------------------------------------------------------------------------	
	protected bool IsProxy()
	{
		return (m_RplComponent && m_RplComponent.IsProxy());
	}
	
	//------------------------------------------------------------------------------------------------
	void SetParentFactionID(int factionID)
	{
		m_iParentFaction = factionID;
		Replication.BumpMe();
		OnParentFactionIDSet();
	}
	
	//------------------------------------------------------------------------------------------------
	void OnParentFactionIDSet()
	{
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		
		if (!campaign)
			return;
		
		Faction playerFaction = SCR_FactionManager.SGetLocalPlayerFaction();
		SCR_CampaignFactionManager fManager = SCR_CampaignFactionManager.Cast(GetGame().GetFactionManager());
		m_ParentFaction = SCR_CampaignFaction.Cast(fManager.GetFactionByIndex(m_iParentFaction));
		
		if (!m_ParentFaction)
			return;
		
		if (m_bIsDeployed)
			m_ParentFaction.SetMobileAssembly(this);
		
		if (!m_MapItem || m_ParentFaction != playerFaction)
			return;
		
		switch (m_ParentFaction.GetFactionKey())
		{
			case campaign.GetFactionKeyByEnum(SCR_ECampaignFaction.BLUFOR):
			{
				m_MapItem.SetFactionIndex(EFactionMapID.WEST);
				break;
			}
		
			case campaign.GetFactionKeyByEnum(SCR_ECampaignFaction.OPFOR):
			{
				m_MapItem.SetFactionIndex(EFactionMapID.EAST);
				break;
			}
			
			default:
			{
				m_MapItem.SetFactionIndex(EFactionMapID.UNKNOWN);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	int GetParentFactionID()
	{
		return m_iParentFaction;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_CampaignFaction GetParentFaction()
	{
		return m_ParentFaction;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_CampaignFaction GetFaction()
	{
		return m_Faction;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetBasesInRange(notnull out array<SCR_CampaignMilitaryBaseComponent> basesInRange)
	{
		return basesInRange.Copy(m_aBasesInRadioRange);
	}
	
	//------------------------------------------------------------------------------------------------
	int GetRadioRange()
	{
		IEntity truck = GetOwner().GetParent();
		
		if (!truck)
			return 0;
		
		BaseRadioComponent comp = BaseRadioComponent.Cast(truck.FindComponent(BaseRadioComponent));
		
		if (!comp)
			return 0;
		
		BaseTransceiver tsv = comp.GetTransceiver(0);
		
		if (!tsv)
			return 0;
		
		return tsv.GetRange();
	}
	
	//------------------------------------------------------------------------------------------------
	bool Deploy(SCR_EMobileAssemblyStatus status, int playerId = 0)
	{
		if (!m_bIsInRadioRange && status == SCR_EMobileAssemblyStatus.DEPLOYED)
			return false;
		
		if (!m_ParentFaction)
			return false;
		
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		
		if (!campaign)
			return false;
		
		if (m_ParentFaction.GetMobileAssembly() && status == SCR_EMobileAssemblyStatus.DEPLOYED)
			return false;
		
		if (status == SCR_EMobileAssemblyStatus.DEPLOYED)
			m_bIsDeployed = true;
		else
			m_bIsDeployed = false;
		
		OnDeployChanged();
		Replication.BumpMe();
		
		Rpc(RpcDo_BroadcastFeedback, status, playerId, GetGame().GetFactionManager().GetFactionIndex(m_ParentFaction));
		
		if (RplSession.Mode() != RplMode.Dedicated)
			RpcDo_BroadcastFeedback(status, playerId, GetGame().GetFactionManager().GetFactionIndex(m_ParentFaction));	
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_BroadcastFeedback(SCR_EMobileAssemblyStatus msgID, int playerID, int factionID)
	{
		SCR_CampaignFeedbackComponent comp = SCR_CampaignFeedbackComponent.GetInstance();
		
		if (!comp)
			return;
		
		comp.MobileAssemblyFeedback(msgID, playerID, factionID)
	}
	
	//------------------------------------------------------------------------------------------------
	void OnDeployChanged()
	{
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		
		if (!campaign)
			return;
		
		Faction playerFaction = SCR_FactionManager.SGetLocalPlayerFaction();
		BaseRadioComponent radioComponent;
		
		IEntity vehicle = GetOwner().GetParent();
		
		if (vehicle)
			radioComponent = BaseRadioComponent.Cast(vehicle.FindComponent(BaseRadioComponent));
		
		array<SCR_CampaignMilitaryBaseComponent> previouslyLinkedBases = {};
		previouslyLinkedBases.Copy(m_aBasesInRadioRange);
		
		if (m_bIsDeployed)
		{
			UpdateBasesInRadioRange();
			GetGame().GetCallqueue().CallLater(UpdateRespawnCooldown, 250, true, null);
			
			if (m_ParentFaction)
				m_ParentFaction.SetMobileAssembly(this);
			
			if (!IsProxy())
			{
				// Only update respawn cooldown if a load state has not been applied
				#ifndef AR_CAMPAIGN_TIMESTAMP
				if (GetGame().GetWorld().GetWorldTime() > 10000)
				{
					m_fRespawnAvailableSince = Replication.Time() + RESPAWN_COOLDOWN;
					m_bCooldownDone = false;
				}
				else
				{
					m_fRespawnAvailableSince = Replication.Time();
					m_bCooldownDone = true;
				}
				#else
				ChimeraWorld world = GetGame().GetWorld();
				if (world.GetWorldTime() > 10000)
				{
					m_fRespawnAvailableSince = world.GetServerTimestamp().PlusMilliseconds(RESPAWN_COOLDOWN);
					m_bCooldownDone = false;
				}
				else
				{
					m_fRespawnAvailableSince = world.GetServerTimestamp();
					m_bCooldownDone = true;
				}
				#endif
				Replication.BumpMe();
				
				if (radioComponent)
					radioComponent.SetPower(true);
				
				if (GetTaskManager())
				{
					SCR_CampaignTaskSupportEntity supportClass = SCR_CampaignTaskSupportEntity.Cast(GetTaskManager().FindSupportEntity(SCR_CampaignTaskSupportEntity));
					
					if (supportClass)
						supportClass.GenerateCaptureTasks(GetOwner());
				}
				
				GetGame().GetCallqueue().CallLater(CheckStatus, 500, true);
			}
			
			if (RplSession.Mode() != RplMode.Dedicated)
			{
				if (m_MapItem && m_ParentFaction == playerFaction)
					m_MapItem.SetVisible(true);
			}
		}
		else
		{
			m_aBasesInRadioRange = {};
			GetGame().GetCallqueue().Remove(UpdateRespawnCooldown);
			
			if (m_ParentFaction)
				m_ParentFaction.SetMobileAssembly(null);
			
			if (!IsProxy())
			{
				m_iFaction = SCR_CampaignMilitaryBaseComponent.INVALID_FACTION_INDEX;
				#ifndef AR_CAMPAIGN_TIMESTAMP
				m_fRespawnAvailableSince = float.MAX;
				#else
				m_fRespawnAvailableSince = null;
				#endif
				Replication.BumpMe();
				
				if (radioComponent)
					radioComponent.SetPower(false);
				
				GetGame().GetCallqueue().Remove(CheckStatus);
			}
			
			if (m_MapItem && RplSession.Mode() != RplMode.Dedicated)
				m_MapItem.SetVisible(false);

			OnFactionChanged();
		}
		
		if (!IsProxy())
			campaign.GetBaseManager().RecalculateRadioConverage(m_ParentFaction);
		
		if (RplSession.Mode() == RplMode.Dedicated)
			return;
		
		foreach (SCR_CampaignMilitaryBaseComponent base: previouslyLinkedBases)
		{
			if (!m_aBasesInRadioRange.Contains(base))
				base.GetMapDescriptor().HandleMapLinks();
		}
		
		if (m_bIsDeployed)
			foreach (SCR_CampaignMilitaryBaseComponent base: m_aBasesInRadioRange)
				base.GetMapDescriptor().HandleMapLinks();
		else
			m_aMapLinks.Clear();
	}
	
	//------------------------------------------------------------------------------------------------
	void CheckStatus()
	{
		IEntity truck = GetOwner().GetParent();
		
		if (!truck)
			return;
		
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
			
		if (!campaign)
			return;
		
		DamageManagerComponent damageComponent = DamageManagerComponent.Cast(truck.FindComponent(DamageManagerComponent));
		
		// Destroyed?
		if (damageComponent && damageComponent.GetState() == EDamageState.DESTROYED)
		{
			Deploy(SCR_EMobileAssemblyStatus.DESTROYED);
			
			return;
		}
		
		// Moved?
		Physics physicsComponent = truck.GetPhysics();
		
		if (!physicsComponent)
			return;
		
		vector vel = physicsComponent.GetVelocity();
		vel[1] = 0;
		
		if (vel.LengthSq() <= 0.01)
			return;
		
		Deploy(SCR_EMobileAssemblyStatus.DISMANTLED);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnFactionChanged()
	{
		SCR_CampaignFactionManager fManager = SCR_CampaignFactionManager.Cast(GetGame().GetFactionManager());
		
		if (!fManager)
			return;
		
		FactionKey fKey;
		Faction f = fManager.GetFactionByIndex(m_iFaction);
		
		if (f)
		{
			fKey = f.GetFactionKey();
			m_Faction = SCR_CampaignFaction.Cast(f);
		}

		if (m_SpawnPoint)
		{
			m_SpawnPoint.SetFaction(f);
			s_OnSpawnPointOwnerChanged.Invoke();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_SpawnPoint GetSpawnPoint()
	{
		return m_SpawnPoint;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsDeployed()
	{
		return m_bIsDeployed;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsInRadioRange()
	{
		return m_bIsInRadioRange;
	}
	
	//------------------------------------------------------------------------------------------------
	MapItem GetMapItem()
	{
		return m_MapItem;
	}
		
	//------------------------------------------------------------------------------------------------
	void SetMapItem(MapItem item)
	{
		m_MapItem = item;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetCountOfExclusivelyLinkedBases(int cnt)
	{
		m_iBasesCoveredByMHQOnly = cnt;
		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	int GetCountOfExclusivelyLinkedBases()
	{
		return m_iBasesCoveredByMHQOnly;
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateRadioCoverage()
	{
		if (!m_ParentFaction)
			return;
		
		bool inRangeNow = SCR_GameModeCampaign.GetInstance().GetBaseManager().IsEntityInFactionRadioSignal(GetOwner(), m_ParentFaction);
		bool refreshLinks = inRangeNow != m_bIsInRadioRange;
		m_bIsInRadioRange = inRangeNow;
		
		if (!IsDeployed())
			return;
		
		if (RplSession.Mode() != RplMode.Dedicated && refreshLinks)
			foreach (SCR_CampaignMilitaryBaseComponent base: m_aBasesInRadioRange)
				base.GetMapDescriptor().HandleMapLinks();
		
		DamageManagerComponent damageComponent = DamageManagerComponent.Cast(GetOwner().FindComponent(DamageManagerComponent));
			
		if (damageComponent && damageComponent.GetState() == EDamageState.DESTROYED)
		{
			if (m_MapItem)
				m_MapItem.SetVisible(false);
			
			if (!IsProxy())
			{
				IEntity vehicle = GetOwner().GetParent();
				
				if (vehicle)
				{
					BaseRadioComponent radioComponent = BaseRadioComponent.Cast(vehicle.FindComponent(BaseRadioComponent));
					
					if (radioComponent)
						radioComponent.SetPower(false);
				}
			}
		}
		
		if (IsProxy())
			return;
		
		if (!m_bIsInRadioRange && m_iFaction != SCR_CampaignMilitaryBaseComponent.INVALID_FACTION_INDEX)
		{
			m_iFaction = SCR_CampaignMilitaryBaseComponent.INVALID_FACTION_INDEX;
			Replication.BumpMe();
			OnFactionChanged();
		}
		
		if (m_bIsInRadioRange && m_iFaction == SCR_CampaignMilitaryBaseComponent.INVALID_FACTION_INDEX && m_bCooldownDone)
		{
			m_iFaction = m_iParentFaction;
			Replication.BumpMe();
			OnFactionChanged();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateRespawnCooldown(Widget w = null)
	{
		if (!IsDeployed() || !m_ParentFaction)
			return;
		
		SCR_CampaignMilitaryBaseComponent hq = m_ParentFaction.GetMainBase();
		
		ImageWidget bg;
		TextWidget Respawn;
		ImageWidget RespawnImg;
		
		ResourceName imageset = hq.GetBuildingIconImageset();
		
		if(w && imageset)
		{
			bg = ImageWidget.Cast(w.FindAnyWidget("Bg"));
			Respawn = TextWidget.Cast(w.FindAnyWidget("Respawn"));
			RespawnImg = ImageWidget.Cast(w.FindAnyWidget("RespawnIMG"));
			RespawnImg.LoadImageFromSet(0, imageset, "RespawnBig");
		}
		
		#ifndef AR_CAMPAIGN_TIMESTAMP
		if (m_fRespawnAvailableSince > Replication.Time())
		#else
		ChimeraWorld world = GetOwner().GetWorld();
		if (m_fRespawnAvailableSince.Greater(world.GetServerTimestamp()))
		#endif
		{
			if (m_bCooldownDone)
				m_bCooldownDone = false;
			
			if (RplSession.Mode() != RplMode.Dedicated)
			{
				if (!w)
					s_OnUpdateRespawnCooldown.Invoke();
				else
				{
					bg.SetVisible(true);
					Respawn.SetVisible(true);
					RespawnImg.SetVisible(true);
					
					#ifndef AR_CAMPAIGN_TIMESTAMP
					float respawnCooldown = Math.Ceil((m_fRespawnAvailableSince - Replication.Time()) / 1000);
					#else
					float respawnCooldown = Math.Ceil(m_fRespawnAvailableSince.DiffMilliseconds(world.GetServerTimestamp()) / 1000);
					#endif
					int d;
					int h;
					int m;
					int s;
					string sStr;
					SCR_DateTimeHelper.GetDayHourMinuteSecondFromSeconds(respawnCooldown, d, h, m, s);
					sStr = s.ToString();
					
					if (s < 10)
						sStr = "0" + sStr;
					
					Respawn.SetTextFormat("#AR-Campaign_MobileAssemblyCooldown", m, sStr);
				}
			}
		}
		else
		{
			GetGame().GetCallqueue().Remove(UpdateRespawnCooldown);
			
			if (w)
			{
				if (RplSession.Mode() != RplMode.Dedicated)
				{
					bg.SetVisible(false);
					Respawn.SetVisible(false);
					RespawnImg.SetVisible(false);
				}
			}
			else
			{
				if (!m_bCooldownDone)
				{
					m_bCooldownDone = true;
					s_OnUpdateRespawnCooldown.Invoke();
					
					if (!IsProxy() && IsInRadioRange())
					{
						m_iFaction = m_iParentFaction;
						Replication.BumpMe();
						OnFactionChanged();
					}
				}
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_CampaignMobileAssemblyComponent()
	{
		if (!m_ParentFaction || !IsDeployed() ||IsProxy())
			return;
		
		Deploy(SCR_EMobileAssemblyStatus.DISMANTLED);
	}
};

enum SCR_EMobileAssemblyStatus
{
	DEPLOYED,
	DISMANTLED,
	DESTROYED
};