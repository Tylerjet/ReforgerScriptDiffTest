//------------------------------------------------------------------------------------------------
class SCR_RadialMenuWeapons: SCR_RadialMenuHandler
{
	// Page name 
	const string PAGENAME_WEAPONS = "Weapons (noloc)";
	
	// Entries data - weapons
	protected BaseWeaponManagerComponent m_WeaponManager;
	protected IEntity m_pPlayer;
	protected IEntity m_pCurrentWeaponEntity;
	protected CharacterControllerComponent m_pController;
	
	protected ref array<WeaponSlotComponent> m_aWeaponSlotsUnsorted = new array<WeaponSlotComponent>();
	protected ref array<WeaponSlotComponent> m_Weapons = new array<WeaponSlotComponent>();
	
	protected ref SCR_ConsumableSelectionMenuEntry m_QuickHealing; 
	
	// Data templates 
	protected ResourceName m_EntryWeaponsLayout = "{121C45A1F59DC1AF}UI/layouts/Common/RadialMenu/RadialEntryElement.layout";
	
	protected bool m_bInVehicle;

	//------------------------------------------------------------------------------------------------
	override protected void Init(IEntity owner)
	{	
		super.Init(owner);
		
		m_pOwner = owner;
		m_bInVehicle = false;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void PageSetup()
	{
		super.PageSetup();
		
		// Edit initial - weapons 
		PageSetupWeapon();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnPlayerDestroy()
	{	
		
	}
	
	//------------------------------------------------------------------------------------------------
	//! Create page with slots for weapons 
	protected void PageSetupWeapon()
	{
		// Edit name 
		SCR_MenuPage pageWeapons = m_aMenuPages[ERadialMenuWeaponsPages.WEAPONS];
		pageWeapons.SetName(PAGENAME_WEAPONS);
		
		// Receive wepon slots 
		FindWeaponManager(m_pPlayer);
		FindCharacterContoller(m_pPlayer);
		
		GetWeaponSlotData();
		
		// Add weapon slots 
		for (int i = 0; i < m_aWeaponSlotsUnsorted.Count(); i++)
		{
			SCR_WeaponSwitchSelectionMenuEntry entry = new ref SCR_WeaponSwitchSelectionMenuEntry(m_pOwner, m_aWeaponSlotsUnsorted[i]);
			entry.SetEntryLayout(m_EntryWeaponsLayout);
			pageWeapons.AddEntry(entry);
		}
		
		// Add healing entry
		ScriptedSelectionMenuEntry healEntry = QuickHealingEntry();
		if (!healEntry)
			healEntry = new SCR_ConsumableSelectionMenuEntry(null, null, null);
		
		pageWeapons.AddEntry(healEntry);

		// Set additional info
		pageWeapons.SetIconName("weapon_secondary");
		
	}
	
	//------------------------------------------------------------------------------------------------
	override protected event void OnUpdate(IEntity owner, float timeSlice)
	{
		super.OnUpdate(owner, timeSlice);

		if (IsOpen())
			GetGame().GetInputManager().ActivateContext("WeaponSelectionContext");
		
		if (IsWeaponSwitched())
		{
			// Update selection if weapon is switched 
			if (IsOpen())
			{
				SetEntriesToDisplay(m_pPlayer);
			}
			
			SetLastSelectedEntry();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override protected event void OnOpen(IEntity owner)
	{
		// Update entries 
		m_aWeaponSlotsUnsorted.Clear();
	
		// On Open
		super.OnOpen(owner);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OpenMenu(IEntity owner, bool isOpen)
	{		
		// Cash player entity 
		if (!m_pPlayer || m_pPlayer != SCR_PlayerController.GetLocalControlledEntity())
		{
			m_pPlayer = SCR_PlayerController.GetLocalControlledEntity();
			PageSetup();
			
			FindWeaponManager(m_pPlayer);
			FindCharacterContoller(m_pPlayer);
			
			m_pSource = m_pPlayer;
		}
		
		// Fail opening when player is in vehicle
		if (IsInVehicle(m_pPlayer, null))
			return;
		
		/*if(m_bInVehicle)
			return;*/
		
		super.OpenMenu(owner, isOpen);
	} 
	
	//------------------------------------------------------------------------------------------------
	override protected void SetLastSelectedEntry()
	{
		int id = -1; 
		
		// Find last selected slot 
		foreach (WeaponSlotComponent slot : m_aWeaponSlotsUnsorted)
		{
			id++;
			if(slot.GetWeaponEntity() == m_pCurrentWeaponEntity) 
				break;
			
			if(id == m_aWeaponSlotsUnsorted.Count() - 1)
				id = -1; 
		}

		// Find selection in entries
		if(id < m_pFilter.m_aAllEntries.Count() && id > -1)
		{
			m_pLastSelected = m_pFilter.m_aAllEntries[id];
		}
		else
			return;
		
		//m_iLastSelectedId = id;
		
		super.SetLastSelectedEntry();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Create entries for menu
	protected void GetWeaponSlotData()
	{
		if (!m_WeaponManager)
			return;
		
		m_WeaponManager.GetWeaponsSlots(m_Weapons);
		
		ClearEntries();
		
		// Create entry for each weapon
		foreach (WeaponSlotComponent slot : m_Weapons)
		{
			m_aWeaponSlotsUnsorted.Insert(slot);
		}
		
		if (m_aWeaponSlotsUnsorted.Count() > 1)
			m_aWeaponSlotsUnsorted.SwapItems(0, 1);
	}

	//------------------------------------------------------------------------------------------------
	override protected void FillEntry(IEntity owner, BaseSelectionMenuEntry entry, int index)
	{
		// Setup of entry
		CompartmentAccessComponent compartmentAcces = null;
		WeaponSlotComponent weaponSlot = m_aWeaponSlotsUnsorted[index];
		
		ref BaseSelectionMenuEntry weaponEntry;
			
		if(!IsInVehicle(m_pPlayer, compartmentAcces)) // Get character weapons
		{
			// Get data
			if (index > m_aWeaponSlotsUnsorted.Count() || m_aWeaponSlotsUnsorted[index] == null)
				return;
			
			// Setup entry
			weaponEntry = new ref SCR_WeaponSwitchSelectionMenuEntry(owner, weaponSlot);
		}
		else // Get vehicle weapons
		{
			if(!compartmentAcces)
				return;
				
			// Get data
			TurretControllerComponent turretController = FindTurretWeaponManager(compartmentAcces);
			if(!turretController)
				return;
			
			// Setup entry
			weaponEntry = new ref SCR_TurretWeaponMenuEntry(owner, weaponSlot, turretController);
		}
		
		// Will just pass entry so it's at the end
		super.FillEntry(owner, weaponEntry, index);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get consumable entry with bandages 
	//! Possible to get entity and count of bandages from entry 
	protected SCR_ConsumableSelectionMenuEntry QuickHealingEntry()
	{
		if (!m_pPlayer)
			return null;
		
		SCR_InventoryStorageManagerComponent inventoryManagerComponent = SCR_InventoryStorageManagerComponent.Cast(m_pPlayer.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!inventoryManagerComponent)
			return null;
		
		IEntity item = inventoryManagerComponent.GetBandageItem();
		if (!item)
			return null;
				
		m_QuickHealing = new SCR_ConsumableSelectionMenuEntry(item, m_pOwner, m_pPlayer);
		
		return m_QuickHealing;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check if character is holding different weapon
	protected bool IsWeaponSwitched()
	{
		if (!m_WeaponManager)
			return false;
		
		WeaponSlotComponent currentSlot = m_WeaponManager.GetCurrentSlot();
		
		if(!currentSlot)
			return false;
		
		if (currentSlot.GetWeaponEntity() == m_pCurrentWeaponEntity)
			return false;

		// Update weapon selection		
		m_pCurrentWeaponEntity = currentSlot.GetWeaponEntity();
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get weapon manager for reading entries data
	protected void FindWeaponManager(IEntity owner)
	{
		GenericEntity ownerGeneric = GenericEntity.Cast(m_pPlayer);
		
		if(!ownerGeneric)
			return;
		
		// Get character weapons
		m_WeaponManager = BaseWeaponManagerComponent.Cast(ownerGeneric.FindComponent(BaseWeaponManagerComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	protected void FindCharacterContoller(IEntity owner)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(owner);
		if (!character)
			return;
		
		m_pController = character.GetCharacterController();
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool IsInVehicle(IEntity pOwner, out CompartmentAccessComponent compartment)
	{
		GenericEntity pPlayer = GenericEntity.Cast(pOwner);
		if (!pPlayer)
			return false;
		
		compartment = CompartmentAccessComponent.Cast(pPlayer.FindComponent(CompartmentAccessComponent));
		if (!compartment)
			return false;
		
		return compartment.IsInCompartment();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check weapon manager on entering vehicle
	protected void EnteringCompartment(IEntity vehicle, BaseCompartmentManagerComponent manager, int mgrID, int slotID)
	{
		GenericEntity pPlayer = GenericEntity.Cast(m_pPlayer);
		if (!pPlayer)
			return;
		
		// Check if it is turret compartment
		CompartmentAccessComponent compartmentAccess = CompartmentAccessComponent.Cast(pPlayer.FindComponent(CompartmentAccessComponent));
		
		if(FindTurretWeaponManager(compartmentAccess))
		{
			m_bShowSelector = false;
			m_fSelectorAngle = 0;
			m_fSelectedAngle = 0;
			m_pCurrentSelection = null;
			OnSelectionUpdate(false, false, 0, 0, null);
			return;
		}
		else
		{
			m_bInVehicle = true;
			Close(m_pOwner);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Update selection on leaving compartment
	protected void LeavingCompartment(IEntity vehicle, BaseCompartmentManagerComponent manager, int mgrID, int slotID)
	{
		// Load entries for character weapons
		if(!IsInVehicle(m_pPlayer, null))
		{
			m_bInVehicle = false;
			FindWeaponManager(m_pPlayer);
			SetLastSelectedEntry();
			
			m_bShowSelector = false;
			m_fSelectorAngle = 0;
			m_fSelectedAngle = 0;
			m_pCurrentSelection = null;
			OnSelectionUpdate(false, false, 0, 0, null);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get weapon manager of curret turret for reading entries data
	TurretControllerComponent FindTurretWeaponManager(CompartmentAccessComponent compartmentAccess)
	{
		BaseCompartmentSlot slot = compartmentAccess.GetCompartment();
		
		if(!slot)
			return null;
		
		// Is current component turret
		TurretControllerComponent turretController = TurretControllerComponent.Cast(slot.GetController());
		
		if(!turretController)
			return null;
		
		m_WeaponManager = turretController.GetWeaponManager();
		return turretController;
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_RadialMenuWeapons()
	{
		
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_RadialMenuWeapons()
	{
	}
};

//------------------------------------------------------------------------------------------------
//! group split  
enum ERadialMenuWeaponsPages
{
	WEAPONS = 0,
};