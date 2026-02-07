[ComponentEditorProps(category: "GameScripted", description: "Counts Session Duration")]
class SCR_SessionDurationComponentClass: ScriptComponentClass
{
};

/**
Broadcasts from server to players the current session duration time.
*/
class SCR_SessionDurationComponent : ScriptComponent
{
	//Reference
	protected BaseWorld m_World;
	
	//Events
	protected ref ScriptInvoker Event_OnSessionDurationChange = new ScriptInvoker();

	[RplProp(condition : RplCondition.None, onRplName : "OnSessionDurationChanged")]
	protected int m_iSessionDuration;

	//------------------------------------------------------------------------------------------------
	//Gets updated every second to keep track of session duration (Server only)
	protected void UpdateSessionDurationAmount()
	{	
		//Get world life time in milliseconds and convert it to seconds
		m_iSessionDuration = (int)(m_World.GetWorldTime() * 0.001);
		Replication.BumpMe();
		
		//Invoke Event_OnSessionDurationChange server only
		OnSessionDurationChanged();
	}
	

	
	//If session duration has been changed
	protected void OnSessionDurationChanged()
	{
		Event_OnSessionDurationChange.Invoke(m_iSessionDuration);
	}
	
	
	/*!
	Used to subscribe to OnSessionDuration Change.
	\return Script invoker is returned to subscribe to.
	*/
	ScriptInvoker GetOnSessionDurationChange()
	{
		return Event_OnSessionDurationChange;
	}
	
	/*!
	Get the current value of the Session duration time.
	\return Current session time duration
	*/
	int GetSessionDurationTime()
	{
		return m_iSessionDuration;
	}


	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{	
		SCR_BaseGameMode gameMode =  SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		
		if (gameMode && gameMode.IsMaster()) 
		{
			m_World = GetGame().GetWorld();
			
			if (m_World)
				GetGame().GetCallqueue().CallLater(UpdateSessionDurationAmount, 1000, true);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_SessionDurationComponent()
	{
		if (m_World)
			GetGame().GetCallqueue().Remove(UpdateSessionDurationAmount);
	}

};
