[EntityEditorProps(category: "GameScripted/Sound", description: "Testing component")]
class SCR_VehicleSoundComponentClass : VehicleSoundComponentClass
{
	// prefab properties here
};

class SCR_VehicleSoundComponent : VehicleSoundComponent
{	
	[Attribute("1.8", UIWidgets.Object, "")]
	protected float m_fRainSoundHeight;
	
	[Attribute("", UIWidgets.Object, "")]
	protected ref array<ref SCR_SignalDefinition> m_aSignalDefinition;

	private float m_fWorldTimeLast;
	
	private SignalsManagerComponent m_SignalsManagerComponent;
	
	// Audio Handles
	private AudioHandle m_LoopedSound;
	private AudioHandle m_RainSound;
	
	private const static float OBB_CROP_FACTOR = 0.7;
	private vector m_vMins;
	private vector m_vMaxs;
	private GameSignalsManager m_GameSignalsManager;
	private int m_iRainIntensitySignalIdx;
	private float m_fCheckRainTime;
	private const static int RAIN_CHECK_TIME = 500;
	private const static float RAIN_RAIN_THRESHOLD = 0.1;
	
	private int m_iOBBdiffSignalIdx;
	
	//------------------------------------------------------------------------------------------------
	
	private vector GetRainSoundPositionOffset(IEntity owner)
	{
		PlayerCamera playerCamera = GetGame().GetPlayerController().GetPlayerCamera();
		
		if (!playerCamera)
			return vector.Zero;
		
		// Local camara position
		vector camera = owner.CoordToLocal(playerCamera.GetOrigin());
			
		// Get offset
		vector offset;
		offset[0] = Math.Clamp(camera[0], m_vMins[0], m_vMaxs[0]);
		offset[1] = m_fRainSoundHeight;
		offset[2] = Math.Clamp(camera[2], m_vMins[2], m_vMaxs[2]);
				
		return owner.VectorToParent(offset);
	}
	
	//------------------------------------------------------------------------------------------------
	override void UpdateSoundJob(IEntity owner, float timeSlice)
	{
		float worldTime = owner.GetWorld().GetWorldTime();

		if (m_fWorldTimeLast == worldTime)
			return;
		
		if (m_SignalsManagerComponent)
		{
			foreach(SCR_SignalDefinition signalDefinition : m_aSignalDefinition)
			{
				m_SignalsManagerComponent.SetSignalValue(signalDefinition.m_iSignalIdx, signalDefinition.GetSignalValue(worldTime));
			}
		}
		
		m_fWorldTimeLast = worldTime;
		
		// Trigger rain sound
		if (m_fCheckRainTime <= worldTime)
		{
			float rainIntensity = m_GameSignalsManager.GetSignalValue(m_iRainIntensitySignalIdx);
			
			if (rainIntensity >= RAIN_RAIN_THRESHOLD)
			{
				if (IsFinishedPlaying(m_RainSound))
					m_RainSound = SoundEvent(SCR_SoundEvent.SOUND_VEHICLE_RAIN);
			}	
			else
			{
				if (!IsFinishedPlaying(m_RainSound))
					Terminate(m_RainSound);
			}
			
			m_fCheckRainTime = worldTime + RAIN_CHECK_TIME;
		}
		
		// Set rain sound position
		if (!IsFinishedPlaying(m_RainSound))
		{
			vector mat[4];
			mat[3] = GetRainSoundPositionOffset(owner) + owner.GetOrigin();
			SetSoundTransformation(m_RainSound, mat);
		}	
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{	
		m_SignalsManagerComponent = SignalsManagerComponent.Cast(owner.FindComponent(SignalsManagerComponent));
		
		if (m_SignalsManagerComponent)
		{
			foreach(SCR_SignalDefinition signalDefinition : m_aSignalDefinition)
			{
				signalDefinition.m_iSignalIdx = m_SignalsManagerComponent.AddOrFindSignal(signalDefinition.m_sSignalName);
				signalDefinition.UpdateSignalPoint(owner.GetWorld().GetWorldTime());
			}
		}
				
		// Get OBB	
		owner.GetBounds(m_vMins, m_vMaxs);
		m_vMins *= OBB_CROP_FACTOR;
		m_vMaxs *= OBB_CROP_FACTOR;
		
		// Get Game Signals Manger
		m_GameSignalsManager = GetGame().GetSignalsManager();
		
		// Get RainIntensity signal index
		m_iRainIntensitySignalIdx = m_GameSignalsManager.AddOrFindSignal("RainIntensity");
		
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