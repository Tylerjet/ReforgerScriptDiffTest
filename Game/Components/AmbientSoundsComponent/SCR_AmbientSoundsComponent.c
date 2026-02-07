enum EQueryType
{
	TreeBush,
	TreeLeafy,
	TreeLeafyDomestic,
	TreeConifer,
	TreeStump,
	TreeWithered,
	Building,
};

enum ELocalAmbientSignal
{
	AboveFreshWater,
	SoundName,
	EntitySize,
	AboveTerrain,
	AboveSea,
	SunAngle,
	EnvironmentType,
	Seed,
	Distance
};

enum EOutputStateSignal
{
	AboveSea,
	AboveFreshWater,
};

[EntityEditorProps(category: "GameScripted/Sound", description: "THIS IS THE SCRIPT DESCRIPTION.", color: "0 0 255 255")]
class SCR_AmbientSoundsComponentClass: AmbientSoundsComponentClass
{
};
		
//------------------------------------------------------------------------------------------------
class SCR_AmbientSoundsComponent : AmbientSoundsComponent
{															
	[Attribute()]
	ref array<ref SCR_AmbientSoundsEffect> m_aAmbientSoundsEffect;
								
	// Components
	private SignalsManagerComponent m_LocalSignalsManager;
			
	// Constants
	private const int QUERY_RADIUS = 25;
	private const int QUERY_PROCESSING_INTERVAL = 2000;
	private const int QUERY_MINIMUM_MOVE_DISTANCE_SQ = 2;
	private const int QUERY_TYPES = 7;
    private const int INVALID = -1;
    const int WINDSPEED_MIN = 2;
    const int WINDSPEED_MAX = 12;
	private float m_fQueryTimer = QUERY_PROCESSING_INTERVAL;
	private float m_fUpdateTimer;
	private const int UPDATE_PROCESSING_INTERVAL = 300;
	private const int LOOPED_SOUND_MINIMUM_MOVE_DISTANCE_SQ = 2;

	// Misc
	private BaseWorld m_World;
	private int m_iEnvironmentTypeSignalValue;
	private float m_fWorldTime;
	private vector m_vCameraPosFrame;
	private vector m_vCameraPosQuery;
	private vector m_vCameraPosLoopedSound;
	
	// Looped sounds pool
	private ref array<ref SCR_AudioHandleLoop> m_aAudioHandleLoop = {};
	//! Stores entity counds for all EQueryType
	private ref array<int> m_aQueryTypeCount = new array<int>;		
	//! AmbientEntity Signal's manager signals indexes
	private ref array<int> m_aLocalAmbientSignalIndex = new array<int>;	
	//! Output state signls indexes
	private ref array<int> m_aOutputStateSignalIndex = new array<int>;
			
	//------------------------------------------------------------------------------------------------
	/*
	Does QueryBySphere and stores all needed data into prepared structures
	*/
	private void HandleQueryEntities()
	{		
		// Limit processing by time and moved distance
		if (m_fWorldTime < m_fQueryTimer)
			return;
		
		if (vector.DistanceSqXZ(m_vCameraPosQuery, m_vCameraPosFrame) < QUERY_MINIMUM_MOVE_DISTANCE_SQ)
			return;
		
		m_fQueryTimer = m_fWorldTime + QUERY_PROCESSING_INTERVAL;							
		m_vCameraPosQuery = m_vCameraPosFrame;
				
		// Get new query values		
		QueryAmbientSoundsBySphere(QUERY_RADIUS);
		GetAmbientSoundsCountPerType(m_aQueryTypeCount);
										
		// Get spawn preset = EnvironmentType
		if (m_aLocalAmbientSignalIndex[ELocalAmbientSignal.EnvironmentType] != INVALID)
			m_iEnvironmentTypeSignalValue = m_LocalSignalsManager.GetSignalValue(m_aLocalAmbientSignalIndex[ELocalAmbientSignal.EnvironmentType]);
	}
		
	//------------------------------------------------------------------------------------------------
	int GetEnvironmentType()
	{
		return m_iEnvironmentTypeSignalValue;
	}
	
	//------------------------------------------------------------------------------------------------
	ETreeSoundTypes GetDominantTree()
	{
		if (m_aQueryTypeCount[EQueryType.TreeLeafy] + m_aQueryTypeCount[EQueryType.TreeConifer] == 0)
			return ETreeSoundTypes.None;
		if (m_aQueryTypeCount[EQueryType.TreeLeafy] > m_aQueryTypeCount[EQueryType.TreeConifer])
			return EQueryType.TreeLeafy;
		else
			return EQueryType.TreeConifer;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
		Uses linear interpolation to get value from curve
		\time Range <0, 1>
		\curve Curve
		\return value from curve
	*/									
	static float GetPoint(float time, Curve curve)
	{		
		time = Math.Clamp(time, 0, 1);
		
		int size = curve.Count();
		int i;
		
		for (i = 0; i < size; i++)
		{
			if (curve[i][0] > time)
				break; 
		}
		
		// One point defined or value befor first point	
		if (i == 0)
			return curve[i][1];	
		else if (i >= size - 1)
			return curve[size - 1][1];
		else
		{
			if (curve[i-1][1] == curve[i][1])
				return curve[i][1];
			else					
				return Interpolate(time, curve[i-1][0], curve[i][0], curve[i-1][1], curve[i][1]);
		}
	}

	//------------------------------------------------------------------------------------------------	
	private static float Interpolate(float in, float Xmin, float Xmax, float Ymin, float Ymax)
	{
		if (in <= Xmin)
			return Ymin;			
		if (in >= Xmax)
			return Ymax;
			
		return ((Ymin * (Xmax - in) + Ymax * (in - Xmin)) / (Xmax - Xmin));		
	}
	
	//------------------------------------------------------------------------------------------------	
	/*!
		Use to play sound events that has looped banks
		\soundEvent Sound event name
		\transformation Sound position
	*/
	
	SCR_AudioHandleLoop SoundEventLooped(string soundEvent, vector transformation[4])
	{
		SCR_AudioHandleLoop audioHandleLoop = new SCR_AudioHandleLoop;
		
		audioHandleLoop.m_aMat = transformation;
		audioHandleLoop.m_sSoundEvent = soundEvent;
		
		m_aAudioHandleLoop.Insert(audioHandleLoop);
				
		return audioHandleLoop;
	}
	
	//------------------------------------------------------------------------------------------------	
	/*!
		Use to terminate looped sounds that were triggered using SoundEventLooped()
	*/	
	void TerminateLooped(SCR_AudioHandleLoop audioHandleLoop)
	{
		if (!audioHandleLoop)
			return;
		
		Terminate(audioHandleLoop.m_AudioHandle);
		m_aAudioHandleLoop.RemoveItem(audioHandleLoop);
	}
		
	//------------------------------------------------------------------------------------------------	
	private void UpdateLoopedSounds()
	{
		foreach (SCR_AudioHandleLoop audioHandleLoop : m_aAudioHandleLoop)
		{
			if (IsFinishedPlaying(audioHandleLoop.m_AudioHandle))
				audioHandleLoop.m_AudioHandle = SoundEventTransform(audioHandleLoop.m_sSoundEvent, audioHandleLoop.m_aMat);
		}
	}
				
	//------------------------------------------------------------------------------------------------	
	override void UpdateSoundJob(IEntity owner, float timeSlice)
	{
		super.UpdateSoundJob(owner, timeSlice);
			
		m_vCameraPosFrame = GetCameraOrigin();	
		m_fWorldTime = m_World.GetWorldTime();
		
		// Handle looped sounds
		if (m_fWorldTime > m_fUpdateTimer)
		{
			// Handle looped sounds
			if (vector.DistanceSqXZ(m_vCameraPosLoopedSound, m_vCameraPosFrame) > LOOPED_SOUND_MINIMUM_MOVE_DISTANCE_SQ)
			{
				m_vCameraPosLoopedSound = m_vCameraPosFrame;
				UpdateLoopedSounds()
			}
														
			m_fUpdateTimer = m_fWorldTime + UPDATE_PROCESSING_INTERVAL;
		}
		
		foreach (SCR_AmbientSoundsEffect ambientSoundsEffect : m_aAmbientSoundsEffect)
			ambientSoundsEffect.Update(m_fWorldTime, m_vCameraPosFrame);
		
		HandleQueryEntities();

		// !!! Make sure, that 0 = default in case AmbientEntity is not present			
		if (m_aOutputStateSignalIndex[EOutputStateSignal.AboveSea] != INVALID)
			AudioSystem.SetOutputStateSignal(AudioSystem.DefaultOutputState, m_aOutputStateSignalIndex[EOutputStateSignal.AboveSea], m_LocalSignalsManager.GetSignalValue(m_aLocalAmbientSignalIndex[ELocalAmbientSignal.AboveSea]) - 0.1);
									
		if (m_aOutputStateSignalIndex[EOutputStateSignal.AboveFreshWater] != INVALID)
			AudioSystem.SetOutputStateSignal(AudioSystem.DefaultOutputState, m_aOutputStateSignalIndex[EOutputStateSignal.AboveFreshWater], m_LocalSignalsManager.GetSignalValue(m_aLocalAmbientSignalIndex[ELocalAmbientSignal.AboveFreshWater]));	

#ifdef ENABLE_DIAG
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_SOUNDS_RELOAD_AMBIENT_SOUNDS_CONFIGS))
		{
			foreach (SCR_AmbientSoundsEffect ambientSoundsEffect : m_aAmbientSoundsEffect)
				ambientSoundsEffect.ReloadConfig();
			
			DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_SOUNDS_RELOAD_AMBIENT_SOUNDS_CONFIGS, false);
		}
#endif		
	}
		
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{		
		super.OnPostInit(owner);
		
		// Get world
		m_World = owner.GetWorld();	
		if (!m_World)
			SetScriptedMethodsCall(false);	
				
		// Get local signals component		
		m_LocalSignalsManager = SignalsManagerComponent.Cast(owner.FindComponent(SignalsManagerComponent));		
		if (!m_LocalSignalsManager)
		{
			SetScriptedMethodsCall(false);
			Print("AUDIO: SCR_AmbientSoundsComponent: Missing SignalsManagerComponent", LogLevel.WARNING);
			return;
		}
											
		// Get Local Ambient Signal Idx		
		typename enumType = ELocalAmbientSignal;
		int size = enumType.GetVariableCount();
		
		for (int i = 0; i < size; i++)
			m_aLocalAmbientSignalIndex.Insert(m_LocalSignalsManager.AddOrFindSignal(typename.EnumToString(ELocalAmbientSignal, i)));
		
		// Get Output Stage Signals Idx
		enumType = EOutputStateSignal;
		size = enumType.GetVariableCount();
		
		for (int i = 0; i < size; i++)
			m_aOutputStateSignalIndex.Insert(AudioSystem.GetOutpuStateSignalIdx(0, typename.EnumToString(EOutputStateSignal, i)));
		
		foreach (SCR_AmbientSoundsEffect ambientSoundsEffect : m_aAmbientSoundsEffect)
			ambientSoundsEffect.OnPostInit(this, m_LocalSignalsManager);
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_AmbientSoundsComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		SetScriptedMethodsCall(true);
	
		m_aQueryTypeCount.Resize(QUERY_TYPES);
		
#ifdef ENABLE_DIAG
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_SOUNDS_RELOAD_AMBIENT_SOUNDS_CONFIGS, "", "Reload AmbientSounds Conf", "Sounds");
#endif	
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_AmbientSoundsComponent()
	{
#ifdef ENABLE_DIAG
		DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_SOUNDS_RELOAD_AMBIENT_SOUNDS_CONFIGS);
#endif	
	}
};