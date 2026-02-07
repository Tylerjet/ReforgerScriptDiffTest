[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_KickPlayerContextAction: SCR_SelectedEntitiesContextAction
{
	[Attribute("0", desc: "How long before kicked-out player can reconnect.\n-1 means permanent ban (at least until exe restart)")]
	protected int m_iKickTimeout;
	
	[Attribute(uiwidget: UIWidgets.SearchComboBox, desc: "Reason for kicking shown to kicked-out player.", enums: ParamEnumArray.FromEnum(SCR_PlayerManagerKickReason))]
	protected SCR_PlayerManagerKickReason m_KickReason;
	
	override bool CanBeShown(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags)
	{
		SCR_EditablePlayerDelegateComponent playerDelegate = SCR_EditablePlayerDelegateComponent.Cast(selectedEntity);
		return playerDelegate && !SCR_Global.IsAdmin(playerDelegate.GetPlayerID());
	}
	override bool CanBePerformed(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags)
	{
		return CanBeShown(selectedEntity, cursorWorldPosition, flags);
	}
	override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition,int flags, int param = -1)
	{
		SCR_EditablePlayerDelegateComponent playerDelegate;		
		foreach (SCR_EditableEntityComponent entity: selectedEntities)
		{
			playerDelegate = SCR_EditablePlayerDelegateComponent.Cast(entity);
			if (playerDelegate)
			{
				GetGame().GetPlayerManager().KickPlayer(playerDelegate.GetPlayerID(), m_KickReason, m_iKickTimeout);
				
				//~ Todo: Move kick and ban notifications to Notification sender and make sure Player leaved notification is not shown
				if (m_iKickTimeout < 0)
					SCR_NotificationsComponent.SendToEveryone(ENotification.PLAYER_BANNED_NO_DURATION, playerDelegate.GetPlayerID());
				else if (m_iKickTimeout > 0)
					SCR_NotificationsComponent.SendToEveryone(ENotification.PLAYER_BANNED, playerDelegate.GetPlayerID(), m_iKickTimeout);
				else 
					SCR_NotificationsComponent.SendToEveryone(ENotification.PLAYER_KICKED, playerDelegate.GetPlayerID(), SCR_PlayerManagerKickReason.KICKED_BY_GM);	
			}			
		}
	}
};