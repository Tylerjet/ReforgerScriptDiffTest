class SCR_PlayableFactionAttributeUIComponent : SCR_CheckboxEditorAttributeUIComponent
{
	[Attribute("0.5", "In seconds")]
	protected float m_fPlayerToBeKilledUpdateTime;
	
	[Attribute("")]
	protected LocalizedString m_sPlayersToBeKilledWarningText;
	
	protected SCR_RespawnSystemComponent m_RespawnSystemComponent;
	protected SCR_PlayerDelegateEditorComponent m_PlayerDelegateManager;
	
	protected Faction m_faction;
	protected bool m_bStartIsPlayable;
	protected bool m_bFactionIsPlayable;
	protected int m_iPlayersWillBeKilled = -1;
	
	override void Init(Widget w, SCR_BaseEditorAttribute attribute)
	{
		super.Init(w, attribute);
		
		m_RespawnSystemComponent = SCR_RespawnSystemComponent.GetInstance();
		m_PlayerDelegateManager = SCR_PlayerDelegateEditorComponent.Cast(SCR_PlayerDelegateEditorComponent.GetInstance(SCR_PlayerDelegateEditorComponent, true));
	}
	
	override void SetFromVar(SCR_BaseEditorAttributeVar var)
	{	
		super.SetFromVar(var);
		
		if (!var)
			return;
		
		m_bStartIsPlayable = var.GetBool();
		m_bFactionIsPlayable = m_bStartIsPlayable;
		
		if (!m_faction)
		{
			vector value = var.GetVector();
			
			FactionManager factionManager = GetGame().GetFactionManager();
			if (!factionManager)
				return;
			
			m_faction = factionManager.GetFactionByIndex((int)value[1]);
		}
	}
	
	override bool OnChange(Widget w, int x, int y, bool finished)
	{
		//Only if start value is true
		if (!m_bStartIsPlayable || !m_faction)
			return super.OnChange(w, x, y, finished);
		
		m_bFactionIsPlayable = x;
		
		if (m_bFactionIsPlayable)
		{
			GetGame().GetCallqueue().Remove(UpdateTotalToBeKilledPlayers);
		}
		else 
		{
			GetGame().GetCallqueue().CallLater(UpdateTotalToBeKilledPlayers, m_fPlayerToBeKilledUpdateTime * 1000, true);
		}
		
		UpdateTotalToBeKilledPlayers();
		
		return super.OnChange(w, x, y, finished);
	}
	
	protected void UpdateTotalToBeKilledPlayers()
	{
		if (m_bFactionIsPlayable)
		{
			if (m_iPlayersWillBeKilled != -1)
			{
				m_iPlayersWillBeKilled = -1;
				OverrideDescription(false);
			}
			return;
		}
		
		int playerCountToBeKilled = 0;
		
		if (!m_RespawnSystemComponent)
				return;
			
		array<int> players = new array<int>;
		GetGame().GetPlayerManager().GetPlayers(players);
		SCR_EditablePlayerDelegateComponent playerEditorDelegate;
		
		foreach(int playerId: players)
		{
			//If has player entity
			if(!SCR_PossessingManagerComponent.GetPlayerMainEntity(playerId))
				continue;
			
			Faction playerFaction = m_RespawnSystemComponent.GetPlayerFaction(playerId);
			
			if (playerFaction != m_faction)
				continue;
			
			//Ignore GM
			if (m_PlayerDelegateManager)
			{
				playerEditorDelegate = m_PlayerDelegateManager.GetDelegate(playerId);
				
				if (playerEditorDelegate)
				{
					if (!playerEditorDelegate.HasLimitedEditor())
						continue;
				}
			}
			
			playerCountToBeKilled++;
		}
		
		if (m_iPlayersWillBeKilled == playerCountToBeKilled)
			return;
			
		m_iPlayersWillBeKilled = playerCountToBeKilled;
		
		if (m_iPlayersWillBeKilled != 0)
			OverrideDescription(true, m_sPlayersToBeKilledWarningText, m_iPlayersWillBeKilled.ToString());
		else 
			OverrideDescription(false);
	}
	
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		
		if (m_faction && m_bStartIsPlayable && !m_bFactionIsPlayable)
			GetGame().GetCallqueue().Remove(UpdateTotalToBeKilledPlayers);
	}
};