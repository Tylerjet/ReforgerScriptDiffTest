//! Wall Generator point metadata that is being operated on with the generator after the data gets extracted from raw polyline/spline data
class SCR_WallGeneratorPoint
{
	vector m_vPos;
	ResourceName m_sCustomMesh;
	float m_fPrePadding;
	float m_fPostPadding;
	bool m_bGenerate;
	bool m_bAlignNext;
	bool m_bClip;
	float m_fOffsetUp;
}
