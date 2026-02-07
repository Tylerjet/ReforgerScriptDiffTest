[CinematicTrackAttribute(name:"Camera Tweek Track", description:"Track used for tweaking camera")]
class CameraTweekCinematicTrack : CinematicTrackBase
{
	
	[Attribute("")]
	string m_sAttachCameraTo;
	
	[Attribute("v_root")]
	string m_sAttachBoneName;
	
	[Attribute("0 0 0")]
	vector m_AttachCameraPositionOffset;
	
	[Attribute("0 0 0")]
	vector m_AttachCameraAngleOffset;
	
	private CinematicEntity cineEntity;
	private GenericEntity m_entityAttachTo;
	private World actualWorld;

	
	override void OnInit(World world)
	{
		cineEntity = CinematicEntity.Cast(world.FindEntityByName(GetSceneName()));
		actualWorld = world;
	}
	
	override void OnApply(float time)
	{
		
		if (cineEntity)
		{			
			//attach camera
			m_entityAttachTo = GenericEntity.Cast(actualWorld.FindEntityByName(m_sAttachCameraTo));
			
			if (m_entityAttachTo && m_sAttachCameraTo != "")
			{
				int boneIndex = m_entityAttachTo.GetAnimation().GetBoneIndex(m_sAttachBoneName);		
				cineEntity.AttachCameraToEntity(m_entityAttachTo, boneIndex, m_AttachCameraPositionOffset, m_AttachCameraAngleOffset);
				
			} else
			{
				cineEntity.DetachCamera();
			}
		}
	}
}
