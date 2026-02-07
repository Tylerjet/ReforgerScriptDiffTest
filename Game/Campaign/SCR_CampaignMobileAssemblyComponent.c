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
	
	protected RplComponent m_RplComponent;
	protected MapItem m_MapItem;
	protected SCR_CampaignFaction m_Faction;
	protected SCR_CampaignFaction m_ParentFaction;
	protected bool m_bIsInRadioRange = false;
	protected bool m_bCooldownDone = true;
	protected RplId m_RplId = RplId.Invalid();
	protected ref array<SCR_CampaignBase> m_aBasesInRange = {};
	protected SCR_SpawnPoint m_SpawnPoint;
	protected ResourceName m_sBuildingIconImageset = "{F7E8D4834A3AFF2F}UI/Imagesets/Conflict/conflict-icons-bw.imageset";
	protected bool m_bIsHovered = false;
	protected ref array<MapLink> m_aMapLinks = {};
	
	[Attribute("{C35F29E48086221A}Configs/Campaign/CampaignGraphLinesConfig.conf")]
	protected ref SCR_GraphLinesData m_GraphLinesData;
	
	[Attribute("3")]
	protected float m_fLineWidth;
	
	
	[RplProp(onRplName: "OnParentFactionIDSet")]
	protected int m_iParentFaction = SCR_CampaignBase.INVALID_FACTION_INDEX;
	[RplProp(onRplName: "OnFactionChanged")]
	protected int m_iFaction = SCR_CampaignBase.INVALID_FACTION_INDEX;
	[RplProp()]
	protected float m_fRespawnAvailableSince = float.MAX;
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		//getting spawn point entity from prefab
		IEntity child = owner.GetChildren();
		while(child)
		{
			if (child.Type() == SCR_SpawnPoint)
				m_SpawnPoint = SCR_SpawnPoint.Cast(child);
				
			child = child.GetSibling();
		}	
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		SetEventMask(owner, EntityEvent.INIT);
		owner.SetFlags(EntityFlags.ACTIVE, true);
		
		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		GetGame().GetCallqueue().CallLater(UpdateRadioCoverage, 1000, true);
		GetGame().GetCallqueue().CallLater(UpdateRespawnCooldown, 250, true);
		
		if (IsProxy())
		{
			SCR_GameModeCampaignMP.s_OnFactionAssignedLocalPlayer.Insert(OnParentFactionIDSet);
			SCR_GameModeCampaignMP.s_OnFactionAssignedLocalPlayer.Insert(OnDeployChanged);
		}
		
		//Show descriptor
		SCR_MapDescriptorComponent descriptor = SCR_MapDescriptorComponent.Cast(owner.FindComponent(SCR_MapDescriptorComponent));
		
		if (descriptor)
		{
			m_MapItem = descriptor.Item();
			
			if (m_MapItem)
				m_MapItem.SetDisplayName("#AR-Vehicle_MobileAssembly_Name");
		}
	}
	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		super.OnDelete(owner);
		SCR_GameModeCampaignMP.s_OnFactionAssignedLocalPlayer.Remove(OnParentFactionIDSet);
		SCR_GameModeCampaignMP.s_OnFactionAssignedLocalPlayer.Remove(OnDeployChanged);
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
		MapLinkProps props = link.GetMapLinkProps();
		if (!props)
			return;
		
		props.SetLineWidth(m_fLineWidth);
		
		Color c = props.GetLineColor();
		if (!c)
			return;
		
		if (m_bIsHovered)
			c.SetA(m_GraphLinesData.GetHighlightedAlpha());
		else
			c.SetA(m_GraphLinesData.GetDefaultAlpha());
		props.SetLineColor(c);
	}
	
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
		SCR_CampaignFactionManager fManager = SCR_CampaignFactionManager.GetInstance();
		m_ParentFaction = SCR_CampaignFaction.Cast(fManager.GetFactionByIndex(m_iParentFaction));
		SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();
		Faction playerFaction;
		
		if (campaign)
			playerFaction = campaign.GetLastPlayerFaction();
		
		if (m_ParentFaction && m_ParentFaction == playerFaction)
		{
			if (m_MapItem)
				switch (m_ParentFaction.GetFactionKey())
				{
					case SCR_GameModeCampaignMP.FACTION_BLUFOR:
					{
						m_MapItem.SetFactionIndex(EFactionMapID.WEST);
						break;
					}
				
					case SCR_GameModeCampaignMP.FACTION_OPFOR:
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
	array<SCR_CampaignBase> GetBasesInRange()
	{
		return m_aBasesInRange;
	}
	
	//------------------------------------------------------------------------------------------------
	bool Deploy(bool deploy)
	{
		if (!m_bIsInRadioRange && deploy)
			return false;
		
		if (!m_ParentFaction)
			return false;
		
		SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();
		
		if (!campaign)
			return false;
		
		if (campaign.IsMobileAssemblyDeployed(m_ParentFaction) && deploy)
			return false;
				
		if (deploy)
		{
			m_aBasesInRange = campaign.GetBasesInRangeOfMobileHQ(GetOwner());
			
			if (campaign && GetOwner())
			{
				SCR_CampaignTaskManager tManager = SCR_CampaignTaskManager.GetCampaignTaskManagerInstance();
				
				if (tManager)
					tManager.GenerateCaptureTasks(GetOwner());
			}
				
			campaign.SetDeployedMobileAssemblyID(m_ParentFaction.GetFactionKey(), Replication.FindId(this));
		}
		else
			campaign.SetDeployedMobileAssemblyID(m_ParentFaction.GetFactionKey(), RplId.Invalid());
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void OnDeployChanged()
	{
		SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();
		Faction playerFaction;
		BaseRadioComponent radioComponent;
		
		if (campaign)
			playerFaction = campaign.GetLastPlayerFaction();
		
		IEntity vehicle = GetOwner().GetParent();
		
		if (vehicle)
			radioComponent = BaseRadioComponent.Cast(vehicle.FindComponent(BaseRadioComponent));
		
		if (!IsProxy())
		{
			SCR_CampaignBaseManager baseManager = SCR_CampaignBaseManager.GetInstance();
			
			if (baseManager)
				baseManager.UpdateBasesSignalCoverage(m_ParentFaction, true);
		}
		
		if (IsDeployed())
		{
			m_aBasesInRange = campaign.GetBasesInRangeOfMobileHQ(GetOwner());
			
			if (!IsProxy())
			{
				m_fRespawnAvailableSince = Replication.Time() + RESPAWN_COOLDOWN;
				Replication.BumpMe();
				
				if (radioComponent)
					radioComponent.TogglePower(true);
			}
			
			if (RplSession.Mode() != RplMode.Dedicated)
			{
				if (m_MapItem && m_ParentFaction == playerFaction)
					m_MapItem.SetVisible(true);
			}
		}
		else
		{
			m_aBasesInRange = {};
			
			if (!IsProxy())
			{
				m_iFaction = SCR_CampaignBase.INVALID_FACTION_INDEX;
				m_fRespawnAvailableSince = float.MAX;
				Replication.BumpMe();
				
				if (radioComponent)
					radioComponent.TogglePower(false);
			}
			
			if (m_MapItem && RplSession.Mode() != RplMode.Dedicated)
				m_MapItem.SetVisible(false);
			
			OnFactionChanged();
		}
		
		SCR_CampaignBaseManager baseManager = SCR_CampaignBaseManager.GetInstance();
		
		if(!baseManager)
			return;
		
		baseManager.UpdateBasesSignalCoverage();
	}
	
	//------------------------------------------------------------------------------------------------
	void OnFactionChanged()
	{
		SCR_CampaignFactionManager fManager = SCR_CampaignFactionManager.GetInstance();
		
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
		if (!m_ParentFaction)
			return false;
		
		SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();
		
		if (!campaign)
			return false;
		
		if (m_RplId.Invalid())
			m_RplId = Replication.FindId(this);
		
		return campaign.GetDeployedMobileAssemblyID(m_ParentFaction.GetFactionKey()) == m_RplId;
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
	void UpdateRadioCoverage()
	{
		SCR_CampaignBaseManager baseManager = SCR_CampaignBaseManager.GetInstance();
		
		if (!baseManager)
			return;
		
		SCR_CampaignFaction f = GetParentFaction();
		
		if (!f)
			return;
		
		SCR_CampaignBase HQ = f.GetMainBase();
		
		if (!HQ)
			return;
		
		bool inRangeNow = baseManager.IsEntityInFactionRadioSignal(GetOwner(), f);
		bool refreshLinks = inRangeNow != m_bIsInRadioRange;
		m_bIsInRadioRange = inRangeNow;
		
		if (refreshLinks)
			foreach (SCR_CampaignBase base: m_aBasesInRange)
				base.HandleMapLinks();
		
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
						radioComponent.TogglePower(false);
				}
			}
		}
		
		if (!IsProxy() && IsDeployed())
		{
			if (!m_bIsInRadioRange && m_iFaction != SCR_CampaignBase.INVALID_FACTION_INDEX)
			{
				m_iFaction = SCR_CampaignBase.INVALID_FACTION_INDEX;
				Replication.BumpMe();
				OnFactionChanged();
			}
			
			if (m_bIsInRadioRange && m_iFaction == SCR_CampaignBase.INVALID_FACTION_INDEX && m_bCooldownDone)
			{
				m_iFaction = m_iParentFaction;
				Replication.BumpMe();
				OnFactionChanged();
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateRespawnCooldown(Widget w = null)
	{
		if (!IsDeployed())
			return;
		ImageWidget bg;
		TextWidget Respawn;
		ImageWidget RespawnImg;
		
		if(w)
		{
			bg = ImageWidget.Cast(w.FindAnyWidget("Bg"));
			Respawn = TextWidget.Cast(w.FindAnyWidget("Respawn"));
			RespawnImg = ImageWidget.Cast(w.FindAnyWidget("RespawnIMG"));
			RespawnImg.LoadImageFromSet(0, m_sBuildingIconImageset, "RespawnBig");
		}
		
		if (m_fRespawnAvailableSince > Replication.Time())
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
					
					float respawnCooldown = Math.Ceil((m_fRespawnAvailableSince - Replication.Time()) / 1000);
					int d;
					int h;
					int m;
					int s;
					string sStr;
					SCR_Global.ConvertSecondsToDaysHoursMinutesSeconds(respawnCooldown, d, h, m, s);
					sStr = s.ToString();
					
					if (s < 10)
						sStr = "0" + sStr;
					
					Respawn.SetTextFormat("#AR-Campaign_MobileAssemblyCooldown", m, sStr);
				}
			}
		}
		else
		{
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
};