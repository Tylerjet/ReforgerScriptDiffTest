//------------------------------------------------------------------------------------------------
//! Returns true if character has provided gadget in their inventory
//! if m_bEquipped true is only returned if the gadget is currently held
[BaseContainerProps()]
class SCR_CharacterHasGadgetCondition : SCR_AvailableActionCondition
{
	//This will check equipemnt of specific widget - otherwise will check any
	[Attribute("1", UIWidgets.CheckBox, "Only if gadget is in hands", "" )]
	protected bool m_bEquipped;

	[Attribute("1", UIWidgets.CheckBox, "", "" )]
	protected bool m_bCheckSpecificGadget;

	[Attribute(defvalue: SCR_Enum.GetDefault(EGadgetType.MAP), UIWidgets.ComboBox, "Gadget type", "", ParamEnumArray.FromEnum(EGadgetType) )]
	protected EGadgetType m_eGadget;

	[Attribute("0", UIWidgets.CheckBox, "Pass if the gadget is turned on", "" )]
	protected bool m_bIsToggledOn;

	[Attribute("0", UIWidgets.CheckBox, "Pass if the gadget is turned off", "" )]
	protected bool m_bIsToggledOff;

	//------------------------------------------------------------------------------------------------
	//! Returns true when current controlled entity has specified gadget
	//! Returns opposite if m_bNegateCondition is enabled
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;

		bool equipped;

		SCR_GadgetComponent heldGadgetComponent = data.GetHeldGadgetComponent();
		if (m_bCheckSpecificGadget)
		{
			// Specific gadget type is required
			equipped = heldGadgetComponent && heldGadgetComponent.GetType() == m_eGadget;
		}
		else if (data.GetHeldGadget())
		{
			// Any gadget will do
			equipped = true;
		}

		bool result;
		if (!m_bEquipped)
		{
			// Check gadgets that are not in hands
			// Fail if gadget of the same type is already equipped
			result = !equipped && data.GetGadget(m_eGadget);
		}
		// Allow only toggled on or off
		else if (equipped && (m_bIsToggledOn || m_bIsToggledOff))
		{
			// Pass if either requirement is met
			bool isToggledOn = heldGadgetComponent && heldGadgetComponent.IsToggledOn();
			result = (isToggledOn && m_bIsToggledOn) || (!isToggledOn && m_bIsToggledOff);
		}
		else
		{
			result = equipped;
		}

		return GetReturnResult(result);
	}
};
