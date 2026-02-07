[EntityEditorProps(category: "GameScripted/UI/RadialMenu", description: "Common radial menu.")]
class SCR_RadialMenuGameModeComponentClass : ScriptComponentClass
{
	// prefab properties here
};

//------------------------------------------------------------------------------------------------
class SCR_RadialMenuGameModeComponent : ScriptComponent
{
	[Attribute()]
	protected ref SCR_RadialMenu m_Menu;

	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (!m_Menu)
		{
			ClearEventMask(owner, EntityEvent.FRAME);
			Print("[SCR_RadialMenuGameModeComponent] - Radial menu not assigned! Can't update!", LogLevel.WARNING);
			return;
		}

		m_Menu.Update(timeSlice);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.FRAME);
	}

	//------------------------------------------------------------------------------------------------
	SCR_RadialMenu GetMenu()
	{
		return m_Menu;
	}
};
