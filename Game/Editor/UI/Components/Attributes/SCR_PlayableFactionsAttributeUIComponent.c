/** @ingroup Editor_UI Editor_UI_Components Editor_UI_Attributes
*/
class SCR_PlayableFactionsAttributeUIComponent: SCR_ButtonBoxMultiSelectAttributeUIComponent
{			
	[Attribute("")]
	protected LocalizedString m_sPlayersToBeKilledWarningText;
	
	[Attribute("0.5", "In seconds")]
	protected float m_fPlayerToBeKilledUpdateTime;
	protected int m_iPlayersWillBeKilled = -1;
	
	protected FactionManager m_FactionManager;
	protected SCR_RespawnSystemComponent m_RespawnSystemComponent;
	protected SCR_PlayerDelegateEditorComponent m_PlayerDelegateManager;
	
	protected ref array<Faction> m_aFactionsDisabled = new ref array<Faction>;
	
	
	override void Init(Widget w, SCR_BaseEditorAttribute attribute)
	{
		super.Init(w, attribute);
		
		m_FactionManager = GetGame().GetFactionManager();
		m_RespawnSystemComponent = SCR_RespawnSystemComponent.GetInstance();
		m_PlayerDelegateManager = SCR_PlayerDelegateEditorComponent.Cast(SCR_PlayerDelegateEditorComponent.GetInstance(SCR_PlayerDelegateEditorComponent, true));
	}
	
	override void SetFromVar(SCR_BaseEditorAttributeVar var)
	{
		super.SetFromVar(var);
		
		//Get all disabled buttonts and the faction associated with it and make sure the system adds a Attribute explanation if any players will die on Accept
		int count = m_ButtonBoxData.GetValueCount();
		SCR_Faction scrFaction;
		
		for(int i = 0; i < count; i++)
        {
			if (!m_ToolBoxComponent.IsItemSelected(i))
			{
				Faction f = m_FactionManager.GetFactionByIndex((int)m_ButtonBoxData.GetEntryFloatValue(i));
				
				scrFaction = SCR_Faction.Cast(f);
				
				//Check if currently playable
				if (scrFaction && scrFaction.IsPlayable())
					m_aFactionsDisabled.Insert(f);
			}
        }
		
		if (!m_aFactionsDisabled.IsEmpty())
			GetGame().GetCallqueue().CallLater(UpdateTotalToBeKilledPlayers, m_fPlayerToBeKilledUpdateTime * 1000, true);
		
		UpdateTotalToBeKilledPlayers();
		
	}
	
	//x is index, y is selectState // x == index, y == state
	override bool OnChange(Widget w, int x, int y, bool finished)
	{	
		if (!m_bButtonValueInitCalled)
			return false;
		
		if (m_FactionManager)
		{
			Faction faction = m_FactionManager.GetFactionByIndex((int)m_ButtonBoxData.GetEntryFloatValue(x));
			
			if (y == false)
			{
				m_aFactionsDisabled.Insert(faction);
				
				if (m_aFactionsDisabled.Count() == 1)
					GetGame().GetCallqueue().CallLater(UpdateTotalToBeKilledPlayers, m_fPlayerToBeKilledUpdateTime * 1000, true);
			}
			else
			{
				int index = m_aFactionsDisabled.Find(faction);
				if (index > -1)
				{
					m_aFactionsDisabled.Remove(index);
					
					if (m_aFactionsDisabled.IsEmpty())
						GetGame().GetCallqueue().Remove(UpdateTotalToBeKilledPlayers);
				}
			}
			
			UpdateTotalToBeKilledPlayers();
		}
		
		return super.OnChange(w, x, y, finished);
	}
	
	protected void UpdateTotalToBeKilledPlayers()
	{
		int playerCountToBeKilled = 0;
		
		if (!m_aFactionsDisabled.IsEmpty())
		{
			if (!m_RespawnSystemComponent)
				return;
			
			array<int> players = new array<int>;
			GetGame().GetPlayerManager().GetPlayers(players);
			SCR_EditablePlayerDelegateComponent playerEditorDelegate;
	
			
			foreach (int playerId: players)
			{
				//If has player entity
				if(!SCR_PossessingManagerComponent.GetPlayerMainEntity(playerId))
					continue;
				
				Faction playerFaction = m_RespawnSystemComponent.GetPlayerFaction(playerId);
				
				if (!m_aFactionsDisabled.Contains(playerFaction))
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
		}
		
		if (m_iPlayersWillBeKilled == playerCountToBeKilled)
			return;
		
		m_iPlayersWillBeKilled = playerCountToBeKilled;
		
		if (!m_aFactionsDisabled.IsEmpty() && m_iPlayersWillBeKilled > 0)
			OverrideDescription(true, m_sPlayersToBeKilledWarningText, m_iPlayersWillBeKilled.ToString());
		else 
			OverrideDescription(false);
	}
	
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		
		if (!m_aFactionsDisabled.IsEmpty())
			GetGame().GetCallqueue().Remove(UpdateTotalToBeKilledPlayers);
	}
};