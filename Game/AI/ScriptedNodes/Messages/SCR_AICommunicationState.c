enum EAICommunicationState
{
	IDLE 			= 0,
	SPEAKING		= 1	
};

void SCR_AISpeech_Callback(EAICommunicationState newState);

typedef func SCR_AISpeech_Callback;
typedef ScriptInvokerBase<SCR_AISpeech_Callback> SCR_AIOnCommunicationStateChanged;


class SCR_AICommunicationState : Managed
{
	protected EAICommunicationState m_iAICommState;
	protected bool m_bMuted;
	protected const float SAMPLE_LENGTH = 2.0; 	 // length of the sound sample; TODO: read the value from the data
	protected float m_fSoundCountDown;			 // will countdown the sound from start to aproximation of the end
	
	ref SCR_AIOnCommunicationStateChanged m_OnCommunicationStateChanged = new SCR_AIOnCommunicationStateChanged();		
	
	//-------------------------------------------------------------------------------------------------------------
	//! Updates countdown timer when speaking, after timer runs out -> goes back to idle and returns false
	bool Update(float timeSlice)
	{
		if (m_iAICommState == EAICommunicationState.SPEAKING)
		{
			m_fSoundCountDown -= timeSlice;
			if (m_fSoundCountDown < 0)
			{
				CommStateTransition(EAICommunicationState.IDLE);				
				return false;
			}	
		}
		return true;		
	}
	
	//-------------------------------------------------------------------------------------------------------------
	bool IsSpeaking()
	{
		return m_iAICommState == EAICommunicationState.SPEAKING;
	}
	
	//-------------------------------------------------------------------------------------------------------------
	private void CommStateTransition(EAICommunicationState newState)
	{
		if (newState == m_iAICommState || m_bMuted)
			return;
		switch (newState)
		{
			case EAICommunicationState.IDLE : 
			{
				m_fSoundCountDown = 0;
				break;
			};
			case EAICommunicationState.SPEAKING :
			{
				m_fSoundCountDown = SAMPLE_LENGTH; // TODO: now length of the sound is fixed, should be taken from data				
				break;
			};			
		}
		m_iAICommState = newState;
		m_OnCommunicationStateChanged.Invoke(newState);
	}
	
	//-------------------------------------------------------------------------------------------------------------
	void Mute(bool enableMute)
	{
		if (enableMute && m_iAICommState == EAICommunicationState.SPEAKING)
			CommStateTransition(EAICommunicationState.IDLE);
		m_bMuted = enableMute;				
	}
	
	//-------------------------------------------------------------------------------------------------------------
	bool StartSpeaking()
	{
		CommStateTransition(EAICommunicationState.SPEAKING);
		return m_iAICommState == EAICommunicationState.SPEAKING;
	}
	
	//-------------------------------------------------------------------------------------------------------------
	SCR_AIOnCommunicationStateChanged GetCommStateChanged()
	{
		return m_OnCommunicationStateChanged;
	}
}