[BaseContainerProps(configRoot: true)]
class SCR_SoundHandle
{
	//! Sound transformation
	vector m_aMat[4];	
	//! soundType, soundEventGroup, soundEventDefinition
	int m_iSoundGroup;
	//! soundEventGroup
	int m_iSoundType;
	//! soundEventDefinition
	int m_iSoundDef;
	//! SoundDefinition
	SCR_SoundDef m_SoundDef;
	//! Sequence of worldTimes, when sound events will be triggered		
	ref array<int> m_aRepTime;
	//! Index of next worldTime, when event will be trigged
	int m_iRepTimeIdx;
	//! Percentage ratio between sequence length and total length of played samples
	float m_fDensity;
	
	//------------------------------------------------------------------------------------------------
	/*!
		Updates density
	*/
	void UpdateDensity(float sampleLenght, float worldTime)
	{		
		float sequenceLenght = GetSequenceLenght(worldTime);
		if (sequenceLenght == 0)
		{
			m_fDensity = 0;
			return;
		}

		m_fDensity = Math.Clamp(sampleLenght * m_aRepTime.Count() / sequenceLenght, 0, 1) * 100;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
		Returns sequence lenght
	*/
	float GetSequenceLenght(float worldTime)
	{
		int size = m_aRepTime.Count();
		if (size == 0)
			return 0;
		else
			return m_aRepTime[size - 1] - worldTime + m_SoundDef.m_iSampleLength;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetRandomInt(int value, int random)
	{
		if (random == 0)
			return value;
		
		int rnd = Math.RandomIntInclusive(value - random, value + random);
		
		if (rnd < 1)
			rnd = 1;
		
		return rnd;
	}
	
	//------------------------------------------------------------------------------------------------	
	void CreateSequence(array<ref SCR_SequenceDef> sequenceDefinition, int idx)
	{
		for (int i = 0, count = GetRandomInt(sequenceDefinition[idx].m_iRepCount, sequenceDefinition[idx].m_iRepCountRnd); i < count; i++)
		{
			if (i > 0)
		    	m_aRepTime.Insert(GetRandomInt(sequenceDefinition[idx].m_iRepTime, sequenceDefinition[idx].m_iRepTimeRnd) + m_aRepTime[m_aRepTime.Count() - 1]);
			
			int idxNew = idx - 1;
			if (idxNew >= 0)
				CreateSequence(sequenceDefinition, idxNew);
		}
	}
		
	//------------------------------------------------------------------------------------------------
	/*!
		Returns sequence lenght
	*/
	void UpdateRepTime(float gameTime)
	{				
		if (m_aRepTime)
			m_aRepTime.Clear();
		else
			m_aRepTime = {};
		
		m_aRepTime.Insert(gameTime + GetRandomInt(m_SoundDef.m_iStartDelay, m_SoundDef.m_iStartDelayRnd));
		
		if (m_SoundDef.m_aSequenceDef.Count() == 0)
			Print("AmbientSoundsComponent: " + typename.EnumToString(ESoundName, m_SoundDef.m_eSoundName) + " is missing sequence definition", LogLevel.WARNING);
		else	
			CreateSequence(m_SoundDef.m_aSequenceDef, m_SoundDef.m_aSequenceDef.Count() - 1);
	}
		
	//------------------------------------------------------------------------------------------------
	void SCR_SoundHandle(int soundGroupIdx, int soundTypeIdx, int soundDefIdx, vector mat[4], array<ref SCR_SoundGroup> soundGroup, float worldTime)
	{
		m_iSoundGroup = soundGroupIdx;
		m_iSoundType = soundTypeIdx;
		m_iSoundDef = soundDefIdx;	
		m_aMat = mat;	
		m_SoundDef = soundGroup[soundGroupIdx].m_aSoundType[soundTypeIdx].m_aSoundDef[soundDefIdx];		
		UpdateRepTime(worldTime);				
		UpdateDensity(m_SoundDef.m_iSampleLength, worldTime);
	}	
};