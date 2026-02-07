[BaseContainerProps(), SCR_BaseContainerStaticTitleField(customTitle: "Keybind List")]
class SCR_FieldManualPiece_KeybindList : SCR_FieldManualPiece
{
	[Attribute()]
	protected ref array<ref SCR_FieldManualPiece_Keybind> m_aKeybinds;

	[Attribute(defvalue: "{3D06849921063735}UI/layouts/Menus/FieldManual/Pieces/FieldManual_Piece_KeybindList.layout", uiwidget: UIWidgets.EditBoxWithButton, params: "layout")]
	protected ResourceName m_Layout;

	protected ref Widget m_KeybindsLayout;

	//------------------------------------------------------------------------------------------------
	void SCR_FieldManualPieceKeybindList()
	{
		if (!m_aKeybinds) // can be config-provided
		{
			m_aKeybinds = {};
		}
	}

	//------------------------------------------------------------------------------------------------
	override void CreateWidget(notnull Widget parent)
	{
		WorkspaceWidget workspace = GetGame().GetWorkspace();

		Widget createdWidget = workspace.CreateWidgets(m_Layout, parent);
		if (!createdWidget)
		{
			Print("could not create Keybind List widget | " + FilePath.StripPath(__FILE__) + ":" + __LINE__, LogLevel.WARNING);
			return;
		}

		Widget keybindsLayout = SCR_WidgetHelper.GetWidgetOrChild(createdWidget, "KeybindsLayout");
		if (!keybindsLayout)
			return;

		m_KeybindsLayout = keybindsLayout;

		CreateKeybinds(!GetGame().GetInputManager().IsUsingMouseAndKeyboard());
		GetGame().OnInputDeviceIsGamepadInvoker().Insert(CreateKeybinds);
	}

	//------------------------------------------------------------------------------------------------
	protected void CreateKeybinds(bool isGamepad)
	{
		SCR_WidgetHelper.RemoveAllChildren(m_KeybindsLayout);

		if (!m_aKeybinds || m_aKeybinds.IsEmpty())
			return;


		array<SCR_EInputTypeCondition> conditions = { SCR_EInputTypeCondition.ALL_INPUTS };

		if (isGamepad)
		{
			conditions.Insert(SCR_EInputTypeCondition.GAMEPAD_ONLY);
		}
		else
		{
			conditions.Insert(SCR_EInputTypeCondition.KEYBOARD_ONLY);
		}

		array<ref SCR_FieldManualPiece_Keybind> arrayCopy = {};
		for (int i = 0; i < m_aKeybinds.Count(); i++)
		{
			if (conditions.Contains(m_aKeybinds[i].m_InputDisplayCondition))
			{
				arrayCopy.Insert(m_aKeybinds[i]);
			}
		}

		if (arrayCopy.IsEmpty())
			return;

		int countMinus1 = arrayCopy.Count() - 1;
		for (int i; i < countMinus1; i++)
		{
			arrayCopy[i].CreateWidget(m_KeybindsLayout, false);
		}
		arrayCopy[countMinus1].CreateWidget(m_KeybindsLayout, true);
	}

	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_FieldManualPiece_KeybindList()
	{
		if (GetGame().OnInputDeviceIsGamepadInvoker())
			GetGame().OnInputDeviceIsGamepadInvoker().Remove(CreateKeybinds);
	}
};
