//#define DEBUG_CAREER
//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_DataCollectorModule : Managed
{
	protected ref map<int, TextWidget> StatsVisualization;
	
	//***************************//
	/* OVERRIDDEN BY ALL MODULES */
	//***************************//
	
	//------------------------------------------------------------------------------------------------
	void Execute(IEntity owner, float timeTick)
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
	protected protected void AddInvokers(IEntity player)
	{
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(player)));
		
		if (!playerController)
			return;
		
		playerController.m_OnControlledEntityChanged.Insert(OnControlledEntityChanged);
	}
	
	//------------------------------------------------------------------------------------------------
	protected protected void RemoveInvokers(IEntity player)
	{
		if (!player)
			return;
		
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(player)));
		
		if (!playerController)
			return;
		
		playerController.m_OnControlledEntityChanged.Remove(OnControlledEntityChanged);
	}
	
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
	void OnPlayerConnected(int playerID)
	{}
	
	//------------------------------------------------------------------------------------------------
	protected void OnControlledEntityChanged(IEntity from, IEntity to)
	{
		//The entity changed. If there's no entity now we can assume the player disconnected or it hasn't spawned yet.
		//In this case we do nothing and keep listening to the spawn invoker. Otherwise we add the invokers
		if (to)
			AddInvokers(to);
		
		//Now for the previous entity, if it still exists we remove the invokers from it
		if (from)
			RemoveInvokers(from);
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
		StatsVisualization = new map<int, TextWidget>();
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
		StatsVisualization.Insert(id, StatValue);
	}
};
