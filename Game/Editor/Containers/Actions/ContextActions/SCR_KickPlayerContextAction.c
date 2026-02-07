[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_KickPlayerContextAction: SCR_SelectedEntitiesContextAction
{
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
		SCR_PlayerPenaltyComponent playerPenaltyComponent = SCR_PlayerPenaltyComponent.Cast(SCR_PlayerPenaltyComponent.GetInstance());
		
		foreach (SCR_EditableEntityComponent entity: selectedEntities)
		{
			playerDelegate = SCR_EditablePlayerDelegateComponent.Cast(entity);
			if (playerDelegate)
			{
				if (playerPenaltyComponent)
					playerPenaltyComponent.KickPlayer(playerDelegate.GetPlayerID(), SCR_PlayerManagerKickReason.KICKED_BY_GM);
				else 
					GetGame().GetPlayerManager().KickPlayer(playerDelegate.GetPlayerID(), SCR_PlayerManagerKickReason.KICKED_BY_GM);
			}
				
		}
	}
};