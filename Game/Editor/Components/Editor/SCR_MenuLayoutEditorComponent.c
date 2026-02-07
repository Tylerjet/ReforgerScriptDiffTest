[ComponentEditorProps(category: "GameScripted/Editor", description: "Layout for in-game editor. Works only with SCR_EditorBaseEntity!", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_MenuLayoutEditorComponentClass: SCR_BaseEditorComponentClass
{
	[Attribute(category: "Editable Entity UI")]
	protected ref SCR_EditableEntityUIConfig m_EditableEntityUI;
	
	SCR_EditableEntityUIConfig GetEditableEntityUI()
	{
		return m_EditableEntityUI;
	}
};

/** @ingroup Editor_Components
*/
class SCR_MenuLayoutEditorComponent : SCR_BaseEditorComponent
{
	[Attribute("", UIWidgets.ResourceNamePicker, "", "layout")]
	protected ResourceName m_Layout;
	
	protected Widget m_Widget;
	protected SCR_CursorEditorUIComponent m_CursorComponent;
	
	/*!
	Get configuration of editable entity UI.
	\return Editable entity UI config
	*/
	SCR_EditableEntityUIConfig GetEditableEntityUI()
	{
		SCR_MenuLayoutEditorComponentClass prefabData = SCR_MenuLayoutEditorComponentClass.Cast(GetComponentData(GetOwner()));
		if (prefabData)
			return prefabData.GetEditableEntityUI();
		else
			return null;
	}
	
	/*!
	Get world position below cursor.
	\param[out] worldPos Vector to be filled with world position
	\return True if the cursor is above world position (e.g., not pointing at sky)
	*/
	bool GetCursorWorldPos(out vector worldPos)
	{
		if (m_CursorComponent)
			return m_CursorComponent.GetCursorWorldPos(worldPos);
		else
			return false;
	}
	
	override void EOnEditorPostActivate()
	{
		SCR_MenuEditorComponent menuEditor = SCR_MenuEditorComponent.Cast(SCR_MenuEditorComponent.GetInstance(SCR_MenuEditorComponent));
		if (!menuEditor)
		{
			Print("SCR_MenuLayoutEditorComponent requires SCR_MenuEditorComponent!", LogLevel.ERROR);
			return;
		}
		
		EditorMenuBase menu = menuEditor.GetMenu();
		if (!menu) return;
	
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace) return;
		
		//--- Find widget which is parent for hiding - layout will be created under it, so it's hidden as well
		SCR_HideEditorUIComponent hideComponent = SCR_HideEditorUIComponent.Cast(menu.GetRootComponent().FindComponent(SCR_HideEditorUIComponent));
		if (!hideComponent || !hideComponent.GetWidget()) return;
		
		Widget parent = hideComponent.GetWidget();
		if (!parent) return;
		
		m_Widget = workspace.CreateWidgets(m_Layout, parent);
		FrameSlot.SetAnchorMin(m_Widget, 0, 0);
		FrameSlot.SetAnchorMax(m_Widget, 1, 1);
		FrameSlot.SetOffsets(m_Widget, 0, 0, 0, 0);
		
		//--- Get cursor component
		m_CursorComponent = SCR_CursorEditorUIComponent.Cast(menu.GetRootComponent().FindComponent(SCR_CursorEditorUIComponent));
		
		//--- Fade in
		//m_Widget.SetOpacity(0);
		//WidgetAnimator.PlayAnimation(m_Widget, WidgetAnimationType.Opacity, 1, 5);
	}
	override void EOnEditorPostDeactivate()
	{
		if (m_Widget)
		{
			m_Widget.RemoveFromHierarchy();
			m_Widget = null;
		}
	}
};