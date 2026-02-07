class SCR_MuzzleEffectComponentClass : MuzzleEffectComponentClass
{
};

class SCR_MuzzleEffectComponent : MuzzleEffectComponent
{
	
	static const string MUZZLEFLASH = "Muzzle flash";
	
	protected LightHandle m_LightHandle;
	protected float m_fTimer;
	
	[Attribute("Color of flash", UIWidgets.ColorPicker, "Color of muzzle flash effect", category: MUZZLEFLASH)]
	protected vector m_vCol;
	[Attribute("Radius of flash", UIWidgets.EditBox, "Radius of muzzle flash effect", "0 100", category: MUZZLEFLASH)]
	protected float m_fRadiusOfFlash;
	[Attribute("LV", UIWidgets.EditBox, "LV of muzzle flash effect", "-8 20" ,category: MUZZLEFLASH)]
	protected float m_fLV;
	[Attribute("EVClip", UIWidgets.EditBox, "EVClip of muzzle flash effect", category: MUZZLEFLASH)]
	protected float m_fEVClip;
	[Attribute("Cast Shadows", UIWidgets.CheckBox, "Should muzzle effect cast shadows", category: MUZZLEFLASH)]
	protected bool m_bShadows;
	[Attribute("Max time of flash", UIWidgets.EditBox, "Maximumum time of muzzle flash effect", "0.01 100", category: MUZZLEFLASH)]
	protected float m_fMaxTimeOfFlash;
	[Attribute("Offset of light", UIWidgets.Auto, "Offset of light from muzzle flash origin", category: MUZZLEFLASH)]
	protected vector m_vOffset;
	
	void StartLight(IEntity entity)
	{
		if (!entity)
			return;
		
		EndLight();
		if (m_fRadiusOfFlash < 0)
			return;
		int lightFlags = LightFlags.CHEAP;
		if (m_bShadows)
			lightFlags |= LightFlags.CASTSHADOW ; 

		vector mat[4];
		entity.GetWorldTransform(mat);
		m_LightHandle = LightHandle.AddStaticLight(entity.GetWorld(), LightType.POINT, lightFlags, m_fRadiusOfFlash, Color.FromVector(m_vCol), m_fLV, m_vOffset.Multiply4(mat));
		m_LightHandle.SetLensFlareType(entity.GetWorld(), LightLensFlareType.Disabled);
		m_LightHandle.SetIntensityEVClip(entity.GetWorld(), m_fEVClip);
		
		m_fTimer = m_fMaxTimeOfFlash;
	}
	
	void EndLight()
	{
		if (m_LightHandle)
		{
			m_LightHandle.RemoveLight(GetGame().GetWorld());
			m_LightHandle = NULL;
		}
	}
		
	override void OnFrame(IEntity owner, float timeSlice)
	{
		m_fTimer -= timeSlice;
		if (m_fTimer < 0)
			EndLight();	
	}
	
	override void OnStop()
	{
		EndLight();
	}
	
	override void OnFired(IEntity effectEntity, BaseMuzzleComponent muzzle, IEntity projectileEntity)
	{
		StartLight(effectEntity);
	}
};