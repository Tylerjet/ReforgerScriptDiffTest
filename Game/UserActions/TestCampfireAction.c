// Script File
class TestCampfireAction : ScriptedUserAction
{
	[Attribute("Light this campfire up.", UIWidgets.EditBox, "Description for radial menu (light up)", "")]
	string m_sLightDescription;
	[Attribute("Extinguish this campfire.", UIWidgets.EditBox, "Description for radial menu (extinguish)", "")]
	string m_sExtinguishDescription;	
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Prefab of fire particle used for a fire action.", "ptc")]
	ResourceName m_sParticle;	
	[Attribute("0 0.2 0", UIWidgets.EditBox, "Particle offset in local space from the origin of the entity", "")]
	vector m_vParticleOffset;
	
	
	private bool m_bIsLit = false;
	private IEntity m_FireEntity = null;
	private IEntity m_Owner = null;
	
	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent) 
	{
		if (!GetGame().GetWorldEntity())
			return;
		
		m_Owner = pOwnerEntity;
	}
	
	
	//------------------------------------------------------------------------------------------------
	void StartBurning()
	{
		if (!m_FireEntity)
		{
			m_FireEntity = SCR_ParticleEmitter.CreateAsChild(m_sParticle, m_Owner, m_vParticleOffset);
		}	
	}
	
	//------------------------------------------------------------------------------------------------
	void StopBurning()
	{
		if (m_FireEntity)
		{
			delete m_FireEntity;
			m_FireEntity = null;				
		}
	}

	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{	
		if (m_sParticle == string.Empty)
			return;
		
		// Different behaviour for boxcomponent
		GenericEntity genEnt = GenericEntity.Cast(pOwnerEntity);
		SCR_InteractableBoxComponent boxComp = SCR_InteractableBoxComponent.Cast(genEnt.FindComponent(SCR_InteractableBoxComponent));
		if (boxComp)
		{			
			boxComp.ToggleIsOnFire(m_sParticle, m_vParticleOffset);		
			return;	
		}
		
		// Standard behaviour
		if (!m_bIsLit)
		{
			StartBurning();
			m_bIsLit = true;
		}
		else
		{
			StopBurning();
			m_bIsLit = false;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{		
		GenericEntity genEnt = GenericEntity.Cast(m_Owner);
		SCR_InteractableBoxComponent boxComp = SCR_InteractableBoxComponent.Cast(genEnt.FindComponent(SCR_InteractableBoxComponent));
		if (boxComp)
		{
			outName = boxComp.GetFireActionName();
			return true;
		}
		
		
		if (m_bIsLit)
			outName = "Extinguish";
		else
			outName = "Light";
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		GenericEntity genEnt = GenericEntity.Cast(m_Owner);
		SCR_InteractableBoxComponent boxComp = SCR_InteractableBoxComponent.Cast(genEnt.FindComponent(SCR_InteractableBoxComponent));
		if (boxComp)
		{			
			return boxComp.CanPerformToggleFire();
		}
		
		return true;
	}
};