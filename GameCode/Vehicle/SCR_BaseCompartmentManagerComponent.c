class SCR_BaseCompartmentManagerComponentClass: BaseCompartmentManagerComponentClass
{
};

class SCR_BaseCompartmentManagerComponent : BaseCompartmentManagerComponent
{
	//Compartment slots associated with crew
	static const ref array<ECompartmentType> CREW_COMPARTMENT_TYPES = {ECompartmentType.Pilot, ECompartmentType.Turret};
	
	//Compartment slots associated with passengers
	static const ref array<ECompartmentType> PASSENGER_COMPARTMENT_TYPES = {ECompartmentType.Cargo};
	
	//~ Default Occupant spawning
	protected ref array<BaseCompartmentSlot> m_aCompartmentsToSpawnDefaultOccupant;
	protected ref array<IEntity> m_aSpawnedDefaultOccupants;
	protected bool m_bIsSpawningDefaultOccupants;
	protected AIGroup m_SpawnedOccupantsAIGroup;
	protected ref ScriptInvoker Event_OnDoneSpawningDefaultOccupants; //~ Send over SCR_BaseCompartmentManagerComponent, array of spawned characters (IEntity) as well as bool WasCanceled
	
	/*!
	Get compartments of specific type
	*/
	void GetCompartmentsOfType(inout array<BaseCompartmentSlot> outCompartments, ECompartmentType compartmentType)
	{
		array<BaseCompartmentSlot> compartments = {}; GetCompartments(compartments);
		foreach (BaseCompartmentSlot compartment: compartments)
		{
			if (compartment && SCR_CompartmentAccessComponent.GetCompartmentType(compartment) == compartmentType)
				outCompartments.Insert(compartment);
		}
	}
	
	/*!
	Get all free compartment slots of the given type
	\param[out] outCompartments array of all free compartments of type
	\param compartmentType The compartment type to check
	*/
	void GetFreeCompartmentsOfType(inout array<BaseCompartmentSlot> outCompartments, ECompartmentType compartmentType, bool checkHasValidOccupantPrefab = false)
	{
		array<BaseCompartmentSlot> compartments = {}; GetCompartments(compartments);
		foreach (BaseCompartmentSlot compartment: compartments)
		{
			if (compartment && SCR_CompartmentAccessComponent.GetCompartmentType(compartment) == compartmentType && !compartment.IsOccupied() && compartment.IsCompartmentAccessible() && (!checkHasValidOccupantPrefab || (checkHasValidOccupantPrefab && !compartment.GetDefaultOccupantPrefab().IsEmpty())))
				outCompartments.Insert(compartment);
		}
	}
	
	/*!
	Go over all compartments of given type and checks if at least one compartment is free
	\return true if at least one compartment of given types is free
	*/
	bool HasFreeCompartmentOfTypes(notnull array<ECompartmentType> compartmentTypes)
	{
		array<BaseCompartmentSlot> compartments = {}; 
		
		foreach(ECompartmentType type: compartmentTypes)
		{
			compartments.Clear();
			GetCompartmentsOfType(compartments, type);
			
			foreach(BaseCompartmentSlot compartment: compartments)
			{
				if (!compartment.IsOccupied() && compartment.IsCompartmentAccessible())
					return true;
			}
		}
		
		return false;
	}
	
	/*!
	Get occupants of all managed compartments.
	*/
	void GetOccupants(inout array<IEntity> occupants)
	{
		array<BaseCompartmentSlot> compartments = {}; GetCompartments(compartments);
		IEntity occupant;
		foreach (BaseCompartmentSlot compartment: compartments)
		{
			occupant = compartment.GetOccupant();
			if (occupant)
				occupants.Insert(occupant);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get occupants of all managed compartments of specified type.
	void GetOccupantsOfType(inout array<IEntity> occupants, ECompartmentType compartmentType)
	{
		array<BaseCompartmentSlot> compartments = {}; GetCompartmentsOfType(compartments, compartmentType);
		IEntity occupant;
		foreach (BaseCompartmentSlot compartment: compartments)
		{
			occupant = compartment.GetOccupant();
			if (occupant)
				occupants.Insert(occupant);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Damage random hitzones depending on amount of damage to be applied. Must be called on authority.
	void DamageOccupants(float damage, EDamageType damageType, IEntity instigator = null, bool gettingIn = false, bool gettingOut = false)
	{
		if (damage == 0)
			return;
		
		array<BaseCompartmentSlot> compartments = {}; 
		GetCompartments(compartments);
		
		foreach (BaseCompartmentSlot compartment: compartments)
		{
			if (compartment)
				compartment.DamageOccupant(damage, damageType, instigator, gettingIn, gettingOut);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Kill all the occupants and eject them if requested. Must be called on authority.
	void KillOccupants(IEntity instigator = null, bool eject = false, bool gettingIn = false, bool gettingOut = false)
	{
		array<BaseCompartmentSlot> compartments = {}; 
		GetCompartments(compartments);
		
		foreach (BaseCompartmentSlot compartment: compartments)
		{
			if (compartment)
				compartment.KillOccupant(instigator, eject, gettingIn, gettingOut);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set AI compartment access for all compartments
	void SetCompartmentsAccessibleForAI(bool accessible)
	{
		array<BaseCompartmentSlot> compartments = {}; 
		GetCompartments(compartments);
		
		foreach (BaseCompartmentSlot compartment: compartments)
			compartment.SetCompartmentAccessible(accessible);
	}
	
	/*!
	Get event called when spawning of occupants is done. 
	Gives an array of IEntity (Spawned characters) and bool if spawning was canceled or not
	\return Spawn done scriptinvoker
	*/
	ScriptInvoker GetOnDoneSpawningDefaultOccupants()
	{
		if (!Event_OnDoneSpawningDefaultOccupants)
			Event_OnDoneSpawningDefaultOccupants = new ref ScriptInvoker;
		
		return Event_OnDoneSpawningDefaultOccupants;
	}
	
	//--------------------------------------------------- Check can occupy with characters ---------------------------------------------------\\
	/*!
	Check if can spawn characters in the vehicle.
	\param compartmentTypes Given compartment types that need to be checked if can be filled.
	\param checkEditorBudget If true will check if characters to spawn are within budget. Function will return false if there isn't enough budget
	\param checkOccupyingFaction If true function will check if the faction of the vehicle is null (empty) or the same/friendly to the static faction as given in the editableUIInfo. Function will return false if faction is hostile or neutral
	\param checkForFreeCompartments If true will check if there are compartments free for the given compartments types. Function will return false if there are no free compartments of given type
	\return Will return true if a character can be spawned in at least one compartment
	*/
	bool CanOccupy(array<ECompartmentType> compartmentTypes, bool checkHasDefaultOccupantsData = true, FactionKey friendlyFaction = string.Empty, bool checkOccupyingFaction = true, bool checkForFreeCompartments = true)
	{
		//~ Check if default character data is defined
		if (checkHasDefaultOccupantsData && !HasDefaultOccupantsDataForTypes(compartmentTypes))
			return false;
		
		//~ Check if all occupants in vehicle are friendly. If not return false
		if (checkOccupyingFaction && !IsOccupiedByFriendlies(friendlyFaction, true))
			return false;

		//~ Check if there is at least one free compartment. If not return false
		if (checkForFreeCompartments && !HasFreeCompartmentOfTypes(compartmentTypes))
			return false;
		
		return true;
	}	
	
	//--------------------------------------------------- Spawn default occupants ---------------------------------------------------\\
	/*!
	Spawns default characters as defined in BaseComparmentSlots inside the vehicle of given compartment types (Server only)
	Places all spawned characters in the same group
	\param compartmentTypes Given compartment types that need to be filled with characters
	\return True if start spawning occupants
	*/
	bool SpawnDefaultOccupants(notnull array<ECompartmentType> compartmentTypes)
	{	
		//~ No compartment types given to spawn
		if (compartmentTypes.IsEmpty())
		{
			if (Event_OnDoneSpawningDefaultOccupants)
				Event_OnDoneSpawningDefaultOccupants.Invoke(this, null, true);
			
			return false;
		}
			
		//~ Is already spawning
		if (m_bIsSpawningDefaultOccupants)
		{
			Print("'SCR_BaseCompartmentManagerComponent' is already spawning default occupants. Wait until spawning is done", LogLevel.WARNING);
			return false; 
		}
		
		m_aCompartmentsToSpawnDefaultOccupant = {};
		
		foreach(ECompartmentType compartmentType: compartmentTypes)
		{
			GetFreeCompartmentsOfType(m_aCompartmentsToSpawnDefaultOccupant, compartmentType, true);
		}
		
		//~ Nothing to spawn so cancel
		if (m_aCompartmentsToSpawnDefaultOccupant.IsEmpty())
		{
			m_aCompartmentsToSpawnDefaultOccupant = null;
			
			if (Event_OnDoneSpawningDefaultOccupants)
				Event_OnDoneSpawningDefaultOccupants.Invoke(this, null, true);
			
			return false;
		}
			
		m_aSpawnedDefaultOccupants = {};
		m_bIsSpawningDefaultOccupants = true;
		m_SpawnedOccupantsAIGroup = null;
		
		GetGame().GetCallqueue().CallLater(SpawnDefaultOccupantEachFrame, 0, true);
		return true;
	}
	
	
	protected void SpawnDefaultOccupantEachFrame()
	{
		BaseCompartmentSlot compartmentToFill = null;
		
		for(int i = 0; i < m_aCompartmentsToSpawnDefaultOccupant.Count(); i++)
        {
			//~ Invalid compartment so remove it from the list
           	if (!m_aCompartmentsToSpawnDefaultOccupant[i] || m_aCompartmentsToSpawnDefaultOccupant[i].IsOccupied() || !m_aCompartmentsToSpawnDefaultOccupant[i].IsCompartmentAccessible())
			{
				m_aCompartmentsToSpawnDefaultOccupant.RemoveOrdered(i);
				i--;
				continue;
			}
			
			compartmentToFill = m_aCompartmentsToSpawnDefaultOccupant[i];
			m_aCompartmentsToSpawnDefaultOccupant.RemoveOrdered(i);
			break;
        }
		
		//~ No compartments found
		if (!compartmentToFill)
		{
			FinishedSpawningDefaultOccupants(false);
			return;
		}
		
		IEntity spawnedCharacter = compartmentToFill.SpawnDefaultCharacterInCompartment(m_SpawnedOccupantsAIGroup);
		if (!spawnedCharacter)
			return;
		
		
		m_aSpawnedDefaultOccupants.Insert(spawnedCharacter);
		
		if (m_aCompartmentsToSpawnDefaultOccupant.IsEmpty())
			FinishedSpawningDefaultOccupants(false);
	}
	
	/*!
	Cancel spawning if in progress
	*/
	void CancelSpawningCharacters()
	{
		if (!m_bIsSpawningDefaultOccupants)
			return;
		
		FinishedSpawningDefaultOccupants(true);
	}
	
	protected void FinishedSpawningDefaultOccupants(bool wasCanceled)
	{
		m_bIsSpawningDefaultOccupants = false;
		if (Event_OnDoneSpawningDefaultOccupants)
			Event_OnDoneSpawningDefaultOccupants.Invoke(this, m_aSpawnedDefaultOccupants, wasCanceled);
		m_aSpawnedDefaultOccupants = null;
		m_aCompartmentsToSpawnDefaultOccupant = null;
		GetGame().GetCallqueue().Remove(SpawnDefaultOccupantEachFrame);
	}
	
	//--------------------------------------------------- Vehicle occupied by friendly ---------------------------------------------------\\
	/*!
	Checks if vehicle is occupied by characters that are friendly towards the given faction
	\param vehicleFaction the faction of the vehicle
	\param allFriendly If true will find all occupants and check if all of them are friendly. If false it will only check the first character it finds, mixed factions might not return a valid value.
	\param trueIfEmpty If this is true then the function will return true if the vehicle is empty. Else it will return false
	\return Will return true if the vehicle is occupied by a friendly/friendlies.
	*/
	bool IsOccupiedByFriendlies(FactionKey vehicleFactionKey, bool allFriendly, bool trueIfEmpty = true)
	{
		//~ No given string so cannot check
		if (vehicleFactionKey.IsEmpty())
			return false;
		
		array<BaseCompartmentSlot> compartments = {}; 
		GetCompartments(compartments);
		
		if (compartments.IsEmpty())
			return false;
		
		//~ No faction manager so return false
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return false;
		
		IEntity occupant; 
		FactionAffiliationComponent factionAffiliationComponent;
		Faction occupantFaction;
		Faction vehicleFaction = factionManager.GetFactionByKey(vehicleFactionKey);
		bool friendlyFound = false;
		bool occupantFound = false;
		
		//~ Go over each compartment and see if it is occupied and if the enetity is an enemy or neutral to the given faction
		foreach (BaseCompartmentSlot compartment: compartments)
		{
			occupant = compartment.GetOccupant();
			
			if (!occupant)
				continue;
			
			occupantFound = true;
			
			factionAffiliationComponent = FactionAffiliationComponent.Cast(occupant.FindComponent(FactionAffiliationComponent));
			if (!factionAffiliationComponent)
				continue;
			
			occupantFaction = factionAffiliationComponent.GetAffiliatedFaction();
			
			//~ Same faction so is friendly
			if (occupantFaction.GetFactionKey() == vehicleFactionKey)
				return true;
			
			//~ Occupent is friendly
			if (vehicleFaction.IsFactionFriendly(occupantFaction))
			{
				if (allFriendly)
					friendlyFound = true;
				else 
					return true;
			}
			//~ Occupent is not friendly!
			else 
			{
				return false;
			}
				
		}
		
		//~ No occupants found
		if (!occupantFound)
			return trueIfEmpty;
		
		//~ Multiple occupants found check if all must be friendly
		if (allFriendly)
			return friendlyFound;
		
		return false;
	}
	
	//--------------------------------------------------- Has default occupent data set of type ---------------------------------------------------\\
	/*!
	Check if there is at least one default occupant prefab set for the given compartment types
	\param compartmentTypes Given compartment types that need to be filled with characters
	\return True if at least one prefab data is set
	*/
	bool HasDefaultOccupantsDataForTypes(array<ECompartmentType> compartmentTypes)
	{
		array<BaseCompartmentSlot> compartments = new array<BaseCompartmentSlot>;
		SCR_DefaultOccupantData defaultOccupantData;
		
		foreach (ECompartmentType compartmentType: compartmentTypes)
		{
			compartments.Clear();
			GetCompartmentsOfType(compartments, compartmentType);
			
			foreach(BaseCompartmentSlot compartment: compartments)
			{
				defaultOccupantData = compartment.GetDefaultOccupantData();
				
				if (defaultOccupantData && defaultOccupantData.IsValid())
					return true;
			}
		}
		return false;
	}
	
	void ~SCR_BaseCompartmentManagerComponent()
	{
		//~ Component destroyed before it could finish spawning all characters
		if (m_bIsSpawningDefaultOccupants)
			FinishedSpawningDefaultOccupants(true);
	}
};