class ScriptEffectComponentClass: BaseScriptEffectComponentClass
{
};

class ScriptEffectComponent: BaseScriptEffectComponent
{
	LightEntity m_LightEntity;

	void StartLight(vector pos)
	{
		if (!m_LightEntity)
		{
			Resource resource = Resource.Load("{301C9A25A075AF5B}entities/Props/CampFireLight.et");		
			m_LightEntity = LightEntity.Cast(GetGame().SpawnEntityPrefab(resource));
			m_LightEntity.SetOrigin(pos);
		}	
	}
	
	void EndLight()
	{
		if (m_LightEntity)
		{
			delete m_LightEntity;
			m_LightEntity = null;
		}
	}
};