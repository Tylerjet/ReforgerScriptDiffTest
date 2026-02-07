//------------------------------------------------------------------------------------------------
//! Returns true if character has provided gadget in their inventory
//! if m_bEquipped true is only returned if the gadget is currently held
[BaseContainerProps()]
class SCR_CharacterHasGadgetCondition : SCR_AvailableActionCondition
{
	//This will check equipemnt of specific widget - otherwise will check aný
	[Attribute("1", UIWidgets.ComboBox, "", "" )]
	private bool m_bCheckSpecificGadget;
	
	[Attribute("0", UIWidgets.ComboBox, "Which gadget?", "", ParamEnumArray.FromEnum(EGadgetType) )]
	private EGadgetType m_eGadget;
	
	// Probably split or sth
	[Attribute("0", UIWidgets.ComboBox, "Does the gadget have to be currently selected for the condition to pass?", "" )]
	private bool m_bEquipped;
	
	//------------------------------------------------------------------------------------------------
	//! Returns true when current controlled entity has specified gadget
	//! Returns opposite if m_bNegateCondition is enabled
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;
		
		//IEntity gadget = data.GetGadget(m_eGadget);
		IEntity heldGadget = data.GetHeldGadget();
		bool result = heldGadget != null;
		
		// We want to have it equipped, but it is not equipped
		if (m_bEquipped && heldGadget)
		{
			SCR_GadgetComponent heldGadgetCom = SCR_GadgetComponent.Cast(heldGadget.FindComponent(SCR_GadgetComponent));
			
			if (m_bCheckSpecificGadget)
			{
				if (!heldGadget || heldGadgetCom.GetType() != m_eGadget)
					result = false;
			}
			else
			{
				if (!heldGadget)
					result = false;
				else
					result = true;
			}
		}
		
		return GetReturnResult(result);
	}
};