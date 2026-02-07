[EntityEditorProps(category: "GameScripted", description: "Particle emitter that includes additional script logic", color: "32 94 200 255")]
class SCR_ParticleEmitterExtendedClass : SCR_ParticleEmitterClass
{
};

//~ ScriptInvokers
void SCR_ParticleEmitterExtended_OnPlayStateChanged(SCR_ParticleEmitterExtended particleEmitter, EParticleEmitterState emitterState);
typedef func SCR_ParticleEmitterExtended_OnPlayStateChanged;

class SCR_ParticleEmitterExtended : SCR_ParticleEmitter
{
	protected ref protected ref ScriptInvokerBase<SCR_ParticleEmitterExtended_OnPlayStateChanged> m_OnPlayStateChanged; //~ Sends over this SCR_ParticleEmitterExtended and EParticleEmitterState
	
	[Attribute("0", "If true then the particle on client will roughly sync to the server. However true sync is never guaranteed as by their nature particles are effected by outside forces. Do not change this value in runtime")]
	protected bool m_bReplicated;
	
	//~ Keeps track of total simulation time 
	protected float m_fTotalSimulationTime;
	
	//----------------------------------------------------------------------------------------------------------------
	//! Called every frame to update the particle effect
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		m_fTotalSimulationTime += timeSlice;
		super.EOnFrame(owner, timeSlice);
	}
	
	//----------------------------------------------------------------------------------------------------------------
	protected override void RestartEmitter()
	{
		m_fTotalSimulationTime = 0;
		super.RestartEmitter();
	}
	
	//----------------------------------------------------------------------------------------------------------------
	/*!
	\return Get total simulation time.
	*/
	float GetTotalSimulationTime()
	{
		return m_fTotalSimulationTime;		
	}
	
	//----------------------------------------------------------------------------------------------------------------
	/*!
	Set new simulation time which instantly sets up the particle to the given time
	Can never go back in time
	This is not broadcasted
	\param newTotalSimulationTime new Similation time
	*/
	protected void UpdateSimulationTime(float newTotalSimulationTime)
	{
		m_fTotalSimulationTime = newTotalSimulationTime;
		GetParticles().SimulateMultiStep(m_fTotalSimulationTime, 0);
	}
	
	//----------------------------------------------------------------------------------------------------------------
	//~ Set playstate
	protected override void SetPlayState(EParticleEmitterState playState)
	{	
		if (GetPlayState() == playState)
			return;	
		
		super.SetPlayState(playState);

		if (m_OnPlayStateChanged)
			m_OnPlayStateChanged.Invoke(this, playState);
	}
	
	//----------------------------------------------------------------------------------------------------------------
	/*!
	Get On playState changed script invoker
	\return Script Invoker
	*/
	ScriptInvokerBase<SCR_ParticleEmitterExtended_OnPlayStateChanged> GetOnPlayStateChanged()
	{
		if (!m_OnPlayStateChanged)
			m_OnPlayStateChanged = new ScriptInvokerBase<SCR_ParticleEmitterExtended_OnPlayStateChanged>();
	
		return m_OnPlayStateChanged;
	}
	
	//----------------------------------------------------------------------------------------------------------------
	override bool RplSave(ScriptBitWriter writer)
    {	
		if (m_bReplicated)
		{
			writer.WriteInt(m_ePlayState);
			writer.WriteFloat(GetTotalSimulationTime());
		}
			 		
        return true;
    }
     
	//----------------------------------------------------------------------------------------------------------------
    override bool RplLoad(ScriptBitReader reader)
    {
		if (m_bReplicated)
		{
			int currentState;
			float totalSimulationTime;
			
			reader.ReadInt(currentState);
			reader.ReadFloat(totalSimulationTime);
			
			SetPlayState(currentState);
			UpdateSimulationTime(totalSimulationTime);
		}
        return true;
    }
};
