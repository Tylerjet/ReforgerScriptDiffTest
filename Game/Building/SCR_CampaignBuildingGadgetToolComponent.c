class SCR_CampaignBuildingGadgetToolComponentClass : SCR_GadgetComponentClass
{
}

class SCR_CampaignBuildingGadgetToolComponent : SCR_GadgetComponent
{
	[Attribute(defvalue: "25", desc: "Max distance at which the ghost preview is shown.")]
	protected float m_fDistanceToShowPreview;

	[Attribute(defvalue: "10", desc: "Max distance from which the composition can be build.")]
	protected float m_fDistanceToBuildComposition;

	[Attribute(defvalue: "10", desc: "How much of construction value this tool adds to composition per one action")]
	protected int m_iConstructionValue;

	protected const static float TRACE_DELAY_VALUE = 0.5;

	protected float m_fTraceDelay;

	protected ref array<SCR_CampaignBuildingLayoutComponent> m_aShownPreview = {};
	protected ref array<SCR_CampaignBuildingLayoutComponent> m_aShownPreviewOld = {};

	protected bool m_bToolActiveWhenEditorOpen;

//	protected SCR_CampaignBuildingLayoutComponent m_LayoutComponent;

	//------------------------------------------------------------------------------------------------
	override void Update(float timeSlice)
	{
		m_fTraceDelay += timeSlice;

		if (m_fTraceDelay < TRACE_DELAY_VALUE)
			return;

		TraceCompositionToShowPreview();

		m_fTraceDelay = 0.0;
	}

	//------------------------------------------------------------------------------------------------
	override void OnToggleActive(bool state)
	{
		m_fTraceDelay = 0.0;

		if (state)
			ToolToHand();
		else
			ToolToInventory();
	}

	//------------------------------------------------------------------------------------------------
	override EGadgetType GetType()
	{
		return EGadgetType.BUILDING_TOOL;
	}

//	//------------------------------------------------------------------------------------------------
//	// Perform one build step - add a given build value to a composition player is building.
//	protected void Build(notnull SCR_CampaignBuildingLayoutComponent layoutComponent)
//	{
//		layoutComponent.AddBuildingValue(m_iConstructionValue);
//	}

	//------------------------------------------------------------------------------------------------
	//! Triggered when an operation with gadget is performed - picked up, taken into hands, back to backpack etc.
	//! \param[in] mode
	//! \param[in] charOwner must have a GadgetManager component
	override void ModeSwitch(EGadgetMode mode, IEntity charOwner)
	{
		if (!charOwner || charOwner != EntityUtils.GetPlayer())
		{
			super.ModeSwitch(mode, charOwner);
			return;
		}

		if (SCR_GadgetManagerComponent.GetGadgetManager(charOwner).GetHeldGadgetComponent() == this)
			ToolToHand();
		else
			ToolToInventory();

		super.ModeSwitch(mode, charOwner);
	}

	//------------------------------------------------------------------------------------------------
	//! Building tool taken to hand - show preview etc.
	void ToolToHand()
	{
		// Disabled for future
//		EventHandlerManagerComponent eventHandlerManager = EventHandlerManagerComponent.Cast(m_CharacterOwner.FindComponent(EventHandlerManagerComponent));
//		if (eventHandlerManager)
//			eventHandlerManager.RegisterScriptHandler("OnADSChanged", this, TraceCompositionToBuild);

		//ToDo: Later the frame has to run on server too because of tracing the composition to build
		if (!System.IsConsoleApp())
			ActivateGadgetUpdate();

		SCR_CharacterControllerComponent characterController = GetCharacterControllerComponent();
		if (!characterController)
			return;
			
		characterController.GetOnPlayerDeath().Insert(ToolToInventory);

		SCR_EditorManagerEntity editorManagerEntity = SCR_EditorManagerEntity.GetInstance();
		if (!editorManagerEntity)
			return;

		editorManagerEntity.GetOnOpened().Insert(OnEditorOpened);
		editorManagerEntity.GetOnClosed().Insert(OnEditorClosed);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] playerID
	//! \param[in] isPossessing
	//! \param[in] mainEntityID
	void OnPossessed(int playerID, bool isPossessing, RplId mainEntityID)
	{
		DeactivateGadgetUpdate();

		SCR_CharacterControllerComponent characterController = GetCharacterControllerComponent();
		if (!characterController)
			return;
		
		characterController.GetOnPlayerDeath().Remove(ToolToInventory);
		
		if (isPossessing)
			return;

		SCR_EditorManagerEntity editorManagerEntity = SCR_EditorManagerEntity.GetInstance();
		if (!editorManagerEntity)
			return;

		editorManagerEntity.GetOnOpened().Remove(OnEditorOpened);
		editorManagerEntity.GetOnClosed().Remove(OnEditorClosed);

		SCR_GadgetManagerComponent gadgetManager = SCR_GadgetManagerComponent.GetGadgetManager(SCR_PlayerController.GetLocalControlledEntity());
		if (!gadgetManager)
			return;

		SCR_GadgetComponent gadgetComponent = gadgetManager.GetHeldGadgetComponent();
		if (!gadgetComponent)
			return;

		ToolToInventory();
		if (gadgetComponent.GetType() == EGadgetType.BUILDING_TOOL)
			gadgetManager.SetGadgetMode(gadgetManager.GetGadgetByType(EGadgetType.BUILDING_TOOL), EGadgetMode.IN_SLOT);
	}

	//------------------------------------------------------------------------------------------------
	//! Building tool out of hands - show preview.
	void ToolToInventory()
	{
		//ToDo: Later the frame has to run on server too because of tracing the composition to build
		if (!System.IsConsoleApp())
		{
			if (!GetBuildingModeEntity())
				RemovePreviews();

			DeactivateGadgetUpdate();

			RemoveEventHandlers();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Search for an editor mode entity (exists only when the player is in editor mode)
	//! \return
	SCR_EditorModeEntity GetBuildingModeEntity()
	{
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (!core)
			return null;

		SCR_EditorManagerEntity editorManager = core.GetEditorManager();
		if (!editorManager)
			return null;

		return editorManager.FindModeEntity(EEditorMode.BUILDING);
	}

	//------------------------------------------------------------------------------------------------
	protected void RemovePreviews()
	{
		foreach (SCR_CampaignBuildingLayoutComponent layoutComponent : m_aShownPreviewOld)
		{
			if (layoutComponent)
				layoutComponent.DeletePreview();
		}

		m_aShownPreviewOld.Clear();
	}

	//------------------------------------------------------------------------------------------------
	//! Method called when character has a shovel in hands and any editor mode is opned.
	protected void OnEditorOpened()
	{
		ChimeraCharacter char = ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
		if (!char)
			return;
		
		m_bToolActiveWhenEditorOpen = true;
		ToolToInventory();
		
		SCR_CharacterControllerComponent controllerComponent = SCR_CharacterControllerComponent.Cast(char.GetCharacterController());
		if (!controllerComponent)
			return;
		
		controllerComponent.m_OnLifeStateChanged.Insert(OnConsciousnessChanged);
		
		SCR_PossessingManagerComponent possessingManager = SCR_PossessingManagerComponent.GetInstance();
		if (!possessingManager)
			return;
			
		possessingManager.GetOnPossessedProxy().Insert(OnPossessed);
	}

	//------------------------------------------------------------------------------------------------
	//! Method called when character has a shovel in hands and any editor mode is closed.
	protected void OnEditorClosed()
	{
		RemoveOnConsciousnessChanged();
		
		SCR_PossessingManagerComponent possessingManager = SCR_PossessingManagerComponent.GetInstance();
		if (!possessingManager)
			return;
		
		possessingManager.GetOnPossessedProxy().Remove(OnPossessed);

		// If the editor is closed, because player possessed entity, don't continue and keep this event hooked for another call.
		if (possessingManager.IsPossessing(SCR_PlayerController.GetLocalPlayerId()))
		{
			m_bToolActiveWhenEditorOpen = false;
			return;
		}

		m_bToolActiveWhenEditorOpen = false;
		RemoveEventHandlers();
		GetGame().GetCallqueue().CallLater(ToolToHand, 0, false);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnConsciousnessChanged(bool conscious)
	{
		if (conscious)
			return;
		
		m_bToolActiveWhenEditorOpen = false;
		RemoveOnConsciousnessChanged();
	}
	
	//------------------------------------------------------------------------------------------------
	void RemoveOnConsciousnessChanged()
	{
		ChimeraCharacter char = ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
		if (!char)
			return;
		
		SCR_CharacterControllerComponent controllerComponent = SCR_CharacterControllerComponent.Cast(char.GetCharacterController());
		if (!controllerComponent)
			return;
		
		controllerComponent.m_OnLifeStateChanged.Remove(OnConsciousnessChanged);
	}
//	//------------------------------------------------------------------------------------------------
//	// Trace a composition to build commented out for future
//	void TraceCompositionToBuild()
//	{
//		if (!m_CharacterOwner)
//			return;
//
//		ChimeraCharacter character = ChimeraCharacter.Cast(m_CharacterOwner);
//
//		TraceParam param = new TraceParam();
//		param.Start = character.EyePosition();
//		param.End = param.Start + GetPlayersDirection() * m_fDistanceToBuildComposition;
//
//		param.Flags =TraceFlags.ENTS;
//		param.Exclude = character;
//		param.LayerMask = EPhysicsLayerPresets.Interaction;
//		BaseWorld world = GetOwner().GetWorld();
//		float traceDistance = world.TraceMove(param, EvaluateBuildEntity);
//
//		// This has to be done after the trace finished, because spawning / deleting or moving with the entity while trace is running can cause an issues.
//		if (m_LayoutComponent)
//			{
//				Build(m_LayoutComponent);
//				m_LayoutComponent = null;
//			}
//
//	}

	//------------------------------------------------------------------------------------------------
	//! Trace the possible previews
	//! \return
	protected bool TraceCompositionToShowPreview()
	{
		BaseWorld world = GetOwner().GetWorld();
		if (!world)
			return false;

		world.QueryEntitiesBySphere(GetOwner().GetOrigin(), m_fDistanceToShowPreview, EvaluatePreviewEntity);

		// Delete those preview which are no more on the list of traced one and then copy the array for next iteration cycle.
		for (int i = m_aShownPreviewOld.Count() - 1; i >= 0; i--)
		{
			if (!m_aShownPreviewOld[i] || !m_aShownPreviewOld[i].HasBuildingPreview())
			{
				m_aShownPreviewOld.Remove(i);
				continue;
			}

			if (!m_aShownPreview.Contains(m_aShownPreviewOld[i]))
				m_aShownPreviewOld[i].DeletePreview();
		}

		foreach (SCR_CampaignBuildingLayoutComponent component : m_aShownPreview)
		{
			if (component && !m_aShownPreviewOld.Contains(component))
				component.SpawnPreview();
		}

		m_aShownPreviewOld.Copy(m_aShownPreview);
		m_aShownPreview.Clear();

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool EvaluatePreviewEntity(IEntity ent)
	{
		if (!ent)
			return true;

		SCR_CampaignBuildingLayoutComponent layoutComponent = SCR_CampaignBuildingLayoutComponent.Cast(ent.FindComponent(SCR_CampaignBuildingLayoutComponent));
		if (!layoutComponent)
			return true;

		m_aShownPreview.Insert(layoutComponent);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! \return the value how much this tool adds to a building value with one action
	int GetToolConstructionValue()
	{
		return m_iConstructionValue;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_CharacterControllerComponent GetCharacterControllerComponent()
	{
		IEntity playerEntity = SCR_PlayerController.GetLocalMainEntity();
		if (!playerEntity)
			return null;

		return SCR_CharacterControllerComponent.Cast(playerEntity.FindComponent(SCR_CharacterControllerComponent));
	}

//	//------------------------------------------------------------------------------------------------
//	// Disabled for future
//	bool EvaluateBuildEntity(IEntity ent, vector start = "0 0 0", vector dir = "0 0 0")
//	{
//		IEntity parent = ent.GetParent();
//		if (!parent)
//			return true;
//
//		SCR_CampaignBuildingLayoutComponent layoutComponent = SCR_CampaignBuildingLayoutComponent.Cast(parent.FindComponent(SCR_CampaignBuildingLayoutComponent));
//		if (!layoutComponent)
//			return true;
//
//		m_LayoutComponent = layoutComponent;
//		return false;
//	}

//	//------------------------------------------------------------------------------------------------
//	//! Returns the direction the player is looking Disabled debug
//	private vector GetPlayersDirection()
//	{
//		vector aimMat[4];
//		ChimeraCharacter character = ChimeraCharacter.Cast(m_CharacterOwner);
//		Math3D.AnglesToMatrix(CharacterControllerComponent.Cast(character.FindComponent(CharacterControllerComponent)).GetInputContext().GetAimingAngles() * Math.RAD2DEG, aimMat);
//		return aimMat[2];
//	}

	//------------------------------------------------------------------------------------------------
	//!
	void RemoveEventHandlers()
	{
		SCR_CharacterControllerComponent characterController = GetCharacterControllerComponent();
		if (characterController)
			characterController.GetOnPlayerDeath().Remove(ToolToInventory);

		SCR_EditorManagerEntity editorManagerEntity = SCR_EditorManagerEntity.GetInstance();
		if (editorManagerEntity)
		{
			editorManagerEntity.GetOnOpened().Remove(OnEditorOpened);

			if (!m_bToolActiveWhenEditorOpen)
				editorManagerEntity.GetOnClosed().Remove(OnEditorClosed);
		}
	}
}

