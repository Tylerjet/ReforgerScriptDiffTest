[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditableEntityState, "m_State")]
class SCR_FriendlyPlayerEditableEntityFilter : SCR_PlayerEditableEntityFilter
{
	protected Faction m_Faction;

	//------------------------------------------------------------------------------------------------
	override bool CanAdd(SCR_EditableEntityComponent entity)
	{
		if (!m_Faction)
			return false;

		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(entity.GetOwner());
		if (!character)
			return false;

		if (!super.CanAdd(entity))
			return false;

		return m_Faction.IsFactionFriendly(character.GetFaction());
	}

	//------------------------------------------------------------------------------------------------
	override void EOnEditorActivate()
	{
		super.EOnEditorActivate();

		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
		if (!character)
			return;

		m_Faction = character.GetFaction();
	}
}