class SCR_AIDebugVisualizationClass: GenericEntityClass
{
};

/*!
Entity which performs various AI debugging visualizations.
*/

class SCR_AIDebugVisualization : GenericEntity
{
	static SCR_AIDebugVisualization s_Instance;
	protected ref array<ref SCR_AIMessageVisualization> m_aElements = {};
	protected ref array<ref SCR_AIAgentDebugPanel> m_aPanels = {};
	
	//------------------------------------------------------------------------------------------------
	static void Init()
	{
		if (s_Instance)
			SCR_EntityHelper.DeleteEntityAndChildren(s_Instance);
		
		s_Instance = SCR_AIDebugVisualization.Cast(GetGame().SpawnEntity(SCR_AIDebugVisualization, GetGame().GetWorld()));
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_AIDebugVisualization GetInstance()
	{
		if (!s_Instance)
			s_Instance = SCR_AIDebugVisualization.Cast(GetGame().SpawnEntity(SCR_AIDebugVisualization, GetGame().GetWorld()));
		
		return s_Instance;
	}
	
	//------------------------------------------------------------------------------------------------
	static void VisualizeMessage(IEntity entity, string message, EAIDebugCategory category, float showTime, Color color = Color.White, float fontSize = 16, bool ignoreCategory = false)
	{
		int i =  DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_AI_DEBUG_CATEGORY);
		if (category != DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_AI_DEBUG_CATEGORY) && !ignoreCategory)
			return;
		
		SCR_AIDebugVisualization inst = GetInstance();
		
		if (!inst || !entity || message == string.Empty)
			return;
		
		if (!color)
			color = Color.White;

		inst.RemoveVisualization(entity);
		
		// Create element
		SCR_AIMessageVisualization visualization = new SCR_AIMessageVisualization(entity, message, showTime, color, fontSize);
		inst.m_aElements.Insert(visualization);
	}
	
	//------------------------------------------------------------------------------------------------
	void ShowAiAgentDebugPanel(AIAgent agent)
	{
		SCR_AIAgentDebugPanel existingPanel;
		foreach (SCR_AIAgentDebugPanel p : m_aPanels)
		{
			if (p.m_Agent == agent)
				existingPanel = p;
		}
		
		// Do nothing if this agent already has a panel
		if (existingPanel)
			return;
		
		SCR_AIAgentDebugPanel newPanel = new SCR_AIAgentDebugPanel(agent);
		m_aPanels.Insert(newPanel);
	}

	//------------------------------------------------------------------------------------------------	
	protected void RemoveVisualization(IEntity entity)
	{
		foreach (SCR_AIMessageVisualization vis : m_aElements)
		{
			if (!vis)
				continue;
			
			if (vis.m_TargetEntity == entity)
			{
				m_aElements.RemoveItem(vis);
				vis = null;
				return;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void EOnFrame(IEntity owner, float timeSlice)
	{
		// Update messages above AIs
		bool enabled = DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_AI_DEBUG_CATEGORY);
		if (enabled)
		{
			int count = m_aElements.Count();
			for (int i = count - 1; i >=0; i--)
			{
				bool finished = m_aElements[i].Draw(timeSlice);
				if (finished)
					m_aElements.Remove(i);
			}
		}
		
		// Update AI debug panels
		bool openDebugPanel = DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AI_OPEN_DEBUG_PANEL);
		if (openDebugPanel)
		{
			AIAgent selectedAgent = GetSelectedAiAgent();
			if (selectedAgent)
				ShowAiAgentDebugPanel(selectedAgent);
			else
				Print("Nothing is selected! You must select some AI with Game Master first!", LogLevel.ERROR);
			
			DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_AI_OPEN_DEBUG_PANEL, false);
		}
		
		int count = m_aPanels.Count();
		for (int i = count - 1; i >= 0; i--)
		{
			bool requestClose = m_aPanels[i].Update(timeSlice);
			if (requestClose)
				m_aPanels.Remove(i);
		}
	}

	//-------------------------------------------------------------------------------------------
	protected AIAgent GetSelectedAiAgent()
	{
		// Get first AI entity selected in Game Master
		SCR_BaseEditableEntityFilter filter = SCR_BaseEditableEntityFilter.GetInstance(EEditableEntityState.SELECTED);
		set<SCR_EditableEntityComponent> selectedEntities = new set<SCR_EditableEntityComponent>();
		filter.GetEntities(selectedEntities);
		
		if (selectedEntities.Count() == 0)
			return null;

		// Try to find a group
		foreach (SCR_EditableEntityComponent comp : selectedEntities)
		{
			SCR_EditableGroupComponent editGroupComp = SCR_EditableGroupComponent.Cast(comp);	 
			if (editGroupComp)
				return AIAgent.Cast(editGroupComp.GetOwner());
		}
		
		// Group not found, select first unit
		SCR_EditableCharacterComponent editCharacterComp = SCR_EditableCharacterComponent.Cast(selectedEntities[0]);
		
		if (editCharacterComp)
			return editCharacterComp.GetAgent();
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_AIDebugVisualization(IEntitySource src, IEntity parent)
	{
		s_Instance = this;
		
		SetEventMask(EntityEvent.FRAME);
		SetFlags(EntityFlags.ACTIVE, true);
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_AIDebugVisualization()
	{
		if (m_aElements)
			m_aElements.Clear();
		m_aElements = null;
	}
};
