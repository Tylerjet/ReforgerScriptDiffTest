//------------------------------------------------------------------------------------------------
class SCR_CampaignFaction : SCR_Faction
{
	[Attribute("", UIWidgets.ResourceNamePicker, "Defenders group prefab", "et")]
	private ResourceName m_DefendersGroupPrefab;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Default transport vehicle", "et")]
	private ResourceName m_DefaultTransportPrefab;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "", "et")]
	private ResourceName m_MobileHQPrefab;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "For radio operators", "et")]
	private ResourceName m_RadioPrefab;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "HQ composition in small bases", "et")]
	private ResourceName m_BaseBuildingHQ;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Supply stash composition", "et")]
	private ResourceName m_BaseBuildingSupplyDepot;
	
	[Attribute()]
	private ref array<ref SCR_CampaignBaseCallsign> m_aBaseCallsigns;
	
	protected SCR_CampaignMilitaryBaseComponent m_MainBase;
	protected SCR_CampaignMilitaryBaseComponent m_PrimaryTarget;
	
	protected SCR_CampaignMobileAssemblyComponent m_MobileAssembly;
	
	#ifndef AR_CAMPAIGN_TIMESTAMP
	protected float m_fVictoryTimestamp;
	protected float m_fPauseByBlockTimestamp;
	#else
	protected WorldTimestamp m_fVictoryTimestamp;
	protected WorldTimestamp m_fPauseByBlockTimestamp;
	#endif
	
	protected int m_iActiveRespawnRadios;
	protected int m_iControlPointsHeld;
	
	//------------------------------------------------------------------------------------------------
	void SendHQMessage(SCR_ERadioMsg msgType, int baseCallsign = SCR_CampaignMilitaryBaseComponent.INVALID_BASE_CALLSIGN, int calledID = SCR_CampaignMilitaryBaseComponent.INVALID_PLAYER_INDEX, bool public = true, int param = SCR_CampaignRadioMsg.INVALID_RADIO_MSG_PARAM)
	{
		if (msgType == SCR_ERadioMsg.NONE)
			return;
		
		SCR_CampaignMilitaryBaseComponent HQ = GetMainBase();
		
		if (!HQ)
			return;
		
		BaseRadioComponent radio = BaseRadioComponent.Cast(HQ.GetOwner().FindComponent(BaseRadioComponent));
		
		if (!radio || !radio.IsPowered())
			return;
		
		BaseTransceiver transmitter = radio.GetTransceiver(0);
		
		if (!transmitter)
			return;
		
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		
		if (!campaign)
			return;
		
		SCR_CallsignManagerComponent callsignManager = SCR_CallsignManagerComponent.Cast(campaign.FindComponent(SCR_CallsignManagerComponent));
		
		if (!callsignManager)
			return;
		
		IEntity called = GetGame().GetPlayerManager().GetPlayerControlledEntity(calledID);
		int companyCallsignIndex, platoonCallsignIndex, squadCallsignIndex, characterCallsignIndex;
		
		if (called && !callsignManager.GetEntityCallsignIndexes(called, companyCallsignIndex, platoonCallsignIndex, squadCallsignIndex, characterCallsignIndex))
	    	return;
		
		SCR_CampaignRadioMsg msg = new SCR_CampaignRadioMsg;
		msg.SetRadioMsg(msgType);
		msg.SetFactionId(GetGame().GetFactionManager().GetFactionIndex(this));
		msg.SetBaseCallsign(baseCallsign);
		msg.SetCalledCallsign(companyCallsignIndex, platoonCallsignIndex, squadCallsignIndex);
		msg.SetIsPublic(public);
		msg.SetParam(param);
		msg.SetPlayerID(calledID);
		msg.SetEncryptionKey(radio.GetEncryptionKey());

		transmitter.BeginTransmission(msg);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetControlPointsHeld(int count)
	{
		m_iControlPointsHeld = count;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetControlPointsHeld()
	{
		return m_iControlPointsHeld;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetActiveRespawnRadios(int count)
	{
		m_iActiveRespawnRadios = count;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetActiveRespawnRadios()
	{
		return m_iActiveRespawnRadios;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetMainBase(SCR_CampaignMilitaryBaseComponent mainBase)
	{
		m_MainBase = mainBase;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetPrimaryTarget(SCR_CampaignMilitaryBaseComponent target)
	{
		m_PrimaryTarget = target;
		SCR_BaseTaskManager taskManager = GetTaskManager();

		if (!taskManager)
			return;
		
		SCR_CampaignMilitaryBaseManager baseManager = SCR_GameModeCampaign.GetInstance().GetBaseManager();

		if (!baseManager)
			return;

		array<SCR_BaseTask> tasks = {};
		taskManager.GetFilteredTasks(tasks, this);

		foreach (SCR_BaseTask task : tasks)
		{
			SCR_CampaignBaseTask conflictTask = SCR_CampaignBaseTask.Cast(task);

			if (!conflictTask)
				continue;

			SCR_CampaignMilitaryBaseComponent base = conflictTask.GetTargetBase();

			if (!base || base.GetFaction() == conflictTask.GetTargetFaction())
				continue;

			conflictTask.SetIsPriority(m_PrimaryTarget == base);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_CampaignMilitaryBaseComponent GetPrimaryTarget()
	{
		return m_PrimaryTarget;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetMobileAssembly(SCR_CampaignMobileAssemblyComponent mobileAssembly)
	{
		m_MobileAssembly = mobileAssembly;
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetRadioPrefab()
	{
		return m_RadioPrefab;
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetDefendersGroupPrefab()
	{
		return m_DefendersGroupPrefab;
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetDefaultTransportPrefab()
	{
		return m_DefaultTransportPrefab;
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetMobileHQPrefab()
	{
		return m_MobileHQPrefab;
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetBuildingPrefab(EEditableEntityLabel type)
	{
		switch (type)
		{
			case EEditableEntityLabel.SERVICE_HQ: {return m_BaseBuildingHQ;};
			case EEditableEntityLabel.SERVICE_SUPPLY_STORAGE: {return m_BaseBuildingSupplyDepot;};
		}
		
		return ResourceName.Empty;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetFactionNameUpperCase()
	{
		SCR_FactionUIInfo UI = SCR_FactionUIInfo.Cast(GetUIInfo());
		
		if (UI)
			return UI.GetFactionNameUpperCase();
		else
			return "";
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_CampaignMilitaryBaseComponent GetMainBase()
	{
		return m_MainBase;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_CampaignMobileAssemblyComponent GetMobileAssembly()
	{
		return m_MobileAssembly;
	}
	
	//------------------------------------------------------------------------------------------------
	#ifndef AR_CAMPAIGN_TIMESTAMP
	void SetVictoryTimestamp(float timestamp)
	#else
	void SetVictoryTimestamp(WorldTimestamp timestamp)
	#endif
	{
		m_fVictoryTimestamp = timestamp;
	}
	
	//------------------------------------------------------------------------------------------------
	#ifndef AR_CAMPAIGN_TIMESTAMP
	float GetVictoryTimestamp()
	#else
	WorldTimestamp GetVictoryTimestamp()
	#endif
	{
		return m_fVictoryTimestamp;
	}
	
	//------------------------------------------------------------------------------------------------
	#ifndef AR_CAMPAIGN_TIMESTAMP
	void SetPauseByBlockTimestamp(float timestamp)
	#else
	void SetPauseByBlockTimestamp(WorldTimestamp timestamp)
	#endif
	{
		m_fPauseByBlockTimestamp = timestamp;
	}
	
	//------------------------------------------------------------------------------------------------
	#ifndef AR_CAMPAIGN_TIMESTAMP
	float GetPauseByBlockTimestamp()
	#else
	WorldTimestamp GetPauseByBlockTimestamp()
	#endif
	{
		return m_fPauseByBlockTimestamp;
	}
	
	//------------------------------------------------------------------------------------------------
	array<int> GetBaseCallsignIndexes()
	{
		array<int> indexes = {};
		
		foreach (SCR_CampaignBaseCallsign callsign : m_aBaseCallsigns)
			indexes.Insert(callsign.GetSignalIndex());
		
		return indexes;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_CampaignBaseCallsign GetBaseCallsignByIndex(int index, int offset = 0)
	{
		index += offset;
		
		if (m_aBaseCallsigns.IsIndexValid(index))
			return m_aBaseCallsigns[index];
		
		index -= m_aBaseCallsigns.Count();
		
		if (m_aBaseCallsigns.IsIndexValid(index))
			return m_aBaseCallsigns[index];
		
		return null;
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_BaseContainerCustomTitleResourceName("m_sCallsign", true)]
class SCR_CampaignBaseCallsign
{
	[Attribute("", UIWidgets.EditBox)]
	protected string m_sCallsign;
	
	[Attribute("", UIWidgets.EditBox)]
	protected string m_sCallsignShort;
	
	[Attribute("", UIWidgets.EditBox)]
	protected string m_sCallsignUpperCase;
	
	[Attribute("0", UIWidgets.EditBox)]
	protected int m_iSignalIndex;
	
	//------------------------------------------------------------------------------------------------
	string GetCallsign()
	{
		return m_sCallsign;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetCallsignShort()
	{
		return m_sCallsignShort;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetCallsignUpperCase()
	{
		return m_sCallsignUpperCase;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetSignalIndex()
	{
		return m_iSignalIndex;
	}
}