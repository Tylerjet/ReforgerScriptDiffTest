class WallGeneratorPointData : ShapePointDataScriptBase
{
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Prefab", "et")]
	protected ResourceName MeshAtPoint;

	[Attribute("0", UIWidgets.Slider, params: "-10 10 0.01")]
	protected float PrePadding;

	[Attribute("0", UIWidgets.Slider, params: "-10 10 0.01")]
	protected float PostPadding;

	[Attribute("0", UIWidgets.Slider, params: "-10 10 0.01")]
	protected float m_fOffsetUp;

	[Attribute("0", UIWidgets.CheckBox)]
	protected bool m_bAlignWithNext;

	[Attribute("0", UIWidgets.CheckBox)]
	protected bool m_bAllowClipping;

	[Attribute("1", UIWidgets.CheckBox)]
	protected bool m_bGenerate;
}
