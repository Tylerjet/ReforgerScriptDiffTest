[EntityEditorProps(category: "Game/Building", description: "Safe zone area around preview entity, disallow its placing if can be blocked.")]
class SCR_CampaignBuildingEntityAreaTriggerClass : ScriptedGameTriggerEntityClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignBuildingEntityAreaTrigger : ScriptedGameTriggerEntity
{
	protected int m_iBlockingObjectCount;

	protected ref ScriptInvoker m_OnEntityEnter;
	protected ref ScriptInvoker m_OnEntityLeave;

	//------------------------------------------------------------------------------------------------
	override void OnInit(IEntity owner)
	{
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (!editorManager)
			return;
		
		SCR_EditorModeEntity modEntity = editorManager.FindModeEntity(EEditorMode.BUILDING);
		if (!modEntity)
			return;
		
		SCR_CampaignBuildingPlacingEditorComponent placingEditorComponent = SCR_CampaignBuildingPlacingEditorComponent.Cast(modEntity.FindComponent(SCR_CampaignBuildingPlacingEditorComponent));
		if (!placingEditorComponent)
			return;
		
		placingEditorComponent.SetPreviewEntityTrigger(this);
	}

	//------------------------------------------------------------------------------------------------
	//! Entity enters the trigger
	override void OnActivate(IEntity ent)
	{	
		if (IsIgnoredEntity(ent))
			return;

		m_iBlockingObjectCount++;
		GetOnEntityEnterTrigger().Invoke();
	}
			
	//------------------------------------------------------------------------------------------------
	//! Entity leaves the trigger
	override void OnDeactivate(IEntity ent)
	{
		if (IsIgnoredEntity(ent))
			return;
				
		m_iBlockingObjectCount--;
		if (m_iBlockingObjectCount == 0)
			GetOnEntityLeaveTrigger().Invoke();
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsNotBlocking()
	{
		return m_iBlockingObjectCount == 0;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsIgnoredEntity(notnull IEntity ent)
	{
		SCR_BasePreviewEntity prevEnt = SCR_BasePreviewEntity.Cast(ent);
		if (prevEnt)
			return true;
		
		BaseGameTriggerEntity trg = BaseGameTriggerEntity.Cast(ent);
		if (trg)
			return true;
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnEntityEnterTrigger()
	{
		if (!m_OnEntityEnter)
			m_OnEntityEnter = new ScriptInvoker();
		
		return m_OnEntityEnter;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnEntityLeaveTrigger()
	{
		if (!m_OnEntityLeave)
			m_OnEntityLeave = new ScriptInvoker();
		
		return m_OnEntityLeave;
	}
}
