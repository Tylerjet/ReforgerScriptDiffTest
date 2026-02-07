[BaseContainerProps(configRoot: true)]
class SCR_SoundHandle
{
	vector m_aMat[4];
	
	// soundType, soundEventGroup, soundEventDefinition
	int m_aSoundID[3];
			
	ref array<float> m_aRepTime = new array<float>;
	
	float m_fDensity;	
};