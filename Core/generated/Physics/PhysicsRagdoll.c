/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup Physics
* @{
*/

sealed class PhysicsRagdoll: pointer
{
	proto external void Destroy(bool keepbones);
	proto external bool Enable();
	proto external bool Disable(bool keepBones);
	proto external Physics GetBoneRigidBodyByNodeIndex(int nodeIndex);
	proto external Physics GetBoneRigidBody(int index);
	proto external int GetNumBones();
	static proto PhysicsRagdoll CreateRagdoll(notnull IEntity owner, string ragdollDefName, float mass, int layerMask);
	static proto PhysicsRagdoll GetRagdoll(notnull IEntity owner);
};

/** @}*/
