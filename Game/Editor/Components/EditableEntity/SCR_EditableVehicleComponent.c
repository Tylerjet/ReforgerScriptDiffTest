[ComponentEditorProps(category: "GameScripted/Editor (Editables)", description: "", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_EditableVehicleComponentClass: SCR_EditableEntityComponentClass
{
};

/** @ingroup Editable_Entities
*/

/*!
Special configuration for editable wehicle.
*/
class SCR_EditableVehicleComponent : SCR_EditableEntityComponent
{
	protected BaseCompartmentManagerComponent m_ComparmentManager;
	protected SCR_VehicleFactionAffiliationComponent m_VehicleFactionAffiliation;
	protected ref ScriptInvoker m_OnUIRefresh = new ScriptInvoker();
	
	protected void OnFactionUpdate()
	{
		m_OnUIRefresh.Invoke();
	}
	protected void OnDestroyed(IEntity owner)
	{
		m_OnUIRefresh.Invoke();
	}
	override Faction GetFaction()
	{
		//--- Destroyed entities have no faction
		if (IsDestroyed())
			return null;
		
		if (m_VehicleFactionAffiliation)
			return m_VehicleFactionAffiliation.GetAffiliatedFaction();
		
		return null;
	}
	override ScriptInvoker GetOnUIRefresh()
	{
		return m_OnUIRefresh;
	}
	override SCR_EditableEntityComponent GetAIGroup()
	{
		if (!m_ComparmentManager)
			return null;
		
		SCR_EditableEntityComponent occupant;
		array<BaseCompartmentSlot> compartments = {};
		for (int i = 0, count =	m_ComparmentManager.GetCompartments(compartments); i < count; i++)
		{
			occupant = SCR_EditableEntityComponent.GetEditableEntity(compartments[i].GetOccupant());
			if (occupant)
				return occupant.GetAIGroup();
		}
		return null;
	}
	override SCR_EditableEntityComponent GetAIEntity()
	{
		if (!m_ComparmentManager)
			return null;
		
		SCR_EditableEntityComponent occupant;
		array<BaseCompartmentSlot> compartments = {};
		for (int i = 0, count =	m_ComparmentManager.GetCompartments(compartments); i < count; i++)
		{
			occupant = SCR_EditableEntityComponent.GetEditableEntity(compartments[i].GetOccupant());
			if (occupant)
				return occupant;
		}
		return null;
	}
	
	override int GetCrew(out notnull array<CompartmentAccessComponent> crewCompartmentAccess, bool ignorePlayers = true)
	{
		SCR_BaseCompartmentManagerComponent compartmentManager = SCR_BaseCompartmentManagerComponent.Cast(GetOwner().FindComponent(SCR_BaseCompartmentManagerComponent));
		if (!compartmentManager)
			return 0;
		
		PlayerManager playerManager;

		//If Ignore players
		if (ignorePlayers)
			playerManager = GetGame().GetPlayerManager();

		array<IEntity> occupants = new array<IEntity>;
		compartmentManager.GetOccupants(occupants);
		CompartmentAccessComponent compartmentAccess;
		
		foreach(IEntity occupant: occupants)
		{
			if (ignorePlayers && playerManager.GetPlayerIdFromControlledEntity(occupant) > 0)
				continue;
			
			compartmentAccess = CompartmentAccessComponent.Cast(occupant.FindComponent(CompartmentAccessComponent));
			
			if (compartmentAccess && compartmentAccess.IsInCompartment())
				crewCompartmentAccess.Insert(compartmentAccess);
		}
		
		return crewCompartmentAccess.Count();	
	}
	
	
	override void EOnPhysicsActive(IEntity owner, bool activeState)
	{
		//--- Move to root when the vehicle is activated
		if (activeState)
			SetParentEntity(null);
		//ClearEventMask(m_Owner, EntityEvent.PHYSICSACTIVE);
	}
	override void OnPostInit(IEntity owner)
	{
		if (DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_DISABLE))
			return;
		
		super.OnPostInit(owner);
		
		//SetEventMask(owner, EntityEvent.PHYSICSACTIVE);
		
		m_ComparmentManager = BaseCompartmentManagerComponent.Cast(owner.FindComponent(BaseCompartmentManagerComponent));
		m_VehicleFactionAffiliation = SCR_VehicleFactionAffiliationComponent.Cast(owner.FindComponent(SCR_VehicleFactionAffiliationComponent));
		if (m_VehicleFactionAffiliation)
			m_VehicleFactionAffiliation.GetOnFactionUpdate().Insert(OnFactionUpdate);
		
		EventHandlerManagerComponent eventHandlerManager = EventHandlerManagerComponent.Cast(owner.FindComponent(EventHandlerManagerComponent));
		if (eventHandlerManager)
			eventHandlerManager.RegisterScriptHandler("OnDestroyed", owner, OnDestroyed);		
	}
	void ~SCR_EditableVehicleComponent()
	{
		if (m_VehicleFactionAffiliation)
			m_VehicleFactionAffiliation.GetOnFactionUpdate().Remove(OnFactionUpdate);
	}
};