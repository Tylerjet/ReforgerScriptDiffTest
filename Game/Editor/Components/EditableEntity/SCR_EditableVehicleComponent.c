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
	//~ Todo: T
	// SCR_EditableVehicleUIInfo should fill types that can be placed when the tool is used
	// Calculate budget when setting crew/passangers placing flags
	// Calculate budget when using context action to add crew
	// Save budget of entities using editable tool and a way to get said budgets (Both when placing and when using context action)
	
	protected SCR_BaseCompartmentManagerComponent m_ComparmentManager;
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
	
	//~ Add feedback to players that they are teleported when inside of the vehicle
	protected void PlayerTeleportedFeedback()
	{
		SCR_BaseCompartmentManagerComponent compartmentManager = SCR_BaseCompartmentManagerComponent.Cast(GetOwner().FindComponent(SCR_BaseCompartmentManagerComponent));
		if (!compartmentManager)
			return;
		
		array<IEntity> occupants = new array<IEntity>;
		SCR_EditableCharacterComponent editableCharacter;
		compartmentManager.GetOccupants(occupants);
		
		foreach(IEntity occupant: occupants)
		{
			editableCharacter = SCR_EditableCharacterComponent.Cast(occupant.FindComponent(SCR_EditableCharacterComponent));
			if (editableCharacter)
				editableCharacter.PlayerTeleportedByParentFeedback(true);
		}
	}
	
	
	override void SetTransform(vector transform[4], bool changedByUser = false)
	{
		super.SetTransform(transform, changedByUser);
		
		//~ Add feedback to players that they are teleported when inside of the vehicle
		PlayerTeleportedFeedback();
	}
	
		//~ Check if there is enough budget to spawn default occupants
	protected bool HasEnoughBudgetForDefaultOccupants(array<ECompartmentType> compartmentTypes)
	{
		array<BaseCompartmentSlot> compartments = new array<BaseCompartmentSlot>;
		
		//~ Get all free compartments of given types
		foreach (ECompartmentType compartmentType: compartmentTypes)
		{
			m_ComparmentManager.GetFreeCompartmentsOfType(compartments, compartmentType);
		}
		
		foreach(BaseCompartmentSlot compartment: compartments)
		{
			//~ TODO: T Calculate budget using prefab
		}
		
		return true;
	}
	
	//--------------------------------------------------- Spawn occupants ---------------------------------------------------\\
	/*!
	Check if vehicle can spawn characters (Of vehicle Faction) in the vehicle. Cannot spawn characters in vehicle if occupied by another faction that is hostile
	\param compartmentTypes Given compartment types that need to be checked if can be filled.
	\param checkEditorBudget If true will check if characters to spawn are within budget. Function will return false if there isn't enough budget
	\param checkOccupyingFaction If true function will check if the faction of the vehicle is null (empty) or the same/friendly to the static faction as given in the editableUIInfo. Function will return false if faction is hostile or neutral
	\param checkForFreeCompartments If true will check if there are compartments free for the given compartments types. Function will return false if there are no free compartments of given type
	\return Will return true if a character can be spawned in at least one compartment
	*/
	bool CanOccupyVehicleWithCharacters(array<ECompartmentType> compartmentTypes, bool checkHasDefaultOccupantsData, bool checkEditorBudget = true, bool checkOccupyingFaction = true, bool checkForFreeCompartments = true)
	{
		FactionKey factionKey = string.Empty;
		
		if (!m_ComparmentManager)
			return false;
		
		if (checkOccupyingFaction)
		{
			SCR_EditableEntityUIInfo uiInfo = SCR_EditableEntityUIInfo.Cast(GetInfo());
			if (uiInfo)
				factionKey = uiInfo.GetFactionKey();
		}
		
		if (checkEditorBudget && !HasEnoughBudgetForDefaultOccupants(compartmentTypes))
			return false;
		
		return m_ComparmentManager.CanOccupy(compartmentTypes, checkHasDefaultOccupantsData, factionKey, checkOccupyingFaction, checkForFreeCompartments);
	}
	
	/*!
	Spawn characters (Of vehicle Faction) within the Vehicle (Server only)
	\param compartmentTypes Given compartment types that need to be filled with characters
	*/
	void OccupyVehicleWithDefaultCharacters(array<ECompartmentType> compartmentTypes)
	{
		m_ComparmentManager.SpawnDefaultOccupants(compartmentTypes);
	}
	
	/*!
	Get Compartment manager of vehicle
	\return Compartment manager
	*/
	SCR_BaseCompartmentManagerComponent GetCompartmentManager()
	{
		return m_ComparmentManager;
	}
	
	override SCR_EditableEntityComponent EOnEditorPlace(out SCR_EditableEntityComponent parent, SCR_EditableEntityComponent recipient, EEditorPlacingFlags flags, bool isQueue)
	{
		//~ No vehicle placing flag
		if (!SCR_Enum.HasPartialFlag(flags, EEditorPlacingFlags.VEHICLE_CREWED | EEditorPlacingFlags.VEHICLE_PASSENGER))
			return this;
		
		//~ Todo: Check if EEditorPlacingFlags.CHARACTER_PLAYER and spawn the first character as player. If Only EEditorPlacingFlags.CHARACTER_PLAYER add itself as pilot (or any other availible slots)
		

		SCR_EditableVehicleUIInfo uiInfo = SCR_EditableVehicleUIInfo.Cast(GetInfo());
		if (!uiInfo)
			return this;
		
		array<ECompartmentType> compartmentsToFill = new array<ECompartmentType>;
		
		//~ Place all as one group
		if (uiInfo.GetEditorPlaceAsOneGroup())
		{
			array<ECompartmentType> compartmentTypeGetter = new array<ECompartmentType>;
			
			//~ Add crew
			if (flags & EEditorPlacingFlags.VEHICLE_CREWED)	
				//~ Gets types from config to make sure that even when more ECompartmentType they only need one place to be included
				compartmentsToFill.InsertAll(SCR_BaseCompartmentManagerComponent.CREW_COMPARTMENT_TYPES);
			
			//~ Add passengers
			if (flags & EEditorPlacingFlags.VEHICLE_PASSENGER)
				//~ Gets types from config to make sure that even when more ECompartmentType they only need one place to be included
				compartmentsToFill.InsertAll(SCR_BaseCompartmentManagerComponent.PASSENGER_COMPARTMENT_TYPES);
			
			//~ Check if can spawn, then spawn. If true spawn the characters
			if (HasEnoughBudgetForDefaultOccupants(compartmentsToFill))
			{
				OccupyVehicleWithDefaultCharacters(compartmentsToFill);
				return this;
			}
			//~ Not enough budget to spawn all characters. Check if both crew and passengers where spawned at the same time. If false return function else continue the function and try to spawn them separately
			else if (!(flags & EEditorPlacingFlags.VEHICLE_CREWED) || !(flags & EEditorPlacingFlags.VEHICLE_PASSENGER))
			{
				//~ Show notification
				SCR_NotificationsComponent.SendLocal(ENotification.EDITOR_PLACING_BUDGET_MAX_FOR_VEHICLE_OCCUPANTS);
				return this;
			}
		}
		
		bool enoughBudget = true;
		
		//~ Place in seperate groups
		//~ Spawn crew
		if (flags & EEditorPlacingFlags.VEHICLE_CREWED)
		{		
			//~ Check if enough budget
			if (HasEnoughBudgetForDefaultOccupants(SCR_BaseCompartmentManagerComponent.CREW_COMPARTMENT_TYPES))
				OccupyVehicleWithDefaultCharacters(SCR_BaseCompartmentManagerComponent.CREW_COMPARTMENT_TYPES);
			else 
				enoughBudget = false;
		}
	
		//~ Spawn passengers, It makes sure passengers are a diffrent group from crew
		if (flags & EEditorPlacingFlags.VEHICLE_PASSENGER)
		{			
			//~ Check if enough budget
			if (HasEnoughBudgetForDefaultOccupants(SCR_BaseCompartmentManagerComponent.PASSENGER_COMPARTMENT_TYPES))
				OccupyVehicleWithDefaultCharacters(SCR_BaseCompartmentManagerComponent.PASSENGER_COMPARTMENT_TYPES);
			else 
				enoughBudget = false;
		}
		
		//~ Not enough budget for one or more of the place flags. So Send notification
		if (!enoughBudget)
			SCR_NotificationsComponent.SendLocal(ENotification.EDITOR_PLACING_BUDGET_MAX_FOR_VEHICLE_OCCUPANTS);
		
		return this;
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
		
		m_ComparmentManager = SCR_BaseCompartmentManagerComponent.Cast(owner.FindComponent(SCR_BaseCompartmentManagerComponent));
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