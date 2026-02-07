/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components
\{
*/

/*!
Component for handling world subscene attached to entity for light portals,
interior audio, visibility, etc.
*/
class WorldSubsceneComponent: GenericComponent
{
	proto void RegisterAperture(const vector mins, const vector maxs, const vector worldTransform[4], out int portalIdx, out int apertureIdx);
	proto external void SetPortalOpening(int portalIdx, int apertureIdx, float value);
	proto external float GetPortalOpening(int portalIdx);
	proto external void SetPortalIntensity(int portalIdx, float value);
	proto external float GetPortalIntensity(int portalIdx);
}

/*!
\}
*/
