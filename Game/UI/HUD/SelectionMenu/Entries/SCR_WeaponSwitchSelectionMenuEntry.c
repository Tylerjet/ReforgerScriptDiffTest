class SCR_WeaponSwitchSelectionMenuEntry : SCR_BaseGroupEntry
{
	const string SLOT_EMTPY = "#AR-WeaponMenu_SlotEmpty";
	const string DESCRIPTION_EQUIP = "#AR-WeaponMenu_DescEquip";
	const string DESCRIPTION_SWAP = "#AR-WeaponMenu_DescSwap";
	
	protected WeaponSlotComponent m_pTargetSlot;
	protected IEntity m_pOwner;
	
	//------------------------------------------------------------------------------------------------
	override void UpdateVisuals()
	{	
		SCR_SelectionEntryWidgetComponent entryWidget = SCR_SelectionEntryWidgetComponent.Cast(GetEntryComponent());
		WeaponSlotComponent slot = GetTargetSlot();
		entryWidget.SetIconFaded(true);
		
		if (!entryWidget || !slot)
			return;
		
		// Weapon and info
		UIInfo uiInfo = GetUIInfo();
		IEntity weapon = slot.GetWeaponEntity();
		if (!weapon || !uiInfo)
			return;
		
		// Widget setup 
		entryWidget.SetLabelText(string.Empty);
		entryWidget.SetPreviewItem(weapon);
		entryWidget.SetIcon(string.Empty);
		entryWidget.SetIconVisible(false);
	}
	
	//------------------------------------------------------------------------------------------------
	protected IEntity GetCurrentCharacterWeapon(IEntity character)
	{
		ChimeraCharacter chimeraCharacter = ChimeraCharacter.Cast(character);
		if (chimeraCharacter)
		{
			BaseWeaponManagerComponent weaponManager = BaseWeaponManagerComponent.Cast(chimeraCharacter.FindComponent(BaseWeaponManagerComponent));
			if (weaponManager)
			{
				auto currentSlot = weaponManager.GetCurrentSlot();
				if (currentSlot)
					return currentSlot.GetWeaponEntity();
			}
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Callback for when this entry is supposed to be performed
	protected override event void OnPerform(IEntity user, BaseSelectionMenu sourceMenu)
	{
		if (!user || !m_pTargetSlot)
			return;
		
		auto character = ChimeraCharacter.Cast(user);
		if (!character)
			return;
		
		auto characterController = SCR_CharacterControllerComponent.Cast(character.FindComponent(SCR_CharacterControllerComponent));
		if (!characterController)
			return;
		
		characterController.SelectWeapon(m_pTargetSlot);
		
		super.OnPerform(user, sourceMenu);
	}

	//------------------------------------------------------------------------------------------------
	//! Can this entry be shown?
	protected override bool CanBeShownScript(IEntity user, BaseSelectionMenu sourceMenu)
	{
		if (user != m_pOwner)
			return false;
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Can this entry be performed?
	protected override bool CanBePerformedScript(IEntity user, BaseSelectionMenu sourceMenu)
	{
		if (user != m_pOwner)
			return false;
		
		if (m_pTargetSlot == null)
			return false;
		
		IEntity targetWeapon = m_pTargetSlot.GetWeaponEntity();
		if (targetWeapon == null)
			return false;
		
		IEntity currentWeapon = GetCurrentCharacterWeapon(m_pOwner);
		if (currentWeapon == targetWeapon)
			return false;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override bool GetEntryNameScript(out string outName)
	{
		/*if (m_pTargetSlot)
		{
			if (m_pTargetSlot.GetWeaponEntity())
			{
				auto uiInfo = m_pTargetSlot.GetUIInfo();
				if (uiInfo)
				{
					string weaponName = uiInfo.GetName();
					outName = weaponName;
					return true;
				}
			}
			else
			{
				outName = SLOT_EMTPY;
				return true;
			}
		}
		*/
		
		if(!m_pTargetSlot)
			return false;
		
		if (!m_pTargetSlot.GetWeaponEntity())
		{
			outName = WeaponTypeName();
			return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override bool GetEntryDescriptionScript(out string outDescription)
	{
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override bool GetEntryIconPathScript(out string outIconPath)
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override UIInfo GetUIInfoScript()
	{
		if (!m_pTargetSlot)
			return null;
		
		IEntity weapon = m_pTargetSlot.GetWeaponEntity();
		if (!weapon)	
			return null;
		
		WeaponAttachmentsStorageComponent storage = WeaponAttachmentsStorageComponent.Cast(weapon.FindComponent(WeaponAttachmentsStorageComponent));
		if (!storage)
			return null;
		
		SCR_ItemAttributeCollection attributes = SCR_ItemAttributeCollection.Cast(storage.GetAttributes());
		if (!attributes)
			return null;
		
		UIInfo uiInfo = attributes.GetUIInfo();
		if (!uiInfo)
			return null;
		
		return uiInfo;
	}
	
	//------------------------------------------------------------------------------------------------
	WeaponSlotComponent GetTargetSlot()
	{
		return m_pTargetSlot;
	}
	
	//------------------------------------------------------------------------------------------------
	string WeaponTypeName()
	{
		if (!m_pTargetSlot)
			return "";
		
		switch (m_pTargetSlot.GetWeaponSlotType())
		{
			case "primary":
			return "#AR-Error_NoPrimaryWeapon";
			
			case "secondary":
			return "#AR-Error_NoSecondaryWeapon";
			
			case "grenade":
			return "#AR-Error_NoGrenades";
		}
		
		return "";
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_WeaponSwitchSelectionMenuEntry(IEntity owner, WeaponSlotComponent weaponSlot)
	{
		m_pOwner = owner;
		m_pTargetSlot = weaponSlot;
	}
};