/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup Components
* @{
*/

class AmbientSoundsComponentClass: SoundComponentClass
{
};

class AmbientSoundsComponent: SoundComponent
{
	proto external void QueryAmbientSoundsBySphere(float radius, EQueryEntitiesFlags queryFlags = EQueryEntitiesFlags.ALL);
	proto external void GetAmbientSoundsCountPerType(out array<int> count);
	proto external vector GetCameraOrigin();
	proto external bool GetRiver(const vector pos, out array<float> count);
	proto external IEntity GetRandomTree(int index, float minHeight);
	proto external void GetClosestEntities(int soundTypeIdx, int nEntities, out array<IEntity> output);
	proto void TracePointToTerrain(const vector point, const vector offset, inout float hitHeight, inout int iSoundGroup);
};

/** @}*/
