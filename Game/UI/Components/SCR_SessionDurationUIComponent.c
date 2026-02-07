
/**
Subscribes to Event_OnSessionDurationChange within the SCR_SessionDurationComponent script and displays the current Session duration time.
*/	
class SCR_SessionDurationUIComponent : MenuRootSubComponent 
{
	protected TextWidget m_SesionDurationText;
	protected float m_fCurrentSessionDuration;
	
	protected void SetSessionDurationTime(int sessionDuration) 
	{
		m_SesionDurationText.SetText(ReturnSessionTimeFormatting(sessionDuration));		
	}
	
	
	protected string ReturnSessionTimeFormatting(int currentSessionTime)
	{			
		return SCR_Global.GetTimeFormatting(currentSessionTime, ETimeFormatParam.DAYS);
	}
	
	
	//======================== ATTACH/DEATTACH HANDLER ========================\\
	//On Init
	override void HandlerAttachedScripted(Widget w)
	{		
		m_SesionDurationText = TextWidget.Cast(w);

		if (m_SesionDurationText)
		{
			SCR_SessionDurationComponent serverSessionComponent = SCR_SessionDurationComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_SessionDurationComponent));
		
			if (serverSessionComponent)
			{
				SetSessionDurationTime(serverSessionComponent.GetSessionDurationTime());
				serverSessionComponent.GetOnSessionDurationChange().Insert(SetSessionDurationTime);
			}
			else 
			{
				Print("'SCR_SessionDurationComponent' needs to be attached to the 'gamemode' in order to display the durration!", LogLevel.ERROR);
			}
		}
		else 
		{
			Print("'SCR_SessionDurationUIComponent' needs to be attached to a 'TextWidget'!", LogLevel.ERROR);
		}
	}
	
	
	//On Destroy
	override void HandlerDeattached(Widget w)
	{
		if (m_SesionDurationText)
		{
			SCR_SessionDurationComponent serverSessionComponent = SCR_SessionDurationComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_SessionDurationComponent));
		
			if (serverSessionComponent)
			{
				serverSessionComponent.GetOnSessionDurationChange().Remove(SetSessionDurationTime);
			}
		}
	}
};