// Script File

enum ParticleCategories
{
	ROCKET_TRACE,			// = 0
};

[BaseContainerProps()]
class ParticleResourceReservation
{
	[Attribute( "", UIWidgets.ResourceNamePicker, "entity prefab", "et" )]
	ResourceName particleResource;
	
	[Attribute( "", UIWidgets.ResourceNamePicker, "particle effect", "ptc" )]
	ResourceName effectResource;
	
	[Attribute( "", UIWidgets.CheckBox, "can be resized when all particles are used" )]
	bool resizable;
	
	[Attribute( "", UIWidgets.EditBox, "reserved capacity")]
	int capacity;
	
};
[BaseContainerProps()]
class ParticleClassReservation
{
	[Attribute( "", UIWidgets.EditBox, "entity prefab", "et" )]
	string particleResource;
	
	[Attribute( "", UIWidgets.ResourceNamePicker, "particle effect", "ptc" )]
	ResourceName effectResource;
	
	[Attribute( "", UIWidgets.CheckBox, "can be resized when all particles are used" )]
	bool resizable;
	
	[Attribute( "", UIWidgets.EditBox, "reserved capacity")]
	int capacity;
	
		
};

class ParticleReservationRequestClass: ScriptComponentClass
{
};

class ParticleReservationRequest : ScriptComponent
{
	[Attribute("", UIWidgets.Object, "prefab registration")]
	ref array<ref ParticleResourceReservation> resourceRequests;
	
	[Attribute("", UIWidgets.Object, "class registration")]
	ref array<ref ParticleClassReservation> classRequests;

	override void OnPostInit(IEntity owner)
	{
		
		for(int i = 0; i < resourceRequests.Count(); i++)
		{
			GetGame().GetParticlesManager().Clear();
			ParticleResourceReservation res = resourceRequests[i];
			if(!res.particleResource.IsEmpty() && res.capacity > 0)
				GetGame().GetParticlesManager().RegisterParticleEntitiesByResource(res.particleResource, res.effectResource, res.capacity, res.resizable);
		}
		for(int i = 0; i < classRequests.Count(); i++)
		{
			ParticleClassReservation res = classRequests[i];
			if(!res.particleResource.IsEmpty() && res.capacity > 0)
				GetGame().GetParticlesManager().RegisterParticleEntitiesByClass(res.particleResource, res.effectResource, res.capacity, res.resizable);
		}
			
	}
};