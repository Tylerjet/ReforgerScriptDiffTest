/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup AI
\{
*/

class AIAnimalClass: AIAgentClass
{
}

class AIAnimal: AIAgent
{
	proto external int GetSoundMapTag();
	//How the entity should react to danger events. This also calls OnReactToDanger for custom effects
	proto external void ReactToDanger();

	// callbacks

	//What effects should play when scared (for example, playing sounds, particles or animations)
	event void OnReactToDanger();
}

/*!
\}
*/
