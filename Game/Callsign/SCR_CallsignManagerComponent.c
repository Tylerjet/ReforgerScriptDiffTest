/*!
Manages availible callsigns for each faction
*/
[ComponentEditorProps(category: "GameScripted/Callsign", description: "")]
class SCR_CallsignManagerComponentClass: ScriptComponentClass
{
};
class SCR_CallsignManagerComponent: ScriptComponent
{
	//All availible callsigns are stored here
	protected ref map<Faction, ref SCR_FactionCallsignData> m_mAvailibleCallsigns = new ref map<Faction, ref SCR_FactionCallsignData>;
	
	//Assigned Duplicate callsigns. If all callsigns are assigned then this stores any assigned dupplicates.
	protected ref array<ref array<int>> m_aDuplicateCallsigns = new ref array<ref array<int>>;
	
	//Holds all the callsigns that are assigned to players
	protected ref map<int, ref SCR_PlayerCallsignData> m_mPlayerCallsignData = new ref map<int, ref SCR_PlayerCallsignData>;
	
	//======================================== GET CALLSIGN ========================================\\
	/*!
	Get the callsign indexes assigned to entity
	\param entity IEntity to get callsign off
	\param[out] company index
	\param[out] platoon index
	\param[out] squad index
	\param[out] character index, either role or character index, will return -1 if not a character
	\return bool returns false if indexes are not assigned
	*/
	bool GetEntityCallsignIndexes(IEntity entity, out int companyCallsignIndex, out int platoonCallsignIndex, out int squadCallsignIndex, out int characterCallsignIndex)
	{
		SCR_CallsignBaseComponent callsignComponent = SCR_CallsignBaseComponent.Cast(entity.FindComponent(SCR_CallsignBaseComponent));
		
		return callsignComponent && callsignComponent.GetCallsignIndexes(companyCallsignIndex, platoonCallsignIndex, squadCallsignIndex, characterCallsignIndex);
	}
	
	/*!
	Get the callsign indexes assigned to entity
	\param editableEntity SCR_EditableEntityComponent to get callsign off
	\param[out] company index
	\param[out] platoon index
	\param[out] squad index
	\param[out] character index, either role or character index, will return -1 if not a character
	\return bool returns false if indexes are not assigned
	*/
	bool GetEntityCallsignIndexes(SCR_EditableEntityComponent editableEntity, out int companyCallsignIndex, out int platoonCallsignIndex, out int squadCallsignIndex, out int characterCallsignIndex)
	{
		return GetEntityCallsignIndexes(editableEntity.GetOwner(), companyCallsignIndex, platoonCallsignIndex, squadCallsignIndex, characterCallsignIndex);
	}
	
	/*!
	Get the callsign names assigned to entity
	\param entity IEntity to get callsign off
	\param[out] company name
	\param[out] platoon name
	\param[out] squad name
	\param[out] character name (Optinal if callsign assigned to a character)
	\param[out] format
	\return bool returns true if names are succesfully found
	*/
	bool GetEntityCallsignNames(IEntity entity, out string companyCallsignName, out string platoonCallsignName, out string squadCallsignName, out string characterCallsignName, out string format)
	{
		SCR_CallsignBaseComponent callsignComponent = SCR_CallsignBaseComponent.Cast(entity.FindComponent(SCR_CallsignBaseComponent));
		
		if (!callsignComponent)
			return false;
		
		return callsignComponent.GetCallsignNames(companyCallsignName, platoonCallsignName, squadCallsignName, characterCallsignName, format);
	}
	
	/*!
	Get the callsign names assigned to editableEntity
	\param editableEntity SCR_EditableEntityComponent to get callsign off
	\param[out] company name
	\param[out] platoon name
	\param[out] squad name
	\param[out] character name (Optinal if callsign assigned to a character)
	\param[out] format
	\return bool returns true if names are succesfully found
	*/
	bool GetEntityCallsignNames(SCR_EditableEntityComponent editableEntity, out string companyCallsignName, out string platoonCallsignName, out string squadCallsignName, out string characterCallsignName, out string format)
	{
		return GetEntityCallsignNames(editableEntity.GetOwner(), companyCallsignName, platoonCallsignName, squadCallsignName, characterCallsignName, format);
	}
	
	
	//======================================== ASSIGN GROUP CALLSIGN ========================================\\
	/*!
	Uses the faction to get a availible company, platoon and squad index. And makes these unavailible so they are not picked again until availible again
	\param faction faction to assign callsigns
	\param[out] companyIndex to assign
	\param[out] platoonIndex to assign
	\param[out] squadIndex to assign
	*/ 
	void AssignCallGroupsign(Faction faction, out int companyIndex, out int platoonIndex, out int squadIndex)
	{
		companyIndex = -1;
		platoonIndex = -1;
		squadIndex = -1;
		
		//Faction has no callsigns
		if (!m_mAvailibleCallsigns.Contains(faction))
			return;
		
		SCR_Faction ScrFaction = SCR_Faction.Cast(faction);
		
		if (!ScrFaction)
			return;
		
		SCR_FactionCallsignInfo callsignInfo = ScrFaction.GetCallsignInfo();
		if (!callsignInfo)
			return;
		
		//~Todo: Add logic for assigning same company callsigns by getting closest (same faction) company
		
		//Assign the first availible callsign
		if (!callsignInfo.GetIsAssignedRandomly())
			AssignFirstAvailibleGroupCallsign(faction, companyIndex, platoonIndex, squadIndex);
		//Assign a random availible callsign
		else 
			AssignRandomGroupCallsigns(faction, companyIndex, platoonIndex, squadIndex);
	}
	
	//---------------------------------------- Assign first availible callsign ----------------------------------------\\
	protected void AssignFirstAvailibleGroupCallsign(Faction faction, out int companyIndex, out int platoonIndex, out int squadIndex)
	{
		SCR_FactionCallsignData factionCallsign; 
		if (m_mAvailibleCallsigns.Find(faction, factionCallsign))
		{
			if (factionCallsign.GetFirstAvailibleCallsign(companyIndex, platoonIndex, squadIndex))
			{
				RemoveAvailibleGroupCallsign(faction, companyIndex, platoonIndex, squadIndex);
				return;
			}	
		}
		
		//Callsign was not assigned as all callsigns are taken
		AssignRandomDuplicateCallsign(faction, companyIndex, platoonIndex, squadIndex);
		Print(string.Format("All availible callsigns are taken for faction '%1', so a random duplicate is assigned instead. If this happenes a lot then more callsigns should be added for this faction!", faction.GetFactionName()), LogLevel.WARNING);
	}
	
	//---------------------------------------- Assign specific company callsign ----------------------------------------\\
	//~Todo: Not yet used. Should check neighbouring groups and assign the same company if close to same faction group
	protected void AssignCompanySpecificGroupCallsign(Faction faction, out int specificCompanyIndex, out int platoonIndex, out int squadIndex)
	{
		SCR_FactionCallsignData factionCallsign; 
		if (m_mAvailibleCallsigns.Find(faction, factionCallsign))
		{
			if (factionCallsign.GetSpecificCompanyCallsign(specificCompanyIndex, platoonIndex, squadIndex))
			{
				RemoveAvailibleGroupCallsign(faction, specificCompanyIndex, platoonIndex, squadIndex);
				return;
			}
		}
		
		//Callsign was not assigned as all callsigns are taken
		AssignRandomDuplicateCallsign(faction, specificCompanyIndex, platoonIndex, squadIndex);
		Print(string.Format("All availible callsigns are taken for faction '%1', so a random duplicate is assigned instead. If this happenes a lot then more callsigns should be added for this faction!", faction.GetFactionName()), LogLevel.WARNING);
	}
	
	//---------------------------------------- Assign random callsign ----------------------------------------\\
	void AssignRandomGroupCallsigns(Faction faction, out int companyIndex, out int platoonIndex, out int squadIndex)
	{
		SCR_FactionCallsignData factionCallsign; 
		if (m_mAvailibleCallsigns.Find(faction, factionCallsign))
		{
			if (factionCallsign.GetRandomCallsign(companyIndex, platoonIndex, squadIndex))
			{
				RemoveAvailibleGroupCallsign(faction, companyIndex, platoonIndex, squadIndex);
				return;
			}	
		}
		
		//Callsign was not assigned as all callsigns are taken
		AssignRandomDuplicateCallsign(faction, companyIndex, platoonIndex, squadIndex);
		Print(string.Format("All availible callsigns are taken for faction '%1', so a random duplicate is assigned instead. If this happenes a lot then more callsigns should be added for this faction!", faction.GetFactionName()), LogLevel.WARNING);
	}
	
	//======================================== ADD/ REMOVE AVAILIBLE CALLSIGNs ========================================\\
	//---------------------------------------- Add availible callsign ----------------------------------------\\
	/*!
	Makes the given callsign indexes availible again for the faction.
	Called when the entity with the callsign is destroyed or switches factions
	\param faction faction of callsigns
	\param companyIndex to make availible
	\param platoonIndex to make availible
	\param squadIndex to make availible
	*/ 
	void MakeGroupCallsignAvailible(Faction faction, int companyIndex, int platoonIndex, int squadIndex)
	{		
		//Check if callsign was assigned duplicate
		if (!m_aDuplicateCallsigns.IsEmpty())
		{
			int count = m_aDuplicateCallsigns.Count();
			
			for(int i = 0; i < count; i++)
			{				
				//The assigned callsign was a duplicate safty. So it is removed from the duplicate list instead of making it availible again
				if (m_aDuplicateCallsigns[i][0] == companyIndex && m_aDuplicateCallsigns[i][1] == platoonIndex && m_aDuplicateCallsigns[i][2] == squadIndex)
				{
					m_aDuplicateCallsigns.Remove(i);
					return;
				}
			}
		}
		
		SCR_FactionCallsignData factionCallsignData;
		
		if (m_mAvailibleCallsigns.Find(faction, factionCallsignData))
			factionCallsignData.AddCallsign(companyIndex, platoonIndex, squadIndex);
	}
	
	//---------------------------------------- Remove availible callsign ----------------------------------------\\
	//A callsign is assigned
	protected void RemoveAvailibleGroupCallsign(Faction faction, int companyIndex, int platoonIndex, int squadIndex)
	{
		SCR_FactionCallsignData factionCallsignData;
		
		if (m_mAvailibleCallsigns.Find(faction, factionCallsignData))
			factionCallsignData.RemoveCallsign(companyIndex, platoonIndex, squadIndex);
	}
	
	
	//======================================== ASSIGN DUPLICATE CALLSIGN ========================================\\
	//If all callsigns are taken then assign a random callsign. This is am edge case safty as otherwise entities will be without callsigns
	protected void AssignRandomDuplicateCallsign(Faction faction, out int company, out int platoon, out int squad)
	{		
		SCR_Faction ScrFaction = SCR_Faction.Cast(faction);
		if (!ScrFaction)
			return;
		
		SCR_FactionCallsignInfo factionCallsignInfo = ScrFaction.GetCallsignInfo();
		if (!factionCallsignInfo)
			return;
		
		if (factionCallsignInfo.GetRandomCallsign(company, platoon, squad))
		{
			array<int> duplicateCallsign = new array<int>;
			duplicateCallsign.Insert(company);
			duplicateCallsign.Insert(platoon);
			duplicateCallsign.Insert(squad);
			
			m_aDuplicateCallsigns.Insert(duplicateCallsign);
		}
	}
	
	//======================================== PLAYER CALLSIGNS ========================================\\
	//---------------------------------------- On Faction joined ----------------------------------------\\
	//On player spawned, assign callsign if new faction or no callsign yet
	protected void OnPlayerSpawn(int playerId, IEntity playerEntity)
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (groupsManager)
			return;
		
		FactionAffiliationComponent factionAffiliation = FactionAffiliationComponent.Cast(playerEntity.FindComponent(FactionAffiliationComponent));
		if (!factionAffiliation)
			return;
		
		Faction faction = factionAffiliation.GetAffiliatedFaction();
		if (!faction)
			return;
		
		SCR_PlayerCallsignData playerCallsignData;
		
		if (m_mPlayerCallsignData.Find(playerId, playerCallsignData))
		{
			if (playerCallsignData.GetFaction() != faction)
			{
				playerCallsignData.SetPlayerCallsign(faction, this);
				m_mPlayerCallsignData.Insert(playerId, playerCallsignData);
			}
		}
		else 
		{			
			playerCallsignData = new SCR_PlayerCallsignData(faction, this);
			m_mPlayerCallsignData.Insert(playerId, playerCallsignData);
		}
	}
	
	//---------------------------------------- On player left ----------------------------------------\\
	protected void OnPlayerLeftGame(int playerId)
	{
		//Make player callsign availible again
		SCR_PlayerCallsignData playerCallsignData;
		if (m_mPlayerCallsignData.Find(playerId, playerCallsignData))
		{			
			playerCallsignData.MakePlayerCallsignAvailible(this);
			m_mPlayerCallsignData.Remove(playerId);
		}
	}
	
	//---------------------------------------- Get Player Callsigns System ----------------------------------------\\
	/*!
	Returns player callsigns. Should only be called by player controlled, use GetPlayerCallsignIndexes instead
	\param playerId, id of player
	\param[out] companyIndex assigned
	\param[out] platoonIndex assigned
	\param[out] squadIndex assigned
	\param[out] characterIndex assigned (Will not return player role!, use GetPlayerCallsignIndexes instead)
	\return returns false if callsign was not assigned
	*/ 
	bool SystemGetPlayerCallsignIndexes(int playerId, out int companyIndex, out int platoonIndex, out int squadIndex, out int characterIndex)
	{
		SCR_PlayerCallsignData playerCallsignData;
		if (m_mPlayerCallsignData.Find(playerId, playerCallsignData))
			return playerCallsignData.GetPlayerCallsignIndexes(companyIndex, platoonIndex, squadIndex, characterIndex);
		else 
			return false;
	}
	
	
	//======================================== INIT ========================================\\
	//---------------------------------------- Fill callsign list ----------------------------------------\\
	protected void FillAvailibleCallsigns()
	{
		array<Faction> factions = new array<Faction>;
		FactionManager factionManager = GetGame().GetFactionManager();
		
		if (!factionManager)
			return;
		
		factionManager.GetFactionsList(factions);
		
		foreach (Faction faction: factions)
		{
			SCR_Faction ScrFaction = SCR_Faction.Cast(faction);
			
			if (!ScrFaction)
				continue;
		
			SCR_FactionCallsignInfo factionCallsignInfo = ScrFaction.GetCallsignInfo();
			
			if (!factionCallsignInfo)
				continue;
			
			array<ref SCR_CallsignInfo> companyArray = new array<ref SCR_CallsignInfo>;
			array<ref SCR_CallsignInfo> platoonArray = new array<ref SCR_CallsignInfo>;
			array<ref SCR_CallsignInfo> squadArray = new array<ref SCR_CallsignInfo>;
			
			factionCallsignInfo.GetCompanyArray(companyArray);
			if (companyArray.IsEmpty())
				continue;
			
			factionCallsignInfo.GetPlatoonArray(platoonArray);
			if (platoonArray.IsEmpty())
				continue;
			
			factionCallsignInfo.GetSquadArray(squadArray);
			if (squadArray.IsEmpty())
				continue;
			
			SCR_FactionCallsignData factionCallsignData = new SCR_FactionCallsignData(factionCallsignInfo);
			
			m_mAvailibleCallsigns.Insert(faction, factionCallsignData);
		}
	}

	//---------------------------------------- On Init ----------------------------------------\\
	override void EOnInit(IEntity owner)
	{
		FillAvailibleCallsigns();
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (gameMode)
		{
			gameMode.GetOnPlayerSpawned().Insert(OnPlayerSpawn);
			gameMode.GetOnPlayerDisconnected().Insert(OnPlayerLeftGame);
		}
	}	
	
	//---------------------------------------- On Post Init ----------------------------------------\\
	override void OnPostInit(IEntity owner)
	{	
		if (SCR_Global.IsEditMode(owner) || Replication.IsClient())
			return;

		owner.SetFlags(EntityFlags.ACTIVE, false);
		SetEventMask(owner, EntityEvent.INIT);
	}
	
	void ~SCR_CallsignManagerComponent()
	{
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (gameMode)
		{
			gameMode.GetOnPlayerSpawned().Remove(OnPlayerSpawn);
			gameMode.GetOnPlayerDisconnected().Remove(OnPlayerLeftGame);
		}
	}
};
