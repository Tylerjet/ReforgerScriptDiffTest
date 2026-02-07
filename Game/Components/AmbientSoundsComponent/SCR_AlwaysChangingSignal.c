[BaseContainerProps()]
class SCR_AlwaysChangingSignal
{
	//Inputs
	
	// Time in ms
	[Attribute("", UIWidgets.EditBox, "[ms]")]
	float m_fInterpolationTimeMin;
	
	[Attribute("", UIWidgets.EditBox, "[ms]")]
	float m_fInterpolationTimeMax;

	[Attribute("", UIWidgets.EditBox, "")]
	float m_fSignalValueMin;
	
	[Attribute("", UIWidgets.EditBox, "")]
	float m_fSignalValueMax;

	[Attribute("", UIWidgets.EditBox, "")]
	string m_sSignalName;
	
	int m_iSignalIdx;

	// Random number <m_fInterpolationTimeMin, m_fInterpolationTimeMax>
	float m_fInterpolationTime;

	// Random number <m_fSignalValueMin, m_fSignalValueMax>
	float m_fSignalTarget;
	float m_fSignalTargetLast;

	float m_fTimer;

	//------------------------------------------------------------------------------------------------
	void SCR_AlwaysChangingSignal()
	{
		m_fInterpolationTimeMin = m_fInterpolationTimeMin * 0.001;
		m_fInterpolationTimeMax = m_fInterpolationTimeMax * 0.001;		
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_AlwaysChangingSignal()
	{
	}

};
