//------------------------------------------------------------------------------------------------
//! Voice over network entry data class, used for management of communication methods
class SCR_VONEntry : Managed
{
	bool m_bIsEnabled; 
	protected string m_sText = string.Empty;        // current display text
	protected SCR_VONController m_VONController;    // von controller
	protected SCR_VoNComponent m_VONComp;           // controlled entity von component
	// Protect me!
	BaseTransceiver m_RadioTransceiver;             // this entry's subject BaseTransceiver, if there is one
	protected SCR_GadgetComponent m_GadgetComp;     // this entry's subject gadget component, if there is one
			
	//------------------------------------------------------------------------------------------------
	// Init entry data
	void InitEntry()
	{}
	
	//------------------------------------------------------------------------------------------------
	//! Activate entry 
	void ActivateEntry()
	{
		if (!m_VONComp)
			return;
		
		m_VONComp.SetCommMethod(ECommMethod.DIRECT);
		m_VONComp.SetTransmitRadio(null);
	}
	
	//------------------------------------------------------------------------------------------------ 
	//! Adjust entry configuration such as radio frequency
	//! \param modifier is custom logic modifier 
	void AdjustEntry(int modifier)
	{}
	
	//------------------------------------------------------------------------------------------------ 
	//! Adjust entry configuration such as radio frequency, modified input
	//! \param modifier is custom logic modifier 
	void AdjustEntryModif(int modifier)
	{}
	
	//------------------------------------------------------------------------------------------------ 
	//! Toggle entry such as radio on/off
	void ToggleEntry()
	{}
	
	//------------------------------------------------------------------------------------------------ 
	//! Determines whether this entry can be adjusted
	//! \return true if adjustable
	bool CanBeAdjusted()
	{
		return false;
	}
	
	//------------------------------------------------------------------------------------------------ 
	//! Determines whether this entry can be toggled
	//! \return true if toggleable
	bool CanBeToggled()
	{
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get entry display text such as current frequency
	string GetDisplayText()
	{
		return m_sText;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get icon resource path string
	string GetIconResource()
	{
		return SCR_VonDisplay.ICON_DIRECT_SPEECH;
	}	
	
	//------------------------------------------------------------------------------------------------
	//! VON method type
	ECommMethod GetVONMethod()
	{
		return ECommMethod.DIRECT;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns gadget asociated with this entry
	SCR_GadgetComponent GetGadget()
	{
		return m_GadgetComp;
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_VONEntry(notnull SCR_VONController VONController, notnull SCR_VoNComponent vonComp, BaseTransceiver transceiver = null, SCR_GadgetComponent gadgetComp = null)
	{
		m_VONController = VONController;
		m_VONComp = vonComp;
		m_RadioTransceiver = transceiver;
		m_GadgetComp = gadgetComp;
	}	
};
