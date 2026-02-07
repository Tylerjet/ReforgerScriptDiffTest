[BaseContainerProps()]
class SCR_SequenceDefinition
{
	[Attribute("5000", UIWidgets.Slider, "[ms]", "0 30000 1")]
	int m_iStartDelay;
	
	[Attribute("5000", UIWidgets.Slider, "[ms]", "0 30000 1")]
	int m_iStartDelayRnd;
		
	[Attribute("4", UIWidgets.Slider, "Repetition count 1", "0 10 1")]
	int m_iRepCount1;
	
	[Attribute("0", UIWidgets.Slider, "Repetition count randomization 1", "0 10 1")]
	int m_iRepCountRnd1;
	
	[Attribute("10000", UIWidgets.Slider, "[ms]", "0 30000 1")]
	int m_iRepTime1;
	
	[Attribute("500", UIWidgets.Slider, "[ms]", "0 30000 1")]
	int m_iRepTimeRnd1;
	
	[Attribute("1", UIWidgets.Slider, "Repetition count 2", "0 10 1")]
	int m_iRepCount2;
	
	[Attribute("0", UIWidgets.Slider, "Repetition count randomization 2", "0 10 1")]
	int m_iRepCountRnd2;
	
	[Attribute("0", UIWidgets.Slider, "[ms]", "0 30000 1")]
	int m_iRepTime2;
	
	[Attribute("0", UIWidgets.Slider, "[ms]", "0 30000 1")]
	int m_iRepTimeRnd2;
};