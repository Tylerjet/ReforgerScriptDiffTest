/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Projectile/Trigger
\{
*/

class BaseTriggerComponent: BaseProjectileComponent
{
	proto external void OnUserTrigger(IEntity owner);
	//Only used when overriding the instigator is necessary. E.g.: Player A shoots explosive of player B. The instigator needs to be overriden with player A.
	proto external void OnUserTriggerOverrideInstigator(IEntity owner, IEntity instigator);
	proto external void SetLive();
	//! Get projectile effects that are inherited from projectileType parameter.
	proto external void GetProjectileEffects(typename projectileType, out notnull array<BaseProjectileEffect> outProjectileEffects);
}

/*!
\}
*/
