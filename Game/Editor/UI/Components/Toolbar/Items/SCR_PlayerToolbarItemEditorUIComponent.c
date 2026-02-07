class SCR_PlayerToolbarItemEditorUIComponent: SCR_EntityToolbarItemEditorUIComponent
{
	[Attribute()]
	protected string m_sPlayerNameWidgetName;
	
	override void SetEntity(SCR_EditableEntityComponent entity, Widget widget, SCR_EditableEntitySlotManagerUIComponent slotManager)
	{
		super.SetEntity(entity, widget, slotManager);
		
		SCR_EditablePlayerDelegateComponent delegate = SCR_EditablePlayerDelegateComponent.Cast(entity);
		if (!delegate) return;
				
		int playerID = delegate.GetPlayerID();
		
		TextWidget nameWidget = TextWidget.Cast(widget.FindAnyWidget(m_sPlayerNameWidgetName));
		if (nameWidget)
			nameWidget.SetText(GetGame().GetPlayerManager().GetPlayerName(playerID));
	}
};