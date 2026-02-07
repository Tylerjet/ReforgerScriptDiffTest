/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup AI
\{
*/

class AIFormationComponentClass: AIComponentClass
{
}

class AIFormationComponent: AIComponent
{
	proto external vector GetOffsetPosition();
	proto external vector GetFormationCenterPosition();
	proto external bool SetFormation(string pFormation);
	proto external AIFormationDefinition GetFormation();
	/*!
	Displacement modifies formation indexes.
	Examples:
	Displacement = 0, Used formation indexes: 0, 1, ..., Max
	Displacement = 2, Used formation indexes: 2, 3, ..., Max
	*/
	proto external void SetFormationDisplacement(int iValue);
	proto external int GetFormationDisplacement();
	proto external bool IsFormationDisplaced();
}

/*!
\}
*/
