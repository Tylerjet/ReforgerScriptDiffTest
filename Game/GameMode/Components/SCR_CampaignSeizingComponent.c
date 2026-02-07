//------------------------------------------------------------------------------------------------
class SCR_CampaignSeizingComponentClass : SCR_SeizingComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignSeizingComponent : SCR_SeizingComponent
{
	protected bool m_bRefreshUI;
	protected SCR_CampaignBase m_Base;
	
	//------------------------------------------------------------------------------------------------
	protected override SCR_Faction EvaluateEntityFaction(IEntity ent)
	{
		SCR_Faction faction = super.EvaluateEntityFaction(ent);
		
		if (!faction)
			return null;
		
		// Players of faction not covering this base with radio signal should not be able to capture or prevent capture
		SCR_CampaignFaction cFaction = SCR_CampaignFaction.Cast(faction);
		
		if (!cFaction)
			return null;
		
		if (faction.IsPlayable() && !m_Base.IsBaseInFactionRadioSignal(cFaction))
			return null;
		
		return faction;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnPrevailingFactionChanged()
	{
		super.OnPrevailingFactionChanged();
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnCaptureStart()
	{
		SCR_CampaignFaction factionC = SCR_CampaignFaction.Cast(m_PrevailingFaction);
		
		if (!factionC)
			return;
		
		m_Base.BeginCapture(factionC);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnCaptureInterrupt()
	{
		m_Base.EndCapture();
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnCaptureFinish()
	{
		m_Base.FinishCapture();
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnSeizingEndTimestampChanged()
	{
		if (!System.IsConsoleApp())
			SetRefreshUI(true);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetRefreshUI(bool refresh)
	{
		m_bRefreshUI = refresh;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetRefreshUI()
	{
		return m_bRefreshUI;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		m_Base = SCR_CampaignBase.Cast(GetOwner());
		
		if (!m_Base)
		{
			Print("SCR_CampaignSeizingComponent: Parent base not found! Terminating...", LogLevel.ERROR);
			return;
		}
		
		if (m_Base.GetIsHQ())
			return;
		
		super.OnPostInit(owner);
	}
};
