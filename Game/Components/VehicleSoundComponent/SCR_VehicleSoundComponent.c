[EntityEditorProps(category: "GameScripted/Sound", description: "Testing component")]
class SCR_VehicleSoundComponentClass : VehicleSoundComponentClass
{
	// prefab properties here
};

class SCR_VehicleSoundComponent : VehicleSoundComponent
{		
	[Attribute("", UIWidgets.Object, "")]
	protected ref array<ref SCR_SignalDefinition> m_aSignalDefinition;
	
	[Attribute("0 0 0", UIWidgets.Coords, "Mins OOB Point for rain sound")]
	vector m_vMins;
	
	[Attribute("0 0 0", UIWidgets.Coords, "Maxs OOB Point for rain sound")]
	vector m_vMaxs;
	
	protected SignalsManagerComponent m_SignalsManagerComponent;
	
	// Audio Handles
	private AudioHandle m_RainSoundAudioHandle = AudioHandle.Invalid;
	
	private GameSignalsManager m_GameSignalsManager;
	private int m_iRainIntensitySignalIdx;
	private const static float RAIN_INTENSITY_THRESHOLD = 0.1;
	private const static string RAIN_INTENSITY_SIGNAL_NAME = "RainIntensity";
	
	//------------------------------------------------------------------------------------------------
	
	private vector GetRainSoundPositionOffset()
	{
		vector cameraTransform[4];
		GetGame().GetWorld().GetCurrentCamera(cameraTransform);
				
		// Local camara position
		vector camera = GetOwner().CoordToLocal(cameraTransform[3]);
			
		// Get offset
		vector offset;
		offset[0] = Math.Clamp(camera[0], m_vMins[0], m_vMaxs[0]);
		offset[1] = Math.Clamp(camera[1], m_vMins[1], m_vMaxs[1]);;
		offset[2] = Math.Clamp(camera[2], m_vMins[2], m_vMaxs[2]);
				
		return GetOwner().VectorToParent(offset);
	}
	
	//------------------------------------------------------------------------------------------------
	override void UpdateSoundJob(IEntity owner, float timeSlice)
	{
		HandleGeneratedSignals();
		HandleRainSound();
	}

	//------------------------------------------------------------------------------------------------
	private void HandleGeneratedSignals()
	{
		if (!m_SignalsManagerComponent)
			return;
		
		float worldTime = GetGame().GetWorld().GetWorldTime();
			
		foreach (SCR_SignalDefinition signalDefinition : m_aSignalDefinition)
			m_SignalsManagerComponent.SetSignalValue(signalDefinition.m_iSignalIdx, signalDefinition.GetSignalValue(worldTime));
	}
	
	//------------------------------------------------------------------------------------------------
	private void HandleRainSound()
	{
		if (m_GameSignalsManager.GetSignalValue(m_iRainIntensitySignalIdx) >= RAIN_INTENSITY_THRESHOLD)
		{
			if (IsFinishedPlaying(m_RainSoundAudioHandle))
				m_RainSoundAudioHandle = SoundEvent(SCR_SoundEvent.SOUND_VEHICLE_RAIN);
			
			if (!IsHandleValid(m_RainSoundAudioHandle))
				return;
				
			// Update position
			vector mat[4];
			mat[3] = GetRainSoundPositionOffset() + GetOwner().GetOrigin();
			SetSoundTransformation(m_RainSoundAudioHandle, mat);
		}	
		else if (m_RainSoundAudioHandle != AudioHandle.Invalid)
		{
			Terminate(m_RainSoundAudioHandle);
			m_RainSoundAudioHandle = AudioHandle.Invalid;
		}	
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{	
		m_SignalsManagerComponent = SignalsManagerComponent.Cast(owner.FindComponent(SignalsManagerComponent));
		
		if (m_SignalsManagerComponent)
		{
			foreach (SCR_SignalDefinition signalDefinition : m_aSignalDefinition)
			{
				signalDefinition.m_iSignalIdx = m_SignalsManagerComponent.AddOrFindSignal(signalDefinition.m_sSignalName);
				signalDefinition.UpdateSignalPoint(owner.GetWorld().GetWorldTime());
			}
		}
						
		// Get Game Signals Manger
		m_GameSignalsManager = GetGame().GetSignalsManager();
		
		// Get RainIntensity signal index
		m_iRainIntensitySignalIdx = m_GameSignalsManager.AddOrFindSignal(RAIN_INTENSITY_SIGNAL_NAME);
		
		// Collision sounds setup
		SetMinTimeAfterImpact(300);
	}

	//------------------------------------------------------------------------------------------------
	void SCR_VehicleSoundComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_VehicleSoundComponent()
	{
	}

};