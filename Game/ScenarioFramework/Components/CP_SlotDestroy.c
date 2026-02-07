[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "ScriptWizard generated script file.")]
class CP_SlotDestroyClass : CP_SlotTaskClass
{
	// prefab properties here
}

//------------------------------------------------------------------------------------------------
/*!
	Class generated via ScriptWizard.
*/
class CP_SlotDestroy : CP_SlotTask
{
	
	//------------------------------------------------------------------------------------------------
	FactionKey GetFaction() { return m_sFaction; }

	//------------------------------------------------------------------------------------------------
	void CP_SlotDestroy(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
	}

	//------------------------------------------------------------------------------------------------
	void ~CP_SlotDestroy()
	{
	}
}
