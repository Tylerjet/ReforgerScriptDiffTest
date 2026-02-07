
/**
Subscribes to Event_OnSessionDurationChange within the SCR_SessionDurationComponent script and displays the current Session duration time.
*/	
class SCR_SessionDurationUIComponent : MenuRootSubComponent 
{
	[Attribute("#AR-ValueUnit_Short_Plus", uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sSessionDurationFormatting;
	
	protected TextWidget m_SesionDurationText;
	protected float m_fCurrentSessionDuration;
	
	protected void SetSessionDurationTime(int sessionDuration) 
	{
		m_SesionDurationText.SetTextFormat(m_sSessionDurationFormatting, SCR_Global.GetTimeFormatting(sessionDuration, ETimeFormatParam.DAYS));		
	}
	
	//======================== ATTACH/DEATTACH HANDLER ========================\\
	//On Init
	override void HandlerAttachedScripted(Widget w)
	{		
		m_SesionDurationText = TextWidget.Cast(w);

		if (!m_SesionDurationText)
		{
			Print("'SCR_SessionDurationUIComponent' needs to be attached to a 'TextWidget'!", LogLevel.ERROR);
			return;
		}
		
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (!editorManager)
		{
			Print("'SCR_SessionDurationUIComponent' needs 'SCR_EditorManagerEntity'!", LogLevel.ERROR);
			return;
		}
		
		SCR_SessionDurationComponent serverSessionComponent = SCR_SessionDurationComponent.Cast(editorManager.FindComponent(SCR_SessionDurationComponent));
		
		if (!serverSessionComponent)
		{
			Print("'SCR_SessionDurationComponent' needs to be attached to the 'editorManager' in order to display the durration!", LogLevel.ERROR);
			return;
		}
		
		SetSessionDurationTime(serverSessionComponent.GetSessionDurationTime());
		serverSessionComponent.GetOnSessionDurationChange().Insert(SetSessionDurationTime);
		
	}
	
	
	//On Destroy
	override void HandlerDeattached(Widget w)
	{
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		
		if (editorManager)
		{
			SCR_SessionDurationComponent serverSessionComponent = SCR_SessionDurationComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_SessionDurationComponent));
		
			if (serverSessionComponent)
				serverSessionComponent.GetOnSessionDurationChange().Remove(SetSessionDurationTime);
		}
	}
};