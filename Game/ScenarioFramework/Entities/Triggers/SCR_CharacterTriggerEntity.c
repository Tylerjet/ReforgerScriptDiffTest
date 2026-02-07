[EntityEditorProps(category: "GameScripted/Triggers", description: "")]
class SCR_CharacterTriggerEntityClass: SCR_BaseTriggerEntityClass
{
};

enum TA_EActivationPresence
{
	PLAYER = 0,
	ALL_PLAYERS,
	ANY_CHARACTER,
	SPECIFIC_CLASS,
	SPECIFIC_PREFAB_NAME,
};

class SCR_CharacterTriggerEntity : SCR_BaseTriggerEntity
{
	[Attribute(desc: "Faction which is used for area control calculation. Leave empty for any faction.", category: "Trigger")]
	protected FactionKey 		m_sOwnerFactionKey;
	
	[Attribute( "0", UIWidgets.ComboBox, "By whom the trigger is activated", "", ParamEnumArray.FromEnum( TA_EActivationPresence ), category: "Trigger") ]
	protected TA_EActivationPresence	m_EActivationPresence;
	
	[Attribute(desc: "If SPECIFIC_CLASS is selected, fill the class name here.", category: "Trigger")]	//TODO: do array of classes
	protected string 	m_sSpecificClassName;
	
	[Attribute(desc: "If SPECIFIC_PREFAB_NAME is selected, fill the class name here.", category: "Trigger")]	//TODO: do array of classes
	protected ResourceName 	m_sSpecificPrefabName;
	
	[Attribute( defvalue: "1", UIWidgets.CheckBox, desc: "Activate the trigger once or everytime the activation condition is true?", category: "Trigger")];
	protected bool 							m_bOnce;
	
	protected ref ScriptInvoker m_OnChange;
	
	protected Faction 			m_OwnerFaction;
	protected int 				m_iEntitiesInside = 0;
	protected bool				m_bInitSequenceDone = false;
	protected ref array<IEntity> 	m_aEntitiesInside = {};
	
	//------------------------------------------------------------------------------------------------
	int GetCountEntitiesInside() { return m_iEntitiesInside; }		
	
	//------------------------------------------------------------------------------------------------
	void SetActivationPresence( TA_EActivationPresence EActivationPresence = TA_EActivationPresence.ANY_CHARACTER )
	{
		m_EActivationPresence = EActivationPresence;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetSpecificClass( string sClassName )
	{
		m_sSpecificClassName = sClassName;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetSpecificPrefabName( string sPrefabName )
	{
		m_sSpecificPrefabName = sPrefabName;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetOnce( bool bOnce )
	{
		m_bOnce = bOnce;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetOwnerFaction( FactionKey sFaction )
	{
		FactionManager factionManager = GetGame().GetFactionManager();
		if (factionManager)
			m_OwnerFaction = factionManager.GetFactionByKey( sFaction );
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsMaster()// IsServer
	{
		RplComponent comp = RplComponent.Cast(FindComponent(RplComponent));
		return comp && comp.IsMaster();
	}
	
	//------------------------------------------------------------------------------------------------
	protected int GetPlayersCountByFaction()
	{
		int iCnt = 0;
		array<int> aPlayerIDs = {};
		SCR_PlayerController pPlayerCtrl;
		GetGame().GetPlayerManager().GetPlayers( aPlayerIDs );
		foreach ( int iPlayerID: aPlayerIDs )
		{
			if ( !m_OwnerFaction ) 
			{
				iCnt++;			//Faction not set == ANY faction
			}
			else
			{
				pPlayerCtrl = SCR_PlayerController.Cast( GetGame().GetPlayerManager().GetPlayerController( iPlayerID ) );
				if ( !pPlayerCtrl )
					continue;
				if ( pPlayerCtrl.GetLocalControlledEntityFaction() == m_OwnerFaction )
					iCnt++;
			}
		}
		return iCnt;
	}
	
	//------------------------------------------------------------------------------------------------
	protected int GetPlayersCountByFactionInsideTrigger()
	{
		int iCnt = 0;
		SCR_PlayerController pPlayerCtrl;
		GetEntitiesInside(m_aEntitiesInside);
		foreach ( IEntity entity: m_aEntitiesInside )
		{
			if ( !m_OwnerFaction ) 
			{
				iCnt++;			//Faction not set == ANY faction
			}
			else
			{
				pPlayerCtrl = SCR_PlayerController.Cast(entity);
				if ( !pPlayerCtrl )
					continue;
				if ( pPlayerCtrl.GetLocalControlledEntityFaction() == m_OwnerFaction )
					iCnt++;
			}
		}
		return iCnt;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void GetPlayersByFaction( notnull out array<IEntity> aOut )
	{
		array<int> aPlayerIDs = {};
		SCR_PlayerController pPlayerCtrl;
		GetGame().GetPlayerManager().GetPlayers( aPlayerIDs );
		foreach ( int iPlayerID: aPlayerIDs )
		{
			pPlayerCtrl = SCR_PlayerController.Cast( GetGame().GetPlayerManager().GetPlayerController( iPlayerID ) );
			if ( !pPlayerCtrl )
				continue;
			if ( pPlayerCtrl.GetLocalControlledEntityFaction() == m_OwnerFaction )
				aOut.Insert( pPlayerCtrl.GetLocalMainEntity() );
		}
	}
	
	
	//------------------------------------------------------------------------------------------------
	//! Override this method in inherited class to define a new filter.
	override bool ScriptedEntityFilterForQuery( IEntity ent )
	{
 		if ( ChimeraCharacter.Cast( ent ) && !IsAlive( ent ) )
			return false;
		
		// take only players
		if ( m_EActivationPresence == TA_EActivationPresence.ALL_PLAYERS || m_EActivationPresence == TA_EActivationPresence.PLAYER )
		{
			if ( !EntityUtils.IsPlayer( ent ) )
				return false;
		}
		else if ( m_EActivationPresence == TA_EActivationPresence.SPECIFIC_CLASS )
		{
			if ( !ent.IsInherited( m_sSpecificClassName.ToType() ) )
				return false;
		}
			
		else if ( m_EActivationPresence == TA_EActivationPresence.SPECIFIC_PREFAB_NAME )
		{
			EntityPrefabData pPrefabData = ent.GetPrefabData();
			if ( !pPrefabData )
				return false;
			ResourceName sPrefabName = pPrefabData.GetPrefabName();
			if ( sPrefabName.IsEmpty() || sPrefabName != m_sSpecificPrefabName )
				return false;
		}
				
		if ( !m_OwnerFaction )
			return true;	//if faction is not specified, any faction can (de)activate the trigger
		FactionAffiliationComponent pFaciliation = FactionAffiliationComponent.Cast( ent.FindComponent( FactionAffiliationComponent ) );
		if ( !pFaciliation )
			return false;
		return pFaciliation.GetAffiliatedFaction() == m_OwnerFaction;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected event void OnActivate(IEntity ent)
	{
		if ( !IsMaster() )
			return;
		
		if ( m_EActivationPresence == TA_EActivationPresence.ALL_PLAYERS )
		{
			if ( GetPlayersCountByFactionInsideTrigger() == GetPlayersCountByFaction() )
			{
				if ( m_bInitSequenceDone )
				{
					if ( m_bOnce )
						Deactivate();
					m_OnActivate.Invoke(ent);
					OnChange( ent );
				}
			}
		}
		else if ( 	m_EActivationPresence == TA_EActivationPresence.PLAYER || 
					m_EActivationPresence == TA_EActivationPresence.ANY_CHARACTER || 
					m_EActivationPresence == TA_EActivationPresence.SPECIFIC_CLASS ||
					m_EActivationPresence == TA_EActivationPresence.SPECIFIC_PREFAB_NAME 
				)
		{
			if ( m_bInitSequenceDone )
			{
				if ( m_bOnce )
					Deactivate();
				m_OnActivate.Invoke(ent);
				OnChange( ent );
			}
		}
		//OnChange( ent );
	}
	
	//------------------------------------------------------------------------------------------------
	override protected event void OnDeactivate(IEntity ent)
	{
		m_iEntitiesInside--;
		
		if ( 	m_EActivationPresence == TA_EActivationPresence.PLAYER || 
				m_EActivationPresence == TA_EActivationPresence.ANY_CHARACTER || 
				m_EActivationPresence == TA_EActivationPresence.SPECIFIC_CLASS ||
				m_EActivationPresence == TA_EActivationPresence.SPECIFIC_PREFAB_NAME 
		   )
		{
			if ( m_bInitSequenceDone )
			{
				m_OnDeactivate.Invoke();
				OnChange( ent );
			}
		}
		
	}
	
	//------------------------------------------------------------------------------------------------
	void OnChange( IEntity ent )
	{
		if ( m_OnChange )
		{
			CP_Param<IEntity> param = new CP_Param<IEntity>( ent );
			m_OnChange.Invoke( param );
		}
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnChange()
	{
		if ( !m_OnChange )
			m_OnChange = new ScriptInvoker();
		return m_OnChange;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetInitSequenceDone()
	{
		m_bInitSequenceDone = true;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void EOnInit(IEntity owner)
	{
		//TODO: we need a time to spawn entities inside the trigger, but we don't want to activate the trigger yet. 
		//It will be done better by knowing the entities inside the trigger on its creation
		GetGame().GetCallqueue().CallLater( SetInitSequenceDone, 1000 );	
		SetOwnerFaction( m_sOwnerFactionKey );
	}
};
