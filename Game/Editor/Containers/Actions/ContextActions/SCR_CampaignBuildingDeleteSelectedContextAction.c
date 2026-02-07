//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_CampaignBuildingDeleteSelectedContextAction : SCR_DeleteSelectedContextAction
{
	//------------------------------------------------------------------------------------------------
	override int GetParam()
	{
		return GetGame().GetPlayerController().GetPlayerId();
	}

	//------------------------------------------------------------------------------------------------
	override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags, int param = -1)
	{
		if (!hoveredEntity)
			return;

		BaseGameMode gameMode = GetGame().GetGameMode();
		SCR_CampaignBuildingManagerComponent buildingManagerComponent = SCR_CampaignBuildingManagerComponent.Cast(gameMode.FindComponent(SCR_CampaignBuildingManagerComponent));
		if (!buildingManagerComponent)
			return;

		buildingManagerComponent.SetDisassemblyCompositionAndRequester(Replication.FindId(hoveredEntity), param);
		super.Perform(hoveredEntity, cursorWorldPosition);
	}

	//------------------------------------------------------------------------------------------------
	bool CanSendNotification(notnull SCR_EditableEntityComponent editableEnt)
	{
		SCR_EditableEntityUIInfo editableEntityUIInfo = SCR_EditableEntityUIInfo.Cast(editableEnt.GetInfo());
		if (!editableEntityUIInfo)
			return false;

		array<EEditableEntityLabel> entityLabels = {};
		editableEntityUIInfo.GetEntityLabels(entityLabels);
		return entityLabels.Contains(EEditableEntityLabel.TRAIT_SERVICE);
	}
};
