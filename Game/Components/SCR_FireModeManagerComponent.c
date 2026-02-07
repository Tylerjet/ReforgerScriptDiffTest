class SCR_FireModeManagerComponentClass: ScriptComponentClass
{
};

class SCR_FireModeManagerComponent : ScriptComponent
{
	[Attribute(desc: "Add weapons groups to fire multiple weapons per shot. See SCR_WeaponsGroup")]
	protected ref array<ref SCR_WeaponGroup> m_aWeaponGroups;
	
	protected TurretControllerComponent turretController;
	protected EWeaponGroupFireMode m_eSetFireMode;
	protected int m_iCurrentWeaponGroup;

	[RplProp(onRplName: "OnWeaponGroupBumped")]
	protected ref array<int> m_aSetWeaponsGroup = {};

	[RplProp(onRplName: "OnFireModeBumped")]
	protected EWeaponGroupFireMode m_eCurrentFireMode;

	[RplProp(onRplName: "OnRippleIntervalBumped")]
	protected float m_fRippleInterval = 100;

	[RplProp(onRplName: "OnRippleQuantityBumped")]
	protected int m_iRippleQuantity;

	protected const	string EVENT_NAME_ENTER_COMPARTMENT = "OnCompartmentEntered";
	
	//------------------------------------------------------------------------------------------------
	protected void OnWeaponGroupBumped()
	{
		SetWeaponsGroup(m_aSetWeaponsGroup);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnFireModeBumped()
	{
		SetFireMode(m_eCurrentFireMode);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnRippleIntervalBumped()
	{
		SetRippleInterval(m_fRippleInterval);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnRippleQuantityBumped()
	{
		SetRippleQuantity(m_iRippleQuantity);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		turretController = TurretControllerComponent.Cast(owner.FindComponent(TurretControllerComponent));
		EventHandlerManagerComponent ev = EventHandlerManagerComponent.Cast(owner.GetRootParent().FindComponent(EventHandlerManagerComponent));
		if (ev)
			ev.RegisterScriptHandler(EVENT_NAME_ENTER_COMPARTMENT, owner.GetRootParent(), OnCompartmentEntered, false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! because weaponGroups are replicated over character controller, we set the base settings only once the first character enters
	protected void OnCompartmentEntered(IEntity vehicle, BaseCompartmentManagerComponent mgr, IEntity occupant, int managerId, int slotID)
	{
		if (m_aWeaponGroups.IsEmpty())
		{
			Print("SCR_FireModeManagerComponent not configured correctly on a turret on this vehicle: " + vehicle, LogLevel.WARNING);
			return;
		}

		if (!m_aWeaponGroups.IsIndexValid(m_iCurrentWeaponGroup))
			return;

		SCR_WeaponGroup currentGroup = m_aWeaponGroups[m_iCurrentWeaponGroup];
		if (!currentGroup)
			return;

		SetWeaponsGroup(currentGroup.m_aWeaponsGroupIds);
		
		array<int> fireModes = {};
		GetAvailableFireModes(fireModes);
		SetFireMode(fireModes[0]);
		if (!currentGroup.m_aRippleFireQuantities.IsEmpty())
			SetRippleQuantity(currentGroup.m_aRippleFireQuantities[0]);
		
		EventHandlerManagerComponent ev = EventHandlerManagerComponent.Cast(vehicle.FindComponent(EventHandlerManagerComponent));
		if (ev)
			ev.RemoveScriptHandler(EVENT_NAME_ENTER_COMPARTMENT, vehicle, OnCompartmentEntered);

		RplComponent rplComp = SCR_EntityHelper.GetEntityRplComponent(vehicle);
		if (rplComp && rplComp.Role() == RplRole.Authority)
			Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	void SetWeaponsGroup(notnull array<int> weaponsArray)
	{
		ConfigureWeaponsGroup(weapons: weaponsArray);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetFireMode(EWeaponGroupFireMode fireMode)
	{
		ConfigureWeaponsGroup(mode: fireMode);
		m_eCurrentFireMode = fireMode;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetRippleQuantity(int quantity)
	{
		ConfigureWeaponsGroup(rippleQuantity: quantity);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetRippleInterval(int interval)
	{
		ConfigureWeaponsGroup(rippleInterval: interval);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ConfigureWeaponsGroup(array<int> weapons = null, int mode = -1, int rippleQuantity = -1, int rippleInterval = -1)
	{
		if (!turretController)
			return;
		
		Vehicle vehicle = Vehicle.Cast(GetOwner().GetRootParent());
		if (!vehicle)
			return;
		
		if (weapons)
		{
			turretController.SetWeaponGroup(weapons, m_eSetFireMode, m_iRippleQuantity, m_fRippleInterval);
			m_aSetWeaponsGroup = weapons;
		}
		else if (mode > -1)
		{
			turretController.SetWeaponGroup(m_aSetWeaponsGroup, mode, m_iRippleQuantity, m_fRippleInterval);
			m_eSetFireMode = mode;
		}
		else if (rippleQuantity > -1)
		{
			turretController.SetWeaponGroup(m_aSetWeaponsGroup, m_eSetFireMode, rippleQuantity, m_fRippleInterval);
			m_iRippleQuantity = rippleQuantity;
		}
		else if (rippleInterval > -1)
		{
			turretController.SetWeaponGroup(m_aSetWeaponsGroup, m_eSetFireMode, m_iRippleQuantity, rippleInterval);
			m_fRippleInterval = rippleInterval;		
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void NextWeaponsGroup()
	{
		if (m_iCurrentWeaponGroup + 1 > m_aWeaponGroups.Count() -1)
			m_iCurrentWeaponGroup = 0;
		else
			m_iCurrentWeaponGroup++;
		
		// if the new weapongroup does not support the current firemode, reset to the new groups default firemode
		array<int> availableFireModes = {};
		GetAvailableFireModes(availableFireModes);
		if (!availableFireModes.IsEmpty() && !availableFireModes.Contains(m_iCurrentWeaponGroup))
		{
			m_eCurrentFireMode = availableFireModes[0];
			m_eSetFireMode = m_eCurrentFireMode;
		}
		
		SetWeaponsGroup(m_aWeaponGroups[m_iCurrentWeaponGroup].m_aWeaponsGroupIds);
	}
	
	//------------------------------------------------------------------------------------------------
	void NextFireMode()
	{
		array<int> availableFireModes = {};
		GetAvailableFireModes(availableFireModes);
		
		// new firemode is the next available firemode
		Print(availableFireModes.Count());
		if (availableFireModes.Find(GetFireMode()) + 1 == availableFireModes.Count())
			m_eCurrentFireMode = availableFireModes[0];
		else
			m_eCurrentFireMode = availableFireModes.Get(availableFireModes.Find(GetFireMode()) + 1);
		
		SetFireMode(m_eCurrentFireMode);
	}
	
	//------------------------------------------------------------------------------------------------
	void NextRippleQuantity()
	{
		array<int> availableRippleQuantities = {};
		GetAvailableRippleQuantities(availableRippleQuantities);
		
		// new firemode is the next available firemode
		if (availableRippleQuantities.Find(GetRippleQuantity()) + 1 == availableRippleQuantities.Count())
			m_iRippleQuantity = availableRippleQuantities[0];
		else
			m_iRippleQuantity = availableRippleQuantities.Get(availableRippleQuantities.Find(GetRippleQuantity()) + 1);
		
		SetRippleQuantity(m_iRippleQuantity);
	}
	
	//------------------------------------------------------------------------------------------------
	void GetAvailableFireModes(notnull out array<int> availableFireModes)
	{
		availableFireModes.Clear();
		array<int> ints = {};		
		SCR_Enum.BitToIntArray(m_aWeaponGroups[m_iCurrentWeaponGroup].m_eFireMode, ints);

		foreach (int i : ints)
		{
			string fireMode = SCR_Enum.GetEnumName(SCR_EWeaponGroupFireMode, i);
			availableFireModes.Insert(typename.StringToEnum(EWeaponGroupFireMode, fireMode));
		}
	}	
	
	//------------------------------------------------------------------------------------------------
	protected void GetAvailableRippleQuantities(notnull out array<int> availableRippleQuantities)
	{
		availableRippleQuantities.Copy(m_aWeaponGroups[m_iCurrentWeaponGroup].m_aRippleFireQuantities);
	}
	
	//------------------------------------------------------------------------------------------------
	int GetWeaponGroupID(out string name)
	{
		name = m_aWeaponGroups[m_iCurrentWeaponGroup].m_sWeaponGroupName;
		return m_iCurrentWeaponGroup;
	}	
	
	//------------------------------------------------------------------------------------------------
	SCR_WeaponGroup GetCurrentWeaponGroup(out string name)
	{
		name = m_aWeaponGroups[m_iCurrentWeaponGroup].m_sWeaponGroupName;
		return m_aWeaponGroups[m_iCurrentWeaponGroup];
	}
	
	//------------------------------------------------------------------------------------------------
	//!	\param[out] weaponGroups returns the m_aWeaponGroups array
	void GetAllWeaponGroups(out array<ref SCR_WeaponGroup> weaponGroups)
	{
		weaponGroups = m_aWeaponGroups;
	}
	
	//------------------------------------------------------------------------------------------------
	EWeaponGroupFireMode GetFireMode(out string name = string.Empty)
	{
		name = SCR_Enum.GetEnumName(EWeaponGroupFireMode, m_eCurrentFireMode);
		return m_eCurrentFireMode;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetRippleQuantity()
	{
		return m_iRippleQuantity;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetRippleInterval()
	{
		return m_fRippleInterval;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		SetEventMask(owner, EntityEvent.INIT);
	}
};