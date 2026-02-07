[EntityEditorProps(category: "GameScripted/UI/RadialMenu", description: "Radial menu for item selection")]
class SCR_RadialMenuItemsComponentClass : ScriptComponentClass
{
	// prefab properties here
};

//------------------------------------------------------------------------------------------------
class SCR_RadialMenuItemsComponent : ScriptComponent
{
	[Attribute()]
	protected ref SCR_RadialMenuController m_MenuController;

	protected const int FIRST_ITEM_SLOT = 4;
	protected const int SLOT_COUNT = 10;

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		if (System.IsConsoleApp())
			return;

		if (!GetGame().InPlayMode())
			return;

		// Will setup radial menu and take control over the menu
		SetEventMask(owner, EntityEvent.INIT);

		if (m_MenuController)
			m_MenuController.Control(owner);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		if (!m_MenuController)
			return;

		m_MenuController.GetOnControllerChanged().Insert(OnControllerChanged);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnOpen()
	{
		if (!m_MenuController)
			return;

		SCR_RadialMenu radialMenu = m_MenuController.GetRadialMenu();
		if (!radialMenu)
			return;

		CreateEntries(radialMenu);

		SetEventMask(GetOwner(), EntityEvent.FIXEDFRAME);

		radialMenu.GetEventOnClose().Insert(OnClose);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnClose()
	{
		if (!m_MenuController)
			return;

		SCR_RadialMenu radialMenu = m_MenuController.GetRadialMenu();
		if (!radialMenu)
			return;

		radialMenu.ClearEntries();

		ClearEventMask(GetOwner(), EntityEvent.FIXEDFRAME);

		radialMenu.GetEventOnClose().Remove(OnClose);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnControllerChanged(SCR_RadialMenuController controller, bool hasControl)
	{
		if (!controller)
			return;

		SCR_RadialMenu radialMenu = controller.GetRadialMenu();
		if (!radialMenu)
			return;

		radialMenu.GetEventOnOpen().Remove(OnOpen);

		if (hasControl)
			radialMenu.GetEventOnOpen().Insert(OnOpen);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnFixedFrame(IEntity owner, float timeSlice)
	{
		SCR_RadialMenu radialMenu = m_MenuController.GetRadialMenu();
		if (!radialMenu)
		{
			ClearEventMask(owner, EntityEvent.FIXEDFRAME);
			Print("[SCR_RadialMenuItemsComponent] - Radial menu not assigned! Can't update!", LogLevel.WARNING);
			return;
		}

		radialMenu.UpdateEntries();
		radialMenu.Update(timeSlice);
	}

	//------------------------------------------------------------------------------------------------
	protected void CreateEntries(notnull SCR_RadialMenu radialMenu)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
		if (!character)
			return;

		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller)
			return;

		SCR_InventoryStorageManagerComponent storageManager = SCR_InventoryStorageManagerComponent.Cast(controller.GetInventoryStorageManager());
		if (!storageManager)
			return;

		SCR_CharacterInventoryStorageComponent storage = storageManager.GetCharacterStorage();
		if (!storage)
			return;

		// Receive items assigned to quick slots
		array<ref SCR_SelectionMenuEntry> entries = {};
		for (int i = FIRST_ITEM_SLOT; i < SLOT_COUNT; i++)
		{
			SCR_ItemSelectionMenuEntry itemEntry = new SCR_ItemSelectionMenuEntry(storage, i);
			entries.Insert(itemEntry);
		}

		radialMenu.AddEntries(entries);
	}
};
