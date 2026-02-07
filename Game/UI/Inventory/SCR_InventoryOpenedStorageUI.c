class SCR_InventoryOpenedStorageUI : SCR_InventoryStorageBaseUI
{
	protected bool m_bFromVicinity;

	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		InitPaging();
		SetRowsAndColumns();
		CreateEmptyLayout();
		CreateSlots();
		SortSlots();
		ShowPage(0);
	}

	protected override int CreateSlots()
	{
		array<IEntity> pItemsInStorage = {};
		GetAllItems(pItemsInStorage, GetCurrentNavigationStorage());
		UpdateOwnedSlots(pItemsInStorage);
		return 0;
	}
	
	protected override void GetAllItems(out notnull array<IEntity> pItemsInStorage, BaseInventoryStorageComponent pStorage = null)
	{
		if (ClothNodeStorageComponent.Cast(pStorage))
		{
			array<BaseInventoryStorageComponent> pStorages = {};
			array<IEntity> pItems = {};
			pStorage.GetOwnedStorages(pStorages, 1, false);
			foreach (BaseInventoryStorageComponent pStor : pStorages)
			{
				if (!pStor)
					continue;
				pStor.GetAll(pItems);
				pItemsInStorage.Copy(pItems);
			}
			return;
		}

		if (pStorage)
		{
			pStorage.GetAll(pItemsInStorage);
			return;
		}

		if (m_Storage)
			m_Storage.GetAll(pItemsInStorage);
	}

	ButtonWidget ActivateCloseStorageButton()
	{
		m_wCloseStorageButton = ButtonWidget.Cast(m_widget.FindAnyWidget("CloseStorageBtn"));
		if (m_wCloseStorageButton)
		{
			m_wCloseStorageButton.SetVisible(true);
			SCR_InventoryNavigationButtonBack nav = SCR_InventoryNavigationButtonBack.Cast(m_wCloseStorageButton.FindHandler(SCR_InventoryNavigationButtonBack));
			if (nav)
			{
				if (GetGame().GetInputManager().GetLastUsedInputDevice() == EInputDeviceType.GAMEPAD)
					nav.EnableCloseWithGamepad(false);
				nav.m_OnActivated.Insert(CloseStorage);
			}
		}

		return m_wCloseStorageButton;
	}

	void CloseStorage()
	{
		m_MenuHandler.RemoveOpenStorage(this);
	}

	bool IsFromVicinity()
	{
		return m_bFromVicinity;
	}

	void SCR_InventoryOpenedStorageUI(
		BaseInventoryStorageComponent storage,
		LoadoutAreaType slotID = null,
		SCR_InventoryMenuUI menuManager = null,
		int iPage = 0,
		array<BaseInventoryStorageComponent> aTraverseStorage = null,
		int cols = 6,
		int rows = 3,
		bool fromVicinity = false)
	{
		m_Storage = storage;
		m_iMaxRows = rows;
		m_iMaxColumns = cols;
		m_bFromVicinity = fromVicinity;
		m_iMatrix = new SCR_Matrix(m_iMaxColumns, m_iMaxRows);
		if (aTraverseStorage)
			m_aTraverseStorage.Copy(aTraverseStorage);
		SetSlotAreaType(slotID);
	}
};