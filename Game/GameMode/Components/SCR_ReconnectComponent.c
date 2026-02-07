//------------------------------------------------------------------------------------------------
//! Data class for reconnecting players
class SCR_ReconnectData
{
	int m_iPlayerId;	
	float m_fCounter;			// counter until this instance is deleted
	IEntity m_ReservedEntity;	// entity of the returning player
	
	//------------------------------------------------------------------------------------------------
	void SCR_ReconnectData(int playerId, IEntity entity, float time)
	{
		m_iPlayerId = playerId;
		m_ReservedEntity = entity;
		m_fCounter = time;
		
		// TODO lock server slot
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_ReconnectData()
	{
		// TODO unlock server slot
	}
};

//------------------------------------------------------------------------------------------------
[EntityEditorProps(category: "GameScripted/GameMode", description: "")]
class SCR_ReconnectComponentClass : SCR_BaseGameModeComponentClass
{
	//------------------------------------------------------------------------------------------------
	static override bool DependsOn(string className)
	{
		if (className == "RplComponentClass")
			return true;
		
		return false;
	}
};

//------------------------------------------------------------------------------------------------
//! Takes care of managing player reconnects in case of involuntary disconnect
//! Authority only component attached to gamemode prefab
class SCR_ReconnectComponent : SCR_BaseGameModeComponent
{
	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Enable reconnect functionality for this gamemode")]
	bool m_bEnableReconnect;
	
	[Attribute(defvalue: "120", uiwidget: UIWidgets.EditBox, desc: "How long is the entity held within the world until it is deleted")]
	float m_fReconnectTime;
	
	static SCR_ReconnectComponent s_Instance;
	
	protected bool m_bIsReconEnabled;	
	protected ref array<ref SCR_ReconnectData> m_ReconnectPlayerList = new array<ref SCR_ReconnectData>();
	
	protected ref ScriptInvoker<SCR_ReconnectData> m_OnAddedToReconnectList = new ScriptInvoker();
	protected ref ScriptInvoker<SCR_ReconnectData> m_OnPlayerReconnect = new ScriptInvoker();  
	
	//------------------------------------------------------------------------------------------------
	static SCR_ReconnectComponent GetInstance() 
	{ 
		return s_Instance; 
	}
		
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnAddedToList()
	{ 
		return m_OnAddedToReconnectList; 
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnReconnect() 
	{ 
		return m_OnPlayerReconnect; 
	}
	
	//------------------------------------------------------------------------------------------------
	//! Is reconnect functionality enabled
	bool IsReconnectEnabled() 
	{ 
		return m_bIsReconEnabled; 
	}
	
	//------------------------------------------------------------------------------------------------
	//! Is subject playerID currently present in list of possible reconnects
	//! \param playerId is the subject
	bool IsInReconnectList(int playerId)
	{
		if (m_ReconnectPlayerList.IsEmpty())
			return false;
		
		int count = m_ReconnectPlayerList.Count();
		for (int i; i < count; i++)
		{
			if (m_ReconnectPlayerList[i].m_iPlayerId == playerId)
			{
				ChimeraCharacter char = ChimeraCharacter.Cast(m_ReconnectPlayerList[i].m_ReservedEntity);
				if (!char || char.GetCharacterController().IsDead())	// entity could have died meanwhile
					return false;
				
				return true;
			}
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Return control of the entity subject controlled before disconnect
	//! \param playerId is the subject
	IEntity ReturnControlledEntity(int playerId)
	{		
		int count = m_ReconnectPlayerList.Count();
		for (int i; i < count; i++)
		{
			if (m_ReconnectPlayerList[i].m_iPlayerId == playerId)
			{
				IEntity ent = m_ReconnectPlayerList[i].m_ReservedEntity;
				PlayerManager playerManager = GetGame().GetPlayerManager();
				SCR_PlayerController playerController = SCR_PlayerController.Cast(playerManager.GetPlayerController(playerId));		
				playerController.SetInitialMainEntity(ent);
	
				m_OnPlayerReconnect.Invoke(m_ReconnectPlayerList[i]);
				
				m_ReconnectPlayerList.Remove(i);
				
				return ent;
			}
		}
		
		return null;
	}
			
	//------------------------------------------------------------------------------------------------
	//! Propagated from SCR_BaseGameMode OnPlayerDisconnected event
	//! \return true if this disconnect is saved as eligible for reconnect
	bool OnPlayerDC(int playerId, KickCauseCode cause)
	{
		KickCauseGroup2 groupInt = KickCauseCodeAPI.GetGroup(cause);
		int reasonInt = KickCauseCodeAPI.GetReason(cause);
				
		if (groupInt != RplKickCauseGroup.REPLICATION)
			return false;
		#ifndef RECONNECT_DEBUG
		else if (reasonInt == RplError.SHUTDOWN)
			return false;
		#endif
				
		bool addEntry = true;
		
		if (!m_ReconnectPlayerList.IsEmpty())
		{
			int count = m_ReconnectPlayerList.Count();
			for (int i; i < count; i++)
			{
				if (m_ReconnectPlayerList[i].m_iPlayerId == playerId)
				{
					addEntry = false;
					break;
				}
			}
		}
		
		if (addEntry)
		{
			IEntity ent = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
			if (!ent)
				return false;
			
			SCR_ReconnectData newEntry = new SCR_ReconnectData(playerId, ent, m_fReconnectTime);
			m_ReconnectPlayerList.Insert(newEntry);
			m_OnAddedToReconnectList.Invoke(newEntry);
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{		
		if (m_ReconnectPlayerList.IsEmpty())
			return;
		
		int count = m_ReconnectPlayerList.Count();
		for (int i; i < count; i++)
		{
			m_ReconnectPlayerList[i].m_fCounter -= timeSlice;
			if (m_ReconnectPlayerList[i].m_fCounter < 0)
			{
				RplComponent.DeleteRplEntity(m_ReconnectPlayerList[i].m_ReservedEntity, false);
				m_ReconnectPlayerList.Remove(i);
				count--;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{		
		RplComponent rplComp = RplComponent.Cast(owner.FindComponent(RplComponent));
		if (!rplComp.IsProxy() && m_bEnableReconnect)		// ends here if not authority
			SetEventMask(owner, EntityEvent.INIT);
		else 
			m_bIsReconEnabled = false;
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		#ifdef RECONNECT_DEBUG
		
		s_Instance = this;
		m_bIsReconEnabled = true;
		SetEventMask(owner, EntityEvent.FRAME);
		
		#endif
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_ReconnectComponent()
	{
		s_Instance = null;
	}
};
