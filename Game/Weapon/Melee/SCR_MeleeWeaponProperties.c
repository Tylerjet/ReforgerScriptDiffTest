[ComponentEditorProps(category: "GameScripted/Weapon", description:"Keeps settings for melee weapon")]
class SCR_MeleeWeaponPropertiesClass: ScriptComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_MeleeWeaponProperties : ScriptComponent
{
	[Attribute("10", UIWidgets.Slider, "Size of damage dealt by the weapon", "0.0 100.0 1.0", category: "Global")]
	private float m_fDamage;	
	[Attribute("1", UIWidgets.Slider, "Range of the weapon [m]", "1 5 0.5", category: "Global")]
	private float m_fRange;
	[Attribute("1", UIWidgets.Slider, "how long it takes to execute the attack", "0.0 10.0 0.1", category: "Global")]
	private float m_fExecutionTime;

	[Attribute("5", UIWidgets.Slider, "Number of measurements in frame", "1 20 1", category: "Hit detection")]
	private int m_iNumOfMeasurements;
	[Attribute("", UIWidgets.Coords, "List of coords where the collision probes will be placed (related to model)", category: "Hit detection")]
	private ref array<vector> m_aCollisionProbesPos;	
	[Attribute("1", UIWidgets.Slider, "Number of failed probes check that cause the attack will be canceled immediately", "1 10 1", category: "Hit detection")]
	private int m_iNumOfFailedProbesTolerance;
	
	//------------------------------------------------------------------------------------------------
	//! Public getters used in SCR_MeleeComponent
	//! How many measurement will be fired while the attack is executed
	//! 	- used for non-simplified hit detection only (when SCR_MELEE_SIMPLE in SCR_MeleeComponent is not defined)
	int GetNumOfMeasurements()
	{
		return m_iNumOfMeasurements;
	}

	//------------------------------------------------------------------------------------------------
	//! How many hit-detection probes can fail and attack will continue
	int GetNumOfFailedProbesTolerance()
	{
		return m_iNumOfFailedProbesTolerance;
	}

	//------------------------------------------------------------------------------------------------	
	//! Value of damage dealt to the target
	float GetWeaponDamage()
	{
		return m_fDamage;
	}

	//------------------------------------------------------------------------------------------------	
	//! Range in meters that is used as max raycast length
	float GetWeaponRange()
	{
		return m_fRange;
	}

	//------------------------------------------------------------------------------------------------
	//! How long it takes to finish the attack
	float GetExecutionTime()
	{
		return m_fExecutionTime;
	}

	//------------------------------------------------------------------------------------------------
	//! Return all collision probes - for hit-detection calculation
	array<vector> GetCollisionProbesPositions()
	{
		return m_aCollisionProbesPos;
	}
	
	#ifdef ENABLE_DIAG
	//------------------------------------------------------------------------------------------------
	//! Drawing of collision probes debug in Edit mode / Ingame
	private void Debug_DrawCollisionProbes(IEntity owner)
	{
		for (int i = 0; i < m_aCollisionProbesPos.Count(); ++i)
		{
			Shape.CreateSphere(0xffa020f0, ShapeFlags.NOZBUFFER|ShapeFlags.ONCE|ShapeFlags.NOOUTLINE, owner.CoordToParent(m_aCollisionProbesPos[i]), 0.025);
		}
	}

	#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	override void _WB_AfterWorldUpdate(IEntity owner, float timeSlice)
	{
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_WEAPONS_MELEE_SHOW_EDITOR))
		{
			Debug_DrawCollisionProbes(owner);
		}
	}
	#endif // WORKBENCH
	
	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_WEAPONS_MELEE_SHOW_GAME))
		{
			Debug_DrawCollisionProbes(owner);
		}
	}	
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		if(DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_WEAPONS_MELEE_ENABLE_GAME))
		{
			SetEventMask(owner, EntityEvent.FRAME);
			owner.SetFlags(EntityFlags.ACTIVE, true);
		}
	}
	#endif // ENABLE_DIAG
};

#ifdef ENABLE_DIAG
int SCR_MeleeWeaponPropertiesDiags()
{
	DiagMenu.RegisterMenu(SCR_DebugMenuID.DEBUGUI_WEAPONS_MELEE_MENU, "Melee", "Weapons");
	DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_WEAPONS_MELEE_SHOW_EDITOR, "", "Show melee debug (Editor)", "Melee");
	DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_WEAPONS_MELEE_ENABLE_GAME, "", "Enable melee debug (Game)", "Melee");
	DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_WEAPONS_MELEE_SHOW_GAME, "", "Show melee debug (Game)", "Melee");
	return 0;
};
int g_MeleeWeaponPropertiesDiags = SCR_MeleeWeaponPropertiesDiags();
#endif // ENABLE_DIAG