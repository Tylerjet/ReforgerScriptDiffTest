[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "ScriptWizard generated script file.")]
class CP_SlotExtractionClass : CP_SlotTaskClass
{
	// prefab properties here
}

//------------------------------------------------------------------------------------------------
class CP_SlotExtraction : CP_SlotTask
{
	//------------------------------------------------------------------------------------------------
	override void Init(CP_Area pArea = null, CP_EActivationType EActivation = CP_EActivationType.SAME_AS_PARENT, bool bInit = true)
    {
        if (m_EActivationType != EActivation)
			return;
		super.Init(pArea, EActivation);
		FactionManager pFactionManager = GetGame().GetFactionManager();
		if (!pFactionManager)
			return;
		if (!SCR_PlayersPresentTriggerEntity.Cast(m_pEntity))
			return;
		SCR_PlayersPresentTriggerEntity.Cast(m_pEntity).SetOwnerFaction(pFactionManager.GetFactionByKey(m_sFaction));
    }
	
	//------------------------------------------------------------------------------------------------
	void CP_SlotExtraction(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
	}

	//------------------------------------------------------------------------------------------------
	void ~CP_SlotExtraction()
	{
	}
}