class SCR_PlayerTeleportFeedbackUIComponent: ScriptedWidgetComponent
{
	
	protected SCR_FadeUIComponent m_FadeUIComponent;
	protected Widget m_wRoot;
	
	protected void OnPlayerTeleported(bool editorIsOpen)
	{
		if (editorIsOpen)
			return;
		
		m_FadeUIComponent.CancelFade(false);
		m_wRoot.SetOpacity(1);
		m_wRoot.SetVisible(true);
		m_FadeUIComponent.FadeOut(false);
	}
	
	override void HandlerAttached(Widget w)
	{
		m_wRoot = w;
		m_FadeUIComponent = SCR_FadeUIComponent.Cast(w.FindHandler(SCR_FadeUIComponent));
		
		if (!m_FadeUIComponent)
		{
			Print("'SCR_PlayerTeleportFeedbackUIComponent' could not find 'm_FadeUIComponent', make sure the fadecomponent is added before this component", LogLevel.ERROR);
			return;
		}
			
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!playerController)
			return;
		
		SCR_PlayerTeleportedFeedbackComponent playerTeleportedComponent = SCR_PlayerTeleportedFeedbackComponent.Cast(playerController.FindComponent(SCR_PlayerTeleportedFeedbackComponent));
		if (!playerTeleportedComponent)
		{
			Print("'SCR_PlayerTeleportFeedbackUIComponent' could not find 'SCR_PlayerTeleportedFeedbackComponent', make sure it is added to the player controller", LogLevel.ERROR);
			return;
		}
			
		playerTeleportedComponent.GetOnPlayerTeleportedByEditor().Insert(OnPlayerTeleported);
	}
	override void HandlerDeattached(Widget w)
	{
		if (!m_FadeUIComponent)
			return;
		
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!playerController)
			return;
		
		SCR_PlayerTeleportedFeedbackComponent playerTeleportedComponent = SCR_PlayerTeleportedFeedbackComponent.Cast(playerController.FindComponent(SCR_PlayerTeleportedFeedbackComponent));
		if (!playerTeleportedComponent)
			return;
		
		playerTeleportedComponent.GetOnPlayerTeleportedByEditor().Remove(OnPlayerTeleported);
		
	}
};
