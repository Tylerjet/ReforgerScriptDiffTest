//! Multi-Phase destruction phase
[BaseContainerProps(), SCR_DamagePhaseTitle()]
class SCR_DamagePhaseData
{
	[Attribute("100", UIWidgets.Slider, "Base health value for the damage phase. Upon switching to this phase, this value replaces the base health of the object", "0.01 100000 0.01")]
	float m_fPhaseHealth;
	
	[Attribute(ResourceName.Empty, UIWidgets.ResourcePickerThumbnail, "Model to use for the damage phase", "xob")]
	ResourceName m_PhaseModel;
	
	[Attribute("", UIWidgets.Object, "List of objects (particles, debris, etc) to spawn on destruction of the phase")]
	ref array<ref SCR_BaseSpawnable> m_PhaseDestroySpawnObjects;
}