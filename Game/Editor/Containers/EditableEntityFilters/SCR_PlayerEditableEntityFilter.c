[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditableEntityState, "m_State")]
/** @ingroup Editor_Components_Entities
*/
/*!
*/
class SCR_PlayerEditableEntityFilter : SCR_BaseEditableEntityFilter
{
	private SCR_PlayersManagerEditorComponent m_PlayersManager;
	
	protected void OnSpawn(int playerID, SCR_EditableEntityComponent entity, SCR_EditableEntityComponent entityPrev)
	{
		Set(entity, entityPrev);
	}
	protected void OnDeath(int playerID, SCR_EditableEntityComponent entity, SCR_EditableEntityComponent killerEntity)
	{
		Remove(entity);
	}
	protected void OnPossessed(int playerID, SCR_EditableEntityComponent entity, bool isPossessing)
	{
		if (isPossessing)
			Add(entity);
		else
			Remove(entity);
	}
	override bool CanAdd(SCR_EditableEntityComponent entity)
	{
		return m_PlayersManager.GetPlayerID(entity) != 0;
	}
	override void EOnEditorActivate()
	{
		m_PlayersManager = SCR_PlayersManagerEditorComponent.Cast(SCR_PlayersManagerEditorComponent.GetInstance(SCR_PlayersManagerEditorComponent, true));
		if (!m_PlayersManager) return;
		
		m_PlayersManager.GetOnSpawn().Insert(OnSpawn);
		m_PlayersManager.GetOnDeath().Insert(OnDeath);
		m_PlayersManager.GetOnPossessed().Insert(OnPossessed);
	}
	override void EOnEditorDeactivate()
	{
		if (!m_PlayersManager) return;
		
		m_PlayersManager.GetOnSpawn().Remove(OnSpawn);
		m_PlayersManager.GetOnDeath().Remove(OnDeath);
		m_PlayersManager.GetOnPossessed().Remove(OnPossessed);
	}
};