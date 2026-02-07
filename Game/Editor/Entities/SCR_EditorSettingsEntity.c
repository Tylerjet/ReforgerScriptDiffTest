[EntityEditorProps(category: "GameScripted/Editor", description: "World settings for the editor", color: "251 91 0 255", icon: "WBData/EntityEditorProps/entityEditor.png")]
class SCR_EditorSettingsEntityClass: SCR_EditorBaseEntityClass
{
};

/** @ingroup Editor_Entities
*/

/*!
Editor settings to override default values in SCR_EditorManagerCore.
*/
class SCR_EditorSettingsEntity : SCR_EditorBaseEntity
{
	[Attribute(uiwidget: UIWidgets.ResourceNamePicker, desc: "Individual editor manager", params: "et", category: "Editor Settings")]
	protected ResourceName m_EditorManagerPrefab;
	
	[Attribute(desc: "True to disable the editor completely.\nFOR DEVELOPMENT PURPOSES ONLY!", category: "Editor Settings")]
	protected bool m_bDisableEditor;
	
	[Attribute(desc: "When enabled, each player will receive editor modes defined by 'Base Modes' attribute instead of the modes defined in core config.", category: "Editor Settings")]
	protected bool m_bOverrideBaseModes;
	
	[Attribute("0", UIWidgets.Flags, "Editor modes added to every pllayer when they connect.", enums: ParamEnumArray.FromEnum(EEditorMode), category: "Editor Settings")]
	protected EEditorMode m_BaseModes;
	
	/*!
	Get editor manager prefab.
	\param basePrefab Prefab used when the entity doesn't define custom prefab
	\return Editor manager prefab.
	*/
	ResourceName GetPrefab(ResourceName basePrefab)
	{
		if (m_EditorManagerPrefab.IsEmpty()) return basePrefab;
		return m_EditorManagerPrefab;
	}
	
	/*!
	Check if editor is disabled.
	FOR DEVELOPMENT PURPOSES ONLY!
	\return True when disabled
	*/
	bool IsEditorDisabled()
	{
		return m_bDisableEditor;
	}
	
	/*!
	When set to true, connecting players will receive editor modes define by these settings, instead of use defaults from SCR_EditorManagerCore.
	\value True to enable override
	*/
	void EnableBaseOverride(bool value)
	{
		m_bOverrideBaseModes = value;
	}
	/*!
	Check if base editor modes are overriden.
	\return True when override is active
	*/
	bool IsBaseOverrideEnabled()
	{
		return m_bOverrideBaseModes;
	}
	/*!
	Get base editor modes in this world.
	\param modes Variable to be filled with editor mode flags.
	\return True if the override is enabled
	*/
	bool GetBaseModes(out EEditorMode modes = 0)
	{
		modes = m_BaseModes;
		return m_bOverrideBaseModes;
	}
	/*!
	Set base editor modes in this world.
	\param modes Editor mode flags.
	\param validate When true, remove non-base modes from already existing editor managers
	*/
	void SetBaseModes(EEditorMode modes, bool validate = true)
	{
		m_BaseModes = modes;
		
		//--- Validate all existing editor modes
		if (validate)
		{
			SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
			if (core)
			{
				array<SCR_EditorManagerEntity> editorManagers = {};
				core.GetEditorEntities(editorManagers);
				foreach (SCR_EditorManagerEntity editorManager: editorManagers)
				{
					editorManager.SetEditorModes(EEditorModeAccess.BASE, m_BaseModes, false);
				}
			}
		}
	}
	
	void SCR_EditorSettingsEntity(IEntitySource src, IEntity parent)
	{
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (core) core.SetSettingsEntity(this);
	}
};