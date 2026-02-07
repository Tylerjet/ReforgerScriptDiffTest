[EntityEditorProps(category: "GameScripted/Sound", description: "THIS IS THE SCRIPT DESCRIPTION.", color: "0 0 255 255")]
class SCR_AmbientSoundsComponentClass: AmbientSoundsComponentClass
{
};
		
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

enum EGlobalSignal
{
	GInterior,
	TimeOfDay,
	InEditor,
	WindSpeed,
	RainIntensity,
	GCurrVehicleCoverage,
	GIsThirdPersonCam,
	WindDir,
	DynamicRange
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

//------------------------------------------------------------------------------------------------
class SCR_AmbientSoundsComponent : AmbientSoundsComponent
{
	static const string AMBIENT_SOUNDS_ENTITY_NAME = "AmbientSoundsEntity";
	
	// Sound setup -----------------------------------------------------------------------	
	//Configs
														
	[Attribute("", UIWidgets.ResourceNamePicker, desc: "SoundInfo to ambient groups selector config", params: "conf")]
	private ResourceName m_3DTerrainDefinitionResource;
	
	[Attribute("", UIWidgets.ResourceNamePicker, desc: "Random positional sounds config", params: "conf")]
	private ResourceName m_3DSoundEventsResource;
	
	//Configs			
	private ref SCR_SoundTerrainDefinitionConfig m_SoundTerrainDefinitionConfig;
	private ref SCR_AmbientSoundsEventsConfig m_AmbientSoundsEventsConfig;			
					
	// ------------------------------------------------------------------------------------
	
	// Components
	protected GameSignalsManager m_GlobalSignalsManager;
	protected SignalsManagerComponent m_LocalSignalsManager;
					
	//Position randomization
	private const int REGION_ANGLE = 30;
	private const int REGION_COUNT = 12;
	
	//Looped sounds	
	private const int LOOP_SOUND_HEIGHT_LIMIT = 25;
	private const int LOOP_SOUND_COUNT = 9;
	
	private ref array<IEntity>  m_aClosestEntityID = new array<IEntity>;

	// Stores playing looped SoundHandles and coresponding entity
	private ref array<IEntity>  m_aLoopedSoundID = new array<IEntity>;
	private ref array<ref SCR_TreeFoliage>  m_aTreeFoliage = new array<ref SCR_TreeFoliage>;
	
	#ifdef ENABLE_DIAG
	private ref array<vector> m_aLoopedSoundPos = new array<vector>;
	private ref array<float> m_aLoopedSoundEntitySize = new array<float>;
	#endif
	
	// Constants		
	private const int QUERY_RADIUS = 25;
	private const int QUERY_PROCESSING_INTERVAL = 2;
	private const int QUERY_MINIMUM_MOVE_DISTANCE_SQ = 2;
	private const int TREE_LEAFY_HEIGHT_LIMIT = 10;
	private const int TREE_LEAFYDOMESTIC_HEIGHT_LIMIT = 6;
	private const int TREE_CONIFER_HEIGHT_LIMIT = 12;		
    private const int TRIGGERED_SOUND_PROCESSING_INTERVAL = 1000;
    const int WINDSPEED_MIN = 2;
    const int WINDSPEED_MAX = 12;
    private const int UPDATE_POSITION_THRESHOLD = 5;
    private const float BOUNDING_BOX_CROP_FACTOR = 0.3;
	private const float LAKE_AREA_LIMIT = 100000;
	
	// Timers
	protected float m_fTimerQuery;
	protected float m_fWorldTime;
	protected float m_fTriggeredSoundTimer;
	
	// Misc
	protected vector m_vCameraPosQuery;
	protected vector m_vCameraPosQueryLast;
			
	// Current query values
	private const int QUERY_TYPES = 7;

	protected ref array<int> m_aQueryTypeCount = new array<int>;
	protected ref array<int> m_aQueryTypeCountLast = new array<int>;
	
	protected int m_iTreeCount;

	protected BaseWorld m_World;
	
	private const vector TERRAIN_TRACE_LENGHT_HALF = "0 2 0";
		
	// Signals Idx
	protected ref array<int> m_aGlobalSignalIndex = new array<int>;	
	protected ref array<int> m_aLocalAmbientSignalIndex = new array<int>;	
	protected ref array<int> m_aOutputStateSignalIndex = new array<int>;

	protected float m_aDensityModifier[SOUND_TYPE_COUNT];
	
	protected float m_fAboveSeaSignalValue;

	vector m_vCameraPosFrame;
	
	// Random triggered sounds
	private float m_fTimeOfDay;
	
	// Number of playing sounds in give group
	protected float m_aDensity[SOUND_TYPE_COUNT];
	protected int m_iHandleSoundGroupIdx;
	protected int m_iSoundSpawnPresetIdx;
	
	ref array<ref SCR_SoundHandle> m_aSoundHandle = new array<ref SCR_SoundHandle>;
			
	// ------------------------------------------------------------------------------------
	// Debug Shapes -----------------------------------------------------------------------
	// ------------------------------------------------------------------------------------	
	
	#ifdef ENABLE_DIAG
	protected ref array<ref Shape> m_DebugShapes = new array<ref Shape>;
	protected ref array<ref DebugTextWorldSpace> m_DebugTextWorldSpace = new array<ref DebugTextWorldSpace>;
	
	private void DBG_Sphere(vector pos, int color, float size)
	{
		vector matx[4];
		Math3D.MatrixIdentity4(matx);
		matx[3] = pos;
		int shapeFlags = ShapeFlags.NOOUTLINE|ShapeFlags.NOZBUFFER|ShapeFlags.VISIBLE;
		Shape s = Shape.CreateSphere(color, shapeFlags, pos, size);
		s.SetMatrix(matx);
		m_DebugShapes.Insert(s);
	}
	
	private void DBG_Text(string text, vector pos)
	{
		int color = ARGB(255, 255, 255, 255);
		int bgcolor = ARGB(255, 0, 0, 0);
		
		DebugTextWorldSpace dtws = DebugTextWorldSpace.Create(m_World, "entitySize = " + text, DebugTextFlags.CENTER, pos[0], pos[1] + 0.5, pos[2], 15.0, color, bgcolor, 1000);
		
		m_DebugTextWorldSpace.Insert(dtws);
	}
	
	private float Log10(float in)
	{
		return Math.Log2(in) / Math.Log2(10);
	}
	
	private void Handle_DBG_Shere()
	{
		m_DebugShapes.Clear();
		m_DebugTextWorldSpace.Clear();
		
		int size = m_aLoopedSoundID.Count();
		
		for (int i = 0; i < size; i++)
		{	
			float gain = 20 * Log10(GetGain(m_aTreeFoliage[i].m_AudioHandle));
							
			DBG_Sphere(m_aLoopedSoundPos[i], ARGB( Interpolate(gain, -70, -20, 50, 180), 0, 127, 0), Interpolate(gain, -70, -20, 0.01, 0.5));
			
			DBG_Text(m_aLoopedSoundEntitySize[i].ToString(), m_aLoopedSoundPos[i]);	
		}
	}
	#endif
	
	// ------------------------------------------------------------------------------------
	// Query Ambient Signals --------------------------------------------------------------
	// ------------------------------------------------------------------------------------
	protected bool m_bQuery = false;
	
	private void HandleQueryEntities(float timeSlice)
	{
		m_bQuery = false;
		
		m_fTimerQuery += timeSlice;
		
		if (m_fTimerQuery < QUERY_PROCESSING_INTERVAL)
			return;
		
		m_fTimerQuery = 0;
							
		m_vCameraPosQuery = m_vCameraPosFrame;
				
		if (vector.DistanceSqXZ(m_vCameraPosQuery, m_vCameraPosQueryLast) < QUERY_MINIMUM_MOVE_DISTANCE_SQ)
		{
			m_fTimerQuery = QUERY_PROCESSING_INTERVAL;
			return;
		}
		
		// Set Last Query Values
		for (int i = 0; i < QUERY_TYPES; i++)
		{
			m_aQueryTypeCountLast[i] = m_aQueryTypeCount[i];
		}
				
		QueryAmbientSoundsBySphere(QUERY_RADIUS);
		GetAmbientSoundsCountPerType(m_aQueryTypeCount);
		
		m_iTreeCount = m_aQueryTypeCount[EQueryType.TreeLeafy] + m_aQueryTypeCount[EQueryType.TreeLeafyDomestic] + m_aQueryTypeCount[EQueryType.TreeConifer];
		
		m_vCameraPosQueryLast = m_vCameraPosQuery;
								
		// Get spawn preset = EnvironmentType
		m_iSoundSpawnPresetIdx = m_LocalSignalsManager.GetSignalValue(ELocalAmbientSignal.EnvironmentType);
				
		m_bQuery = true;
	}
		
	// -----------------------------------------------------------------------------------
	// Looped Vegetation Positional Sounds -----------------------------------------------
	// -----------------------------------------------------------------------------------
	
	private const int LOOPED_SOUNDS_PROCESSING_INTERVAL = 250;
	protected float m_fTimerLoopedSounds;
	protected vector m_vCameraPosLooped;
	protected vector m_vCameraPosLoopedLast;
	
	private void HandleLoopedSounds()
	{
		if (m_fWorldTime < m_fTimerLoopedSounds)
			return;
			
		m_vCameraPosLooped = m_vCameraPosFrame;
				
		if (vector.DistanceSqXZ(m_vCameraPosLooped, m_vCameraPosLoopedLast) < QUERY_MINIMUM_MOVE_DISTANCE_SQ)
		{
			return;
		}
		
		m_fTimerLoopedSounds = m_fWorldTime + LOOPED_SOUNDS_PROCESSING_INTERVAL;
		
		m_aClosestEntityID.Clear();
			
		// Get Bush sound count
		// Play at least 4 bush sounds. Use full LOOP_SOUND_COUNT limit when no tree is present.
		int bushLoopSoundLimit = LOOP_SOUND_COUNT - (m_aQueryTypeCount[EQueryType.TreeLeafy] + m_aQueryTypeCount[EQueryType.TreeLeafyDomestic]);
		if (bushLoopSoundLimit < 4)
			bushLoopSoundLimit = 4;
		
		//Get closest entities
		//TO DO: Change GetClosestEntities() in C++ so it can search through multiple EQueryTypes at once
		//TreeLeafyDomestic type is not used in data at this moment
		GetClosestEntities(EQueryType.TreeBush, bushLoopSoundLimit, m_aClosestEntityID);
		GetClosestEntities(EQueryType.TreeLeafy, LOOP_SOUND_COUNT, m_aClosestEntityID);	
		GetClosestEntities(EQueryType.TreeLeafyDomestic, LOOP_SOUND_COUNT, m_aClosestEntityID);
		
		// Stop sounds that are no longer among closest entities
		StopLoopedSounds(m_aLoopedSoundID, m_aClosestEntityID);
		
		// Play sounds on entities, that become closest				
		PlayLoopedSounds(m_aLoopedSoundID, m_aClosestEntityID);
		
		m_vCameraPosLoopedLast = m_vCameraPosFrame;
	}
	
	private void StopLoopedSounds(array<IEntity> entityWithSound, array<IEntity> closestEntity)
	{		
		int i = entityWithSound.Count();
				
		if (i > 0)
		{
			for (i; i > 0; i--)
			{				
				int index = closestEntity.Find(entityWithSound[i-1]);
			
				if (index == -1)
				{
					entityWithSound.Remove(i-1);
					Terminate(m_aTreeFoliage[i-1].m_AudioHandle);
					m_aTreeFoliage.Remove(i-1);
					
					#ifdef ENABLE_DIAG
					m_aLoopedSoundPos.Remove(i-1);
					m_aLoopedSoundEntitySize.Remove(i-1);
					#endif			
				}		
			}
		}	
	}
			
	private void PlayLoopedSounds(array<IEntity> entityWithSound, array<IEntity> closestEntity)
	{
		int size = closestEntity.Count();
		
		for (int i = 0; i < size; i++)
		{
			int index = entityWithSound.Find(closestEntity[i]);
			
			if (index == -1 && closestEntity[i] != null)
			{
				entityWithSound.Insert(closestEntity[i]);
				
				// Get world vounding box
				vector mins;
				vector maxs;			
				closestEntity[i].GetWorldBounds(mins, maxs);
				
				// Get entity height
				float height =  maxs[1] - mins[1];
				
				// Get treeFoliage height local
				Tree tree = Tree.Cast(closestEntity[i]);
				TreeClass treeClass = TreeClass.Cast(tree.GetPrefabData());
				float treeFoliageAboveGround = treeClass.m_iFoliageHeight * closestEntity[i].GetScale();
				float treeFoliageHeight = height - treeFoliageAboveGround;

				// Set engity size signal
				if (m_aLocalAmbientSignalIndex[ELocalAmbientSignal.EntitySize] != -1)
				{	
					m_LocalSignalsManager.SetSignalValue(m_aLocalAmbientSignalIndex[ELocalAmbientSignal.EntitySize], treeFoliageHeight);
				}
				
				// Set Tree treeFoliage	
				ref SCR_TreeFoliage treeFoliage = new SCR_TreeFoliage();
				vector mat[4];
				
				if (height < UPDATE_POSITION_THRESHOLD)
				{
					//Small tree
					
					//Get center of bouding box
					for (int j = 0; j <= 2; j++)
					{
						mat[3][j] = mins[j] + (maxs[j] - mins[j]) * 0.5;
					}
				}
				else
				{
					//Tall tree
										
					// Get average width
					float widthX = (maxs[0] - mins[0]) * 0.5;
					float widthZ = (maxs[2] - mins[2]) * 0.5;
					treeFoliage.m_fWidth = 0.5 * (widthX + widthZ) * BOUNDING_BOX_CROP_FACTOR;
					
					// Get max/min height
					treeFoliage.m_fMaxY = maxs[1];
					treeFoliage.m_fMinY = treeFoliage.m_fMaxY - treeFoliageHeight;
	
					// Get Center 2D
					vector center;
				
					center[0] = mins[0] + widthX;
					center[2] = mins[2] + widthZ;
					
					treeFoliage.m_vCenter = center;
					
					// Set update position flag
					treeFoliage.m_bUpdatePosition = true;
					
					// Set sound position
					mat[3] = GetSoundPosition(treeFoliage);
				}
				
				// Get event name
				string eventName;
				
				if (treeClass.SoundType == 2 || treeClass.SoundType == 6)
				{					
					if (treeFoliageAboveGround < 3)
						eventName = SCR_SoundEvent.SOUND_LEAFYTREE_SMALL_LP;
					else if (treeFoliageAboveGround < 5)
						eventName = SCR_SoundEvent.SOUND_LEAFYTREE_MEDIUM_LP;
					else if (treeFoliageAboveGround < 7)
						eventName = SCR_SoundEvent.SOUND_LEAFYTREE_LARGE_LP;
					else
						eventName = SCR_SoundEvent.SOUND_LEAFYTREE_VERYLARGE_LP;
				}
				else
				{
					// Set Seed signal
					m_LocalSignalsManager.SetSignalValue(m_aLocalAmbientSignalIndex[ELocalAmbientSignal.Seed], VectorToRandomNumber(mat[3]));
					
					eventName = SCR_SoundEvent.SOUND_BUSH_LP;
				}
														
				// Set sound position							
				SetTransformation(mat);
				
				// Play sound					
				treeFoliage.m_AudioHandle = SoundEvent(eventName);
				m_aTreeFoliage.Insert(treeFoliage);
							
				#ifdef ENABLE_DIAG
				m_aLoopedSoundPos.Insert(mat[3]);										
				m_aLoopedSoundEntitySize.Insert(height);
				#endif										
			}
		}
	}
	
	private void UpdateSoundPosition(array<IEntity> entityWithSound)
	{
		int size = entityWithSound.Count();
		
		for (int i = 0; i < size; i++)
		{
			if (m_aTreeFoliage[i].m_bUpdatePosition == true)
			{		
				vector mat[4];										
				mat[3] = GetSoundPosition(m_aTreeFoliage[i]);
	
				// Update sound position
				SetSoundTransformation(m_aTreeFoliage[i].m_AudioHandle, mat);
			}
		}
	}
	
	private vector GetSoundPosition(SCR_TreeFoliage treeFoliage)
	{
		vector position;
		
		// Set distance
		float cameraDistance =  vector.DistanceXZ(m_vCameraPosFrame, treeFoliage.m_vCenter);
			
		if (cameraDistance <= treeFoliage.m_fWidth || cameraDistance == 0)
			position = m_vCameraPosFrame;
		else
		{					
			vector cameraPos2D = vector.Zero;
			cameraPos2D[0] = m_vCameraPosFrame[0];
			cameraPos2D[2] = m_vCameraPosFrame[2];
			
			position = treeFoliage.m_vCenter + (cameraPos2D - treeFoliage.m_vCenter).Normalized() * treeFoliage.m_fWidth;
		}
			
		// Set height
		position[1] = Math.Max(treeFoliage.m_fMinY, m_vCameraPosFrame[1]);				
		if (m_vCameraPosFrame[1] > treeFoliage.m_fMaxY)
			position[1] = treeFoliage.m_fMaxY;
		
		return position;
	}
	
	private float VectorToRandomNumber(vector v)
	{		
		int i = v[0];
		int j = v[2];		
		int mod = (i * j) % 100;
		
		return Math.AbsInt(mod) * 0.01;
	}
							
	// ---------------------------------------------------------------------------------------
	// Triggered Positional Sounds -----------------------------------------------------------
	// ---------------------------------------------------------------------------------------
	
	private void TriggeredSoundHandler(float timeSlice)
	{
		// Process every TRIGGERED_SOUND_PROCESSING_INTERVAL		
		if (m_fWorldTime < m_fTriggeredSoundTimer)
			return; 
				
		RemoveOutOfRangeEvent();
		
		// Get TimeOfDay signal
		m_fTimeOfDay = m_LocalSignalsManager.GetSignalValue(m_aLocalAmbientSignalIndex[ELocalAmbientSignal.SunAngle]) / 360;
	
		// Process one sound group per frame
		HandleSoundGroup(m_iHandleSoundGroupIdx);
		
		m_iHandleSoundGroupIdx ++;
		
		if (m_iHandleSoundGroupIdx == SOUND_TYPE_COUNT)
		{
			m_iHandleSoundGroupIdx = 0;
			m_fTriggeredSoundTimer = m_fWorldTime + TRIGGERED_SOUND_PROCESSING_INTERVAL;
			
			GetDensity();
						
			// Get Density Modifier			
			float modifier = GetPoint(m_fTimeOfDay, m_AmbientSoundsEventsConfig.m_TimeModifier);
			modifier = modifier * GetPoint(m_GlobalSignalsManager.GetSignalValue(m_aGlobalSignalIndex[EGlobalSignal.RainIntensity]), m_AmbientSoundsEventsConfig.m_RainModifier);
			
			//Update values of wind modifiers curves
			float windSpeedScaled = Interpolate01(m_GlobalSignalsManager.GetSignalValue(m_aGlobalSignalIndex[EGlobalSignal.WindSpeed]), WINDSPEED_MIN, WINDSPEED_MAX);			
			for (int i = 0; i < SOUND_WIND_CURVE_COUNT; i++)
			{
				m_AmbientSoundsEventsConfig.m_aWindModifier[i].m_fValue = GetPoint(windSpeedScaled, m_AmbientSoundsEventsConfig.m_aWindModifier[i].m_Curve);
			}
						
			// Get modifiers for all sound types			
			for (int i = 0; i < SOUND_TYPE_COUNT; i++)
			{
				m_aDensityModifier[i] = modifier * m_AmbientSoundsEventsConfig.m_aWindModifier[m_AmbientSoundsEventsConfig.m_aSoundSpawnDefinitionGroup[m_iSoundSpawnPresetIdx].m_aSoundSpawnDefinition[i].m_eWindModifier].m_fValue;
			}
			
			// Update values of day time curves
			m_AmbientSoundsEventsConfig.UpdateDayTimeCurveValue(m_fTimeOfDay);								
		}	
	}
	
	private void GetRepTime(out array<float> repTime, out float sequenceLenght, SCR_SequenceDefinition sequenceDefinition, float gameTime)
	{
		int startDelay = Math.RandomIntInclusive(sequenceDefinition.m_iStartDelay - sequenceDefinition.m_iStartDelayRnd , sequenceDefinition.m_iStartDelay + sequenceDefinition.m_iStartDelayRnd);
		
		repTime.Insert(gameTime + startDelay);
		
		int sequenceCount2 = Math.RandomIntInclusive(sequenceDefinition.m_iRepCount2 - sequenceDefinition.m_iRepCountRnd2, sequenceDefinition.m_iRepCount2 + sequenceDefinition.m_iRepCountRnd2);
		
		if (sequenceCount2 < 1)
			sequenceCount2 = 1;
		
		for (int i = 0; i < sequenceCount2; i++)
		{
		
			int sequenceCount1 = Math.RandomIntInclusive(sequenceDefinition.m_iRepCount1 - sequenceDefinition.m_iRepCountRnd1, sequenceDefinition.m_iRepCount1 + sequenceDefinition.m_iRepCountRnd1);
		
			if (sequenceCount1 < 1)
				sequenceCount1 = 1;
			
			for (int j = 0; j < sequenceCount1; j++)
			{			
				
				if (j < sequenceCount1 - 1)
				{
					int repTime1 = Math.RandomIntInclusive(sequenceDefinition.m_iRepTime1 - sequenceDefinition.m_iRepTimeRnd1, sequenceDefinition.m_iRepTime1 + sequenceDefinition.m_iRepTimeRnd1);
					repTime.Insert(repTime1);	
				}
			}
			
			if (i < sequenceCount2 - 1)
			{
				int repTime2 = Math.RandomIntInclusive(sequenceDefinition.m_iRepTime2 - sequenceDefinition.m_iRepTimeRnd2, sequenceDefinition.m_iRepTime2 + sequenceDefinition.m_iRepTimeRnd2);				
				repTime.Insert(repTime2);
			}			
		}
		
		int size = repTime.Count();
		
		for (int i = 1; i < size; i++)
		{
			repTime[i] = repTime[i] + repTime[i-1];				
		}
		
		sequenceLenght = repTime[size - 1] - gameTime;			
	}
	
	private void SoundEventSequence(int soundType, int soundGroup, int soundEvent, vector mat[], out float sequenceLenght)
	{
		ref SCR_SoundHandle soundHandle = new SCR_SoundHandle;
		
		soundHandle.m_aSoundID[0] = soundType;
		soundHandle.m_aSoundID[1] = soundGroup;
		soundHandle.m_aSoundID[2] = soundEvent;
		
		soundHandle.m_aMat[0] = mat[0];
		soundHandle.m_aMat[1] = mat[1];
		soundHandle.m_aMat[2] = mat[2];
		soundHandle.m_aMat[3] = mat[3];
		
		GetRepTime(soundHandle.m_aRepTime, sequenceLenght, m_AmbientSoundsEventsConfig.m_aSoundType[soundType].m_aSoundEventGroup[soundGroup].m_aSoundEventDefinition[soundEvent].m_SequenceDefinition, m_fWorldTime);
		
		soundHandle.m_fDensity = GetSequenceDensity(soundHandle.m_aRepTime, sequenceLenght, m_AmbientSoundsEventsConfig.m_aSoundType[soundType].m_aSoundEventGroup[soundGroup].m_aSoundEventDefinition[soundEvent]);
				
		m_aSoundHandle.Insert(soundHandle);
	}
	
	private void HandleSoundEventSequence(float timeSlice, float gameTime)
	{
		int size = m_aSoundHandle.Count();

		if (size > 0)
		{		
			for (int i = 0; i < size; i++)
			{
				if (gameTime > m_aSoundHandle[i].m_aRepTime[0])
				{
					SetTransformation(m_aSoundHandle[i].m_aMat);
					
					// Set SoundName signal
					if (m_aLocalAmbientSignalIndex[ELocalAmbientSignal.SoundName] != -1)
						m_LocalSignalsManager.SetSignalValue(m_aLocalAmbientSignalIndex[ELocalAmbientSignal.SoundName],m_AmbientSoundsEventsConfig.m_aSoundType[m_aSoundHandle[i].m_aSoundID[0]].m_aSoundEventGroup[m_aSoundHandle[i].m_aSoundID[1]].m_aSoundEventDefinition[m_aSoundHandle[i].m_aSoundID[2]].m_eSoundName);
										
					// Set Distance signal
					if (m_aLocalAmbientSignalIndex[ELocalAmbientSignal.Distance] != -1)
						m_LocalSignalsManager.SetSignalValue(ELocalAmbientSignal.Distance, vector.Distance(m_vCameraPosFrame, m_aSoundHandle[i].m_aMat[3]));
					
					SoundEvent(m_AmbientSoundsEventsConfig.m_aSoundType[m_aSoundHandle[i].m_aSoundID[0]].m_sSoundEvent);
						
					if (m_aSoundHandle[i].m_aRepTime.Count() > 1)
					{
						m_aSoundHandle[i].m_aRepTime.RemoveOrdered(0);
					}
					else
					{						
						m_aSoundHandle.RemoveOrdered(i);
												
						size--;
						i--;
					}
				}						
			}
		}
	}
		
	private void RemoveOutOfRangeEvent()
	{
		int size = m_aSoundHandle.Count();

		if (size > 0)
		{		
			for (int i = size - 1; i >= 0; i--)
			{
				float fDistance = vector.Distance(m_vCameraPosFrame, m_aSoundHandle[i].m_aMat[3]);

				if (m_AmbientSoundsEventsConfig.m_aSoundSpawnDefinitionGroup[m_iSoundSpawnPresetIdx].m_aSoundSpawnDefinition[m_aSoundHandle[i].m_aSoundID[0]].m_iPlayDistMin > fDistance || m_AmbientSoundsEventsConfig.m_aSoundSpawnDefinitionGroup[m_iSoundSpawnPresetIdx].m_aSoundSpawnDefinition[m_aSoundHandle[i].m_aSoundID[0]].m_iPlayDistMax < fDistance)
				{							
					m_aSoundHandle.Remove(i);			
				}					
			}
		}		
	}

	private void HandleSoundGroup(int soundType)
	{
		// Return, if density was reached
		// Config density modified by time, wind and rain modifiers
		if (m_aDensity[soundType] >= m_AmbientSoundsEventsConfig.m_aSoundSpawnDefinitionGroup[m_iSoundSpawnPresetIdx].m_aSoundSpawnDefinition[soundType].m_iDensityLimit * m_aDensityModifier[soundType])
			return;
		
		// Generate sound position		
		vector vPos;								
		int soundGroup = -1;
			
		if (soundType == 0)
		{	
			IEntity tree;
							
			if (m_iTreeCount != 0)
			{				
				// Choose dominant tree type
					
				if (m_aQueryTypeCount[3] >= m_aQueryTypeCount[1] && m_aQueryTypeCount[3] >=  m_aQueryTypeCount[2])
				{
					soundGroup = 0;		
				}
				else if (m_aQueryTypeCount[1] > m_aQueryTypeCount[3] && m_aQueryTypeCount[1] > m_aQueryTypeCount[2])
				{
					soundGroup = 1;
				}
				else
				{
					soundGroup = 2;
				}

				// Get random tree entity
																			
				switch (soundGroup)
				{
					// Conifer
					case 0:
					{
						tree = GetRandomTree(EQueryType.TreeConifer, TREE_CONIFER_HEIGHT_LIMIT);					
						break;
					};
					// Leafy
					case 1:
					{
						tree = GetRandomTree(EQueryType.TreeLeafy, TREE_LEAFY_HEIGHT_LIMIT);					
						break;
					};
					// Leafy Domestic
					case 2:
					{
						tree = GetRandomTree(EQueryType.TreeLeafyDomestic, TREE_LEAFYDOMESTIC_HEIGHT_LIMIT);					
						break;
					};
				}

				if (tree)
				{
					vPos = GetRandomSoundPositionOnEntity(tree);
				}
				else
				{
					soundGroup = -1;
				}

			}		
		}
		else
		{
			// Generate position based on random region
			
			int iRandomRegion = Math.RandomIntInclusive(0, 11);
			//float fDist = Math.RandomFloatInclusive(m_AmbientSoundsEventsConfig.m_aSoundSpawnDefinitionGroup[m_iSoundSpawnPresetIdx].m_aSoundSpawnDefinition[soundType].m_iPlayDistMin, m_AmbientSoundsEventsConfig.m_aSoundSpawnDefinitionGroup[m_iSoundSpawnPresetIdx].m_aSoundSpawnDefinition[soundType].m_iPlayDistMax);			
			float fDist = m_AmbientSoundsEventsConfig.m_aSoundSpawnDefinitionGroup[m_iSoundSpawnPresetIdx].m_aSoundSpawnDefinition[soundType].m_iSpawnDist;			
				
			vPos = GetVectorFromRegion(iRandomRegion, REGION_ANGLE) * fDist;
				
			vPos[0] = vPos[0] + m_vCameraPosFrame[0];
			vPos[1] = m_vCameraPosFrame[1];
			vPos[2] = vPos[2] + m_vCameraPosFrame[2];
							
			// Trace for terrain material and get coresponding soundGroup index		
			float fSoundHeight = -1;	
							
			soundGroup = GetESoundGroupFromTerrain(vPos, fSoundHeight);

			vPos[1] = fSoundHeight;		
		}
			
		if (soundGroup != -1)
		{											
			// UpdateAvailableEvents
			m_AmbientSoundsEventsConfig.m_aSoundType[soundType].m_aSoundEventGroup[soundGroup].GetAvailableEvents(m_fWorldTime, m_fTimeOfDay, m_AmbientSoundsEventsConfig.m_aDayTimeCurveValue);
			
			// Get Sound definition						
			int soundEvent = m_AmbientSoundsEventsConfig.m_aSoundType[soundType].m_aSoundEventGroup[soundGroup].GetRandomEventIdx();

			if (soundEvent != -1)
			{			
				// Play SoundEventSequence					
				vector mat[4];
				mat[3] = vPos;					
				float sequenceLenght;
					
				SoundEventSequence(soundType, soundGroup, soundEvent, mat, sequenceLenght);
													
				// Set CoolDownEnd
				float timeOfDayFactor = m_AmbientSoundsEventsConfig.m_aDayTimeCurveValue[m_AmbientSoundsEventsConfig.m_aSoundType[soundType].m_aSoundEventGroup[soundGroup].m_aSoundEventDefinition[soundEvent].m_eDayTimeCurve];
										
				timeOfDayFactor = (1 - Math.Clamp(timeOfDayFactor, 0, 1));
				timeOfDayFactor = Math.Pow(timeOfDayFactor, 3) * 4 + 1;
											
				if (m_AmbientSoundsEventsConfig.m_aSoundType[soundType].m_aSoundEventGroup[soundGroup].m_aSoundEventDefinition[soundEvent].m_bAddSequenceLength)
				{	
					m_AmbientSoundsEventsConfig.m_aSoundType[soundType].m_aSoundEventGroup[soundGroup].m_aSoundEventDefinition[soundEvent].m_fCoolDownEnd = m_fWorldTime + sequenceLenght + m_AmbientSoundsEventsConfig.m_aSoundType[soundType].m_aSoundEventGroup[soundGroup].m_aSoundEventDefinition[soundEvent].m_iCoolDown * timeOfDayFactor;
				}
				else
				{
					m_AmbientSoundsEventsConfig.m_aSoundType[soundType].m_aSoundEventGroup[soundGroup].m_aSoundEventDefinition[soundEvent].m_fCoolDownEnd = m_fWorldTime + m_AmbientSoundsEventsConfig.m_aSoundType[soundType].m_aSoundEventGroup[soundGroup].m_aSoundEventDefinition[soundEvent].m_iCoolDown * timeOfDayFactor;
				}					
			}			
		}
	}
				
	private vector GetVectorFromRegion(int region, int regionAngle)
	{	
		vector angle;
		angle[0] = regionAngle * (region + 0.5);
		
		return angle.AnglesToVector();
	}

	// Filter out Trees, Vehicles
	static bool PositionalSounds_FilterCallback(Class target, vector rayorigin, vector raydirection)
	{
		if (target.IsInherited(Tree) || target.IsInherited(Vehicle))
			return false;
		
		return true;
	}
	
	private ESoundGroup GetESoundGroup(int soundInfo)
	{
		int count = m_SoundTerrainDefinitionConfig.m_aSoundTerrainDefinition.Count();
		
		for (int i = 0; i < count; i++)
		{
			if (soundInfo >= m_SoundTerrainDefinitionConfig.m_aSoundTerrainDefinition[i].m_iRangeMin && soundInfo < m_SoundTerrainDefinitionConfig.m_aSoundTerrainDefinition[i].m_iRangeMax)
			{
				return m_SoundTerrainDefinitionConfig.m_aSoundTerrainDefinition[i].m_eSoundGroup;
			}
		}
		
		return -1;		
	}
	
	private int GetESoundGroupFromTerrain(vector worldPos, out float hitHeight)
	{	
		// Get surface height
		float surfaceHeight = m_World.GetSurfaceY(worldPos[0], worldPos[2]);		
		hitHeight = surfaceHeight;
		
		// Coast
		if (surfaceHeight >= -3 && surfaceHeight < 3)
		{		
			return -1;
		}
		
		worldPos[1] = surfaceHeight;
			
		EWaterSurfaceType waterSurfaceType;
		float lakeArea;
		float waterSurfaceY = GetWaterSurfaceY(worldPos, waterSurfaceType, lakeArea);
		
		// Ocean
		if (waterSurfaceType == 1)
		{	
			hitHeight = 0;
			return ESoundGroup.OCEAN;
		}
		// Pond
		else if (waterSurfaceType == 2)
		{
			if (lakeArea > LAKE_AREA_LIMIT)
			{
				hitHeight = waterSurfaceY;
				return ESoundGroup.POND;
			}
			
			return -1;
		}
		// River
		else if (waterSurfaceType == 3)
			return -1;
	
		// Get terrain soundInfo
		int soundInfo = -1;		
		TracePointToTerrain(worldPos, TERRAIN_TRACE_LENGHT_HALF, hitHeight, soundInfo);
		
		// Get soundGroup		
		ESoundGroup soundGroup = GetESoundGroup(soundInfo);
				
		return soundGroup;
	}
	
	private float GetWaterSurfaceY(vector worldPos, out EWaterSurfaceType waterSurfaceType, out float lakeArea)
	{		
		vector waterSurfacePos;
		vector transformWS[4];
		vector obbExtents;

		ChimeraWorldUtils.TryGetWaterSurface(m_World, worldPos, waterSurfacePos, waterSurfaceType, transformWS, obbExtents);
		lakeArea = obbExtents[0] * obbExtents[2];
		
		return waterSurfacePos[1];
	}
	
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
		{
			return curve[i][1];
		}		
		else if (i >= size - 1)
		{
			return curve[size - 1][1];
		}
		else
		{
			if (curve[i-1][1] == curve[i][1])
			{
				return curve[i][1];
			}
			else
			{		
				return Interpolate(time, curve[i-1][0], curve[i][0], curve[i-1][1], curve[i][1]);
			}
		}
	}
	
	static float Interpolate(float in, float Xmin, float Xmax, float Ymin, float Ymax)
	{
		if (in <= Xmin)
			return Ymin;
			
		if (in >= Xmax)
			return Ymax;
			
		return ((Ymin * (Xmax - in) + Ymax * (in - Xmin)) / (Xmax - Xmin));		
	}
	
	private float Interpolate01(float in, float Xmin, float Xmax)
	{
		if (in <= Xmin)
			return 0;
		
		if (in >= Xmax)
			return 1;
		
		return (in - Xmin) / (Xmax - Xmin);
	}
	
	private vector GetRandomSoundPositionOnEntity(IEntity entity)
	{
		vector mins;
		vector maxs;
		entity.GetWorldBounds(mins, maxs);				
				
		vector mat;							
			
		// Get position
		mat = entity.GetOrigin();
				
		// Set random height
		mat[1] = mat[1] + Math.RandomFloatInclusive((maxs[1] - mins[1])/2, maxs[1] - mins[1]);
		
		return mat;	
	}
	
	// Ratio between sequence length and total length of played samples
	
	private float GetSequenceDensity(array<float> repTime, float sequenceLenght, SCR_SoundEventDefinition eventDefinition)
	{
		if (sequenceLenght == 0)
			return 0;
		
		return Math.Clamp(eventDefinition.m_iSampleLength * repTime.Count() / sequenceLenght, 0, 1) * 100;
	}
	
	// Density sum for every sound type
	
	private void GetDensity()
	{		
		for (int i = 0; i < SOUND_TYPE_COUNT; i++)
		{
			m_aDensity[i] = 0;
		}
		
		int size = m_aSoundHandle.Count();
				
		for (int i = 0; i < size; i++)
		{
			m_aDensity[m_aSoundHandle[i].m_aSoundID[0]] = m_aDensity[m_aSoundHandle[i].m_aSoundID[0]] + m_aSoundHandle[i].m_fDensity;
		}	
	}
	
	// ---------------------------------------------------------------------------
	// Always changing signal ----------------------------------------------------
	// ---------------------------------------------------------------------------
	
	private void HandleAlwaysChangingSignal(float timeSlice)
	{
		foreach (SCR_AlwaysChangingSignal alwaysChangingSignal: m_AmbientSoundsEventsConfig.m_aAlwaysChangingSignal)
		{
			alwaysChangingSignal.m_fTimer += timeSlice;
			
			float fInterpolationTimeCurrent =  alwaysChangingSignal.m_fTimer / alwaysChangingSignal.m_fInterpolationTime;
						
			// New interpolation values
			if (fInterpolationTimeCurrent > 1)
			{
				alwaysChangingSignal.m_fTimer = 0;
				fInterpolationTimeCurrent = 0;
				alwaysChangingSignal.m_fSignalTargetLast = alwaysChangingSignal.m_fSignalTarget;
				alwaysChangingSignal.m_fInterpolationTime = Math.RandomFloat(alwaysChangingSignal.m_fInterpolationTimeMin, alwaysChangingSignal.m_fInterpolationTimeMax);
				alwaysChangingSignal.m_fSignalTarget = Math.RandomFloat(alwaysChangingSignal.m_fSignalValueMin, alwaysChangingSignal.m_fSignalValueMax);
			}
			m_LocalSignalsManager.SetSignalValue(alwaysChangingSignal.m_iSignalIdx, Math.Lerp(alwaysChangingSignal.m_fSignalTargetLast, alwaysChangingSignal.m_fSignalTarget, fInterpolationTimeCurrent));
		}		
	}
			
	// ---------------------------------------------------------------------------
	// Debug  --------------------------------------------------------------------
	// ---------------------------------------------------------------------------
	
	#ifdef ENABLE_DIAG
	private void GlobalSignalsDebug()
	{	
		DbgUI.Begin("Globals Sounds Signals");
		
		typename enumType = EGlobalSignal;
		int size = enumType.GetVariableCount();
		
		for (int i = 0; i < size; i++)
			DbgUI.Text(typename.EnumToString(EGlobalSignal, i) + ": " + m_GlobalSignalsManager.GetSignalValue(m_aGlobalSignalIndex[i]).ToString());
		
		DbgUI.End();
	}
	
	private void AmbientSignalsDebug()
	{	
		DbgUI.Begin("Ambient Sounds Signals");
		
		typename enumType = ELocalAmbientSignal;
		int size = enumType.GetVariableCount();
		
		for (int i = 0; i < size; i++)
			DbgUI.Text(typename.EnumToString(ELocalAmbientSignal, i) + ": " + m_LocalSignalsManager.GetSignalValue(m_aLocalAmbientSignalIndex[i]).ToString());
		
		DbgUI.Text("Spawn Preset     : " + typename.EnumToString(ESoundEnvironmentType, m_iSoundSpawnPresetIdx));
		
		DbgUI.End();
	}
	
	private void DensityDebug()
	{	
		DbgUI.Begin("Density");
				
		for (int i = 0; i < SOUND_TYPE_COUNT; i++)
			DbgUI.Text(typename.EnumToString(ESoundType, i) + ": " + m_aDensity[i].ToString() + " / " + (m_AmbientSoundsEventsConfig.m_aSoundSpawnDefinitionGroup[m_iSoundSpawnPresetIdx].m_aSoundSpawnDefinition[i].m_iDensityLimit * m_aDensityModifier[i]).ToString());

		DbgUI.End();
	}
	
	private void OutputStateSignalsDebug()
	{	
		DbgUI.Begin("Output State Signals");
				
		DbgUI.Text("GInterior: " + m_GlobalSignalsManager.GetSignalValue(m_aGlobalSignalIndex[EGlobalSignal.GInterior]).ToString());
		DbgUI.Text("GCurrVehicleCoverage: " + m_GlobalSignalsManager.GetSignalValue(m_aGlobalSignalIndex[EGlobalSignal.GCurrVehicleCoverage]).ToString());
		DbgUI.Text("GIsThirdPersonCam: " + m_GlobalSignalsManager.GetSignalValue(m_aGlobalSignalIndex[EGlobalSignal.GIsThirdPersonCam]).ToString());
		DbgUI.Text("AboveSea : " + m_LocalSignalsManager.GetSignalValue(m_aLocalAmbientSignalIndex[ELocalAmbientSignal.AboveSea]).ToString());
		DbgUI.Text("AboveFreshWater: " + m_LocalSignalsManager.GetSignalValue(m_aLocalAmbientSignalIndex[ELocalAmbientSignal.AboveFreshWater]).ToString());

		DbgUI.End();
	}
			
	const int STRING_LENGTH = 20;
	const int PANEL_WIDTH = 20;
	const int PANEL_HEIGHT = 20;
	
	private void RandomPositionalSoundDebug()
	{				
		int defaultColor = ARGB(255, 50, 50, 50);
		int blackColor = ARGB(255, 0, 0, 0);
		int availableColor = ARGB(255, 0, 0, 255);
		int playingColor = ARGB(255, 255, 255, 0);
		int wrongTimeColor = ARGB(255, 125, 125, 125);
		int inCooldownColor = ARGB(255, 255, 0, 255);
		
		DbgUI.Begin("Ambient Triggered Sounds");	
		
		DbgUI.Panel("av", PANEL_WIDTH, PANEL_HEIGHT, availableColor);
		DbgUI.SameLine();
		DbgUI.Text("Available");
		
		DbgUI.Panel("pl", PANEL_WIDTH, PANEL_HEIGHT, playingColor);
		DbgUI.SameLine();
		DbgUI.Text("Playing");	
		
		DbgUI.Panel("wt", PANEL_WIDTH, PANEL_HEIGHT, wrongTimeColor);
		DbgUI.SameLine();
		DbgUI.Text("WrongTime");	
		
		DbgUI.Panel("in", PANEL_WIDTH, PANEL_HEIGHT, inCooldownColor);
		DbgUI.SameLine();
		DbgUI.Text("InCooldown");	
				
		for (int soundType = 0; soundType < SOUND_TYPE_COUNT; soundType++)
		{				
			string name = typename.EnumToString(ESoundType, soundType);

			name = NormalizeLength(name);			
			name = soundType.ToString() + ". " + name;
			
			bool bSoundTypeChecked;
						
			DbgUI.Check(name, bSoundTypeChecked);
				
			int limitCount = 8;
			int playingCount = 0;
			
			for (int i = 0; i < m_aSoundHandle.Count(); i++)
			{
				if (m_aSoundHandle[i].m_aSoundID[0] == soundType)
					playingCount ++;
			}
			
			for (int i = 0; i < limitCount; i++)
			{
				int color = defaultColor;
				
				if (i < playingCount)
				{
					color = playingColor;
				}
				
				DbgUI.SameLine();		
				DbgUI.Panel(soundType.ToString() + "." + i.ToString(), PANEL_WIDTH, PANEL_HEIGHT, color);
			}
				
			if (bSoundTypeChecked)
			{					
				int soundGroupCount = m_AmbientSoundsEventsConfig.m_aSoundType[soundType].m_aSoundEventGroup.Count();
						
				for (int soundGroup = 0; soundGroup < soundGroupCount; soundGroup++)
				{				
					m_AmbientSoundsEventsConfig.m_aSoundType[soundType].m_aSoundEventGroup[soundGroup].GetAvailableEvents(m_fWorldTime, m_fTimeOfDay, m_AmbientSoundsEventsConfig.m_aDayTimeCurveValue);
					
					string soundGroupName = m_AmbientSoundsEventsConfig.m_aSoundType[soundType].m_aSoundEventGroup[soundGroup].m_sSoundEventGroupName;
					soundGroupName = NormalizeLength(soundGroupName);
					
					DbgUI.Panel(soundType.ToString() + soundGroup.ToString(), PANEL_WIDTH, PANEL_HEIGHT, blackColor);
					DbgUI.SameLine();
					bool bSoundGroupChecked;
					DbgUI.Check(soundType.ToString() + "." + soundGroup.ToString() + ". " + soundGroupName, bSoundGroupChecked);
					DbgUI.SameLine();		
					
					DbgUI.Text(soundType.ToString() + "." + soundGroup.ToString() + ". " + "Available :" + m_AmbientSoundsEventsConfig.m_aSoundType[soundType].m_aSoundEventGroup[soundGroup].m_aAvailableEvents.Count().ToString() + " of (" + m_AmbientSoundsEventsConfig.m_aSoundType[soundType].m_aSoundEventGroup[soundGroup].m_aSoundEventDefinition.Count() + ")");					
					
					if (bSoundGroupChecked)
					{
						int soundDefinitionCount = m_AmbientSoundsEventsConfig.m_aSoundType[soundType].m_aSoundEventGroup[soundGroup].m_aSoundEventDefinition.Count();
						
						for (int soundDefinition = 0; soundDefinition < soundDefinitionCount; soundDefinition++)
						{
							int color = defaultColor;
							string aditionalInfo = "";
							
							// -----------------------
							// -----Status panel------
							// -----------------------
														
							// Tag if sound is in cooldown
							if (m_AmbientSoundsEventsConfig.m_aSoundType[soundType].m_aSoundEventGroup[soundGroup].m_aSoundEventDefinition[soundDefinition].m_fCoolDownEnd > m_fWorldTime)
							{
								color = inCooldownColor;
								aditionalInfo = " | Coldown ends in : " + Math.Round((m_AmbientSoundsEventsConfig.m_aSoundType[soundType].m_aSoundEventGroup[soundGroup].m_aSoundEventDefinition[soundDefinition].m_fCoolDownEnd - m_fWorldTime) * 0.001).ToString();
								
								float timeOfDayFactor = m_AmbientSoundsEventsConfig.m_aDayTimeCurveValue[m_AmbientSoundsEventsConfig.m_aSoundType[soundType].m_aSoundEventGroup[soundGroup].m_aSoundEventDefinition[soundDefinition].m_eDayTimeCurve];
								timeOfDayFactor = (1 - Math.Clamp(timeOfDayFactor, 0, 1));
								timeOfDayFactor = Math.Pow(timeOfDayFactor, 3) * 4 + 1;
								timeOfDayFactor = Math.Round(timeOfDayFactor * 100) * 0.01;
								
								aditionalInfo = aditionalInfo + " | TFactor : " + timeOfDayFactor.ToString();							
							}
													
							// Tag sounds that can not play in given time of day
							if (m_AmbientSoundsEventsConfig.m_aDayTimeCurveValue[m_AmbientSoundsEventsConfig.m_aSoundType[soundType].m_aSoundEventGroup[soundGroup].m_aSoundEventDefinition[soundDefinition].m_eDayTimeCurve] <= 0)
							{
								color = wrongTimeColor;
							}
							
							// Tag available
							if (m_AmbientSoundsEventsConfig.m_aDayTimeCurveValue[m_AmbientSoundsEventsConfig.m_aSoundType[soundType].m_aSoundEventGroup[soundGroup].m_aSoundEventDefinition[soundDefinition].m_eDayTimeCurve] > 0 && m_AmbientSoundsEventsConfig.m_aSoundType[soundType].m_aSoundEventGroup[soundGroup].m_aSoundEventDefinition[soundDefinition].m_fCoolDownEnd <= m_fWorldTime)
							{
								color = availableColor;
							}
							
							DbgUI.Panel(soundType.ToString() + "." + soundGroup.ToString() + "." + soundDefinition.ToString(), PANEL_WIDTH * 2 + 5, PANEL_HEIGHT, color);
							
							// -----------------------
							// -Sound definition name-
							// -----------------------
							
							string soundDefinitionName = typename.EnumToString(ESoundName, m_AmbientSoundsEventsConfig.m_aSoundType[soundType].m_aSoundEventGroup[soundGroup].m_aSoundEventDefinition[soundDefinition].m_eSoundName);
							soundDefinitionName = NormalizeLength(soundDefinitionName);							
							DbgUI.SameLine();				
							DbgUI.Text(soundType.ToString() + "." + soundGroup.ToString() + "." + soundDefinition.ToString() + ". " + soundDefinitionName + " ");
							
							// -----------------------
							// - Number of instances -
							// -----------------------
							
							int instanceCount = 0;
							float playbackEnd = 0;
							
							foreach (SCR_SoundHandle sh: m_aSoundHandle)
							{
								if (sh.m_aSoundID[0] == soundType && sh.m_aSoundID[1] == soundGroup && sh.m_aSoundID[2] == soundDefinition)
								{
									instanceCount ++;
									
									if (playbackEnd < Math.Round(sh.m_aRepTime[sh.m_aRepTime.Count() - 1] - m_fWorldTime))
									{
										playbackEnd = Math.Round(sh.m_aRepTime[sh.m_aRepTime.Count() - 1] - m_fWorldTime);
									}
								}
							}
							
							playbackEnd = Math.Round(playbackEnd * 0.001);
							
							if (playbackEnd > 0)
							{
								aditionalInfo = aditionalInfo + " | Playback ends in : " + playbackEnd.ToString();
							}
							
							for (int j = 0; j < instanceCount; j++)
							{
								DbgUI.SameLine();
								DbgUI.Panel(soundType.ToString() + "." + soundGroup.ToString() + "." + soundDefinition.ToString() + "Play" + j.ToString(), PANEL_WIDTH, PANEL_HEIGHT, playingColor);
							}
							
							// -----------------------
							// -----Aditional info----
							// -----------------------
							
							DbgUI.SameLine();																
							DbgUI.Text(aditionalInfo);																		

						}						
					}					
				}
			}
		}
		
		DbgUI.End();
	}
	
	private string NormalizeLength(string name)
	{
		int length = name.Length();
		string s = name;
			
		if (length <= STRING_LENGTH)
		{
			for (int i = 0; i < (STRING_LENGTH - length); i++)
			{
				s = s + " ";
			}
		}
		else
		{
			s = s.Substring(0, STRING_LENGTH * 0.5 - 2) + "...." + s.Substring(length - STRING_LENGTH * 0.5, STRING_LENGTH * 0.5);
		}
				
		return s;
	}
	
	#endif
	
	//--- Editor signal
	protected void OnEditorOpen()
	{
		m_GlobalSignalsManager.SetSignalValue(m_aGlobalSignalIndex[EGlobalSignal.InEditor], 1);
	}
	protected void OnEditorClose()
	{
		m_GlobalSignalsManager.SetSignalValue(m_aGlobalSignalIndex[EGlobalSignal.InEditor], 0);
	}
	protected void OnEditorManagerInit(SCR_EditorManagerEntity editorManager)
	{
		editorManager.GetOnOpened().Insert(OnEditorOpen);
		editorManager.GetOnClosed().Insert(OnEditorClose);
	}
	protected void OnEditorManagerExit(SCR_EditorManagerEntity editorManager)
	{
		editorManager.GetOnOpened().Remove(OnEditorOpen);
		editorManager.GetOnClosed().Remove(OnEditorClose);
	}
		
	//--- Dynamic range signal
	
	protected void SetDynamicRangeValue()
	{	
		float value;
		BaseContainer settings = GetGame().GetGameUserSettings().GetModule("SCR_AudioSettings");    
		
    	if (settings)        
        	settings.Get("m_fDynamicRange", value);    
	
		m_GlobalSignalsManager.SetSignalValue(m_aGlobalSignalIndex[EGlobalSignal.DynamicRange], value * 0.01 - 1);
	}

	// ---------------------------------------------------------------------------
	// ---------------------------------------------------------------------------
	// ---------------------------------------------------------------------------
			
	override void UpdateSoundJob(IEntity owner, float timeSlice)
	{
		super.UpdateSoundJob(owner, timeSlice);
		
		if (!m_World)
			return;
		
		m_vCameraPosFrame = GetCameraOrigin();
		
		m_fWorldTime = m_World.GetWorldTime();
		
		// ---------------------------------------------------------------------------------------------
		// Query Ambient Sounds ------------------------------------------------------------------------
		// ---------------------------------------------------------------------------------------------
				
		HandleQueryEntities(timeSlice);
	
		if (!m_bQuery)
		{
			// ---------------------------------------------------------------------------------------------
			// Looped Positional Sounds --------------------------------------------------------------------
			// ---------------------------------------------------------------------------------------------
					
			HandleLoopedSounds();
			
			UpdateSoundPosition(m_aLoopedSoundID);
																										
			// ---------------------------------------------------------------------------------------------
			// Random Positional Sound ---------------------------------------------------------------------
			// ---------------------------------------------------------------------------------------------	
				
			HandleSoundEventSequence(timeSlice * 1000, m_fWorldTime);
			
			TriggeredSoundHandler(timeSlice * 1000);
												
			// Always changing signal ----------------------------------------------------------------------
			
			HandleAlwaysChangingSignal(timeSlice);
						
			// ---------------------------------------------------------------------------------------------
			// Set Output Stage Signals --------------------------------------------------------------------
			// ---------------------------------------------------------------------------------------------
			
			// !!! Make sure, that 0 = default in case AmbientEntity is not present
						
			m_fAboveSeaSignalValue = m_LocalSignalsManager.GetSignalValue(m_aLocalAmbientSignalIndex[ELocalAmbientSignal.AboveSea]);
			
			if (m_aOutputStateSignalIndex[EOutputStateSignal.AboveSea] != -1)
				AudioSystem.SetOutputStateSignal(AudioSystem.DefaultOutputState, m_aOutputStateSignalIndex[EOutputStateSignal.AboveSea], m_fAboveSeaSignalValue - 0.1);
									
			if (m_aOutputStateSignalIndex[EOutputStateSignal.AboveFreshWater] != -1)
				AudioSystem.SetOutputStateSignal(AudioSystem.DefaultOutputState, m_aOutputStateSignalIndex[EOutputStateSignal.AboveFreshWater], m_LocalSignalsManager.GetSignalValue(m_aLocalAmbientSignalIndex[ELocalAmbientSignal.AboveFreshWater]));
		}
												
		// Debug ---------------------------------------------------------------------------------------
				
		#ifdef ENABLE_DIAG
		bool ntag = DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_SOUNDS_GLOBAL_SIGNALS);
		if (ntag)
		{
			GlobalSignalsDebug();
		}		
		ntag = DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_SOUNDS_AMBIENT_SIGNALS);
		if (ntag)
		{
			AmbientSignalsDebug();
			RandomPositionalSoundDebug();
			DensityDebug();
			//Handle_DBG_Shere();
		}	
		#endif		
	}
		
	private ref map<ResourceName, SCR_SoundTerrainDefinitionConfig> m_SoundTerrainDefinitionConfigMap = new ref map<ResourceName, SCR_SoundTerrainDefinitionConfig>();
	private ref map<ResourceName, SCR_AmbientSoundsEventsConfig> m_AmbientSoundsEventsConfigMap = new ref map<ResourceName, SCR_AmbientSoundsEventsConfig>();

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{		
		super.OnPostInit(owner);
		
		// Set owner name		
		owner.SetName(AMBIENT_SOUNDS_ENTITY_NAME);
		
		// Disable parameter
		#ifdef DISABLE_SCRIPTAMBIENTSOUNDS
			return;
		#endif
					
		// Get components
		m_GlobalSignalsManager = GetGame().GetSignalsManager();
		
		GenericEntity genEnt = GenericEntity.Cast(owner);
		
		m_LocalSignalsManager = SignalsManagerComponent.Cast(genEnt.FindComponent(SignalsManagerComponent));		
		if (!m_LocalSignalsManager)
		{
			Print("AUDIO: SCR_AmbientSoundsComponent: Missing SignalsManagerComponent", LogLevel.WARNING);
			return;
		}
					
		// Get config			
		
		// ------
		if (m_3DTerrainDefinitionResource != string.Empty)
		{			
			SCR_SoundTerrainDefinitionConfig config = m_SoundTerrainDefinitionConfigMap.Get(m_3DTerrainDefinitionResource);
			if (!config)
			{
				Resource holder = BaseContainerTools.LoadContainer(m_3DTerrainDefinitionResource);
				if (!holder)
					return;
				
				m_SoundTerrainDefinitionConfig = SCR_SoundTerrainDefinitionConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(holder.GetResource().ToBaseContainer()));
			}		
		}
		
		if (!m_SoundTerrainDefinitionConfig)
		{
			Print("AUDIO: SCR_AmbientSoundsComponent: Missing Sound Definition Terrain Config", LogLevel.WARNING);
			return;
		}
		
		m_SoundTerrainDefinitionConfigMap.Clear();
		

		// ------
		if (m_3DSoundEventsResource != string.Empty)
		{			
			SCR_AmbientSoundsEventsConfig config = m_AmbientSoundsEventsConfigMap.Get(m_3DSoundEventsResource);
			if (!config)
			{
				Resource holder = BaseContainerTools.LoadContainer(m_3DSoundEventsResource);
				if (!holder)
					return;
				
				m_AmbientSoundsEventsConfig = SCR_AmbientSoundsEventsConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(holder.GetResource().ToBaseContainer()));
			}		
		}
		
		if (!m_AmbientSoundsEventsConfig)
		{
			Print("AUDIO: SCR_AmbientSoundsComponent: Missing Ambience Sound Events Config", LogLevel.WARNING);
			return;
		}
		
		m_AmbientSoundsEventsConfigMap.Clear();
			
		// Set OnFrame
		SetEventMask(owner, EntityEvent.FRAME | EntityEvent.INIT);
		
		m_vCameraPosFrame = GetCameraOrigin();
		
		// Get Global Signal Idx
		typename enumType = EGlobalSignal;
		int size = enumType.GetVariableCount();
		
		for (int i = 0; i < size; i++)
		{
			m_aGlobalSignalIndex.Insert(m_GlobalSignalsManager.AddOrFindSignal(typename.EnumToString(EGlobalSignal, i)));
		}
		
		// Get Local Ambient Signal Idx		
		enumType = ELocalAmbientSignal;
		size = enumType.GetVariableCount();
		
		for (int i = 0; i < size; i++)
		{
			m_aLocalAmbientSignalIndex.Insert(m_LocalSignalsManager.AddOrFindSignal(typename.EnumToString(ELocalAmbientSignal, i)));
		}
		
		// Get Output Stage Signals Idx
		enumType = EOutputStateSignal;
		size = enumType.GetVariableCount();
		
		for (int i = 0; i < size; i++)
		{
			m_aOutputStateSignalIndex.Insert(AudioSystem.GetOutpuStateSignalIdx(0, typename.EnumToString(EOutputStateSignal, i)));
		}
			
		m_World = owner.GetWorld();				
				
		// Init values for always changing signal
		foreach (SCR_AlwaysChangingSignal alwaysChangingSignal: m_AmbientSoundsEventsConfig.m_aAlwaysChangingSignal)
		{
			alwaysChangingSignal.m_fInterpolationTime = Math.RandomFloat(alwaysChangingSignal.m_fInterpolationTimeMin, alwaysChangingSignal.m_fInterpolationTimeMax);
			alwaysChangingSignal.m_fSignalTarget = Math.RandomFloat(alwaysChangingSignal.m_fSignalValueMin, alwaysChangingSignal.m_fSignalValueMax);
			alwaysChangingSignal.m_iSignalIdx = m_LocalSignalsManager.AddOrFindSignal(alwaysChangingSignal.m_sSignalName);
		}
		
		//--- Initialize editor manager
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (editorManager)
		{
			OnEditorManagerInit(editorManager);
		}
		else
		{
			SCR_EditorManagerCore editorManagerCore = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
			if (editorManagerCore) editorManagerCore.Event_OnEditorManagerInitOwner.Insert(OnEditorManagerInit);
		}
				
		// Event listeners for audio game settings
		GetGame().OnUserSettingsChangedInvoker().Insert(SetDynamicRangeValue);
		
		m_fTimerQuery = QUERY_PROCESSING_INTERVAL;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(IEntity owner)
	{
		super.OnInit(owner);
		
		// Set Dynamic range init signal value
		SetDynamicRangeValue();
	}

	//------------------------------------------------------------------------------------------------
	void SCR_AmbientSoundsComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		SetScriptedMethodsCall(true);
	
		m_aQueryTypeCountLast.Resize(QUERY_TYPES);
		
		// Set Query Values To 0
		for (int i = 0; i < QUERY_TYPES; i++)
		{
			m_aQueryTypeCount.Insert(0);
		}
		
		#ifdef ENABLE_DIAG
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_SOUNDS_GLOBAL_SIGNALS, "", "Global Signals", "Sounds");
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_SOUNDS_AMBIENT_SIGNALS, "", "Ambient Signals", "Sounds");
		#endif	
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_AmbientSoundsComponent()
	{
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (editorManager) OnEditorManagerExit(editorManager);
		
		SCR_EditorManagerCore editorManagerCore = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (editorManagerCore) editorManagerCore.Event_OnEditorManagerInitOwner.Remove(OnEditorManagerInit);
		
		#ifdef ENABLE_DIAG
			DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_SOUNDS_GLOBAL_SIGNALS);
			DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_SOUNDS_AMBIENT_SIGNALS);
		#endif
	}
};