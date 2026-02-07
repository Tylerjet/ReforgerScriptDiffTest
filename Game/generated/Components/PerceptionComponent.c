/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup Components
* @{
*/

class PerceptionComponentClass: AIComponentEntityClass
{
};

class PerceptionComponent: AIComponentEntity
{
	proto external int GetTargetsList(out notnull array<BaseTarget> outTargets, ETargetCategory category);
	proto external BaseTarget GetTargetPerceptionObject(IEntity entityToFind, ETargetCategory targetCategory);
	proto external BaseTarget GetClosestTarget(ETargetCategory category, float seenBeforeTime);
};

/** @}*/
