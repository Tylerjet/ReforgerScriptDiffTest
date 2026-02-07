enum ESoundSpawnType
{
	Entity,
	Top,
	TopMiddleX,
	TopMiddleZ,
};

[ComponentEditorProps(category: "GameScripted/Sound", description: "THIS IS THE SCRIPT DESCRIPTION.", color: "0 0 255 255")]
class SCR_BuildingSoundComponentClass: SoundComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_BuildingSoundComponent : SoundComponent
{
	[Attribute("0", UIWidgets.ComboBox, "", "", ParamEnumArray.FromEnum(ESoundSpawnType))]
	ESoundSpawnType m_eSoundSpawnType;
	
	[Attribute("10", UIWidgets.Slider, "Time interval when maximum WindSpeed [s]", "0 60 1")]
	int m_iTimeIntervalMin;
	
	[Attribute("30", UIWidgets.Slider, "Time interval when minimum WindSpeed [s]", "0 60 1")]
	int m_iTimeIntervalMax;
			
	[Attribute("100", UIWidgets.Slider, "Percentage amount of bouding box used for sound spawning", "0 100 1")]
	int m_iBoundingBoxUsage;
	
	static protected GameSignalsManager m_GlobalSignalsManager;
	
	static protected int m_iWindSpeedSignalIdx;
	
	private static const string SOUND_EVENT = "SOUND_CREAK";
	private static const float RANDOM_PERCENTAGE_MIN = 0.7;
	private static const float RANDOM_PERCENTAGE_MAX = 1.3;
	
	protected float m_fTimer;
	protected float m_fTriggerInterval;

	//------------------------------------------------------------------------------------------------				
	private void TriggeredSoundHandler(IEntity owner, float timeSlice)
	{	
		if (m_eSoundSpawnType != ESoundSpawnType.Entity)
		{ 
			vector mat[4];
			mat[3] = GetSoundPosition(owner);
			SetTransformation(mat);
		}

		SoundEvent(SOUND_EVENT);
		
		GetTimeInterval();
		m_fTimer = 0;
	}
	
	private void GetTimeInterval()
	{
		if (m_iWindSpeedSignalIdx == -1)
		{
			m_fTriggerInterval = Math.RandomFloat(m_iTimeIntervalMax * RANDOM_PERCENTAGE_MIN, m_iTimeIntervalMax * RANDOM_PERCENTAGE_MAX);
			return;
		}
		
		float windSpeed = m_GlobalSignalsManager.GetSignalValue(m_iWindSpeedSignalIdx);		
		float timeInterval = Interpolate(windSpeed, SCR_AmbientSoundsComponent.WINDSPEED_MIN, SCR_AmbientSoundsComponent.WINDSPEED_MAX, m_iTimeIntervalMax, m_iTimeIntervalMin);
			
		m_fTriggerInterval = Math.RandomFloat(timeInterval * RANDOM_PERCENTAGE_MIN, timeInterval * RANDOM_PERCENTAGE_MAX);
	}
	
	private vector GetSoundPosition(IEntity owner)
	{
		vector mins = vector.Zero;
		vector maxs = vector.Zero;						
		vector v;
		
		owner.GetBounds(mins, maxs);
		
		// Unused perfentage of bounding box <0, 1>
		float scaleFactor = (100 - m_iBoundingBoxUsage) * 0.005;
		
		switch (m_eSoundSpawnType)
		{
			case ESoundSpawnType.Top:
			{				
				v[0] = Math.RandomFloat(mins[0] + (maxs[0] - mins[0]) * scaleFactor, maxs[0] - (maxs[0] - mins[0]) * scaleFactor); 
				v[1] = maxs[1];
				v[2] = Math.RandomFloat(mins[2] + (maxs[2] - mins[2]) * scaleFactor, maxs[2] - (maxs[2] - mins[2]) * scaleFactor);
				break;
			};
			case ESoundSpawnType.TopMiddleX:
			{				
				v[0] = mins[0] + (maxs[0] - mins[0]) * Math.RandomFloat(scaleFactor, 1 - scaleFactor);
				v[1] = maxs[1];
				v[2] = mins[2] + (maxs[2] - mins[2]) * 0.5;			
				break;
			};
			case ESoundSpawnType.TopMiddleZ:
			{				
				v[0] = mins[0] + (maxs[0] - mins[0]) * 0.5; 
				v[1] = maxs[1];
				v[2] = mins[2] + (maxs[2] - mins[2]) * Math.RandomFloat(scaleFactor, 1 - scaleFactor);
				break;
			};
		};
		
		// From Local To World Space;	
		v = owner.CoordToParent(v);
			
		return v;	
	}
		
	private float Interpolate(float in, float Xmin, float Xmax, float Ymin, float Ymax)
	{
		if (in <= Xmin)
			return Ymin;
			
		if (in >= Xmax)
			return Ymax;
			
		return ((Ymin * (Xmax - in) + Ymax * (in - Xmin)) / (Xmax - Xmin));		
	}
		
	//------------------------------------------------------------------------------------------------		
	override void UpdateSoundJob(IEntity owner, float timeSlice)
	{
		m_fTimer += timeSlice;
	
		if (m_fTimer < m_fTriggerInterval)
			return;
		
		TriggeredSoundHandler(owner, timeSlice);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnUpdateSoundJobBegin(IEntity owner)
	{
		GetTimeInterval();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
			
		// Disable parameter
		#ifdef DISABLE_SCRIPTAMBIENTSOUNDS
			return;
		#endif
							
		// Get GlobalSignalsManager
		m_GlobalSignalsManager = GetGame().GetSignalsManager();
			
		// Get WindSpeed signal index
		m_iWindSpeedSignalIdx = m_GlobalSignalsManager.AddOrFindSignal("WindSpeed");
	}

	//------------------------------------------------------------------------------------------------
	void SCR_BuildingSoundComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		SetScriptedMethodsCall(true);	
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_BuildingSoundComponent()
	{
	}
};