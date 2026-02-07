[EntityEditorProps(category: "GameScripted/Gadgets", description: "Map gadget", color: "0 0 255 255")]
class SCR_MapGadgetComponentClass: SCR_GadgetComponentClass
{
};

//------------------------------------------------------------------------------------------------
//! Map gadget component
class SCR_MapGadgetComponent : SCR_GadgetComponent
{
	[Attribute("0.35", UIWidgets.EditBox, desc: "seconds, delay before map gets activated giving time for the animation to be visible", params: "1 1000", category: "Map")]
	protected float m_fActivationDelay;
		
	[Attribute("0.35", UIWidgets.EditBox, desc: "seconds, how long it taks to fade in completely", params: "1 1000", category: "Map")]
	protected float m_fFadeInTime;
		
	protected bool m_bIsMapOpen;
	protected bool m_bIsFirstTimeOpened = true;		// whether the map has bene opened since put into a lot
	protected SCR_MapEntity m_MapEntity = null;		// map instance
	
	//------------------------------------------------------------------------------------------------
	//! Switch between map view
	//! \param state is desired state: true = open, false = close
	void SetMapMode(bool state)
	{		
		if (!m_MapEntity || !m_CharacterOwner)
			return;
				
		// no delay/fade for forced cancel
		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(m_CharacterOwner.FindComponent(SCR_CharacterControllerComponent));
		SCR_GadgetManagerComponent gadgetMgr =  SCR_GadgetManagerComponent.GetGadgetManager(m_CharacterOwner);
		if (!gadgetMgr)
			return;
		
		float delay; 
		
		if (controller.IsDead()) 
		{
			delay = 0;
			
			if (gadgetMgr.GetFadeWidget())
				gadgetMgr.GetFadeWidget().SetOpacity(0);
		}
		else 
		{
			delay = m_fActivationDelay*1000;
		
			// fade in
			if (gadgetMgr.GetFadeWidget())
				WidgetAnimator.PlayAnimation( gadgetMgr.GetFadeWidget(), WidgetAnimationType.Opacity, 1.0, 1/m_fFadeInTime);
		}
				
		GetGame().GetCallqueue().Remove(ToggleMapGadget);
		
		
		if (state)
			GetGame().GetCallqueue().CallLater(ToggleMapGadget, delay, false, true);
		else		
			GetGame().GetCallqueue().CallLater(ToggleMapGadget, delay, false, false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Open/close map
	//! \param state is desired state: true = open, false = close
	protected void ToggleMapGadget(bool state)
	{
		if (!m_MapEntity)
			return;
				
		SCR_GadgetManagerComponent gadgetMgr =  SCR_GadgetManagerComponent.GetGadgetManager(m_CharacterOwner);
		if (gadgetMgr && gadgetMgr.GetFadeWidget())
		{
			WidgetAnimator.StopAnimation(gadgetMgr.GetFadeWidget());
			WidgetAnimator.PlayAnimation( gadgetMgr.GetFadeWidget(), WidgetAnimationType.Opacity, 0.0, 1/m_fFadeInTime);
		}
				
		if (state)
		{
			SCR_MapEntity.GetOnMapOpen().Insert(OnMapOpen);
			
			MenuManager menuManager = g_Game.GetMenuManager();
			menuManager.OpenMenu(ChimeraMenuPreset.MapMenu);
			m_bIsMapOpen = true;
		}
		else
		{			
			MenuManager menuManager = g_Game.GetMenuManager();
			menuManager.CloseMenuByPreset(ChimeraMenuPreset.MapMenu);
			m_bIsMapOpen = false;
			
			SCR_MapEntity.GetOnMapOpen().Remove(OnMapOpen);
		}		
	}
				
	//------------------------------------------------------------------------------------------------
	//! SCR_MapEntity event
	protected void OnMapOpen(MapConfiguration config)
	{
		// first open
		if (m_bIsFirstTimeOpened)
		{
			m_bIsFirstTimeOpened = false;
			m_MapEntity.ZoomOut();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void ModeSwitch(EGadgetMode mode, IEntity charOwner)
	{
		super.ModeSwitch(mode, charOwner);
			
		if (mode == EGadgetMode.ON_GROUND)
			m_bIsFirstTimeOpened = true;
				
		// not current player	
		IEntity controlledEnt = SCR_PlayerController.GetLocalControlledEntity();
		if ( !controlledEnt || controlledEnt != m_CharacterOwner)
			return;
		
		if (mode == EGadgetMode.IN_HAND)
			ToggleFocused(true);
	}
	
	//------------------------------------------------------------------------------------------------
	override void ModeClear(EGadgetMode mode)
	{	
		// not current player	
		IEntity controlledEnt = SCR_PlayerController.GetLocalControlledEntity();		
		if ( !controlledEnt || controlledEnt != m_CharacterOwner )
		{
			super.ModeClear(mode);
			return;
		}
		
		if (mode == EGadgetMode.IN_HAND)
		{
			if (m_bFocused)
				ToggleFocused(false);
		}
		
		super.ModeClear(mode);
	}
	
	//------------------------------------------------------------------------------------------------
	override void ToggleFocused(bool enable)
	{
		super.ToggleFocused(enable);

		SetMapMode(enable);		
	}
			
	//------------------------------------------------------------------------------------------------
	override EGadgetType GetType()
	{
		return EGadgetType.MAP;
	}
		
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{		
		if (!m_MapEntity)
			m_MapEntity = SCR_MapEntity.GetMapInstance();
	}
	
};