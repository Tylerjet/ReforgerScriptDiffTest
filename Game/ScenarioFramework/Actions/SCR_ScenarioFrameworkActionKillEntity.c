[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionKillEntity : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Entity to be killed (Optional if action is attached on Slot that spawns target entity)")]
	ref SCR_ScenarioFrameworkGet m_Getter;

	[Attribute(desc: "If target entity is Character, it will randomize ragdoll upon death")]
	bool m_bRandomizeRagdoll;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		IEntity entity;
		if (!ValidateInputEntity(object, m_Getter, entity))
			return;

		SCR_DamageManagerComponent damageMananager = SCR_DamageManagerComponent.GetDamageManager(entity);
		if (damageMananager)
			damageMananager.Kill(Instigator.CreateInstigator(object));

		if (!m_bRandomizeRagdoll)
			return;

		CharacterAnimationComponent animationComponent = CharacterAnimationComponent.Cast(entity.FindComponent(CharacterAnimationComponent));
		if (!animationComponent)
		{
			Print(string.Format("ScenarioFramework Action: Entity does not have animation component needed for action %1. Action won't randomize the ragdoll.", this), LogLevel.ERROR);
			return;
		}

		Math.Randomize(-1);

		vector randomDir = "0 0 0";
		randomDir[0] = Math.RandomIntInclusive(1, 3);
		randomDir[1] = Math.RandomIntInclusive(1, 3);
		randomDir[2] = Math.RandomIntInclusive(1, 3);

		animationComponent.AddRagdollEffectorDamage("1 1 1", randomDir, Math.RandomFloatInclusive(0, 50), Math.RandomFloatInclusive(0, 10), Math.RandomFloatInclusive(0, 20));
	}
}