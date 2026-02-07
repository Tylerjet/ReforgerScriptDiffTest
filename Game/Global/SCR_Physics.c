//------------------------------------------------------------------------------------------------
//! SCR_Physics Class
//!
//! Contains various physics functions
//------------------------------------------------------------------------------------------------
class SCR_Physics
{
	//------------------------------------------------------------------------------------------------
	//! Iterates through a physics object's colliders and remaps the input physics interaction layer to the replace physics interaction layer
	//! Returns how many colliders were remapped
	static int RemapInteractionLayer(notnull Physics physicsObject, EPhysicsLayerDefs inputLayer, EPhysicsLayerDefs replaceLayer)
	{
		int numRemapped = 0; 
		for (int i = physicsObject.GetNumGeoms() - 1; i >= 0; i--)
		{
			int layerMask = physicsObject.GetGeomInteractionLayer(i);
			if (!(layerMask & inputLayer))
				continue;
			
			layerMask &= ~inputLayer; // Remove input layer
			layerMask |= replaceLayer; // Add replace layer
			
			physicsObject.SetGeomInteractionLayer(i, layerMask);
			numRemapped++;
		}
		
		return numRemapped;
	}
};