[EntityEditorProps(category: "GameScripted/Triggers", description: "")]
class SCR_CharacterTriggerEntityClass: SCR_BaseTriggerEntityClass
{
};

enum TA_EActivationPresence
{
	PLAYER = 0,
	ANY_CHARACTER,
	SPECIFIC_CLASS,
	SPECIFIC_PREFAB_NAME,
};

class SCR_CharacterTriggerEntity : SCR_BaseTriggerEntity
{
	[Attribute(desc: "Faction which is used for area control calculation. Leave empty for any faction.", category: "Trigger")]
	protected FactionKey 		m_sOwnerFactionKey;
	
	[Attribute("0", UIWidgets.ComboBox, "By whom the trigger is activated", "", ParamEnumArray.FromEnum(TA_EActivationPresence), category: "Trigger") ]
	protected TA_EActivationPresence	m_EActivationPresence;
	
	[Attribute(desc: "If SPECIFIC_CLASS is selected, fill the class name here.", category: "Trigger")]	//TODO: do array of classes
	protected string 	m_sSpecificClassName;
	
	[Attribute(desc: "If SPECIFIC_PREFAB_NAME is selected, fill the class name here.", category: "Trigger")]	//TODO: do array of classes
	protected ResourceName 	m_sSpecificPrefabName;
	
	[Attribute(defvalue: "1", UIWidgets.CheckBox, desc: "Activate the trigger once or everytime the activation condition is true?", category: "Trigger")];
	protected bool		m_bOnce;
	
	[Attribute(defvalue: "0", UIWidgets.CheckBox, desc: "Whether or not the notification is allowed to be displayed", category: "Trigger")]
	protected bool		m_bNotificationEnabled;
	
	[Attribute(defvalue: "0", UIWidgets.Slider, desc: "Minimm players needed to activate this trigger when PLAYER Activation presence is selected", params: "0 1 0.01", precision: 2, category: "Trigger")]
	protected float		m_fMinimumPlayersNeededPercentage;
	
	[Attribute(desc: "Notification title text that will be displayed when the PLAYER Activation presence is selected", category: "Trigger")]
	protected string 	m_sPlayerActivationNotificationTitle;
	
	[Attribute(desc: "Notification subtitle text that will be displayed when the PLAYER Activation presence is selected", category: "Trigger")]
	protected string 	m_sPlayerActivationNotificationSubtitle;
	
	[Attribute(defvalue: "0", UIWidgets.Slider, desc: "For how long the trigger conditions must be true in order for the trigger to activate. If conditions become false, timer resets", params: "0 86400 1", category: "Trigger")]
	protected float 	m_iActivationCountdownTimer;
	
	[Attribute(desc: "Notification text that will be displayed when Activation Countdown Timer is set to value higher than 0", category: "Trigger")]
	protected string 	m_sActivationCountdownTimerNotification;
	
	[Attribute(defvalue: "0", UIWidgets.CheckBox, desc: "Whether or not the audio sound is played and affected by the trigger", category: "Trigger")]
	protected bool		m_bEnableAudio;
	
	[Attribute("", UIWidgets.EditBox, desc: "Audio sound that will be playing when countdown is active.", category: "Trigger")]
	protected string 	m_sCountdownAudio;
	
	protected ref ScriptInvoker 	m_OnChange;
	
	protected Faction 				m_OwnerFaction;
	protected int 					tempWaitTime = m_iActivationCountdownTimer;
	protected int					m_iPlayerCountInsideTrigger;
	protected int 					m_iCharCountInsideTrigger;
	protected int 					m_iSpecificClassCountInsideTrigger;
	protected int 					m_iSpecificPrefabCountInsideTrigger;
	protected bool					m_bInitSequenceDone = false;
	protected bool 					m_bCountdownMusicPlaying;
	protected bool 					m_bTriggerCountdownOverlap;
	protected ref array<IEntity> 	m_aEntitiesInside = {};
	protected bool 					bNotificationCanPopUp = true;
	protected MusicManager 			m_MusicManager;
	
	//------------------------------------------------------------------------------------------------
	void SetActivationPresence(TA_EActivationPresence EActivationPresence)
	{
		m_EActivationPresence = EActivationPresence;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetSpecificClass(string sClassName)
	{
		m_sSpecificClassName = sClassName;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetSpecificPrefabName(string sPrefabName)
	{
		m_sSpecificPrefabName = sPrefabName;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetOnce(bool bOnce)
	{
		m_bOnce = bOnce;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetNotificationEnabled(bool notificationEnabled)
	{
		m_bNotificationEnabled = notificationEnabled;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetEnableAudio(bool enableAudio)
	{
		m_bEnableAudio = enableAudio;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetMinimumPlayersNeeded(float minimumPlayersNeededPercentage)
	{
		m_fMinimumPlayersNeededPercentage = minimumPlayersNeededPercentage;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetPlayerActivationNotificationTitle(string sTitle)
	{
		m_sPlayerActivationNotificationTitle  = sTitle;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetPlayerActivationNotificationSubtitle(string sSubtitle)
	{
		m_sPlayerActivationNotificationSubtitle   = sSubtitle;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetActivationCountdownTimer(int activationCountdownTimer)
	{
		m_iActivationCountdownTimer = activationCountdownTimer;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetActivationCountdownTimerNotification(string sTitle)
	{
		m_sActivationCountdownTimerNotification   = sTitle;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetCountdownAudio(string sAudioName)
	{
		m_sCountdownAudio  = sAudioName;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetNotificationCanPopUp()
	{
		bNotificationCanPopUp  = true;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetOwnerFaction(FactionKey sFaction)
	{
		FactionManager factionManager = GetGame().GetFactionManager();
		if (factionManager)
			m_OwnerFaction = factionManager.GetFactionByKey(sFaction);
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsMaster()// IsServer
	{
		RplComponent comp = RplComponent.Cast(FindComponent(RplComponent));
		return comp && comp.IsMaster();
	}
	
	//------------------------------------------------------------------------------------------------
	int GetCountEntitiesInside()
	{
		GetEntitiesInside(m_aEntitiesInside);
		return m_aEntitiesInside.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	protected int GetPlayersCountByFaction()
	{
		int iCnt = 0;
		array<int> aPlayerIDs = {};
		SCR_PlayerController pPlayerCtrl;
		GetGame().GetPlayerManager().GetPlayers(aPlayerIDs);
		foreach (int iPlayerID: aPlayerIDs)
		{
			if (!m_OwnerFaction) 
			{
				iCnt++;			//Faction not set == ANY faction
			}
			else
			{
				pPlayerCtrl = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(iPlayerID));
				if (!pPlayerCtrl)
					continue;
				if (pPlayerCtrl.GetLocalControlledEntityFaction() == m_OwnerFaction)
					iCnt++;
			}
		}
		return iCnt;
	}
	
	//------------------------------------------------------------------------------------------------
	protected int GetSpecificClassCountInsideTrigger()
	{
		int iCnt = 0;
		GetEntitiesInside(m_aEntitiesInside);
		foreach (IEntity entity: m_aEntitiesInside)
		{
			if (entity.IsInherited(m_sSpecificClassName.ToType()))
				iCnt++;
		}
		return iCnt;
	}
	
	//------------------------------------------------------------------------------------------------
	protected int GetSpecificPrefabCountInsideTrigger()
	{
		int iCnt = 0;
		GetEntitiesInside(m_aEntitiesInside);
		foreach (IEntity entity: m_aEntitiesInside)
		{
			EntityPrefabData pPrefabData = entity.GetPrefabData();
			if (!pPrefabData)
				continue;
			
			ResourceName sPrefabName = pPrefabData.GetPrefabName();
			if (sPrefabName.IsEmpty() || sPrefabName != m_sSpecificPrefabName)
				continue;
			
			iCnt++;
		}
		return iCnt;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetCharacterCountByFactionInsideTrigger(Faction faction)
	{
		int iCnt = 0;
		SCR_ChimeraCharacter pChimeraChar;
		GetEntitiesInside(m_aEntitiesInside);
		foreach (IEntity entity: m_aEntitiesInside)
		{
			if (!faction) 
			{
				pChimeraChar = SCR_ChimeraCharacter.Cast(entity);
				if (!pChimeraChar)
					continue;
				
				iCnt++;			//Faction not set == ANY faction
			}
			else
			{
				pChimeraChar = SCR_ChimeraCharacter.Cast(entity);
				if (!pChimeraChar)
					continue;
				
				if (pChimeraChar.GetFaction() == faction)
					iCnt++;
			}
		}
		return iCnt;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetPlayersCountByFactionInsideTrigger(Faction faction)
	{
		int iCnt = 0;
		SCR_PlayerController pPlayerCtrl;
		GetEntitiesInside(m_aEntitiesInside);
		foreach (IEntity entity: m_aEntitiesInside)
		{
			if (!faction) 
			{
				iCnt++;			//Faction not set == ANY faction
			}
			else
			{
				pPlayerCtrl = SCR_PlayerController.Cast(entity);
				if (!pPlayerCtrl)
					continue;
				if (pPlayerCtrl.GetLocalControlledEntityFaction() == faction)
					iCnt++;
			}
		}
		return iCnt;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void GetPlayersByFactionInsideTrigger(notnull out array<IEntity> aOut)
	{
		SCR_ChimeraCharacter pChimeraChar;
		GetEntitiesInside(m_aEntitiesInside);
		foreach (IEntity entity : m_aEntitiesInside)
		{
			//Faction not set == ANY faction
			if (!m_OwnerFaction) 
			{
				pChimeraChar = SCR_ChimeraCharacter.Cast(entity);
				if (!pChimeraChar)
					continue;
				
				if (!EntityUtils.IsPlayer(entity))
					continue;
				
				aOut.Insert(entity)
			}
			else
			{
				
				pChimeraChar = SCR_ChimeraCharacter.Cast(entity);
				if (!pChimeraChar)
					continue;
				
				if (!EntityUtils.IsPlayer(entity))
					continue;
				
				if (pChimeraChar.GetFaction() == m_OwnerFaction)
					aOut.Insert(entity);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void GetPlayersByFaction(notnull out array<IEntity> aOut)
	{
		array<int> aPlayerIDs = {};
		SCR_PlayerController pPlayerCtrl;
		GetGame().GetPlayerManager().GetPlayers(aPlayerIDs);
		foreach (int iPlayerID: aPlayerIDs)
		{
			pPlayerCtrl = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(iPlayerID));
			if (!pPlayerCtrl)
				continue;
			if (pPlayerCtrl.GetLocalControlledEntityFaction() == m_OwnerFaction)
				aOut.Insert(pPlayerCtrl.GetLocalMainEntity());
		}
	}
	
	
	//------------------------------------------------------------------------------------------------
	//! Override this method in inherited class to define a new filter.
	override bool ScriptedEntityFilterForQuery(IEntity ent)
	{
 		if (ChimeraCharacter.Cast(ent) && !IsAlive(ent))
			return false;
		
		// take only players
		if (m_EActivationPresence == TA_EActivationPresence.PLAYER)
		{
			if (!EntityUtils.IsPlayer(ent))
				return false;
		}
		else if (m_EActivationPresence == TA_EActivationPresence.SPECIFIC_CLASS)
		{
			if (!ent.IsInherited(m_sSpecificClassName.ToType()))
				return false;
		}
			
		else if (m_EActivationPresence == TA_EActivationPresence.SPECIFIC_PREFAB_NAME)
		{
			EntityPrefabData pPrefabData = ent.GetPrefabData();
			if (!pPrefabData)
				return false;
			ResourceName sPrefabName = pPrefabData.GetPrefabName();
			if (sPrefabName.IsEmpty() || sPrefabName != m_sSpecificPrefabName)
				return false;
		}
				
		if (!m_OwnerFaction)
			return true;	//if faction is not specified, any faction can (de)activate the trigger
		FactionAffiliationComponent pFaciliation = FactionAffiliationComponent.Cast(ent.FindComponent(FactionAffiliationComponent));
		if (!pFaciliation)
			return false;
		return pFaciliation.GetAffiliatedFaction() == m_OwnerFaction;
	}
	
	//------------------------------------------------------------------------------------------------
	void DecreaseCountdown()
	{
		if (m_bNotificationEnabled)
		{
			string title = "";
			if ((tempWaitTime % 5) == 0)
				title = string.Format(m_sActivationCountdownTimerNotification, tempWaitTime);
								
			if (((tempWaitTime - 1) % 5) == 0)
				title = string.Format(m_sActivationCountdownTimerNotification, (tempWaitTime - 1));
								
			if (tempWaitTime <= 0)
				title = string.Format(m_sActivationCountdownTimerNotification, 0);
								
			if (title != "")
				PopUpMessage(title, -1, "", 1, 0, 0);
		}
		//This is because the trigger is querried every 2 seconds
		tempWaitTime -= 2;
		m_bTriggerCountdownOverlap = false;
	}
	
	//------------------------------------------------------------------------------------------------
	void FinishTrigger(IEntity ent)
	{
		if (m_sCountdownAudio != "" && m_bEnableAudio)
		{
			StopMusic(m_sCountdownAudio);
			m_bCountdownMusicPlaying = false;
		}
		if (m_bOnce)
			Deactivate();
		
		m_OnActivate.Invoke(ent);
		OnChange(ent);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected event void OnActivate(IEntity ent)
	{
		if (!IsMaster())
			return;
		
		if (m_EActivationPresence == TA_EActivationPresence.PLAYER)
		{
			int playersCountByFactionInsideTrigger = GetPlayersCountByFactionInsideTrigger(m_OwnerFaction);
			float minPlayersNeeded = Math.Ceil(GetPlayersCountByFaction() * m_fMinimumPlayersNeededPercentage);
			
			if (!m_bTriggerCountdownOverlap)
			{
				m_iPlayerCountInsideTrigger = playersCountByFactionInsideTrigger;
				m_bTriggerCountdownOverlap = true;
			}
			
			if (playersCountByFactionInsideTrigger >= minPlayersNeeded)
			{
				if (m_bInitSequenceDone)
				{
					if (tempWaitTime <= -1)
					{
						FinishTrigger(ent);
					}
					else
					{
						if (m_iPlayerCountInsideTrigger <= 1)
							DecreaseCountdown();
						else
							m_iPlayerCountInsideTrigger--;
						
						if (!m_bCountdownMusicPlaying && m_bEnableAudio)
						{
							PlayMusic(m_sCountdownAudio);
							m_bCountdownMusicPlaying = true;	
						}
					}	
				}
			}
			else
			{
				tempWaitTime = m_iActivationCountdownTimer;
				PopUpMessage("", 1, "", 1, 0, 0);
				if (m_sCountdownAudio != "" || m_bEnableAudio)
				{
					StopMusic(m_sCountdownAudio);
					m_bCountdownMusicPlaying = false;
				}
				if (m_bNotificationEnabled && playersCountByFactionInsideTrigger > 0 && bNotificationCanPopUp)
				{
					string title = string.Format(m_sPlayerActivationNotificationTitle, playersCountByFactionInsideTrigger, GetPlayersCountByFaction());
					string description;
					if (minPlayersNeeded > 0)
						description = string.Format(m_sPlayerActivationNotificationSubtitle, minPlayersNeeded);
					
					PopUpMessage(title, 4, description, -1, 0, 0);
					bNotificationCanPopUp = false;
					//We do not want to spam the notifications that often, so we set the variable on and off after some time
					GetGame().GetCallqueue().CallLater(SetNotificationCanPopUp, 10000);
				}
			}
		}
		else if (m_EActivationPresence == TA_EActivationPresence.ANY_CHARACTER)
		{
			int countInsideTrigger = GetCharacterCountByFactionInsideTrigger(m_OwnerFaction);
			if (!m_bTriggerCountdownOverlap)
			{
				m_iCharCountInsideTrigger = countInsideTrigger;
				m_bTriggerCountdownOverlap = true;
			}
			
			if (countInsideTrigger > 0)
			{
				if (m_bInitSequenceDone)
				{
					if (tempWaitTime <= -1)
					{
						FinishTrigger(ent);
					}
					else
					{
						if (m_iCharCountInsideTrigger <= 1)
							DecreaseCountdown();
						else
							m_iCharCountInsideTrigger--;
						
						if (!m_bCountdownMusicPlaying && m_bEnableAudio)
						{
							PlayMusic(m_sCountdownAudio);
							m_bCountdownMusicPlaying = true;
						}
					}	
				}
			}
			else
			{
				tempWaitTime = m_iActivationCountdownTimer;
				PopUpMessage("", 1, "", 1, 0, 0);
			}	
		}
		else if (m_EActivationPresence == TA_EActivationPresence.SPECIFIC_CLASS)
		{
			int countInsideTrigger = GetSpecificClassCountInsideTrigger();
			if (!m_bTriggerCountdownOverlap)
			{
				m_iSpecificClassCountInsideTrigger = countInsideTrigger;
				m_bTriggerCountdownOverlap = true;
			}
			if (countInsideTrigger > 0)
			{
				if (m_bInitSequenceDone)
				{
					if (tempWaitTime <= -1)
					{
						FinishTrigger(ent);
					}
					else
					{
						if (m_iSpecificClassCountInsideTrigger <= 1)
							DecreaseCountdown();
						else
							m_iSpecificClassCountInsideTrigger--;
						
						if (!m_bCountdownMusicPlaying && m_bEnableAudio)
						{
							PlayMusic(m_sCountdownAudio);
							m_bCountdownMusicPlaying = true;
						}
					}	
				}
			}
			else
			{
				tempWaitTime = m_iActivationCountdownTimer;
				PopUpMessage("", 1, "", 1, 0, 0);
			}	
		}
		else if (m_EActivationPresence == TA_EActivationPresence.SPECIFIC_PREFAB_NAME)
		{
			int countInsideTrigger = GetSpecificPrefabCountInsideTrigger();
			if (!m_bTriggerCountdownOverlap)
			{
				m_iSpecificPrefabCountInsideTrigger = countInsideTrigger;
				m_bTriggerCountdownOverlap = true;
			}
			if (countInsideTrigger > 0)
			{
				if (m_bInitSequenceDone)
				{
					if (tempWaitTime <= -1)
					{
						FinishTrigger(ent);
					}
					else
					{
						if (m_iSpecificPrefabCountInsideTrigger <= 1)
							DecreaseCountdown();
						else
							m_iSpecificPrefabCountInsideTrigger--;
						
						if (!m_bCountdownMusicPlaying && m_bEnableAudio)
						{
							PlayMusic(m_sCountdownAudio);
							m_bCountdownMusicPlaying = true;
						}
					}	
				}
			}
			else
			{
				tempWaitTime = m_iActivationCountdownTimer;
				PopUpMessage("", 1, "", 1, 0, 0);
			}
		}	
		else
		{
			tempWaitTime = m_iActivationCountdownTimer;
			PopUpMessage("", 1, "", 1, 0, 0);
			if (m_sCountdownAudio != "" || !m_bEnableAudio)
			{
				StopMusic(m_sCountdownAudio);;
				m_bCountdownMusicPlaying = false;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override protected event void OnDeactivate(IEntity ent)
	{
		m_bTriggerCountdownOverlap = false;
		
		if (	m_EActivationPresence == TA_EActivationPresence.PLAYER || 
				m_EActivationPresence == TA_EActivationPresence.ANY_CHARACTER || 
				m_EActivationPresence == TA_EActivationPresence.SPECIFIC_CLASS ||
				m_EActivationPresence == TA_EActivationPresence.SPECIFIC_PREFAB_NAME 
		  )
		{
			if (m_bInitSequenceDone)
			{
				PopUpMessage("", 1, "", 1, 0, 0);
				if (m_sCountdownAudio != "" || !m_bEnableAudio)
				{
					StopMusic(m_sCountdownAudio);
					m_bCountdownMusicPlaying = false;
				}
				
				m_OnDeactivate.Invoke();
				OnChange(ent);
			}
		}
		
	}
	
	//------------------------------------------------------------------------------------------------
	void PlayMusic(string sAudio)
	{
		
		if(!m_MusicManager)
		{
			ChimeraWorld world = GetGame().GetWorld();
			if (world)
				m_MusicManager = world.GetMusicManager();
		}
		
		if (m_MusicManager)
			m_MusicManager.Play(sAudio);
		
		Rpc(RpcDo_PlayMusic, sAudio);
		
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_PlayMusic(string sAudio)
	{
		if(!m_MusicManager)
		{
			ChimeraWorld world = GetGame().GetWorld();
			if (world)
				m_MusicManager = world.GetMusicManager();
		}
		
		if (m_MusicManager)
			m_MusicManager.Play(sAudio);
	}
	
	//------------------------------------------------------------------------------------------------
	void StopMusic(string sAudio)
	{
		if(!m_MusicManager)
		{
			ChimeraWorld world = GetGame().GetWorld();
			if (world)
				m_MusicManager = world.GetMusicManager();
		}
		
		if (m_MusicManager)
			m_MusicManager.Stop(sAudio);
		
		Rpc(RpcDo_StopMusic, sAudio);
		
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_StopMusic(string sAudio)
	{
		if(!m_MusicManager)
		{
			ChimeraWorld world = GetGame().GetWorld();
			if (world)
				m_MusicManager = world.GetMusicManager();
		}
		
		if (m_MusicManager)
			m_MusicManager.Stop(sAudio);
	}
	
	//------------------------------------------------------------------------------------------------
	void PopUpMessage(string sTitle, float fDuration, string sSubtitle, int iPrio, float fProgressStart, float fProgressEnd)
	{
		if (IsMaster())
		{
			SCR_PopUpNotification.GetInstance().HideCurrentMsg();
			SCR_PopUpNotification.GetInstance().PopupMsg(text: sTitle, duration: fDuration, text2: sSubtitle, prio: iPrio, progressStart: fProgressStart, progressEnd: fProgressEnd);
		}
		
		Rpc(RpcDo_PopUpMessage, sTitle, fDuration, sSubtitle, iPrio, fProgressStart, fProgressEnd);
		
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_PopUpMessage(string sTitle, float fDuration, string sSubtitle, int iPrio, float fProgressStart, float fProgressEnd)
	{
		SCR_PopUpNotification.GetInstance().HideCurrentMsg();
		SCR_PopUpNotification.GetInstance().PopupMsg(text: sTitle, duration: fDuration, text2: sSubtitle, prio: iPrio, progressStart: fProgressStart, progressEnd: fProgressEnd);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnChange(IEntity ent)
	{
		if (m_OnChange)
		{
			CP_Param<IEntity> param = new CP_Param<IEntity>(ent);
			m_OnChange.Invoke(param);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnChange()
	{
		if (!m_OnChange)
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
		GetGame().GetCallqueue().CallLater(SetInitSequenceDone, 1000);	
		SetOwnerFaction(m_sOwnerFactionKey);
		
		ChimeraWorld world = GetGame().GetWorld();
		if (world)
			m_MusicManager = world.GetMusicManager();
	}
};
