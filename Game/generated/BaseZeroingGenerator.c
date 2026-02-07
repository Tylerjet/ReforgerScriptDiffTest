/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
BaseZeroingGenerator is an object used for generating
zeroing information in Workbench
*/
class BaseZeroingGenerator: ScriptAndConfig
{
	/*!
	Gets the owning entity of the generator object, which might or might not be the same as
	the owner of the Sights component.
	*/
	proto external IEntity GetOwnerEntity();
	/*!
	Get the sights component that the zeroing is computed for
	*/
	proto external BaseSightsComponent GetSights();
	/*!
	Get the Weapon animation component of the sights. This will might be nullptr if the sights do not have
	a WeaponAnimationComponent, like e.g. with the UGL's.
	*/
	proto external WeaponAnimationComponent GetWeaponAnimationComponent();
}
