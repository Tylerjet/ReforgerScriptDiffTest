[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_BanPlayerContextAction: SCR_KickPlayerContextAction
{
	[Attribute("86400", desc: "In seconds, -1 is forever")]
	protected int m_iBanDuration;
	
	override bool CanBeShown(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{			
		return SCR_PlayerPenaltyComponent.GetInstance() && super.CanBeShown(hoveredEntity, selectedEntities, cursorWorldPosition, flags);
	}
	override bool CanBePerformed(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		return SCR_PlayerPenaltyComponent.GetInstance() && super.CanBePerformed(hoveredEntity, selectedEntities, cursorWorldPosition, flags);
	}
	override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition,int flags, int param = -1)
	{
		SCR_PlayerPenaltyComponent playerPenaltyComponent = SCR_PlayerPenaltyComponent.Cast(SCR_PlayerPenaltyComponent.GetInstance());
		SCR_EditablePlayerDelegateComponent playerDelegate;
		foreach (SCR_EditableEntityComponent entity: selectedEntities)
		{
			playerDelegate = SCR_EditablePlayerDelegateComponent.Cast(entity);
			playerPenaltyComponent.BanPlayer(playerDelegate.GetPlayerID(), m_iBanDuration, SCR_PlayerManagerKickReason.BANNED_BY_GM);
		}
	}
};