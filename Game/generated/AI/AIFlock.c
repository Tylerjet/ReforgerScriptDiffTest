/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup AI
\{
*/

class AIFlockClass: AIAnimalClass
{
}

class AIFlock: AIAnimal
{
	proto external void ActivateAllBirds(bool val);
	//For the individual birds, how they determine they next position to go to
	proto external vector CreateRandomBirdTarget(int i);
	//For the individual birds, what will be its center, this is used as the starting position as well as for some calculations
	proto external vector CreateRandomBirdCenter();
}

/*!
\}
*/
