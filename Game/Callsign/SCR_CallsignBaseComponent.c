[ComponentEditorProps(category: "GameScripted/Callsign", description: "")]
class SCR_CallsignBaseComponentClass: ScriptComponentClass
{
};

/*!
Component of assigning and storing squad names
*/
class SCR_CallsignBaseComponent : ScriptComponent
{
	protected Faction m_Faction;
	protected SCR_FactionCallsignInfo m_CallsignInfo;
	
	//Callsign IDs(Broadcast)
	protected int m_iCompanyCallsign = -1;
	protected int m_iPlatoonCallsign = -1;
	protected int m_iSquadCallsign = -1;
	
	//State
	protected bool m_bIsServer;
	
	//Ref
	protected SCR_CallsignManagerComponent m_CallsignManager;
	
	//ScriptEnvokers
	//Will always return int Company, int Platoon, int squad, int character number and character role (See GetCallsignIndexes() comments for more information). Also called when assigning callsigns for first time
	protected ref ScriptInvoker Event_OnCallsignChanged = new ref ScriptInvoker();

	
	//======================================== GET CALLSIGN NAMES ========================================\\
	/*!
	Get the callsign names assigned
	\param[out] company Company name
	\param[out] platoon Platoon name
	\param[out] squad Squad name
	\param[out] character Character name (Optional if callsign assigned to a character). Will return Role name if any role is assigned to the character
	\param[out] format Format of callsign
	\return bool returns true if names are succesfully found
	*/
	bool GetCallsignNames(out string company, out string platoon, out string squad, out string character, out string format)
	{	
		return false;
	}
	
	//======================================== GET CALLSIGN INDEXES ========================================\\
	/*!
	Get the callsign indexes assigned
	\param[out] company company index. Eg: Company Alpha is index 0 and Beta is index 1
	\param[out] platoon platoon index. Is an index similar to how Company works.
	\param[out] squad squad index. Is an index similar to how Company works.
	\param[out] character character callsign number. This is not an index. Soldier 1 will have character 1. The second soldier in the group will have character 2. Will return -1 if component is on a group
	\param[out] characterRole Character role number, roles can be checked with ERoleCallsign eg: characterRole == ERoleCallsign.SQUAD_LEADER will let you know if the character is a squad leader etc. Will return -1 if component is on a group
	\return bool returns false if indexes are not assigned
	*/
	bool GetCallsignIndexes(out int company, out int platoon, out int squad, out int character = -1, out int characterRole = -1)
	{
	}
	
	protected void ClearCallsigns()
	{
		m_CallsignInfo = null;
		
		m_iCompanyCallsign = -1;
		m_iPlatoonCallsign = -1;
		m_iSquadCallsign = -1;
	}
	
	//======================================== SET CALLSIGN INFO ========================================\\
	protected bool SetCallsignInfo()
	{
		Faction currentFaction;
		
		SCR_AIGroup group = SCR_AIGroup.Cast(GetOwner());
		if (group && group.IsPlayable())
				currentFaction = group.GetFaction();
		else
		{
			SCR_EditableEntityComponent editableEntityComponent = SCR_EditableEntityComponent.GetEditableEntity(GetOwner());
			if (editableEntityComponent)
				currentFaction = editableEntityComponent.GetFaction();
		}
		
		if (m_Faction != currentFaction && currentFaction != null)
		{
			m_Faction = currentFaction;
			m_CallsignInfo = null;
		}
		else 
		{
			if (m_CallsignInfo)
				return true;
		}
		
		SCR_Faction scrFaction = SCR_Faction.Cast(m_Faction);
		
		if (!scrFaction)
			return false;
		
		m_CallsignInfo = scrFaction.GetCallsignInfo();
		return m_CallsignInfo != null;
	}
	
	//======================================== GETTERS ========================================\\
	ScriptInvoker GetOnCallsignChanged()
	{
		return Event_OnCallsignChanged;
	}
	
	//======================================== INIT ========================================\\
	//---------------------------------------- Server Init ----------------------------------------\\
	protected void InitOnServer(IEntity owner)
	{
	}
	

	
	//---------------------------------------- On Init ----------------------------------------\\
	override void EOnInit(IEntity owner)
	{
		if (SCR_Global.IsEditMode(owner))
			return;
		
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return;
		
		m_CallsignManager = SCR_CallsignManagerComponent.Cast(gameMode.FindComponent(SCR_CallsignManagerComponent));
		if (!m_CallsignManager)
			return;
		
		SCR_EditableEntityComponent editableEntityComponent = SCR_EditableEntityComponent.GetEditableEntity(owner);
		if (editableEntityComponent)
			m_Faction = editableEntityComponent.GetFaction();
		m_bIsServer = Replication.IsServer();
		
		if (!m_bIsServer)
			return;
		
		GetGame().GetCallqueue().CallLater(InitOnServer, 1, false, owner);
	}
	
	//---------------------------------------- Post Init ----------------------------------------\\
	override void OnPostInit(IEntity owner)
	{
		owner.SetFlags(EntityFlags.ACTIVE, false);
		SetEventMask(owner, EntityEvent.INIT);
	}
};

