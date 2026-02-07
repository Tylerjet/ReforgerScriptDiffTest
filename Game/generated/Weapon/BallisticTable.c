/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Weapon
\{
*/

class BallisticTable: ScriptAndConfig
{
	/*!
	Returns aim height and time for next projectile shot with the muzzle at given distance.
	If the distance is further than the max bullet range, it returns negative time and aim height of the projectile max range.
	*/
	static proto float GetAimHeightOfNextProjectile(float distance, out float time, BaseMuzzleComponent muzzleComp);
	/*!
	Returns aim height and time for given projectile at given distance. This should be used if the projectile is not shot with muzzle. For example for grenades.
	If the distance is further than the max bullet range, it returns negative time and aim height of the projectile max range.
	*/
	static proto float GetHeightFromProjectile(float distance, out float time, IEntity projectile, float initSpeedCoef = 1.0);
	/*!
	Returns aim height and time for given projectile source at given distance. This should be used if the projectile is not shot with muzzle. For example for grenades.
	If the distance is further than the max bullet range, it returns negative time and aim height of the projectile max range.
	*/
	static proto float GetHeightFromProjectileSource(float distance, out float fTimeResult, IEntitySource projectileSource, float initSpeedCoef = 1.0);
	/*!
	Returns distance and time for given projectile shot at given angle. This should be used if the projectile is not shot with muzzle. For example for grenades.
	If the angle is not within the ballistic table range, it returns negative time and distance of the nearest angle projectile .
	*/
	static proto float GetDistanceOfProjectile(float angle, out float time, IEntity projectile, float initSpeedCoef = 1.0);
	/*!
	Returns distance and time for given projectile source shot at given angle. This should be used if the projectile is not shot with muzzle. For example for grenades.
	If the angle is not within the ballistic table range, it returns negative time and distance of the nearest angle projectile .
	*/
	static proto float GetDistanceOfProjectileSource(float angle, out float fTimeResult, IEntitySource projectileSource, float initSpeedCoef = 1.0);
}

/*!
\}
*/
