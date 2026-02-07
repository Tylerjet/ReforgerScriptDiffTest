class DeployMenuSystem : GameSystem
{	
	protected SCR_PlayerDeployMenuHandlerComponent m_LocalMenuHandler;
	protected bool m_bReady = false;

	protected override ESystemPoint GetSystemPoint()
	{
		return ESystemPoint.Frame;
	}

	protected override void OnInit()
	{
		SCR_RespawnSystemComponent rsc = SCR_RespawnSystemComponent.GetInstance();
		Enable(
			!System.IsConsoleApp() && 
			(rsc && rsc.CanOpenDeployMenu())
		);
	}

	void Register(SCR_PlayerDeployMenuHandlerComponent handler)
	{
		if (handler.GetPlayerController() == GetGame().GetPlayerController())
			m_LocalMenuHandler = handler;
	}

	void Unregister(SCR_PlayerDeployMenuHandlerComponent handler)
	{
		if (handler.GetPlayerController() == GetGame().GetPlayerController())
			m_LocalMenuHandler = null;
	}

	void SetReady(bool ready)
	{
		m_bReady = ready;
	}

	override protected void OnUpdate(ESystemPoint point)
	{
		if (!m_LocalMenuHandler)
			return;

		float dt = GetWorld().GetTimeSlice();

		if (!m_LocalMenuHandler.UpdateLoadingPlaceholder(dt))
			return;

		if (!m_bReady)
			return;

		m_LocalMenuHandler.Update(dt);
	}
}