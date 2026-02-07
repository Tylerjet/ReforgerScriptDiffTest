/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup RoadNetwork
\{
*/

class RoadNetworkManager: Managed
{
	proto int GetClosestRoad(vector pos, out Road foundRoad, out float distance, bool skipNavlinks = false);
	proto int GetRoadsInAABB(vector aabbMin, vector aabbMax, out array<Road> outRoads);
}

/*!
\}
*/
