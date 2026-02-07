[ComponentEditorProps(category: "GameScripted/AI", description: "Component for AI checking state of characters", color: "0 0 255 255")]
class SCR_AIInfoComponentClass: SCR_AIInfoBaseComponentClass
{
};

enum EUnitRole
{
	NONE 			= 0,
	RIFLEMAN 		= 1,
	MEDIC 			= 2,
	MACHINEGUNNER 	= 4,
	AT_SPECIALIST 	= 8,
	GRENADIER 		= 16,
};

enum EUnitState
{
	NONE 			= 0,
	WOUNDED 		= 1,
	STATIC 			= 2,
	IN_VEHICLE		= 4,
};

enum EFireTeams
{
	NONE,
	BLUE,
	RED,
	GREEN,
	YELLOW,
};

enum EUnitAIState
{
	AVAILABLE,
	BUSY,
	UNRESPONSIVE,
};


//------------------------------------------------------------------------------------------------
class SCR_AIInfoComponent : SCR_AIInfoBaseComponent
{
	private EUnitRole m_iUnitRoles;
	private EUnitState m_iUnitStates;
	private EUnitAIState m_iAIStates;
	private EFireTeams m_iFireTeams;
	private SCR_InventoryStorageManagerComponent m_inventoryManagerComponent;
	private BaseWeaponManagerComponent m_weaponManagerComponent;
	private SCR_CompartmentAccessComponent m_CompartmentAccessComponent;
	private ref map<typename,int> m_MagazineWells;
	private SCR_AIThreatSystem m_ThreatSystem;
	private ScriptedDamageManagerComponent m_DamageManager;
	
	private ECharacterStance m_eStance;
	private EMovementType m_eMovementType;
	private bool m_bWeaponRaised;
	private int m_iAttackCount;
	private int m_unitID;
	
	override protected void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		SetEventMask(owner, EntityEvent.INIT);
		m_MagazineWells = new map<typename,int>;
	}	
	
	void OnVehicleEntered( IEntity vehicle, BaseCompartmentManagerComponent manager, int mgrID, int slotID )
	{
		BaseCompartmentSlot compSlot = manager.FindCompartment(slotID);
		if (TurretCompartmentSlot.Cast(compSlot))
			AddUnitState(EUnitState.STATIC);			
	}
	
	void OnVehicleLeft( IEntity vehicle, BaseCompartmentManagerComponent manager, int mgrID, int slotID )
	{
		auto aiworld = SCR_AIWorld.Cast(GetGame().GetAIWorld());
		if (!aiworld)
			return;
		BaseCompartmentSlot compSlot = manager.FindCompartment(slotID);
		if (!TurretCompartmentSlot.Cast(compSlot))
			return;
		
		RemoveUnitState(EUnitState.STATIC);
		
		AIAgent owner = AIAgent.Cast(GetOwner());
		if (!owner)
			return;
		AICommunicationComponent mailbox = owner.GetCommunicationComponent();
		if (mailbox)
		{
			SCR_AIMessage_AttackStaticDone msg1 = SCR_AIMessage_AttackStaticDone.Cast(mailbox.CreateMessage(aiworld.GetGoalMessageOfType(EMessageType_Goal.ATTACK_STATIC_DONE)));
			if ( !msg1 )
			{
				Debug.Error("Unable to create valid message!");
				return ;
			}
			msg1.SetText("Get out of turret");
			msg1.SetReceiver(owner);
			mailbox.RequestBroadcast(msg1,owner);
		}
		
	}	
	
	override protected void EOnInit(IEntity owner)
	{
		IEntity ent = owner;
		AIAgent agent = AIAgent.Cast(owner);
		if (agent)
			ent = agent.GetControlledEntity();	
		
		if (ent)
		{
			m_inventoryManagerComponent = SCR_InventoryStorageManagerComponent.Cast(ent.FindComponent(SCR_InventoryStorageManagerComponent));
			m_weaponManagerComponent = BaseWeaponManagerComponent.Cast(ent.FindComponent(BaseWeaponManagerComponent));
			m_CompartmentAccessComponent = SCR_CompartmentAccessComponent.Cast(ent.FindComponent(SCR_CompartmentAccessComponent));
			m_DamageManager = ScriptedDamageManagerComponent.Cast(ent.FindComponent(ScriptedDamageManagerComponent));
		}
		
		if (m_CompartmentAccessComponent)
		{
			m_CompartmentAccessComponent.GetOnCompartmentEntered().Insert(OnVehicleEntered);
			m_CompartmentAccessComponent.GetOnCompartmentLeft().Insert(OnVehicleLeft);			
		}

		if (m_inventoryManagerComponent)
		{
			m_inventoryManagerComponent.m_OnItemAddedInvoker.Insert(OnItemAdded);
			m_inventoryManagerComponent.m_OnItemRemovedInvoker.Insert(OnItemRemoved);
		}							
		
		if (m_DamageManager)
		{
			m_DamageManager.GetOnDamageOverTimeAdded().Insert(OnDamageOverTimeAdded);
			m_DamageManager.GetOnDamageOverTimeRemoved().Insert(OnDamageOverTimeRemoved);
			EvaluateWoundedState();
		}
	}
	
	override protected void OnDelete(IEntity owner)
	{
		if (m_inventoryManagerComponent)
		{
			m_inventoryManagerComponent.m_OnItemAddedInvoker.Remove(OnItemAdded);
			m_inventoryManagerComponent.m_OnItemAddedInvoker.Remove(OnItemRemoved);
		}
		
		if (m_CompartmentAccessComponent)
		{
			m_CompartmentAccessComponent.GetOnCompartmentEntered().Remove(OnVehicleEntered);
			m_CompartmentAccessComponent.GetOnCompartmentLeft().Remove(OnVehicleLeft);			
		}
		
		if (m_DamageManager)
		{
			m_DamageManager.GetOnDamageOverTimeAdded().Remove(OnDamageOverTimeAdded);
			m_DamageManager.GetOnDamageOverTimeRemoved().Remove(OnDamageOverTimeRemoved);
		}
	}
	
	bool IsOwnerAgent(AIAgent agent)
	{
		return GetOwner() == agent;
	}
				
	void OnItemAdded(IEntity item, BaseInventoryStorageComponent storageOwner)
	{
		if  (!m_inventoryManagerComponent) 
			return;
		
		if (m_inventoryManagerComponent.GetHealthComponentCount() > 0)
			AddRole(EUnitRole.MEDIC);
		
		if (storageOwner.Type() == EquipedWeaponStorageComponent) // is this item a weapon 
		{
			BaseWeaponComponent wpnComponent = BaseWeaponComponent.Cast(item.FindComponent(BaseWeaponComponent));	
			if (wpnComponent)
			{
				switch (wpnComponent.GetWeaponType())
				{
					case EWeaponType.WT_RIFLE : 
					{
						AddRole(EUnitRole.RIFLEMAN);
						break;
					}
					case EWeaponType.WT_MACHINEGUN : 
					{
						AddRole(EUnitRole.MACHINEGUNNER);
						break;
					}
					case EWeaponType.WT_ROCKETLAUNCHER : 
					{
						AddRole(EUnitRole.AT_SPECIALIST);
						break;
					}
					case EWeaponType.WT_GRENADELAUNCHER : 
					{
						AddRole(EUnitRole.GRENADIER);
						break;
					}
				}
			}
		}
		else
		{
			MagazineComponent magComp = MagazineComponent.Cast(item.FindComponent(MagazineComponent));
			if (magComp)
			{
				BaseMagazineWell baseMagwell;
				baseMagwell = magComp.GetMagazineWell();
				if (!baseMagwell)
					return;
				typename magWell = baseMagwell.Type();
				int magNum;
				if (m_MagazineWells.Find(magWell,magNum))
					m_MagazineWells.Set(magWell,magNum + 1);
				else
					m_MagazineWells.Insert(magWell,1);
			}
		}
	}
	
	void OnItemRemoved(IEntity item, BaseInventoryStorageComponent storageOwner)
	{
		if  (!m_inventoryManagerComponent || !item || !storageOwner)
			return;
		
		if (m_inventoryManagerComponent.GetHealthComponentCount() == 0)
			RemoveRole(EUnitRole.MEDIC);
				
				
		if (storageOwner.Type() == EquipedWeaponStorageComponent) // is this item a weapon 
		{
			BaseWeaponComponent wpnComponent = BaseWeaponComponent.Cast(item.FindComponent(BaseWeaponComponent));
			if (wpnComponent)
			{
				switch (wpnComponent.GetWeaponType())
				{
					case EWeaponType.WT_RIFLE : 
					{
						RemoveRole(EUnitRole.RIFLEMAN);
						break;
					}
					case EWeaponType.WT_MACHINEGUN : 
					{
						RemoveRole(EUnitRole.MACHINEGUNNER);
						break;
					}
					case EWeaponType.WT_ROCKETLAUNCHER : 
					{
						RemoveRole(EUnitRole.AT_SPECIALIST);
						break;
					}
					case EWeaponType.WT_GRENADELAUNCHER : 
					{
						RemoveRole(EUnitRole.GRENADIER);
						break;
					}
				}
			}
		}
		else 
		{
			MagazineComponent magComp = MagazineComponent.Cast(item.FindComponent(MagazineComponent));
			if (magComp)
			{
				BaseMagazineWell baseMagwell;
				baseMagwell = magComp.GetMagazineWell();
				if (!baseMagwell)
					return;
				typename magWell = baseMagwell.Type();
				int magNum;
				m_MagazineWells.Find(magWell,magNum);
				if (magNum > 0)
					m_MagazineWells.Set(magWell, magNum - 1);
				else 
					Debug.Error("Magazine count does not match!");
			}
		}
	}
	
//----------- BIT operations on Roles
	void AddRole(EUnitRole role)
	{
		m_iUnitRoles = m_iUnitRoles | role;
	}
	
	void RemoveRole(EUnitRole role)
	{
		if (HasRole(role))
			m_iUnitRoles = m_iUnitRoles & ~role;
	}
	
	bool HasRole(EUnitRole role)
	{
		return ( m_iUnitRoles & role );	
	}
	
	// ! Use only for debugging!
	EUnitRole GetRoles()
	{
		return m_iUnitRoles;
	}
	
//---------- BIT operation on States
	void AddUnitState(EUnitState state)
	{
		m_iUnitStates = m_iUnitStates | state;
	}
	
	void RemoveUnitState(EUnitState state)
	{
		if (HasUnitState(state))
			m_iUnitStates = m_iUnitStates & ~state;
	}
	
	bool HasUnitState(EUnitState state)
	{
		return ( m_iUnitStates & state );
	}
	
	// ! Use only for debugging!
	EUnitState GetUnitStates()
	{
		return m_iUnitStates;
	}
	
//--------- Fire team are disjoined - one can be member of only one fire team at the time
	void SetFireTeam(EFireTeams fireTeam)
	{
		m_iFireTeams = fireTeam;
	}
	
	EFireTeams GetFireTeam()
	{
		return m_iFireTeams; 
	}
	
//--------- AI states are disjoined - one can be in only one state at the time
	void SetAIState(EUnitAIState state)
	{
		m_iAIStates = state; 
	}
	
	EUnitAIState GetAIState()
	{
		return m_iAIStates;
	}
	
//--------  Info about magazines available to SCR_AIResupplyActivity
	
	int GetMagazineCountByWellType(typename magazinyWellType)
	{
		int result; 
		if (m_MagazineWells.Find(magazinyWellType,result))
			return result;
		return 0;
	}
		
//-------- 	Set or get AI stance, speed, raising weapon etc.
	void SetStance(ECharacterStance stance)
	{
		m_eStance = stance;
	}
	
	ECharacterStance GetStance()
	{
		return m_eStance;
	}
	
	void SetMovementType(EMovementType mode)
	{
		m_eMovementType = mode;
	}
	
	EMovementType GetMovementType()
	{
		return m_eMovementType;
	}
	
	void SetWeaponRaised(bool raised)
	{
		m_bWeaponRaised = raised;
	}
	
	bool GetWeaponRaised()
	{
		return m_bWeaponRaised;
	}
	
	void InitThreatSystem(SCR_AIThreatSystem threatSystem)
	{
		m_ThreatSystem = threatSystem;	
	}
	
	EAIThreatState GetThreatState()
	{
		if (m_ThreatSystem)
			return m_ThreatSystem.GetState();
		else 
			return EAIThreatState.NONE;
	}
	
	
//-------- 	Evaluation of wounded state of AI
	//! Returns true when state has changed and we must invoke an event
	protected void EvaluateWoundedState()
	{
		bool oldWounded = m_iUnitStates & EUnitState.WOUNDED;
		bool newWounded = m_DamageManager.IsDamagedOverTime(EDamageType.BLEEDING);
				
		// Here we can have custom logic to decide if soldier is considered wounded or not
		
		if (newWounded)
			AddUnitState(EUnitState.WOUNDED);
		else
			RemoveUnitState(EUnitState.WOUNDED);
	}
	
	protected void OnDamageOverTimeAdded(EDamageType dType, float dps, HitZone hz)
	{
		if (dType != EDamageType.BLEEDING)
			return;
		
		EvaluateWoundedState();
	}
	
	protected void OnDamageOverTimeRemoved(EDamageType dType, HitZone hz)
	{
		if (dType != EDamageType.BLEEDING)
			return;
		
		EvaluateWoundedState();
	}
	
//-------- Debugging
	// Used in SCR_AIDebugInfoComponent
	void DebugPrintToWidget(TextWidget w)
	{
		string str = string.Format("\n%1 %2", m_iFireTeams, typename.EnumToString(EFireTeams, m_iFireTeams));
		str = str + string.Format("\n%1 %2", m_iAIStates, typename.EnumToString(EUnitAIState, m_iAIStates));
		w.SetText(str);
	}
	
	
	
	string GetBehaviorEditorDebugName()
	{
		AIAgent agent = AIAgent.Cast(GetOwner());
		
		SCR_CallsignCharacterComponent callsignComp = SCR_CallsignCharacterComponent.Cast(agent.GetControlledEntity().FindComponent(SCR_CallsignCharacterComponent));
		
		FactionAffiliationComponent factionComp = FactionAffiliationComponent.Cast(agent.GetControlledEntity().FindComponent(FactionAffiliationComponent));
		
		string str;
		
		if (factionComp)
		{
			string faction = factionComp.GetAffiliatedFaction().GetFactionKey();
			str = str + string.Format("[%1] ", faction);
		}
		
		if (callsignComp)
		{			
			string company, platoon, squad, character, format;
			bool setCallsign = callsignComp.GetCallsignNames(company, platoon, squad, character, format);
			if (setCallsign)
			{
				string callsign = WidgetManager.Translate(format, company, platoon, squad, character);
				str = str + string.Format(" %1", callsign);
			}
		}
		return str;
	}
};