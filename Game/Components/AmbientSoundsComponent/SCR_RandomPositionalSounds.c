enum ESoundMapType
{
	MEADOW,
	FOREST,
	HOUSES,
	SEA,
	POND,
};

/*
Spawns sounds arround camera based on define behaviour
Basic building blocks are:
SCR_SoundGroup defines a group of sounds. E.g. birds playing on trees close to the camera, or insects, or birds playing in the distance
SCR_SoundType defines a group of sounds that can be played on a given location or entity. The definition varies based on the used ESpawnMethod
SCR_SoundDef defines basic sound behaviour such as the number of repetitions, or daytime/wind behaviour
*/
[BaseContainerProps(configRoot: true)]
class SCR_RandomPositionalSounds: SCR_AmbientSoundsEffect
{
	[Attribute()]
	private ref array<ref SCR_RandomPositionalSoundsDef> m_aRandomPositionalSoundsDef;
	
	[Attribute("", UIWidgets.ResourceNamePicker, desc: "SoundInfo to ambient groups selector config", params: "conf")]
	private ResourceName m_TerrainDefinitionResource;
		
	[Attribute("", UIWidgets.ResourceNamePicker, desc: "Daytime curves config", params: "conf")]
	private ResourceName m_DayTimeCurveResource;
	
	[Attribute("", UIWidgets.ResourceNamePicker, desc: "Wind modifier curves config", params: "conf")]
	private ResourceName m_WindCurveResource;
	
	// Configuration
	private ref array<ref SCR_SoundGroup> m_aSoundGroup = new array<ref SCR_SoundGroup>;
	private ref array<ref SCR_SpawnDef> m_aSpawnDef = new array<ref SCR_SpawnDef>;
	private ref array<ref SCR_TerrainDef> m_aTerrainDef;
	private ref array<ref SCR_DayTimeCurveDef> m_aDayTimeCurve;
	private ref array<ref SCR_WindCurveDef> m_aWindModifier;


	// Components
	private GameSignalsManager m_GlobalSignalsManager;
	private ChimeraWorld m_World;
	private SoundWorld m_SoundWorld;
	
	private const int PROCESSING_INTERVAL = 300;
	private const int INVALID = -1;
	private const int TREE_LEAFY_HEIGHT_LIMIT = 10;
	private const int TREE_CONIFER_HEIGHT_LIMIT = 12;
	private const int LAKE_AREA_LIMIT = 100000;
	private const vector TERRAIN_TRACE_LENGHT_HALF = "0 2 0";
	private const int COAST_HIGHT_LIMIT = 3;	
	private const string SUN_ANGLE_SIGNAL_NAME = "SunAngle";
	private const string SOUND_NAME_SIGNAL_NAME = "SoundName";
	private const string DISTANCE_SIGNAL_NAME = "Distance";
	private const string RAIN_INTENSITY_SIGNAL_NAME = "RainIntensity";
	private const string WIND_SPEED_SIGNAL_NAME = "WindSpeed";
	
	private int m_iSunAngleSignalIdx;
	private int m_iSoundNameSignalIdx;
	private int m_iDistanceSignalIdx;
	private int m_iRainIntensitySignalIdx;
	private int m_iWindSpeedSignalIdx;
	
	private int m_iUpdatedSoundGroupIdx;
	private float m_fTimer;
	private float m_fTimeOfDay;
	private ref array<int> m_aDensity = {};
	private ref array<float> m_aDensityModifier = {};
	private ref array<int> m_aAngleOffset = {};
	private float m_aDayTimeCurveValue[DAY_TIME_CURVE_COUNT];
	private float m_aWindCurveValue[WIND_CURVE_COUNT];
	
	private ref array<ref SCR_SoundHandle> m_aSoundHandle = new array<ref SCR_SoundHandle>;

	//------------------------------------------------------------------------------------------------
	/*
	Called by SCR_AmbientSoundComponent in UpdateSoundJob()
	*/	
	override void Update(float worldTime, vector cameraPos)
	{
		Process(worldTime, cameraPos);
		UpdateSoundSequence(worldTime, cameraPos);
#ifdef ENABLE_DIAG
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_SOUNDS_RANDOM_POSITIONAL_SOUNDS))
		{
			AmbientSignalsDebug();
			RandomPositionalSoundDebug(worldTime);
			DensityDebug(m_AmbientSoundsComponent.GetEnvironmentType());				
			PlaySound(worldTime);
		}	
#endif	
	}
	
	//------------------------------------------------------------------------------------------------		
	private void Process(float worldTime, vector cameraPos)
	{
		// Process every PROCESSING_INTERVAL		
		if (worldTime < m_fTimer)
			return; 
		
		m_fTimer = worldTime + PROCESSING_INTERVAL;
				
		RemoveOutOfRangeSound(cameraPos);
		UpdateSoundGroup(worldTime, cameraPos, m_iUpdatedSoundGroupIdx);
		
		m_iUpdatedSoundGroupIdx ++;		
		if (m_iUpdatedSoundGroupIdx < m_aSoundGroup.Count())
			return;
		
		m_iUpdatedSoundGroupIdx = 0;	

		// Get time of day based on sun position
		m_fTimeOfDay = m_LocalSignalsManager.GetSignalValue(m_iSunAngleSignalIdx) / 360;
		
		UpdateGlobalModifiers();									
	}
	
	//------------------------------------------------------------------------------------------------	
	private void UpdateGlobalModifiers()
	{
		UpdateDensity();
								
		//Update values of wind modifiers curves
		float windSpeedScaled = Interpolate01(m_GlobalSignalsManager.GetSignalValue(m_iWindSpeedSignalIdx), SCR_AmbientSoundsComponent.WINDSPEED_MIN, SCR_AmbientSoundsComponent.WINDSPEED_MAX);			
		for (int i = 0; i < WIND_CURVE_COUNT; i++)
			m_aWindCurveValue[i] = SCR_AmbientSoundsComponent.GetPoint(windSpeedScaled, m_aWindModifier[i].m_Curve);
					
		// Get modifiers for all sound types
		int environmentType = m_AmbientSoundsComponent.GetEnvironmentType();
					
		for (int i = 0, count = m_aSpawnDef.Count(); i < count; i++)
		{
			float modifier = SCR_AmbientSoundsComponent.GetPoint(m_fTimeOfDay, m_aSpawnDef[i].m_TimeModifier);
			modifier = modifier * SCR_AmbientSoundsComponent.GetPoint(m_GlobalSignalsManager.GetSignalValue(m_iRainIntensitySignalIdx), m_aSpawnDef[i].m_RainModifier);
			m_aDensityModifier[i] = modifier * m_aWindCurveValue[m_aSpawnDef[i].m_eWindModifier];
		}
		
		// Update values of day time curves
		UpdateDayTimeCurveValue(m_fTimeOfDay);	
	}
	
	//------------------------------------------------------------------------------------------------	
	private int GetDensityMax(int soundGroup, int environmentType)
	{
		if (environmentType == EEnvironmentType.MEADOW)
			return m_aSpawnDef[soundGroup].m_iMeadowDensityMax;
		else if (environmentType == EEnvironmentType.FOREST)
			return m_aSpawnDef[soundGroup].m_iForestDensityMax;		
		else if (environmentType == EEnvironmentType.HOUSES)
			return m_aSpawnDef[soundGroup].m_iHousesDensityMax;
		else
			return m_aSpawnDef[soundGroup].m_iSeaDensityMax;
	}
	
	//------------------------------------------------------------------------------------------------	
	private void UpdateSoundSequence(float worldTime, vector cameraPos)
	{	
		for (int i = m_aSoundHandle.Count() - 1; i >= 0; i--)
		{
			if (worldTime > m_aSoundHandle[i].m_aRepTime[m_aSoundHandle[i].m_iRepTimeIdx])
			{
				// Set SoundName signal
				if (m_iSoundNameSignalIdx != INVALID)
					m_LocalSignalsManager.SetSignalValue(m_iSoundNameSignalIdx, m_aSoundHandle[i].m_SoundDef.m_eSoundName);
																																																			
				// Set Distance signal
				if (m_iDistanceSignalIdx != INVALID)
					m_LocalSignalsManager.SetSignalValue(m_iDistanceSignalIdx, vector.Distance(cameraPos, m_aSoundHandle[i].m_aMat[3]));
				
				// Play sound event	
				m_AmbientSoundsComponent.SoundEventTransform(m_aSoundGroup[m_aSoundHandle[i].m_iSoundGroup].m_sSoundEvent, m_aSoundHandle[i].m_aMat);
				
				// Remove if last event in the sequence	
				m_aSoundHandle[i].m_iRepTimeIdx ++;										
				if (m_aSoundHandle[i].m_aRepTime.Count() == m_aSoundHandle[i].m_iRepTimeIdx)
					m_aSoundHandle.Remove(i);											
			}						
		}
	}

	//------------------------------------------------------------------------------------------------			
	private void RemoveOutOfRangeSound(vector cameraPos)
	{			
		for (int i = m_aSoundHandle.Count() - 1; i >= 0; i--)
		{
			if (!Math.IsInRange(vector.Distance(cameraPos, m_aSoundHandle[i].m_aMat[3]), m_aSpawnDef[m_aSoundHandle[i].m_iSoundGroup].m_iPlayDistMin, m_aSpawnDef[m_aSoundHandle[i].m_iSoundGroup].m_iPlayDistMax))					
				m_aSoundHandle.Remove(i);							
		}		
	}

	//------------------------------------------------------------------------------------------------	
	/*
	Attempts to find new position to play sound based on defined ESpawnMethod
	*/
	private void UpdateSoundGroup(float worldTime, vector camPos, int soundGroup)
	{
		int environmentType = m_AmbientSoundsComponent.GetEnvironmentType();
		// Return, if density was reached
		// Config density modified by time, wind and rain modifiers
		if (m_aDensity[soundGroup] >= GetDensityMax(soundGroup, environmentType) * m_aDensityModifier[soundGroup])
			return;
		
		// Generate sound position		
		vector vPos;								
		int soundType = INVALID;
			
		if (m_aSoundGroup[soundGroup].m_eSpawnMethod == ESpawnMethod.ENTITY)
		{				
			// Choose dominant tree type
			ETreeSoundTypes treeSoundType = m_AmbientSoundsComponent.GetDominantTree();					
			if (treeSoundType == EQueryType.TreeConifer)
			{					
				IEntity tree = m_AmbientSoundsComponent.GetRandomTree(EQueryType.TreeConifer, TREE_CONIFER_HEIGHT_LIMIT);
				if (!tree)
					return;
					
				soundType = 0;
				vPos = GetRandomPositionOnEntity(tree);			
			}
			else if  (treeSoundType == EQueryType.TreeLeafy)
			{
				IEntity tree = m_AmbientSoundsComponent.GetRandomTree(EQueryType.TreeLeafy, TREE_LEAFY_HEIGHT_LIMIT);
				if (!tree)
					return;
					
				soundType = 1;
				vPos = GetRandomPositionOnEntity(tree);
			}	
		}
		else if (m_aSoundGroup[soundGroup].m_eSpawnMethod == ESpawnMethod.TERRAIN)	
		{										
			vPos = GenerateRandomPosition(soundGroup, camPos);
			soundType = GetTerrainTypeFromTerrain(vPos);
		}	
		else if (m_aSoundGroup[soundGroup].m_eSpawnMethod == ESpawnMethod.SOUNDMAP)
		{			
			vPos = GenerateRandomPosition(soundGroup, camPos);
			soundType = GetSoundMapTypeFromTerrain(vPos);		
		}
			
		if (soundType != INVALID)
		{										
			// Get random sound definition
			int soundDef = GetRandomSoundDef(m_aSoundGroup[soundGroup].m_aSoundType[soundType], worldTime);
						
			if (soundDef != INVALID)
			{			
				// Create sound event sequence					
				vector mat[4];
				mat[3] = vPos;					
				
				ref SCR_SoundHandle soundHandle = new SCR_SoundHandle(soundGroup, soundType, soundDef, mat, m_aSoundGroup, worldTime);		
				m_aSoundHandle.Insert(soundHandle);
													
				// Set CoolDownEnd time
				float timeOfDayFactor = m_aDayTimeCurveValue[soundHandle.m_SoundDef.m_eDayTimeCurve];								
				timeOfDayFactor = (1 - Math.Clamp(timeOfDayFactor, 0, 1));
				timeOfDayFactor = Math.Pow(timeOfDayFactor, 3) * 4 + 1;
											
				if (soundHandle.m_SoundDef.m_bAddSequenceLength)
					soundHandle.m_SoundDef.m_fCoolDownEnd = worldTime + soundHandle.GetSequenceLenght(worldTime) + soundHandle.m_SoundDef.m_iCoolDown * timeOfDayFactor;
				else
					soundHandle.m_SoundDef.m_fCoolDownEnd = worldTime + soundHandle.m_SoundDef.m_iCoolDown * timeOfDayFactor;					
			}			
		}
	}

	//------------------------------------------------------------------------------------------------								
	private vector GenerateRandomPosition(int soundGroup, vector camPos)
	{
		// Angle for random position is rotated by 40 deg for each GenerateRandomPosition() call
		float angle = m_aAngleOffset[soundGroup] * 36;
		
		m_aAngleOffset[soundGroup] = m_aAngleOffset[soundGroup] + 1;
		if (m_aAngleOffset[soundGroup] > 9)
			m_aAngleOffset[soundGroup] = 0;	
				
		float fDist = m_aSpawnDef[soundGroup].m_iSpawnDist;												
		vector vPos = vector.FromYaw(angle) * fDist;
			
		return vPos + camPos;	
	}
	
	//------------------------------------------------------------------------------------------------						
	private int GetRandomSoundDef(SCR_SoundType soundType, float worldTime)
	{
		int size = soundType.m_aSoundDef.Count();
		if (size == 0)
			return INVALID;
		
		int offset = Math.RandomIntInclusive(0, size - 1);
		
		for (int i = 0; i < size; i++)
		{
			int idx = i + offset;
			if (idx >= size)
				idx -= size;
					
			if (m_aDayTimeCurveValue[soundType.m_aSoundDef[idx].m_eDayTimeCurve] > 0 && soundType.m_aSoundDef[idx].m_fCoolDownEnd < worldTime)
				return idx;
		}			
		
		return INVALID;
	}
	
	//------------------------------------------------------------------------------------------------	
	private ETerrainType GetTerrainType(int soundInfo)
	{		
		for (int i = 0, count = m_aTerrainDef.Count(); i < count; i++)
		{
			if (soundInfo >= m_aTerrainDef[i].m_iRangeMin && soundInfo < m_aTerrainDef[i].m_iRangeMax)
				return m_aTerrainDef[i].m_eTerrainType;
		}

		return INVALID;		
	}
	
	//------------------------------------------------------------------------------------------------		
	private ESoundMapType GetSoundMapTypeFromTerrain(inout vector worldPos)
	{
		// Check for fresh water
		float surfaceHeight = m_World.GetSurfaceY(worldPos[0], worldPos[2]);		
		worldPos[1] = surfaceHeight;
							
		EWaterSurfaceType waterSurfaceType;
		float lakeArea;
		float waterSurfaceY = GetWaterSurfaceY(worldPos, waterSurfaceType, lakeArea);
			
		switch (waterSurfaceType)
		{
			case EWaterSurfaceType.WST_NONE:
			{
				m_SoundWorld = m_World.GetSoundWorld();
				
				if(!m_SoundWorld)
					return INVALID;
				
				float sea, forest, city, meadow;
				m_SoundWorld.GetMapValuesAtPos(worldPos, sea, forest, city, meadow);
				
				// Get maximum
				float vals[] = { meadow, forest, city, sea };
				float v = vals[0];
				int idx;
				for (int i = 1; i < 4; i++)
				{
					if (vals[i] > v)
					{
						v = vals[i];
						idx = i;
					}
				}
				
				return idx;
			}
			case EWaterSurfaceType.WST_OCEAN:
			{
				worldPos[1] = 0;
				return ESoundMapType.SEA;
			}
			case EWaterSurfaceType.WST_POND:
			{
				if (lakeArea > LAKE_AREA_LIMIT)
				{
					worldPos[1] = waterSurfaceY;
					return ESoundMapType.POND;
				}
				else
					return INVALID;
			}
		}
		
		return INVALID;			
	}
	
	//------------------------------------------------------------------------------------------------		
	private ETerrainType GetTerrainTypeFromTerrain(inout vector worldPos)
	{	
		// Get surface height
		float surfaceHeight = m_World.GetSurfaceY(worldPos[0], worldPos[2]);		
		worldPos[1] = surfaceHeight;
		
		// Coast
		if (Math.AbsFloat(worldPos[1]) < COAST_HIGHT_LIMIT)	
			return INVALID;
					
		EWaterSurfaceType waterSurfaceType;
		float lakeArea;
		float waterSurfaceY = GetWaterSurfaceY(worldPos, waterSurfaceType, lakeArea);
		
		if (waterSurfaceType == EWaterSurfaceType.WST_OCEAN)
		{	
			worldPos[1] = 0;
			return ETerrainType.OCEAN;
		}
		else if (waterSurfaceType == EWaterSurfaceType.WST_POND)
		{
			if (lakeArea > LAKE_AREA_LIMIT)
			{
				worldPos[1] = waterSurfaceY;
				return ETerrainType.POND;
			}
			else
				return ETerrainType.POND_SMALL;
		}
		else if (waterSurfaceType == EWaterSurfaceType.WST_RIVER)
			return INVALID;
	
		// Trace and get soundInfo
		int soundInfo = INVALID;		
		
		float hitHeight = worldPos[1];
		m_AmbientSoundsComponent.TracePointToTerrain(worldPos, TERRAIN_TRACE_LENGHT_HALF, hitHeight, soundInfo);
		worldPos[1] = hitHeight;
		
		// Get soundGroup				
		return GetTerrainType(soundInfo);
	}
	
	//------------------------------------------------------------------------------------------------		
	private float GetWaterSurfaceY(vector worldPos, out EWaterSurfaceType waterSurfaceType, out float lakeArea)
	{		
		vector waterSurfacePos;
		vector transformWS[4];
		vector obbExtents;

		ChimeraWorldUtils.TryGetWaterSurface(m_World, worldPos, waterSurfacePos, waterSurfaceType, transformWS, obbExtents);
		lakeArea = obbExtents[0] * obbExtents[2];
		
		return waterSurfacePos[1];
	}

	//------------------------------------------------------------------------------------------------		
	private vector GetRandomPositionOnEntity(IEntity entity)
	{
		vector mins, maxs;
		entity.GetWorldBounds(mins, maxs);				
				
		vector mat;							
			
		// Get position
		mat = entity.GetOrigin();
				
		// Set random height
		mat[1] = mat[1] + Math.RandomFloatInclusive((maxs[1] - mins[1]) * 0.5, maxs[1] - mins[1]);
		
		return mat;	
	}
	
	//------------------------------------------------------------------------------------------------
	private void UpdateDensity()
	{		
		ref array<int> density = {};
		int count = m_aSpawnDef.Count();
		density.Resize(count);
						
		for (int i = 0, size = m_aSoundHandle.Count(); i < size; i++)
			density[m_aSoundHandle[i].m_iSoundGroup] = density[m_aSoundHandle[i].m_iSoundGroup] + m_aSoundHandle[i].m_fDensity;
		
		for (int i = 0; i < count; i++)
			m_aDensity[i] = (m_aDensity[i] + density[i]) * 0.5;
	}

	//------------------------------------------------------------------------------------------------	
	private float Interpolate01(float in, float Xmin, float Xmax)
	{
		if (in <= Xmin)
			return 0;
		
		if (in >= Xmax)
			return 1;
		
		return (in - Xmin) / (Xmax - Xmin);
	}
	//------------------------------------------------------------------------------------------------		
	private void UpdateDayTimeCurveValue(float time)
	{
		for (int i = 0; i < DAY_TIME_CURVE_COUNT; i++)
		{
			m_aDayTimeCurveValue[i] = SCR_AmbientSoundsComponent.GetPoint(time, m_aDayTimeCurve[i].m_Curve);
		}
	}
	
	//------------------------------------------------------------------------------------------------	
	private void UpdateSignlsIdx()
	{
		m_iSunAngleSignalIdx = m_LocalSignalsManager.AddOrFindSignal(SUN_ANGLE_SIGNAL_NAME);
		m_iSoundNameSignalIdx = m_LocalSignalsManager.AddOrFindSignal(SOUND_NAME_SIGNAL_NAME);
		m_iDistanceSignalIdx = m_LocalSignalsManager.AddOrFindSignal(DISTANCE_SIGNAL_NAME);
		
		m_iRainIntensitySignalIdx = m_GlobalSignalsManager.AddOrFindSignal(RAIN_INTENSITY_SIGNAL_NAME);
		m_iWindSpeedSignalIdx = m_GlobalSignalsManager.AddOrFindSignal(WIND_SPEED_SIGNAL_NAME);	
	}

	//------------------------------------------------------------------------------------------------
	private void LoadConfigs(SCR_AmbientSoundsComponent ambientSoundsComponent)
	{
		foreach (SCR_RandomPositionalSoundsDef def: m_aRandomPositionalSoundsDef)
		{
			SCR_SoundGroup soundGrop;
			if (def.m_SoundGroupResource != string.Empty)
			{			
				Resource holder = BaseContainerTools.LoadContainer(def.m_SoundGroupResource);
				if (holder)
					soundGrop = SCR_SoundGroup.Cast(BaseContainerTools.CreateInstanceFromContainer(holder.GetResource().ToBaseContainer()));	
			}
			
			if (!soundGrop)
			{
				ambientSoundsComponent.SetScriptedMethodsCall(false);
				Print("AUDIO: SCR_AmbientSoundsComponent: Missing Ambience Sound Gropu Config", LogLevel.WARNING);
				return;
			}
						
			m_aSoundGroup.Insert(soundGrop);
			
			// ---
			SCR_SpawnDef spawnDef;
			if (def.m_SpawnDefResource != string.Empty)
			{			
				Resource holder = BaseContainerTools.LoadContainer(def.m_SpawnDefResource);
				if (holder)
					spawnDef = SCR_SpawnDef.Cast(BaseContainerTools.CreateInstanceFromContainer(holder.GetResource().ToBaseContainer()));	
			}
			
			if (!spawnDef)
			{
				ambientSoundsComponent.SetScriptedMethodsCall(false);
				Print("AUDIO: SCR_AmbientSoundsComponent: Missing Spawn Definition Config", LogLevel.WARNING);
				return;
			}
			
			m_aSpawnDef.Insert(spawnDef);
		}
		
		// ---
		ref SCR_TerrainDefConfig soundTerrainDefinitionConfig;								
		if (m_TerrainDefinitionResource != string.Empty)
		{
			Resource holder = BaseContainerTools.LoadContainer(m_TerrainDefinitionResource);
			if (holder)
				soundTerrainDefinitionConfig = SCR_TerrainDefConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(holder.GetResource().ToBaseContainer()));
		}			

		if (!soundTerrainDefinitionConfig)
		{
			ambientSoundsComponent.SetScriptedMethodsCall(false);
			Print("AUDIO: SCR_AmbientSoundsComponent: Missing Sound Definition Terrain Config", LogLevel.WARNING);
			return;
		}
		
		m_aTerrainDef = soundTerrainDefinitionConfig.m_aTerrainDef;

		// ---
		SCR_DayTimeCurveDefConfig ambientSoundsDayTimeCurveDefinitionConfig;
		if (m_DayTimeCurveResource != string.Empty)
		{			
			Resource holder = BaseContainerTools.LoadContainer(m_DayTimeCurveResource);
			if (holder)
				ambientSoundsDayTimeCurveDefinitionConfig = SCR_DayTimeCurveDefConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(holder.GetResource().ToBaseContainer()));	
		}
		
		if (!ambientSoundsDayTimeCurveDefinitionConfig)
		{
			ambientSoundsComponent.SetScriptedMethodsCall(false);
			Print("AUDIO: SCR_AmbientSoundsComponent: Missing Daytime Curve Deffinition Config", LogLevel.WARNING);
			return;
		}
		
		m_aDayTimeCurve = ambientSoundsDayTimeCurveDefinitionConfig.m_aDayTimeCurve;
		
		// ---
		SCR_WindCurveDefConfig ambientSoundsWindDefinitionConfig;
		if (m_WindCurveResource != string.Empty)
		{			
			Resource holder = BaseContainerTools.LoadContainer(m_WindCurveResource);
			if (holder)
				ambientSoundsWindDefinitionConfig = SCR_WindCurveDefConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(holder.GetResource().ToBaseContainer()));	
		}
		
		if (!ambientSoundsWindDefinitionConfig)
		{
			ambientSoundsComponent.SetScriptedMethodsCall(false);
			Print("AUDIO: SCR_AmbientSoundsComponent: Missing Wind Curve Definition Config", LogLevel.WARNING);
			return;
		}
		
		m_aWindModifier = ambientSoundsWindDefinitionConfig.m_aWindModifier;
	}
		
	//------------------------------------------------------------------------------------------------
#ifdef ENABLE_DIAG
	private const int STRING_LENGTH = 25;
	private const int PANEL_WIDTH = 20;
	private const int PANEL_HEIGHT = 20;
	
	//------------------------------------------------------------------------------------------------	
	void AmbientSignalsDebug()
	{	
		DbgUI.Begin("Ambient Sounds Signals", 178, 0);					
		DbgUI.Text(SUN_ANGLE_SIGNAL_NAME + ": " + m_LocalSignalsManager.GetSignalValue(m_iSunAngleSignalIdx).ToString());
		DbgUI.Text(SOUND_NAME_SIGNAL_NAME + ": " + m_LocalSignalsManager.GetSignalValue(m_iSoundNameSignalIdx).ToString());
		DbgUI.Text(DISTANCE_SIGNAL_NAME + ": " + m_LocalSignalsManager.GetSignalValue(m_iDistanceSignalIdx).ToString());		
		DbgUI.Text("Environent Type : " + typename.EnumToString(EEnvironmentType, m_AmbientSoundsComponent.GetEnvironmentType()));		
		DbgUI.End();
	}
	
	//------------------------------------------------------------------------------------------------	
	void DensityDebug(int environmentType)
	{	
		DbgUI.Begin("Density", 420, 0);							
		for (int i = 0; i < m_aSpawnDef.Count(); i++)
			DbgUI.Text(m_aSoundGroup[i].m_sSoundEvent + ": " + m_aDensity[i].ToString() + " / " + (Math.Round(GetDensityMax(i, environmentType) * m_aDensityModifier[i])).ToString());
		DbgUI.End();
	}
	
	//------------------------------------------------------------------------------------------------
	void RandomPositionalSoundDebug(float worldTime)
	{				
		int defaultColor = ARGB(255, 50, 50, 50);
		int blackColor = ARGB(255, 0, 0, 0);
		int availableColor = ARGB(255, 0, 0, 255);
		int playingColor = ARGB(255, 255, 255, 0);
		int wrongTimeColor = ARGB(255, 125, 125, 125);
		int inCooldownColor = ARGB(255, 255, 0, 255);
		
		DbgUI.Begin("Random Ambient Sounds", 0, 185);	
		
		DbgUI.Panel("av", PANEL_WIDTH, PANEL_HEIGHT, availableColor);
		DbgUI.SameLine();
		DbgUI.Text("Available");
		DbgUI.SameLine();
		DbgUI.Panel("pl", PANEL_WIDTH, PANEL_HEIGHT, playingColor);
		DbgUI.SameLine();
		DbgUI.Text("Playing");	
		DbgUI.SameLine();
		DbgUI.Panel("wt", PANEL_WIDTH, PANEL_HEIGHT, wrongTimeColor);
		DbgUI.SameLine();
		DbgUI.Text("WrongTime");	
		DbgUI.SameLine();
		DbgUI.Panel("in", PANEL_WIDTH, PANEL_HEIGHT, inCooldownColor);
		DbgUI.SameLine();
		DbgUI.Text("InCooldown");	
				
		for (int soundGroup = 0; soundGroup < m_aSpawnDef.Count(); soundGroup++)
		{				
			// Get SoundGropu name
			string name = m_aSoundGroup[soundGroup].m_sSoundEvent;
			name = NormalizeLength(name);			
			name = soundGroup.ToString() + ". " + name;
			
			// Show number of playing sounds
			int limitCount = 8;
			int playingCount = 0;
			
			bool bSoundTypeChecked = false;					
			DbgUI.Check(name, bSoundTypeChecked);
			
			for (int i = 0; i < m_aSoundHandle.Count(); i++)
			{
				if (m_aSoundHandle[i].m_iSoundGroup == soundGroup)
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
				DbgUI.Panel(soundGroup.ToString() + "." + i.ToString(), PANEL_WIDTH, PANEL_HEIGHT, color);
			}
				
			if (bSoundTypeChecked)
			{					
				int soundTypeCount = m_aSoundGroup[soundGroup].m_aSoundType.Count();
						
				for (int soundType = 0; soundType < soundTypeCount; soundType++)
				{				
					//m_aSoundGroup[soundGroup].m_aSoundType[soundType].GetAvailableEvents(worldTime, m_fTimeOfDay, m_aDayTimeCurveValue);
					
					// Get SoundType name
					string soundTypeName;
					if (m_aSoundGroup[soundGroup].m_eSpawnMethod == ESpawnMethod.ENTITY)
					{
						if (soundType == 0)
							soundTypeName = "CONIFER";
						else
							soundTypeName = "LEAFY";	
					}
					else if (m_aSoundGroup[soundGroup].m_eSpawnMethod == ESpawnMethod.TERRAIN)
						soundTypeName = typename.EnumToString(ETerrainType, soundType);
					else
						soundTypeName = typename.EnumToString(ESoundMapType, soundType);
									
					soundTypeName = NormalizeLength(soundTypeName);
					
					DbgUI.Panel(soundGroup.ToString() + soundType.ToString(), PANEL_WIDTH, PANEL_HEIGHT, blackColor);
					DbgUI.SameLine();
					bool bSoundGroupChecked = false;
					DbgUI.Check(soundGroup.ToString() + "." + soundType.ToString() + ". " + soundTypeName, bSoundGroupChecked);
					DbgUI.SameLine();							
					
					int size = m_aSoundGroup[soundGroup].m_aSoundType[soundType].m_aSoundDef.Count();					
					int available = 0;
					for (int idx = 0; idx < size; idx++)
					{			
						if (m_aDayTimeCurveValue[m_aSoundGroup[soundGroup].m_aSoundType[soundType].m_aSoundDef[idx].m_eDayTimeCurve] > 0 && m_aSoundGroup[soundGroup].m_aSoundType[soundType].m_aSoundDef[idx].m_fCoolDownEnd < worldTime)
							available++;
					}			

					DbgUI.Text(soundGroup.ToString() + "." + soundType.ToString() + ". " + "Available :" + available.ToString() + " of (" + m_aSoundGroup[soundGroup].m_aSoundType[soundType].m_aSoundDef.Count() + ")");					
					
					if (bSoundGroupChecked)
					{
						int soundDefCount = m_aSoundGroup[soundGroup].m_aSoundType[soundType].m_aSoundDef.Count();
						
						for (int soundDef = 0; soundDef < soundDefCount; soundDef++)
						{
							int color = defaultColor;
							string aditionalInfo = "";
							
							// -----------------------
							// -----Status panel------
							// -----------------------
														
							// Tag if sound is in cooldown
							if (m_aSoundGroup[soundGroup].m_aSoundType[soundType].m_aSoundDef[soundDef].m_fCoolDownEnd > worldTime)
							{
								color = inCooldownColor;
								aditionalInfo = " | Coldown ends in : " + Math.Round((m_aSoundGroup[soundGroup].m_aSoundType[soundType].m_aSoundDef[soundDef].m_fCoolDownEnd - worldTime) * 0.001).ToString();
								
								float timeOfDayFactor = m_aDayTimeCurveValue[m_aSoundGroup[soundGroup].m_aSoundType[soundType].m_aSoundDef[soundDef].m_eDayTimeCurve];
								timeOfDayFactor = (1 - Math.Clamp(timeOfDayFactor, 0, 1));
								timeOfDayFactor = Math.Pow(timeOfDayFactor, 3) * 4 + 1;
								timeOfDayFactor = Math.Round(timeOfDayFactor * 100) * 0.01;
								
								aditionalInfo = aditionalInfo + " | TFactor : " + timeOfDayFactor.ToString();							
							}
													
							// Tag sounds that can not play in given time of day
							if (m_aDayTimeCurveValue[m_aSoundGroup[soundGroup].m_aSoundType[soundType].m_aSoundDef[soundDef].m_eDayTimeCurve] <= 0)
								color = wrongTimeColor;
							
							// Tag available
							if (m_aDayTimeCurveValue[m_aSoundGroup[soundGroup].m_aSoundType[soundType].m_aSoundDef[soundDef].m_eDayTimeCurve] > 0 && m_aSoundGroup[soundGroup].m_aSoundType[soundType].m_aSoundDef[soundDef].m_fCoolDownEnd <= worldTime)
								color = availableColor;
							
							DbgUI.Panel(soundGroup.ToString() + "." + soundType.ToString() + "." + soundDef.ToString(), PANEL_WIDTH * 2 + 5, PANEL_HEIGHT, color);
							
							// -----------------------
							// -Sound definition name-
							// -----------------------
							
							string soundDefinitionName = typename.EnumToString(ESoundName, m_aSoundGroup[soundGroup].m_aSoundType[soundType].m_aSoundDef[soundDef].m_eSoundName);
							soundDefinitionName = NormalizeLength(soundDefinitionName);							
							DbgUI.SameLine();				
							DbgUI.Text(soundGroup.ToString() + "." + soundType.ToString() + "." + soundDef.ToString() + ". " + soundDefinitionName + " ");
							
							// -----------------------
							// - Number of instances -
							// -----------------------
							
							int instanceCount = 0;
							float playbackEnd = 0;
							
							foreach (SCR_SoundHandle sh: m_aSoundHandle)
							{
								if (sh.m_iSoundGroup == soundGroup && sh.m_iSoundType == soundType && sh.m_iSoundDef == soundDef)
								{
									instanceCount ++;
									
									if (playbackEnd < Math.Round(sh.m_aRepTime[sh.m_aRepTime.Count() - 1] - worldTime))
									{
										playbackEnd = Math.Round(sh.m_aRepTime[sh.m_aRepTime.Count() - 1] - worldTime);
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
								DbgUI.Panel(soundGroup.ToString() + "." + soundType.ToString() + "." + soundDef.ToString() + "Play" + j.ToString(), PANEL_WIDTH, PANEL_HEIGHT, playingColor);
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

	//------------------------------------------------------------------------------------------------
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
			s = s.Substring(0, STRING_LENGTH * 0.5 - 2) + "...." + s.Substring(length - STRING_LENGTH * 0.5, STRING_LENGTH * 0.5);
				
		return s;
	}
		
	//------------------------------------------------------------------------------------------------
	override void ReloadConfig()
	{
		super.ReloadConfig();
		
		m_aSoundGroup.Clear();
		m_aSpawnDef.Clear();
		
		foreach (SCR_RandomPositionalSoundsDef def: m_aRandomPositionalSoundsDef)
		{
			SCR_SoundGroup soundGrop;
			if (def.m_SoundGroupResource != string.Empty)
			{			
				Resource holder = BaseContainerTools.LoadContainer(def.m_SoundGroupResource);
				if (holder)
					soundGrop = SCR_SoundGroup.Cast(BaseContainerTools.CreateInstanceFromContainer(holder.GetResource().ToBaseContainer()));	
			}
			
			if (soundGrop)						
				m_aSoundGroup.Insert(soundGrop);
			
			// ---
			SCR_SpawnDef spawnDef;
			if (def.m_SpawnDefResource != string.Empty)
			{			
				Resource holder = BaseContainerTools.LoadContainer(def.m_SpawnDefResource);
				if (holder)
					spawnDef = SCR_SpawnDef.Cast(BaseContainerTools.CreateInstanceFromContainer(holder.GetResource().ToBaseContainer()));	
			}
			
			if (spawnDef)	
				m_aSpawnDef.Insert(spawnDef);
		}
		
		// ---
		ref SCR_TerrainDefConfig soundTerrainDefinitionConfig;								
		if (m_TerrainDefinitionResource != string.Empty)
		{
			Resource holder = BaseContainerTools.LoadContainer(m_TerrainDefinitionResource);
			if (holder)
				soundTerrainDefinitionConfig = SCR_TerrainDefConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(holder.GetResource().ToBaseContainer()));
		}			

		if (soundTerrainDefinitionConfig)
			m_aTerrainDef = soundTerrainDefinitionConfig.m_aTerrainDef;

		// ---
		SCR_DayTimeCurveDefConfig ambientSoundsDayTimeCurveDefinitionConfig;
		if (m_DayTimeCurveResource != string.Empty)
		{			
			Resource holder = BaseContainerTools.LoadContainer(m_DayTimeCurveResource);
			if (holder)
				ambientSoundsDayTimeCurveDefinitionConfig = SCR_DayTimeCurveDefConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(holder.GetResource().ToBaseContainer()));	
		}
		
		if (!ambientSoundsDayTimeCurveDefinitionConfig)
			m_aDayTimeCurve = ambientSoundsDayTimeCurveDefinitionConfig.m_aDayTimeCurve;
		
		// ---
		SCR_WindCurveDefConfig ambientSoundsWindDefinitionConfig;
		if (m_WindCurveResource != string.Empty)
		{			
			Resource holder = BaseContainerTools.LoadContainer(m_WindCurveResource);
			if (holder)
				ambientSoundsWindDefinitionConfig = SCR_WindCurveDefConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(holder.GetResource().ToBaseContainer()));	
		}
		
		if (ambientSoundsWindDefinitionConfig)
			m_aWindModifier = ambientSoundsWindDefinitionConfig.m_aWindModifier;
		
		m_aSoundHandle.Clear();
	}
	
	//------------------------------------------------------------------------------------------------
	private void PlaySound(float worldTime)
	{
			DbgUI.Begin("Play Sound", 0, 0);
		
			int soundGroup = 3;
			int soundType = 2;
			int soundDef = 0;
			float distanceFactor = 0.15;
			DbgUI.InputInt("Sound Group:", soundGroup, 50);
			DbgUI.InputInt("Sound Type :", soundType, 50);
			DbgUI.InputInt("Sound Def  :", soundDef, 50);
			DbgUI.InputFloat("Dist Factor:", distanceFactor, 50);
			
			bool play = DbgUI.Button("Play"); 	
			DbgUI.SameLine();
			bool clear = DbgUI.Button("Clear");
			
			if (play)
			{
				// Check if sound definition is correct				
				if (soundGroup >= m_aSoundGroup.Count() || soundType >= m_aSoundGroup[soundGroup].m_aSoundType.Count() || soundDef >= m_aSoundGroup[soundGroup].m_aSoundType[soundType].m_aSoundDef.Count())
					return;
				
				// Get camera transf
				vector mat[4];
				GetGame().GetCameraManager().CurrentCamera().GetTransform(mat);
				
				// Get direction
				vector yaw = mat[0];
				float angle = yaw.ToYaw();
				angle -= 90;
				yaw = vector.FromYaw(angle);
				
				// Get sound position
				float dist =  Math.Lerp(m_aSpawnDef[soundGroup].m_iPlayDistMin, m_aSpawnDef[soundGroup].m_iPlayDistMax, distanceFactor);
				mat[3] = mat[3] + yaw * dist;		
				
				// Add sound to pool 
				ref SCR_SoundHandle soundHandle = new SCR_SoundHandle(soundGroup, soundType, soundDef, mat, m_aSoundGroup, worldTime);		
				m_aSoundHandle.Insert(soundHandle);				
			}
			
			if (clear)
			{
				m_aSoundHandle.Clear();
			}
		
			DbgUI.End();
	}
	
	
#endif

	//------------------------------------------------------------------------------------------------	
	/*
	Called by SCR_AmbientSoundComponent in OnPostInit()
	*/	
	override void OnPostInit(SCR_AmbientSoundsComponent ambientSoundsComponent, SignalsManagerComponent signalsManagerComponent)
	{
		super.OnPostInit(ambientSoundsComponent, signalsManagerComponent);
				
		LoadConfigs(ambientSoundsComponent);
			
		m_LocalSignalsManager = signalsManagerComponent;
		m_GlobalSignalsManager = GetGame().GetSignalsManager();
		m_AmbientSoundsComponent = ambientSoundsComponent;

		int count = m_aSpawnDef.Count();		
		m_aDensity.Resize(count);
		m_aAngleOffset.Resize(count);
		m_aDensityModifier.Resize(count);
	
		UpdateSignlsIdx();
		m_World = GetGame().GetWorld();
		
#ifdef ENABLE_DIAG
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_SOUNDS_RANDOM_POSITIONAL_SOUNDS, "", "Random Ambient Sounds", "Sounds");
#endif		
	}

	//------------------------------------------------------------------------------------------------		
	void ~SCR_RandomPositionalSounds()
	{
#ifdef ENABLE_DIAG
			DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_SOUNDS_RANDOM_POSITIONAL_SOUNDS);
#endif	
	}
}