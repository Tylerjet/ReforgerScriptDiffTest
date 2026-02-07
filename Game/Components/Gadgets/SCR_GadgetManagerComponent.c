//------------------------------------------------------------------------------------------------
//! Controls invoker resgistration for gadget manager
class SCR_GadgetInvokersInitState
{
	SCR_GadgetManagerComponent m_GadgetManager;
	SCR_CharacterControllerComponent m_Controller;
	
	bool m_bIsControlledInit = false;	// init flag for controlled entity invokers
	bool m_bIsDefaultInit = false;		// init flag for default invokers
	bool m_bIsControlledEnt = false;	// controlled entity flag
	
	//------------------------------------------------------------------------------------------------
	bool IsInit()
	{
		if (m_bIsControlledEnt)
			return m_bIsDefaultInit && m_bIsControlledInit;
		
		return m_bIsDefaultInit;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Invokers for all characters
	void InitInvokers(IEntity character, SCR_CharacterControllerComponent controller)
	{
		if (m_bIsDefaultInit)
			return;
		
		m_Controller = controller;
		SCR_InventoryStorageManagerComponent invManager = SCR_InventoryStorageManagerComponent.Cast(character.FindComponent(SCR_InventoryStorageManagerComponent));
		if (invManager)
		{			
			m_Controller.m_OnGadgetStateChangedInvoker.Insert(m_GadgetManager.OnGadgetStateChanged);
			m_Controller.m_OnControlledByPlayer.Insert(m_GadgetManager.OnControlledByPlayer);
			invManager.m_OnItemAddedInvoker.Insert(m_GadgetManager.OnItemAdded); 
			invManager.m_OnItemRemovedInvoker.Insert(m_GadgetManager.OnItemRemoved); 
			
			m_bIsDefaultInit = true;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Invokers for controlled character only
	void InitControlledInvokers(IEntity character, SCR_CharacterControllerComponent controller)
	{
		if (m_bIsControlledInit)
			return;
		
		m_Controller = controller;
		m_Controller.m_OnPlayerDeathWithParam.Insert(m_GadgetManager.OnPlayerDeath);
		m_Controller.m_OnGadgetFocusStateChangedInvoker.Insert(m_GadgetManager.OnGadgetFocusStateChanged);
		
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (editorManager)
			editorManager.GetOnOpened().Insert(m_GadgetManager.OnEditorOpened);
					
		m_bIsControlledEnt = true;
		m_bIsControlledInit = true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Cleanup invokers when entity is destroyed
	void CleanupInvokers(GenericEntity character)
	{	
		if (m_Controller)
		{
			m_Controller.m_OnGadgetStateChangedInvoker.Remove(m_GadgetManager.OnGadgetStateChanged);
			m_Controller.m_OnControlledByPlayer.Remove(m_GadgetManager.OnControlledByPlayer);
		}
				
		SCR_InventoryStorageManagerComponent invManager = SCR_InventoryStorageManagerComponent.Cast(character.FindComponent(SCR_InventoryStorageManagerComponent));
		if (invManager)
		{
			invManager.m_OnItemAddedInvoker.Remove(m_GadgetManager.OnItemAdded); 
			invManager.m_OnItemRemovedInvoker.Remove(m_GadgetManager.OnItemRemoved); 
		}
		
		m_bIsDefaultInit = false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Cleanup local invokers when entity is no longer controlled
	void CleanupLocalInvokers(GenericEntity character)
	{	
		if (m_Controller)
		{
			m_Controller.m_OnPlayerDeathWithParam.Remove(m_GadgetManager.OnPlayerDeath);
			m_Controller.m_OnGadgetFocusStateChangedInvoker.Remove(m_GadgetManager.OnGadgetFocusStateChanged);
		}
		
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (editorManager)
			editorManager.GetOnOpened().Remove(m_GadgetManager.OnEditorOpened);
		
		m_bIsControlledInit = false;
		m_bIsControlledEnt = false;
	}
				
	//------------------------------------------------------------------------------------------------
	void Clear(GenericEntity entity)
	{
		if (!entity)
			return;
		
		CleanupLocalInvokers(entity);
		CleanupInvokers(entity);
	}
		
	//------------------------------------------------------------------------------------------------
	void SCR_GadgetInvokersInitState(SCR_GadgetManagerComponent gadgetManager)
	{
		m_GadgetManager = gadgetManager;
	}
};

//------------------------------------------------------------------------------------------------
[EntityEditorProps(category: "GameScripted/Gadgets", description: "Gadget manager", color: "0 0 255 255")]
class SCR_GadgetManagerComponentClass: ScriptGameComponentClass
{
	//------------------------------------------------------------------------------------------------
	static override bool DependsOn(string className)
	{
		if (className == "SCR_CharacterInventoryStorageComponent")	// enforce load order here to be able to access storage
			return true;
		
		return false;
	}
};

//------------------------------------------------------------------------------------------------
class SCR_GadgetManagerComponent : ScriptGameComponent
{			
	ref ScriptInvoker m_OnGadgetAdded = new ref ScriptInvoker();			// called when gadget is added to inventory
	ref ScriptInvoker m_OnGadgetRemoved = new ref ScriptInvoker();			// called when gadget is removed from inventory
	
	protected bool m_bIsGadgetADS = false;		// is gadget currently in raised state
	protected IEntity m_HeldGadget = null;
	protected SCR_GadgetComponent m_HeldGadgetComponent = null;
	protected InputManager m_InputManager = null;
	protected SCR_InventoryStorageManagerComponent m_InventoryStorageMgr;
	protected SCR_CharacterControllerComponent m_Controller;
	protected ref SCR_GadgetInvokersInitState m_pInvokersState = new ref SCR_GadgetInvokersInitState(this);
	protected Widget m_wFade;
	protected Widget m_wEffectsRoot;	// cache so it can be cleaned up on manager destruction
	
	protected ref array<ref array <SCR_GadgetComponent>> m_aInventoryGadgetTypes = {};	// array of gadget types > array of gadget components
	protected ref map<EGadgetType, int> m_aGadgetArrayMap = new map<EGadgetType, int>; 	// map of gadget types -> gadget type array ID
	
	//TODO	
	protected SCR_GadgetComponent m_LastHeldGadgetComponent; // hack to make thingy work, unhack me 

	//------------------------------------------------------------------------------------------------
	// GET/SET METHODS
	//------------------------------------------------------------------------------------------------
	//! Get gadget manager of entity
	//! \return gadget manager
	static SCR_GadgetManagerComponent GetGadgetManager(IEntity entity)
	{		
		GenericEntity player = GenericEntity.Cast(entity);
		if (!player)
			return null;
		
		SCR_GadgetManagerComponent gadgetManager = SCR_GadgetManagerComponent.Cast( player.FindComponent(SCR_GadgetManagerComponent) );		

		return gadgetManager;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get held (currently in hand) gadget
	//! \return held gadget entity
	IEntity GetHeldGadget()
	{
		return m_HeldGadget;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Get is player in gadget ADS
	//! \return if gadget is in ADS
	bool GetIsGadgetADS()
	{
		return m_bIsGadgetADS;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get held (currently in hand) gadget component
	//! \return held gadget component
	SCR_GadgetComponent GetHeldGadgetComponent()
	{
		return m_HeldGadgetComponent;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set mode of a gadget
	//! When changing mode FROM EGadgetMode.IN_HAND to inventory, target should be EGadgetMode.IN_STORAGE -> it will do EGadgetMode.IN_SLOT automatically if it is slotted
	//! \param gadget is the subject
	//! \param targetMode is the mode being switched into
	//! \param doFocus determines whether the gadget will becomes focused straight away
	void SetGadgetMode(IEntity gadget, EGadgetMode targetMode, bool doFocus = true)
	{		
		if (!gadget)
			return;
		
		SCR_GadgetComponent gadgetComp = SCR_GadgetComponent.Cast( gadget.FindComponent(SCR_GadgetComponent) );
		if (!gadgetComp)
			return;
		
		if ( targetMode == EGadgetMode.IN_HAND && !gadgetComp.CanBeHeld() )
			return;
		
		// switch mode rules
		CanChangeGadgetMode(gadget, gadgetComp, targetMode, doFocus);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Remove gadget held in hand
	void RemoveHeldGadget()
	{
		if (!m_HeldGadget)
			return;
		
		SetGadgetMode(m_HeldGadget, EGadgetMode.IN_SLOT); // whether the gadget is actually slotted or put to storage is handled in OnGadgetStateChanged
	}
	
	//------------------------------------------------------------------------------------------------
	//! Toggle held gadget on/off
	//! \param state true = ON / false = OFF
	void ToggleHeldGadget(bool state)
	{
		if (!m_HeldGadgetComponent)
			return;
		
		m_HeldGadgetComponent.ToggleActive(state);
	}
						
	//------------------------------------------------------------------------------------------------
	//! Get all owned gadgets by type	
	//! \param type is the type of gadget being searched
	//! \return Returns array of gadget of the given type or null if not found
	array<SCR_GadgetComponent> GetGadgetsByType(EGadgetType type)
	{
		int gadgetArrayID = m_aGadgetArrayMap.Get(type);
		if (gadgetArrayID != -1)
		{
			return m_aInventoryGadgetTypes[gadgetArrayID];
		}
		
		return null;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Get first owned gadget by type
	//! \param type is the type of gadget being searched
	//! \return Returns gadget of given type or null if not found
	IEntity GetGadgetByType(EGadgetType type)
	{
		array<SCR_GadgetComponent> gadgets = GetGadgetsByType(type);
		if (gadgets && !gadgets.IsEmpty())
			return gadgets[0].GetOwner();

		return null;
	}
			
	//------------------------------------------------------------------------------------------------
	//! Get first owned gadget assigned in quickslot by type
	//! \param type is the type of gadget being searched
	//! \return Returns gadget of given type or null if not found
	IEntity GetQuickslotGadgetByType(EGadgetType type)
	{
		SCR_CharacterInventoryStorageComponent charStorage = SCR_CharacterInventoryStorageComponent.Cast( GetOwner().FindComponent(SCR_CharacterInventoryStorageComponent) );
		array<IEntity> quickslotEntities = charStorage.GetQuickSlotItems();
		
		foreach (IEntity entity : quickslotEntities )
		{
			if (!entity)	// empty slots will be null
				continue;
			
			SCR_GadgetComponent gadgetComp = SCR_GadgetComponent.Cast( entity.FindComponent(SCR_GadgetComponent) );
			if (gadgetComp)
			{
				if (gadgetComp.GetType() == type)
					return entity;
			}
		}
				
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check whether gadget is owned by this entity
	//! \return true if owned 
	bool IsGadgetOwned(SCR_GadgetComponent gadgetComp)
	{
		int gadgetArrayID = m_aGadgetArrayMap.Get(gadgetComp.GetType());
		if (gadgetArrayID != 1)
		{
			return m_aInventoryGadgetTypes[gadgetArrayID].Contains(gadgetComp);
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get widget used for gadget fade in/out
	//! \return fade widget 
	Widget GetFadeWidget()
	{
		return m_wFade;
	}
							
	//------------------------------------------------------------------------------------------------
	// MANAGER METHODS
	//------------------------------------------------------------------------------------------------
	//! Rules for gadget mode change
	//! \param gadget is the subject
	//! \param gadget is the subjects gadget component
	//! \param targetMode is the wanted mode
	//! \param doFocus determines whether the gadget will becomes focused straight away
	protected void CanChangeGadgetMode(IEntity gadget, SCR_GadgetComponent gadgetComp, EGadgetMode targetMode, bool doFocus)
	{								
		if ( targetMode == EGadgetMode.IN_HAND || gadgetComp.GetMode() == EGadgetMode.IN_HAND )	// If going TO or FROM mode1 -> anims need to happen first. Mode is synched from finish events instead of here 
		{
			CompartmentAccessComponent compAccess = CompartmentAccessComponent.Cast(GetOwner().FindComponent(CompartmentAccessComponent));	// if in vehicle
			if (compAccess && compAccess.IsInCompartment())
			{
				InventoryItemComponent invItemComponent = InventoryItemComponent.Cast(gadget.FindComponent(InventoryItemComponent));
				if (!invItemComponent)
					return;
					
				CharacterModifierAttributes charModifData = CharacterModifierAttributes.Cast(invItemComponent.FindAttribute(CharacterModifierAttributes));
				if (charModifData)
				{
					if (!charModifData.CanBeEquippedInVehicle())
						return;
				}	
			}
					
			if (!m_Controller)
				return;
							
			if (targetMode == EGadgetMode.IN_HAND)	// Gadget to hand
			{

				if (gadgetComp.CanBeRaised() && gadgetComp.IsSingleHanded())
					m_Controller.TakeGadgetInLeftHand(gadget, gadgetComp.GetAnimVariable(), doFocus);
				else 
					m_Controller.TakeGadgetInLeftHand(gadget, gadgetComp.GetAnimVariable());
			}
			else									// Gadget from hand
			{
				m_Controller.RemoveGadgetFromHand();
				
				if (gadgetComp.m_bFocused)	// close focus state
					gadgetComp.ToggleFocused(false);
			}
		}
		else	// If switching between NONE and MODE0
			gadgetComp.OnModeChanged(targetMode, GetOwner());
		
		return; 
	}
			
	//------------------------------------------------------------------------------------------------
	//! Update currently held(in hand) gadget
	//! \param gadget is the subject
	protected void UpdateHeldGadget(IEntity gadget)
	{
		// no need ot update
		if (gadget && gadget == m_HeldGadget)
			return;
		
		m_bIsGadgetADS = false;
		
		// clear
		if (!gadget)
		{
			if (m_HeldGadgetComponent && m_HeldGadgetComponent.m_bFocused)
				OnGadgetFocusStateChanged(m_HeldGadget, false);
			
			m_HeldGadget = null;
			m_LastHeldGadgetComponent = m_HeldGadgetComponent;
			m_HeldGadgetComponent = null;
		}
		else
		{
			SCR_GadgetComponent gadgetComp = SCR_GadgetComponent.Cast( gadget.FindComponent(SCR_GadgetComponent) );
			if (!gadgetComp)
				return;
			
			m_HeldGadget = gadget;
			m_HeldGadgetComponent = gadgetComp;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Additonal pre SetGadgetMode logic for local player, modfying the behavior based on input used & current held gadget state 
	//! \param gadget is the subject 
	//! \param inputVal determines type of input we want to execute, click/hold version
	void HandleInput(IEntity gadget, float inputVal)
	{
		EGadgetMode targetMode = EGadgetMode.IN_HAND;
		bool doFocus = inputVal == 1;	// input modifier from config
		
		if (m_HeldGadget == gadget)		// already holding this gadget
			targetMode = EGadgetMode.IN_STORAGE;
		
		SCR_GadgetComponent gadgetComp = SCR_GadgetComponent.Cast( gadget.FindComponent(SCR_GadgetComponent) );
		if (!gadgetComp)
			return;
		
		if (inputVal != 1 && gadgetComp.CanBeToggled())
			gadgetComp.ToggleActive(!gadgetComp.IsToggledOn());
		else
			SetGadgetMode(gadget, targetMode, doFocus);
	} 
		
	//------------------------------------------------------------------------------------------------
	// EVENTS
	//------------------------------------------------------------------------------------------------
	//! SCR_InventoryStorageManagerComponent, called on every add to slot, not only to inventory
	protected void OnItemAdded(IEntity item, BaseInventoryStorageComponent storageOwner)
	{		
		SCR_GadgetComponent gadgetComp = SCR_GadgetComponent.Cast(item.FindComponent(SCR_GadgetComponent));
		if (!gadgetComp)
			return;
		
		EGadgetType type = gadgetComp.GetType();
		
		int gadgetArrayID = m_aGadgetArrayMap.Get(type);
		if (gadgetArrayID == -1)
			return;
		
		m_aInventoryGadgetTypes[gadgetArrayID].Insert(gadgetComp);
		
		// Check whether the gadget is in equipment slot (OnAddedToSlot will happen before this)
		EquipmentStorageComponent equipmentComp = EquipmentStorageComponent.Cast(storageOwner);
		if (equipmentComp)
		{
			InventoryStorageSlot storageSlot = equipmentComp.FindItemSlot(item);
			if (storageSlot)
				SetGadgetMode(item, EGadgetMode.IN_SLOT);
		}
		else if (type == EGadgetType.RADIO_BACKPACK) 	// special case for backpack radios, always in (character) slot
			SetGadgetMode(item, EGadgetMode.IN_SLOT);
		else
			SetGadgetMode(item, EGadgetMode.IN_STORAGE); 
			
		m_OnGadgetAdded.Invoke(gadgetComp);
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_InventoryStorageManagerComponent, called on every remove from slot, not only from inventory
	protected void OnItemRemoved(IEntity item, BaseInventoryStorageComponent storageOwner)
	{
		SCR_GadgetComponent gadgetComp = SCR_GadgetComponent.Cast(item.FindComponent(SCR_GadgetComponent));
		if (!gadgetComp)
			return;
				
		SetGadgetMode(item, EGadgetMode.ON_GROUND);			// TODO chck if this could create issues within modes (MODEX->GROUND->MODEX) when swapping slots inside inventory?
		
		int gadgetArrayID = m_aGadgetArrayMap.Get(gadgetComp.GetType());
		if (gadgetArrayID == -1)
			return;
		
		int gadgetPos = m_aInventoryGadgetTypes[gadgetArrayID].Find(gadgetComp);
		if (gadgetPos != -1)
			m_aInventoryGadgetTypes[gadgetArrayID].Remove(gadgetPos);
		
		m_OnGadgetRemoved.Invoke(gadgetComp);
	}
			
	//------------------------------------------------------------------------------------------------
	//! SCR_CharacterControllerComponent event
	protected void OnPlayerDeath(SCR_CharacterControllerComponent charController, IEntity instigator)
	{		
		if (!charController || charController != m_Controller)
			return;
				
		m_pInvokersState.Clear(GetOwner());
		UnregisterInputs();
	}
		
	//------------------------------------------------------------------------------------------------
	//! SCR_CharacterControllerComponent event
	protected void OnControlledByPlayer(IEntity owner, bool controlled)
	{		
		if (!controlled)
		{
			m_pInvokersState.CleanupLocalInvokers(GetOwner());
			UnregisterInputs();
			ClearEventMask(owner, EntityEvent.FRAME);
		}
		else if (owner)
		{			
			m_pInvokersState.InitControlledInvokers(owner, m_Controller);
			RegisterInputs();
			SetEventMask(owner, EntityEvent.FRAME);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! CharacterControllerComponent event
	protected void OnGadgetStateChanged(IEntity gadget, bool isInHand, bool isOnGround)
	{
		SCR_GadgetComponent gadgetComp = SCR_GadgetComponent.Cast(gadget.FindComponent(SCR_GadgetComponent));
		if (!gadgetComp)
			return;
		
		if (isInHand)
		{
			UpdateHeldGadget(gadget);
			gadgetComp.OnModeChanged(EGadgetMode.IN_HAND, GetOwner());
		}
		else 
		{
			UpdateHeldGadget(null);
			
			if (isOnGround)	// this means it has been moved directly to ground from hand
			{
				gadgetComp.OnModeChanged(EGadgetMode.ON_GROUND, GetOwner());
				return;
			}
					
			// Check whether this gadget is slotted in a parent's equipment storage in order to set correct mode
			IEntity parent = gadget.GetParent();
			if (parent)
			{
				EquipmentStorageComponent equipStorage = EquipmentStorageComponent.Cast(parent.FindComponent(EquipmentStorageComponent));
				if (equipStorage)
				{
					if (equipStorage.Contains(gadget))
					{
						gadgetComp.OnModeChanged(EGadgetMode.IN_SLOT, GetOwner());
						return;
					}
				}
			}
					
			gadgetComp.OnModeChanged(EGadgetMode.IN_STORAGE, GetOwner());
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! CharacterControllerComponent event, used only for local character
	protected void OnGadgetFocusStateChanged(IEntity gadget, bool isFocused)
	{
		if (!gadget)	// when gadget is switched, this gets called after gadget state change with null TODO preferably this shouldnt happen :(
		{
			if (!isFocused && m_LastHeldGadgetComponent)	// handle unfocus
			{
				m_LastHeldGadgetComponent.ToggleFocused(false);
				m_bIsGadgetADS = false;
				
				if (m_LastHeldGadgetComponent.IsCombinedFocus() || m_Controller.GetMaxZoomInADS())
				{
					SCR_PlayerController controller = SCR_PlayerController.Cast(GetGame().GetPlayerController());
					if (controller)
						controller.SetFocusOverride(false);
				}
			}
			
			return;
		}
		
		m_bIsGadgetADS = isFocused;
		m_HeldGadgetComponent.ToggleFocused(isFocused);
				
		if (m_HeldGadgetComponent.IsCombinedFocus() || m_Controller.GetMaxZoomInADS())	// combined focus with ADS or MaxZoomInSights setting enabled
		{
			SCR_PlayerController controller = SCR_PlayerController.Cast(GetGame().GetPlayerController());
			if (controller)
				controller.SetFocusOverride(isFocused);
		}
	}
			
	//------------------------------------------------------------------------------------------------
	//! SCR_EditorManagerEntity event  //TODO we shouldnt need this, probably needs to be dependent on camera instead
	protected void OnEditorOpened()
	{
		m_bIsGadgetADS = false;
		if (m_HeldGadgetComponent)
		{
			m_HeldGadgetComponent.ToggleFocused(false);
			m_Controller.SetGadgetRaisedModeWanted(false); 
		}
	}

	//------------------------------------------------------------------------------------------------
	// INPUTS & INIT
	//------------------------------------------------------------------------------------------------
	//! Input action callback
	protected void OnGadgetInput(float value, EActionTrigger reason)
	{
		EGadgetType input = GetGadgetInputAction();	
		if (input == 0)
			return;
			
		// Search quickslots first, then inventory
		IEntity gadget = GetQuickslotGadgetByType(input);
		if (!gadget)
			gadget = GetGadgetByType(input);
		
		if (!gadget)
			return;
		
		HandleInput(gadget, value);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Input action callback
	protected void OnPauseMenu()
	{
		if (SCR_MapGadgetComponent.Cast(m_HeldGadgetComponent))
			SetGadgetMode( m_HeldGadget, EGadgetMode.IN_STORAGE ); // TODO update state transition
	}
	
	//------------------------------------------------------------------------------------------------
	//! Input action callback
	protected void OnGadgetADS()
	{
		if (!m_HeldGadgetComponent || !m_Controller)
			return;
				
		if (m_HeldGadgetComponent.CanBeRaised())
		{
			m_Controller.SetGadgetRaisedModeWanted(!m_bIsGadgetADS); 
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Input action callback
	protected void OnGadgetADSHold(float value, EActionTrigger reason)
	{
		if (!m_HeldGadgetComponent || !m_Controller || !m_HeldGadgetComponent.CanBeRaised())
			return;
		
		if (reason == EActionTrigger.DOWN)
		{
			m_Controller.SetGadgetRaisedModeWanted(true);
		}
		else 
		{
			m_Controller.SetGadgetRaisedModeWanted(false);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gather initial gadgets
	//! \param character is the query target
	protected void RegisterInitialGadgets(IEntity character)
	{
		array<IEntity> foundItems = {};
		array<typename> componentsQuery = {SCR_GadgetComponent};
	
		m_InventoryStorageMgr = SCR_InventoryStorageManagerComponent.Cast( character.FindComponent(SCR_InventoryStorageManagerComponent) );
		if (m_InventoryStorageMgr)
			m_InventoryStorageMgr.FindItemsWithComponents(foundItems, componentsQuery, EStoragePurpose.PURPOSE_ANY);
	
		int count = foundItems.Count();
		
		for (int i = 0; i < count; i++)
		{
			SCR_GadgetComponent gadgetComp = SCR_GadgetComponent.Cast( foundItems[i].FindComponent(SCR_GadgetComponent) );
			int gadgetArrayID = m_aGadgetArrayMap.Get(gadgetComp.GetType());
			if (gadgetArrayID == -1)
				continue;
			
			m_aInventoryGadgetTypes[gadgetArrayID].Insert(gadgetComp);

			if (gadgetComp.GetMode() == EGadgetMode.ON_GROUND) // initial gadgets for host will likely be added to storage before manager starts catching OnAdded events and will default to ground  
			{
				InventoryItemComponent invItemComp = InventoryItemComponent.Cast( foundItems[i].FindComponent(InventoryItemComponent) );
				if (invItemComp)
				{
					EquipmentStorageSlot storageSlot = EquipmentStorageSlot.Cast(invItemComp.GetParentSlot());	// Check whether the gadget is in equipment slot (OnAddedToSlot will happen before this)
					if (storageSlot)
					{
						SetGadgetMode(foundItems[i], EGadgetMode.IN_SLOT);
						continue;
					}
				}
				
				SetGadgetMode(foundItems[i], EGadgetMode.IN_STORAGE);
			}
			else 	// initial gadget for client can/will already have set mode members by replication, set the entire mode process properly here
				SetGadgetMode(foundItems[i], gadgetComp.GetMode());
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Register input actions
	protected void RegisterInputs()
	{
		m_InputManager = GetGame().GetInputManager();
		if (!m_InputManager)
			return;
								
		// Gadgets
		m_InputManager.AddActionListener("GadgetMap", EActionTrigger.DOWN, OnGadgetInput);
		m_InputManager.AddActionListener("MapEscape", EActionTrigger.DOWN, OnPauseMenu);
		m_InputManager.AddActionListener("GadgetCompass", EActionTrigger.DOWN, OnGadgetInput);
		m_InputManager.AddActionListener("GadgetBinoculars", EActionTrigger.DOWN, OnGadgetInput);
		m_InputManager.AddActionListener("GadgetFlashlight", EActionTrigger.DOWN, OnGadgetInput);
		m_InputManager.AddActionListener("GadgetWatch", EActionTrigger.DOWN, OnGadgetInput);
		m_InputManager.AddActionListener("GadgetADS", EActionTrigger.DOWN, OnGadgetADS);
		m_InputManager.AddActionListener("GadgetADSHold", EActionTrigger.DOWN, OnGadgetADSHold);
		m_InputManager.AddActionListener("GadgetADSHold", EActionTrigger.UP, OnGadgetADSHold);
				
		// TODO this can be moved to map?
		m_wEffectsRoot = GetGame().GetWorkspace().CreateWidgets("{BB249DB0ADF007C7}UI/layouts/HUD/GadgetEffects.layout");
		m_wFade = m_wEffectsRoot.FindAnyWidget("Fade");
	}
	
	//------------------------------------------------------------------------------------------------
	//! Register input actions
	protected void UnregisterInputs()
	{
		m_InputManager = GetGame().GetInputManager();
		if (!m_InputManager)
			return;
		
		m_InputManager.RemoveActionListener("GadgetMap", EActionTrigger.DOWN, OnGadgetInput);
		m_InputManager.RemoveActionListener("MapEscape", EActionTrigger.DOWN, OnPauseMenu);
		m_InputManager.RemoveActionListener("GadgetCompass", EActionTrigger.DOWN, OnGadgetInput);
		m_InputManager.RemoveActionListener("GadgetBinoculars", EActionTrigger.DOWN, OnGadgetInput);
		m_InputManager.RemoveActionListener("GadgetFlashlight", EActionTrigger.DOWN, OnGadgetInput);
		m_InputManager.RemoveActionListener("GadgetWatch", EActionTrigger.DOWN, OnGadgetInput);
		m_InputManager.RemoveActionListener("GadgetADS", EActionTrigger.DOWN, OnGadgetADS);
		m_InputManager.RemoveActionListener("GadgetADSHold", EActionTrigger.DOWN, OnGadgetADSHold);
		m_InputManager.RemoveActionListener("GadgetADSHold", EActionTrigger.UP, OnGadgetADSHold);
		
		if (m_wEffectsRoot)
			m_wEffectsRoot.RemoveFromHierarchy();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Convert invoked input to gadget type
	//! \return gadget type
	protected EGadgetType GetGadgetInputAction()
	{		
		if ( m_InputManager.GetActionValue("GadgetMap") )			
			return EGadgetType.MAP;
		
		if ( m_InputManager.GetActionValue("GadgetCompass") )
			return EGadgetType.COMPASS;
		
		if ( m_InputManager.GetActionValue("GadgetBinoculars") )
			return EGadgetType.BINOCULARS;
		
		if ( m_InputManager.GetActionValue("GadgetFlashlight") )
			return EGadgetType.FLASHLIGHT;
		
		if ( m_InputManager.GetActionValue("GadgetWatch") )
			return EGadgetType.WRISTWATCH;
		
		return 0;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Initialize component
	//! \param owner is the owner entity
	protected void InitComponent(IEntity owner)
	{ 		
		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast( owner.FindComponent(SCR_CharacterControllerComponent) );	
		if (!controller)
			return;
		
		m_Controller = controller;
		RplComponent rplComp = RplComponent.Cast(owner.FindComponent(RplComponent));
	
		m_pInvokersState.InitInvokers(owner, m_Controller); // default (all entities with gadget manager) invokers
						
		if (rplComp.IsOwner() && SCR_PlayerController.GetLocalControlledEntity() == owner)	// local (controlled entity) invokers, ignore owned non controlled characters
			m_pInvokersState.InitControlledInvokers(owner, m_Controller);
		
		// Owner specific behavior 
		if (m_pInvokersState.IsInit())
		{
			if (m_pInvokersState.m_bIsControlledEnt)
				RegisterInputs();
			else
				ClearEventMask(owner, EntityEvent.FRAME);	// if not controlled entity, clear frame
			
			RegisterInitialGadgets(owner);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// RPC
	//------------------------------------------------------------------------------------------------
	//! Ask method for toggle synchronization called from individual gadgets
	//! \param gadgetComp is gadget component
	//! \param state is desired toggle state, true: active 
	void AskToggleGadget(SCR_GadgetComponent gadgetComp, bool state)
	{				
		RplId id = Replication.FindId(gadgetComp);
		
		if ( id && id.IsValid() )
			Rpc(RPC_AskToggleGadget, id, state);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Ask RPC to server for gadget toggle
	//! \param id is unique Rpl id of the gadget 
	//! \param state is desired toggle state, true: active 
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
    protected void RPC_AskToggleGadget(RplId id, bool state)
	{	
		// TODO sanity check
					
		SCR_GadgetComponent gadgetComp = SCR_GadgetComponent.Cast(Replication.FindItem(id));
		if (!gadgetComp)
			return;
				
		gadgetComp.OnToggleActive(state);
		Rpc(RPC_DoToggleGadget, id, state);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Do RPC to clients for gadget toggle 
	//! \param id is unique Rpl id of the gadget 
	//! \param state is desired toggle state, true: active 
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast, RplCondition.NoOwner)]
    protected void RPC_DoToggleGadget(RplId id, bool state)
    {						
		// TODO sanity check
		
		SCR_GadgetComponent gadgetComp = SCR_GadgetComponent.Cast(Replication.FindItem(id));
		if (!gadgetComp)
			return;
		
		gadgetComp.OnToggleActive(state);
    }
		
	#ifndef DISABLE_GADGETS	
		//------------------------------------------------------------------------------------------------
		override bool OnTicksOnRemoteProxy() 
		{ 
			return true; // FRAME mask needed for couple frames to add invoker listeners before it is removed
		};
	
		//------------------------------------------------------------------------------------------------	
		override void EOnFrame(IEntity owner, float timeSlice)
		{			
			if (!m_pInvokersState.IsInit())
				InitComponent(owner);
			
			// context
			if (m_InputManager && m_HeldGadgetComponent)	
			{
				m_InputManager.ActivateContext("GadgetContext");
			
				if (m_HeldGadgetComponent.IsUsingADSControls())
					m_InputManager.ActivateContext("GadgetContextRaisable");
			
				if (m_HeldGadgetComponent.GetType() == EGadgetType.MAP)
					m_InputManager.ActivateContext("GadgetMapContext");
			}
		}

		//------------------------------------------------------------------------------------------------
		override void OnPostInit(IEntity owner)
		{
			if ( g_Game.InPlayMode() )
				SetEventMask(owner, EntityEvent.FRAME);	

			// init arrays
			typename gadgetTypes = EGadgetType;
			int count = gadgetTypes.GetVariableCount();
			for ( int i = 0; i < count; i++ )
			{
				array <SCR_GadgetComponent> gadgets = new array <SCR_GadgetComponent>;
				m_aInventoryGadgetTypes.Insert(gadgets);
				m_aGadgetArrayMap.Insert( Math.Pow(2, i), i); // EGadgetType flag + array ID
			}
		}		
	#endif
};