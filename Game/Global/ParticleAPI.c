/* 
This API handles all particle effects, whenever they are created in engine or by script.

TO DO:
	- Do not create particles on server side (a prevention needs to be created first)
	- Create a hierarchy of particle controllers out of purely scripted class (no GenericEntity or such)
	- Solve the issue with m_DeleteOnFinish when the particle has flag Repeat starting at true, but then is later set to false during lifetime.
*/

class SCR_ParticleAPI
{
	static const int EMITTERS_MAX = 16;
	
	// particle staging
	static const string EMITTER_STAGE_PREFIX = "s"; // Defines tag that marks the start position of stage ID.
	static const string EMITTER_STAGE_SUFIX = "_"; // Defines tag that marks the end position of stage ID. Between these two positions goes a number from 1 to 16.
	
	static string m_ErrorBadClassname = "Error! Particle failed to spawn because of wrong class name!: ";
	static string m_ErrorBadPTC = "Error! Particle failed to spawn because of incorrect path to the PTC file!: ";
	
	
	//------------------------------------------------------------------------------------------------
	//! Create and play a particle effect (from file) at given transformation matrix
	//! \param ptc_path File path to the particle effect
	//! \param mat World transform to spawn the particle effect at
	static SCR_ParticleEmitter PlayOnTransformPTC(string ptc_path, vector mat[4])
	{
		SCR_ParticleEmitter ptc = SCR_ParticleEmitter.Cast(GetGame().SpawnEntity(SCR_ParticleEmitter));
		if (ptc)
		{
			ptc.m_DeleteOnFinish = true;
			ptc.SetPathToPTC(ptc_path);
			ptc.SetWorldTransform(mat);
			ptc.Play();
			
			return ptc;
		}
		
		Debug.Error(m_ErrorBadPTC + ptc_path);
		return null;
	}
	
	
	//------------------------------------------------------------------------------------------------
	//! Create and play a particle effect (from file) at given position and angles
	//! \param ptc_path File path to the particle effect
	//! \param pos World position to spawn the particle effect at
	//! \param ang World angles (deg) to spawn the particle effect at
	static SCR_ParticleEmitter PlayOnPositionPTC(string ptc_path, vector pos, vector ang = "0 0 0")
	{
		SCR_ParticleEmitter ptc = SCR_ParticleEmitter.Cast(GetGame().SpawnEntity(SCR_ParticleEmitter));
		
		if (ptc)
		{
			ptc.m_DeleteOnFinish = true;
			ptc.SetPathToPTC(ptc_path);
			ptc.SetOrigin(pos);
			ptc.SetAngles(ang);
			ptc.Play();
			
			return ptc;
		}
		
		Debug.Error(m_ErrorBadPTC + ptc_path);
		return null;
	}
	
	//! Creates the given particle class and plays it on the given position.
	static SCR_ParticleEmitter PlayOnPosition(typename particle_class, vector pos, vector ang = "0 0 0")
	{
		SCR_ParticleEmitter ptc = SCR_ParticleEmitter.Cast(GetGame().SpawnEntity( particle_class ));
		
		if (ptc)
		{
			ptc.m_DeleteOnFinish = true;
			ptc.SetOrigin(pos);
			ptc.SetAngles(ang);
			ptc.Play();
			
			return ptc;
		}
		
		Debug.Error(m_ErrorBadClassname + particle_class.ToString());
		return null;
	}
	
	//! Creates the given particle class and plays it on the given object with optional parameters: offset, rotation and bone ID.
	static SCR_ParticleEmitter PlayOnObject(IEntity object, typename particle_class, vector local_pos = "0 0 0", vector local_ang = "0 0 0", int bone_id = -1, bool rotate_with_object = true )
	{
		SCR_ParticleEmitter ptc = SCR_ParticleEmitter.Cast(GetGame().SpawnEntity( particle_class ));
		
		if (ptc)
		{
			ptc.m_DeleteOnFinish = true;
			ptc.SetOrigin(local_pos);
			ptc.SetAngles(local_ang);
			object.AddChild(ptc, bone_id, rotate_with_object);
			ptc.Play();
			
			return ptc;
		}
		
		Debug.Error(m_ErrorBadClassname + particle_class.ToString());
		return null;
	}
	
	//! Creates the given particle from file and plays it on the given object with optional parameters: offset, rotation and bone ID.
	static SCR_ParticleEmitter PlayOnObjectPTC(IEntity object, string ptc_path, vector local_pos = "0 0 0", vector local_ang = "0 0 0", int bone_id = -1, bool rotate_with_object = true )
	{
		SCR_ParticleEmitter ptc = SCR_ParticleEmitter.Cast(GetGame().SpawnEntity(SCR_ParticleEmitter));
		
		if (ptc)
		{
			ptc.m_DeleteOnFinish = true;
			ptc.SetPathToPTC(ptc_path);
			ptc.SetOrigin(local_pos);
			ptc.SetAngles(local_ang);
			object.AddChild(ptc, bone_id, rotate_with_object);
			ptc.Play();
			
			return ptc;
		}
		
		Debug.Error(m_ErrorBadPTC + ptc_path);
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Lerp to a percentage of the original of the input parameter of all emmiters
	//! \param effect Particle effect instance
	//! \param scale Percentage of the original emitter parameter to lerp to
	//! \param param Emitter parameter to lerp
	static void LerpAllEmitters(IEntity effect, float scale, int param)
	{
		if(effect)
		{
			string emitters[EMITTERS_MAX];
			int emitter_id = effect.GetParticleEmitters(emitters, EMITTERS_MAX);
			
			for (int i = 0; i < emitter_id; i++)
			{
				LerpEmitter(effect, i, scale, param);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Prints the name of all emitters
	static void PrintAllEmitters(SCR_ParticleEmitter effect)
	{
		if(effect)
		{
			string emitters[EMITTERS_MAX];
			int emitter_id = effect.GetParticleEmitters(emitters, EMITTERS_MAX);
			
			for (int i = 0; i < emitter_id; i++)
			{
				Print(i.ToString() + ": " + emitters[i]);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns the index of an emitter by name
	//! \param name String name of the emitter
	static int GetEmitterIndexByName(IEntity effect, string name)
	{
		if(effect)
		{
			string emitters[EMITTERS_MAX];
			int emitter_ids = effect.GetParticleEmitters(emitters, EMITTERS_MAX);
			
			for (int i = 0; i < emitter_ids; i++)
			{
				if (emitters[i] == name)
					return i;
			}
		}
		
		return -1;
	}
	
	
	//------------------------------------------------------------------------------------------------
	//! Returns the amount of stages the given particle has.
	//! \param name IEntity particle instance
	static int GetStagesCount(IEntity effect)
	{
		int stages_count = 0;
		
		if(effect)
		{
			for (int i = 1; i < SCR_ParticleAPI.EMITTERS_MAX + 1; i++)
			{
				array<int> stage_ids = SCR_ParticleAPI.FindEmittersByString(effect, EMITTER_STAGE_PREFIX + i.ToString() + EMITTER_STAGE_SUFIX);
				
				if (stage_ids.Count() > 0)
				{
					stages_count = i;
				}
				else
				{
					break;
				}
			}
		}
		
		return stages_count;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Enables the given stage ID, so only the emitters that contain 's#_' in their name will be enabled. The # starts at 1 and can end at 16.
	//! \param name IEntity particle instance
	//! \param name int emitter id
	static void EnableStage(IEntity effect, int stage_id)
	{
		array<int> relevant_emitters_ids = FindEmittersByString(effect, EMITTER_STAGE_PREFIX + stage_id.ToString() + EMITTER_STAGE_SUFIX);
		
		for (int i = 0; i < relevant_emitters_ids.Count(); i++)
		{
			SCR_ParticleAPI.LerpEmitter(effect, relevant_emitters_ids[i], 1, EmitterParam.BIRTH_RATE );
			SCR_ParticleAPI.LerpEmitter(effect, relevant_emitters_ids[i], 1, EmitterParam.BIRTH_RATE_RND );
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Disables the given stage ID, so only the emitters that contain 's#_' in their name will be disabled. The # starts at 1 and can end at 16.
	//! \param name IEntity particle instance
	//! \param name int emitter id
	static void DisableStage(IEntity effect, int stage_id)
	{
		array<int> relevant_emitters_ids = FindEmittersByString(effect, EMITTER_STAGE_PREFIX + stage_id.ToString() + EMITTER_STAGE_SUFIX);
		
		for (int i = 0; i < relevant_emitters_ids.Count(); i++)
		{
			SCR_ParticleAPI.LerpEmitter(effect, relevant_emitters_ids[i], 0, EmitterParam.BIRTH_RATE );
			SCR_ParticleAPI.LerpEmitter(effect, relevant_emitters_ids[i], 0, EmitterParam.BIRTH_RATE_RND );
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Changes the given stage ID, so only the emitters that contain 's#_' are affected. The # starts at 1 and can end at 16.
	//! \param name IEntity particle instance
	//! \param name int stage id
	//! \param name float parameter value
	//! \param name int parameter type
	static void LerpStage(IEntity effect, int stage_id, float value, int parameter_type)
	{
		array<int> relevant_emitters_ids = FindEmittersByString(effect, EMITTER_STAGE_PREFIX + stage_id.ToString() + EMITTER_STAGE_SUFIX);
		
		for (int i = 0; i < relevant_emitters_ids.Count(); i++)
		{
			SCR_ParticleAPI.LerpEmitter(effect, relevant_emitters_ids[i], value, parameter_type );
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns array of indexes of emitter that contain the given string in their name
	//! \param name String Search term
	static array<int> FindEmittersByString(IEntity effect, string search)
	{
		array<int> result_ids = new array <int>;
		
		if(effect)
		{
			string emitters[EMITTERS_MAX];
			int emitter_ids = effect.GetParticleEmitters(emitters, EMITTERS_MAX);
			
			for (int i = 0; i < emitter_ids; i++)
			{
				if (emitters[i].Contains(search))
					result_ids.Insert(i);
			}
		}
		
		return result_ids;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Lerp to a percentage of the original of the input parameter for the input named emitter
	//! \param effect Particle effect instance
	//! \param name String name of the emitter
	//! \param scale Percentage of the original emitter parameter to lerp to
	//! \param param Emitter parameter to lerp
	void LerpNamedEmitter(IEntity effect, string name, float scale, int param)
	{
		if(effect)
		{	
			int index = GetEmitterIndexByName(effect, name);
			
			if (index == -1)
				return;
			
			float oLT = 0;
			effect.GetParticleParmOriginal(index, param, oLT);
			oLT *= scale;
			effect.SetParticleParm(index, param, oLT);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Lerp to a percentage of the original of the input parameter for the input emitter index.
	//! \param effect Particle effect instance
	//! \param index Index of the emitter
	//! \param scale Percentage of the original emitter parameter to lerp to
	//! \param param Emitter parameter to lerp
	static void LerpEmitter(IEntity effect, int index, float scale, int param)
	{
		if (index == -1  ||  !effect)
			return;
		
		float oLT = 0;
		effect.GetParticleParmOriginal(index, param, oLT);
		oLT *= scale;
		effect.SetParticleParm(index, param, oLT);
	}
	
	
	//------------------------------------------------------------------------------------------------
	void SCR_ParticleAPI(IEntitySource src, IEntity parent)
	{
		
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_ParticleAPI()
	{
		
	}
};