class SCR_GarbageManagerClass : GarbageManagerClass
{
};
class SCR_GarbageManager : GarbageManager
{
	[Attribute(category: "Garbage", desc: "When enabled, remove only entities that were destroyed, not abandoned (e.g., vehicles players left behind).\nEntities that cannot be destroyed are always marked for removal.")]
	protected bool m_bOnlyDestroyed;
	
	protected override bool CanInsert(IEntity ent)
	{
		if (m_bOnlyDestroyed)
		{
			DamageManagerComponent damageManager = DamageManagerComponent.Cast(ent.FindComponent(DamageManagerComponent));
			if (damageManager && damageManager.GetState() != EDamageState.DESTROYED)
				return false;
		}
		
		return super.CanInsert(ent);
	}
};
