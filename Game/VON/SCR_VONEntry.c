//------------------------------------------------------------------------------------------------
//! Voice over network entry data class, used for management of communication methods
class SCR_VONEntry : Managed
{
	bool m_bIsEnabled; 
	protected string m_sText = string.Empty;		// current display text
	protected SCR_VONController m_VONController;	// von controller
	protected SCR_VoNComponent m_VONComp;			// controlled entity von component
	BaseRadioComponent m_RadioComp;					// this entry's subject radio component, if there is one
	
	bool m_bIsSuspended; // TODO temporary hack
		
	//------------------------------------------------------------------------------------------------
	// Init entry data
	void InitEntry()
	{}
	
	//------------------------------------------------------------------------------------------------
	//! Activate entry 
	void ActivateEntry()
	{
		m_VONComp.SetCommMethod(ECommMethod.DIRECT);
		m_VONComp.SetTransmitRadio(null);
	}
	
	//------------------------------------------------------------------------------------------------ 
	//! Adjust entry configuration such as radio frequency
	//! \param modifier is custom logic modifier 
	void AdjustEntry(int modifier)
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
	void SCR_VONEntry(SCR_VONController VONController, SCR_VoNComponent vonComp, BaseRadioComponent radioComp = null)
	{
		m_VONController = VONController;
		m_VONComp = vonComp;
		m_RadioComp = radioComp;
	}	
};
