//-----------------------------------------------------------------------------------------------
//! Testing class providing data for radial menu
class SCR_RadialMenuData : SCR_InfoDisplay
{
/*	[Attribute("", UIWidgets.Object)]
	protected ref SCR_RadialMenuHandler m_pSelectionMenu; 
	
	// Entries data - weapons
	protected BaseWeaponManagerComponent m_WeaponManager;
	protected BaseControllerComponent m_Controller;
	protected GenericEntity m_Owner;
	
	protected ref array<WeaponSlotComponent> m_aWeaponSlotsUnsorted = new array<WeaponSlotComponent>;
	
	//------------------------------------------------------------------------------------------------
	override event void OnInit(IEntity owner)
	{
		// Add listeners
		//m_pSelectionMenu.onMenuToggleInvoker.Insert(MenuToggleAction);
		
		m_pSelectionMenu.Init(owner);
		PrepareEntriesData(owner);
	}
	
	//------------------------------------------------------------------------------------------------
	override event void UpdateValues(IEntity owner, float timeSlice)
	{
		if(m_pSelectionMenu)
		{
			//m_pSelectionMenu.GetRadialMenuInteraction().Update(owner, timeSlice);
			m_pSelectionMenu.Update(owner, timeSlice);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void PrepareEntriesData(IEntity owner)
	{
		m_Owner = GenericEntity.Cast(owner);
		if (!m_Owner)
			return;
		
		if (Vehicle.Cast(m_Owner))
		{
			BaseVehicleNodeComponent nodeComponent = BaseVehicleNodeComponent.Cast(m_Owner.FindComponent(BaseVehicleNodeComponent));
			if (!nodeComponent)
				return;
			m_Controller = BaseControllerComponent.Cast(nodeComponent.FindComponent(BaseControllerComponent));
			m_WeaponManager = BaseWeaponManagerComponent.Cast(nodeComponent.FindComponent(BaseWeaponManagerComponent));

		}
		else
		{
			m_Controller = BaseControllerComponent.Cast(m_Owner.FindComponent(BaseControllerComponent));
			m_WeaponManager = BaseWeaponManagerComponent.Cast(m_Owner.FindComponent(BaseWeaponManagerComponent));
		}
		
		UpdateEntries();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Actions performed on opening and closing menu
	void MenuToggleAction(IEntity owner, bool isOpen)
	{
		if(isOpen)
		{
			UpdateWeaponData(owner, 0);
			//UpdateEntries();
		}
		
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateEntries()
	{
		if (!m_WeaponManager)
			return;
		
		array<BaseWeaponComponent> weapons = new array<BaseWeaponComponent>;
		m_WeaponManager.GetWeaponsList(weapons);
		
		m_pSelectionMenu.ClearEntries();
		
		foreach (BaseWeaponComponent comp : weapons)
		{
			WeaponSlotComponent slot = WeaponSlotComponent.Cast(comp);
			m_aWeaponSlotsUnsorted.Insert(slot);
			
			BaseSelectionMenuEntry entry = new BaseSelectionMenuEntry();
			m_pSelectionMenu.AddEntry(entry);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateWeaponData(IEntity owner, float timeSlice)
	{
		if (m_pSelectionMenu)
		{
			// Clear radial menu entries
			m_pSelectionMenu.ClearEntries();
			
			// Prepare all entries
			foreach (WeaponSlotComponent weaponSlot : m_aWeaponSlotsUnsorted)
			{
				if (weaponSlot == null)
					continue;
				
				// Add each weapon item into the radial menu
				ref auto entry = new ref SCR_WeaponSwitchSelectionMenuEntry(owner, weaponSlot);				
				m_pSelectionMenu.AddEntry(entry);
			}	
			
			// Update radial menu
			//m_pSelectionMenu.Update(owner, timeSlice);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_RadialMenuData()
	{

	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_RadialMenuData()
	{

	}*/
};