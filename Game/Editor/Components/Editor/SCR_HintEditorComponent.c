[ComponentEditorProps(category: "GameScripted/Editor", description: "Hints for editor interface. Works only with SCR_EditorBaseEntity!", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_HintEditorComponentClass: SCR_BaseEditorComponentClass
{
	[Attribute()]
	protected ref SCR_HintConditionList m_Hints;
	
	void Init(IEntity owner)
	{
		m_Hints.Init(owner);
	}
	void Exit(IEntity owner)
	{
		m_Hints.Exit(owner);
	}
};
class SCR_HintEditorComponent: SCR_BaseEditorComponent
{
	[Attribute("1", desc: "When enabled, hints in the editor won't fade out on their own, players have to close them manually.")]
	protected bool m_bInfiniteHints;
	
	
	//--- Refresh hint, as it sometimes uses device-specific lines
	protected void OnInputDeviceIsGamepad(bool isGamepad)
	{
		SCR_HintManagerComponent hintManager = SCR_HintManagerComponent.GetInstance();
		if (hintManager)
			hintManager.Refresh();
	}
	
	override void EOnEditorActivate()
	{
		SCR_HintManagerComponent hintManager = SCR_HintManagerComponent.GetInstance();
		if (!hintManager)
			return;
		
		if (m_bInfiniteHints)
			hintManager.SetDurationOverride(-1);
		
		SCR_HintEditorComponentClass prefabData = SCR_HintEditorComponentClass.Cast(GetEditorComponentData());
		if (prefabData)
			prefabData.Init(GetOwner());
		
		GetGame().OnInputDeviceIsGamepadInvoker().Insert(OnInputDeviceIsGamepad);
	}
	override void EOnEditorDeactivate()
	{
		SCR_HintManagerComponent hintManager = SCR_HintManagerComponent.GetInstance();
		if (!hintManager)
			return;
		
		if (m_bInfiniteHints)
			hintManager.SetDurationOverride(0);
		
		SCR_HintEditorComponentClass prefabData = SCR_HintEditorComponentClass.Cast(GetEditorComponentData());
		if (prefabData)
			prefabData.Exit(GetOwner());
		
		GetGame().OnInputDeviceIsGamepadInvoker().Remove(OnInputDeviceIsGamepad);
	}
};
