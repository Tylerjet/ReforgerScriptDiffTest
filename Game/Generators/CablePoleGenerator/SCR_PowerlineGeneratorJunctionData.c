[BaseContainerProps()]
class SCR_PowerlineGeneratorJunctionData
{
	[Attribute(desc: "Junction prefab to be used (if not specified, the generator's default one is used)", uiwidget: UIWidgets.ResourcePickerThumbnail, params: "et")] //  class=SCR_PowerPole : no, allow power station too
	ResourceName m_sJunctionPrefab;

	// not a slider for performance reason (type the value directly)
	[Attribute(defvalue: "0", desc: "Set the junction's yaw offset; can be used to setup the prefab properly", /* uiwidget: UIWidgets.Slider, */params: "0 360")]
	float m_fYawOffset;

	[Attribute(defvalue: "0", desc: "Define whether or not this junction is a power source")]
	bool m_bPowerSource;
}
