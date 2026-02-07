[BaseContainerProps()]
class SCR_DataCollectorModule : Managed
{
	protected ref map<int, TextWidget> m_StatsVisualization;
	
	protected float m_fTimeSinceUpdate = 0;
	protected float m_fTimeToUpdate = 1;
	
	//***************************//
	/* OVERRIDDEN BY ALL MODULES */
	//***************************//
	
	//------------------------------------------------------------------------------------------------
	void Update(IEntity owner, float timeTick)
	{}
	
	//***********************************************************//
	/* OVERRIDDEN BY SOME MODULES BUT CALLING THEIR SUPER METHOD */
	//***********************************************************//
	
	//------------------------------------------------------------------------------------------------
	void OnPlayerDisconnected(int playerID, IEntity controlledEntity = null)
	{
		if (!controlledEntity)
			controlledEntity = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID);
		
		if (controlledEntity)
			RemoveInvokers(controlledEntity);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void AddInvokers(IEntity player)
	{}
	
	//------------------------------------------------------------------------------------------------
	protected void RemoveInvokers(IEntity player)
	{}
	
	//------------------------------------------------------------------------------------------------
	void OnPlayerSpawned(int playerID, IEntity controlledEntity)
	{
		AddInvokers(controlledEntity);
	}
	
	//***********************************************************//
	/* OVERRIDDEN BY SOME MODULES BUT CALLING THEIR SUPER METHOD */
	/************ (unless the call is very specific) *************/
	//***********************************************************//
	
	//------------------------------------------------------------------------------------------------
	void OnPlayerKilled(int playerID, IEntity player, IEntity killer)
	{}
	
	//*****************//
	/* NEVER OVERRIDDEN*/
	//*****************//
	
	//------------------------------------------------------------------------------------------------
	sealed void OnPlayerAuditSuccess(int playerID)
	{}
	
	//------------------------------------------------------------------------------------------------
	sealed void OnControlledEntityChanged(IEntity from, IEntity to)
	{
		//The entity changed. 
		
		//The previous entity, if it still exists we remove the invokers from it
		if (from)
			RemoveInvokers(from);
		
		//The new entity. If there's no entity yet we can assume the player disconnected or it hasn't spawned.
		//In this case we do nothing and keep listening to the spawn invoker. Otherwise we add the invokers
		if (to)
			AddInvokers(to);
	}
	
	//************************************************************************//
	/*********************** CREATED ONLY FOR PROTOTYPE ***********************/
	/* These create a textwidget on screen with the stats updated dynamically */
	//************************************************************************//
	//************************************************************************//
	
	//------------------------------------------------------------------------------------------------
	void CreateVisualization()
	{
		#ifdef DEBUG_CAREER
			m_StatsVisualization = new map<int, TextWidget>();
		#endif
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CreateEntry(string text, float value, int id)
	{
		if (!GetGame().GetDataCollector() || !GetGame().GetDataCollector().GetUIComponent())
			return;
		
		Widget entry = GetGame().GetDataCollector().GetUIComponent().CreateEntry();
		
		TextWidget StatName = TextWidget.Cast(entry.FindAnyWidget("StatName"));
		TextWidget StatValue = TextWidget.Cast(entry.FindAnyWidget("StatValue"));
		if (!StatName || !StatValue)
			return;
		
		StatName.SetText(text); StatValue.SetText(value.ToString());
		m_StatsVisualization.Insert(id, StatValue);
	}
};
