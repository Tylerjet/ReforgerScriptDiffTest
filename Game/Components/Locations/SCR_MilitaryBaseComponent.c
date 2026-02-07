//------------------------------------------------------------------------------------------------
class SCR_MilitaryBaseComponentClass: ScriptComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_MilitaryBaseComponent : ScriptComponent
{
	[Attribute("100"), RplProp(onRplName: "OnRadiusChanged")]
	protected int m_iRadius;
	
	[Attribute("0", desc: "Not all capture points controlled are required for seizing this base, only the majority.")]
	protected bool m_bSeizedByMajority;
	
	[Attribute("1")]
	protected bool m_bShowNotifications;
	
	[Attribute("1")]
	protected bool m_bShowMapIcon;
	
	[Attribute(ENotification.BASE_SEIZING_DONE_FRIENDLIES.ToString(), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ENotification))]
	protected ENotification m_eCapturedByFriendliesNotification;
	
	[Attribute(ENotification.BASE_SEIZING_DONE_ENEMIES.ToString(), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ENotification))]
	protected ENotification m_eCapturedByEnemiesNotification;
	
	protected ref ScriptInvoker m_OnRadiusChanged;
	protected ref ScriptInvoker m_OnFactionChanged;
	protected ref ScriptInvoker m_OnServiceRegistered;
	protected ref array<SCR_MilitaryBaseLogicComponent> m_aSystems = {};
	
	protected RplComponent m_RplComponent;
	protected SCR_FactionAffiliationComponent m_FactionComponent;
	
	[RplProp(onRplName: "OnCapturingFactionChanged")]
	protected FactionKey m_sCapturingFaction;
	
	//------------------------------------------------------------------------------------------------
	protected bool IsProxy()
	{
		return (m_RplComponent && m_RplComponent.IsProxy());
	}
	
	//------------------------------------------------------------------------------------------------
	//! Show default notifications upon base capture (setter)
	void AllowNotifications(bool allow)
	{
		m_bShowNotifications = allow;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Show default notifications upon base capture (getter)
	bool NotificationsAllowed()
	{
		return m_bShowNotifications;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnRadiusChanged()
	{
		if (!m_OnRadiusChanged)
			m_OnRadiusChanged = new ScriptInvoker();
		
		return m_OnRadiusChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnFactionChanged()
	{
		if (!m_OnFactionChanged)
			m_OnFactionChanged = new ScriptInvoker();
		
		return m_OnFactionChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnServiceRegistered()
	{
		if (!m_OnServiceRegistered)
			m_OnServiceRegistered = new ScriptInvoker();
		
		return m_OnServiceRegistered;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get all registered systems inherited from SCR_ServicePointComponent
	int GetServices(out array<SCR_ServicePointComponent> services = null)
	{
		int count;
		
		foreach (SCR_MilitaryBaseLogicComponent component : m_aSystems)
		{
			SCR_ServicePointComponent service = SCR_ServicePointComponent.Cast(component);
			
			if (service)
			{
				count++;
				
				if (services)
					services.Insert(service);
			}
		}
		
		return count;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetServicesByType(out array<SCR_ServicePointComponent> services, SCR_EServicePointType type)
	{
		int count;
		
		foreach (SCR_MilitaryBaseLogicComponent component : m_aSystems)
		{
			SCR_ServicePointComponent service = SCR_ServicePointComponent.Cast(component);
			
			if (service && service.GetType() == type)
			{
				count++;
				
				if (services)
					services.Insert(service);
			}
		}
		
		return count;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_ServicePointComponent GetServiceByType(SCR_EServicePointType type)
	{
		foreach (SCR_MilitaryBaseLogicComponent component : m_aSystems)
		{
			SCR_ServicePointComponent service = SCR_ServicePointComponent.Cast(component);
			
			if (service && service.GetType() == type)
				return service;
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get all registered systems inherited from SCR_SeizingComponent
	int GetCapturePoints(out array<SCR_SeizingComponent> capturePoints)
	{
		int count;
		
		foreach (SCR_MilitaryBaseLogicComponent component : m_aSystems)
		{
			SCR_SeizingComponent capturePoint = SCR_SeizingComponent.Cast(component);
			
			if (capturePoint)
			{
				count++;
				
				if (capturePoints)
					capturePoints.Insert(capturePoint);
			}
		}
		
		return count;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get all registered systems inherited from SCR_FlagComponent
	int GetFlags(out array<SCR_FlagComponent> flags)
	{
		int count;
		
		foreach (SCR_MilitaryBaseLogicComponent component : m_aSystems)
		{
			SCR_FlagComponent flag = SCR_FlagComponent.Cast(component);
			
			if (flag)
			{
				count++;
				
				if (flags)
					flags.Insert(flag);
			}
		}
		
		return count;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetRadius(int radius)
	{
		if (IsProxy())
			return;
		
		if (radius == m_iRadius)
			return;
		
		if (radius <= 0)
		{
			Print("SCR_MilitaryBaseComponent: SetRadius called with suspicious value (" + radius + ").", LogLevel.WARNING);
			radius = 0;
		}
		
		m_iRadius = radius;
		
		Replication.BumpMe();
		OnRadiusChanged();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnRadiusChanged()
	{
		if (m_OnRadiusChanged)
			m_OnRadiusChanged.Invoke(m_iRadius, this);
	}
	
	//------------------------------------------------------------------------------------------------
	int GetRadius()
	{
		return m_iRadius;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetFaction(Faction faction)
	{
		if (IsProxy())
			return;
		
		if (!m_FactionComponent)
			return;
		
		m_FactionComponent.SetAffiliatedFaction(faction);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetFaction(FactionKey faction)
	{
		if (IsProxy())
			return;
		
		if (!m_FactionComponent)
			return;
		
		m_FactionComponent.SetAffiliatedFactionByKey(faction);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnFactionChanged(Faction faction)
	{
		if (!IsProxy())
			m_sCapturingFaction = FactionKey.Empty;
		
		if (m_OnFactionChanged)
			m_OnFactionChanged.Invoke(faction, this);
		
		SCR_MilitaryBaseManager.GetInstance().OnBaseFactionChanged(faction, this);
		
		if (!faction)
			return;
		
		ChangeFlags(faction);
		
		if (m_bShowNotifications)
			NotifyPlayerInRadius(faction);
	}
	
	//------------------------------------------------------------------------------------------------
	Faction GetFaction()
	{
		if (!m_FactionComponent)
			return null;
		
		return m_FactionComponent.GetAffiliatedFaction();
	}
	
	//------------------------------------------------------------------------------------------------
	Faction GetCapturingFaction()
	{
		return GetGame().GetFactionManager().GetFactionByKey(m_sCapturingFaction);
	}
	
	//------------------------------------------------------------------------------------------------
	void RegisterLogicComponent(notnull SCR_MilitaryBaseLogicComponent component)
	{
		if (m_aSystems.Contains(component))
			return;
		
		m_aSystems.Insert(component);
		
		SCR_ServicePointComponent service = SCR_ServicePointComponent.Cast(component);
		
		if (service)
		{
			if (m_OnServiceRegistered)
				m_OnServiceRegistered.Invoke(this, service);
			
			SCR_MilitaryBaseManager.GetInstance().OnServiceRegisteredInBase(service, this);
		}
		
		SCR_FlagComponent flag = SCR_FlagComponent.Cast(component);
		
		if (flag && GetFaction())
			GetGame().GetCallqueue().CallLater(ChangeFlags, 500, false, GetFaction());	// Give the system time to properly register slots
		
		if (!IsProxy())
		{
			SCR_SeizingComponent seizingComponent = SCR_SeizingComponent.Cast(component);
			
			if (seizingComponent)
			{
				seizingComponent.GetOnCaptureFinish().Insert(OnPointCaptured);
				seizingComponent.GetOnCaptureStart().Insert(OnPointContested);
				seizingComponent.GetOnCaptureInterrupt().Insert(OnPointSecured);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void UnregisterLogicComponent(notnull SCR_MilitaryBaseLogicComponent component)
	{
		m_aSystems.RemoveItem(component);
		
		if (!IsProxy())
		{
			SCR_SeizingComponent seizingComponent = SCR_SeizingComponent.Cast(component);
		
			if (seizingComponent)
			{
				seizingComponent.GetOnCaptureFinish().Remove(OnPointCaptured);
				seizingComponent.GetOnCaptureStart().Remove(OnPointContested);
				seizingComponent.GetOnCaptureInterrupt().Remove(OnPointSecured);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Triggered when a registered SCR_SeizingComponent gets captured
	void OnPointCaptured(notnull SCR_Faction faction, notnull SCR_SeizingComponent point)
	{
		if (faction == GetFaction())
			return;
		
		array<SCR_SeizingComponent> capturePoints = {};
		int pointsCount = GetCapturePoints(capturePoints);
		int controlled;
		
		foreach (SCR_SeizingComponent capturePoint : capturePoints)
		{
			if (capturePoint == point || capturePoint.GetFaction() == faction)
				controlled++;
		}
		
		// Capture this base if the required amount of registered capture points has been seized
		if (pointsCount == controlled || (m_bSeizedByMajority && controlled > (pointsCount - controlled)))
			OnRequiredPointsCaptured(faction);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Triggered when a registered SCR_SeizingComponent starts getting captured
	void OnPointContested(notnull SCR_Faction faction, notnull SCR_SeizingComponent point)
	{
		if (faction == GetFaction())
			return;
		
		array<SCR_SeizingComponent> capturePoints = {};
		int pointsCount = GetCapturePoints(capturePoints);
		int controlled;
		
		foreach (SCR_SeizingComponent capturePoint : capturePoints)
		{
			if (capturePoint.GetFaction() == faction)
				controlled++;
		}
		
		// This base is currently being seized
		if (controlled + 1 == pointsCount || (m_bSeizedByMajority && controlled + 1 > (pointsCount - controlled)))
		{
			m_sCapturingFaction = faction.GetFactionKey();
			OnCapturingFactionChanged();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Triggered when a registered SCR_SeizingComponent stops getting captured
	void OnPointSecured(notnull SCR_Faction faction, notnull SCR_SeizingComponent point)
	{
		if (faction != GetCapturingFaction())
			return;
		
		array<SCR_SeizingComponent> capturePoints = {};
		int pointsCount = GetCapturePoints(capturePoints);
		int controlled;
		
		foreach (SCR_SeizingComponent capturePoint : capturePoints)
		{
			if (capturePoint.GetFaction() == faction)
				controlled++;
		}
		
		// This base is no longer being seized
		if (controlled + 1 == pointsCount || (m_bSeizedByMajority && controlled + 1 > (pointsCount - controlled)))
		{
			m_sCapturingFaction = FactionKey.Empty;
			OnCapturingFactionChanged();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Triggered when a the required amount of SCR_SeizingComponents to seize the base is captured
	void OnRequiredPointsCaptured(notnull SCR_Faction faction)
	{
		SetFaction(faction);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnCapturingFactionChanged()
	{
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ChangeFlags(notnull Faction faction)
	{
		SCR_Faction factionCast = SCR_Faction.Cast(faction);
		
		if (!factionCast)
			return;
		
		array<SCR_FlagComponent> flags = {};
		GetFlags(flags);
		
		foreach (SCR_FlagComponent flagComponent : flags)
			flagComponent.ChangeMaterial(factionCast.GetFactionFlagMaterial());
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool GetIsLocalPlayerPresent()
	{
		IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(SCR_PlayerController.GetLocalPlayerId());
		
		if (!player)
			return false;
		
		return (vector.DistanceSqXZ(player.GetOrigin(), GetOwner().GetOrigin()) <= (m_iRadius * m_iRadius));
	}
	
	//------------------------------------------------------------------------------------------------
	protected void NotifyPlayerInRadius(notnull Faction faction)
	{
		Faction playerFaction = SCR_RespawnSystemComponent.GetLocalPlayerFaction();
		
		if (!playerFaction)
			return;
		
		if (!GetIsLocalPlayerPresent())
			return;
		
		if (playerFaction == faction)
			SCR_NotificationsComponent.SendLocal(m_eCapturedByFriendliesNotification);
		else
			SCR_NotificationsComponent.SendLocal(m_eCapturedByEnemiesNotification);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		m_FactionComponent = SCR_FactionAffiliationComponent.Cast(owner.FindComponent(SCR_FactionAffiliationComponent));
		
		if (m_FactionComponent)
			m_FactionComponent.GetOnFactionUpdate().Insert(OnFactionChanged);
		
		// Attributes check
		if (!m_RplComponent)
			Print("SCR_MilitaryBaseComponent: RplComponent not found on owner entity. Multiplayer functionality will not be available.", LogLevel.WARNING);
		
		if (!m_FactionComponent)
			Print("SCR_MilitaryBaseComponent: SCR_FactionAffiliationComponent not found on owner entity. Faction handling will not be available.", LogLevel.WARNING);
		
		if (m_iRadius <= 0)
		{
			Print("SCR_MilitaryBaseComponent: m_iRadius set to suspicious value (" + m_iRadius + ").", LogLevel.WARNING);
			m_iRadius = 0;
		}
		
		if (!GetGame().InPlayMode())
			return;
		
		SCR_MilitaryBaseManager.GetInstance().RegisterBase(this);
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_MilitaryBaseComponent()
	{
		SCR_MilitaryBaseManager.UnregisterBaseStatic(this);
		
		foreach (SCR_MilitaryBaseLogicComponent component : m_aSystems)
		{
			if (component)
				component.UnregisterBase(this);
		}
	}
};