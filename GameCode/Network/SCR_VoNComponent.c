//------------------------------------------------------------------------------------------------
class SCR_VoNComponentClass : VoNComponentClass
{}

//------------------------------------------------------------------------------------------------
class SCR_VoNComponent : VoNComponent 
{
	const float TRANSMISSION_TIMEOUT_MS = 500;
	private float m_fTransmitingTimeout;
	private SCR_VonDisplay m_VONDisplay;
	
	ref ScriptInvoker m_OnReceivedVON = new ref ScriptInvoker();
	ref ScriptInvoker m_OnCaptureVON = new ref ScriptInvoker();
	
	//------------------------------------------------------------------------------------------------
	//! Get Display Info script
	//! \return SCR_VonDisplay reference
	SCR_VonDisplay GetDisplay()
	{
		if (m_VONDisplay)
			return m_VONDisplay;
		
		SCR_VONController vonContr = SCR_VONController.Cast(GetGame().GetPlayerController().FindComponent(SCR_VONController));
		if (vonContr)
		{
			m_VONDisplay = vonContr.GetDisplay();
			return m_VONDisplay;
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Simple getter for when transmission was recently active, we assume that player's avatar should not produce various sounds while he's talking.
	//! \return true if recently active
	bool IsTransmiting()
	{
		return m_fTransmitingTimeout > GetGame().GetWorld().GetWorldTime();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected event void OnCapture(BaseRadioComponent radio)
	{
		SCR_VonDisplay display = GetDisplay();
		
		if (display)
			display.OnCapture(radio);
		
		m_OnCaptureVON.Invoke(radio);
		
		m_fTransmitingTimeout = GetGame().GetWorld().GetWorldTime() + TRANSMISSION_TIMEOUT_MS;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected event void OnReceive(int playerId, BaseRadioComponent radio, int frequency, float quality, int transceiverIdx)
	{
		SCR_VonDisplay display = GetDisplay();
		
		if (display)
			display.OnReceive(playerId, radio, frequency, quality, transceiverIdx);
		
		m_OnReceivedVON.Invoke(playerId, radio, frequency, quality, transceiverIdx);
	}
};