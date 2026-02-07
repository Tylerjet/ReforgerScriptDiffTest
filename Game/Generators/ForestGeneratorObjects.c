//-----------------------------------------------------------------------
// Bindings of C++ helpers for ForestGeneratorEntity
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
//! Do not change this unless you change c++ as well
class ForestGeneratorDistaceAttribute
{
	DistanceType m_iDistanceType;
	void ForestGeneratorDistaceAttribute(DistanceType distanceType)
	{
		m_iDistanceType = distanceType;
	}
};

//-----------------------------------------------------------------------
class ForestGeneratorGroupIndexAttribute
{
};

//-----------------------------------------------------------------------
/*!
Used to anotate a vector which signifies the first point of a capsule segment
If a member with the ForestGeneratorCapsuleEndAttribute is not specified, 
this is used as an offset i.e. both ends of the segment are set to this value.
Only X and Z fields of the vector are used.
*/
class ForestGeneratorCapsuleStartAttribute
{
};

//-----------------------------------------------------------------------
//! \see ForestGeneratorCapsuleStartAttribute
class ForestGeneratorCapsuleEndAttribute
{
};

//-----------------------------------------------------------------------
class SCR_ForestGeneratorTreeBase : ForestGeneratorTreeBase
{
	[Attribute("0.8", UIWidgets.SpinBox, "Min scale of this object", params: "0 1000 0.01")]
	float m_fMinScale;
	
	[Attribute("1.2", UIWidgets.SpinBox, "Max scale of this object", params: "0 1000 0.01")]
	float m_fMaxScale;
	
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Prefab of this object", "et")]
	ResourceName m_Prefab;
	
	[Attribute(defvalue: "1", uiwidget: UIWidgets.SpinBox, "Minimum required radius in the bottom layer for this object to spawn"), ForestGeneratorDistaceAttribute(DistanceType.BOTTOM)];
	float m_fBotDistance;
	
	[Attribute("0", uiwidget: UIWidgets.SpinBox, "Maximum random pitch angle", "0 180 1")]
	float m_fRandomPitchAngle;
	
	[Attribute("0", uiwidget: UIWidgets.SpinBox, "Maximum random roll angle", "0 180 1")]
	float m_fRandomRollAngle;
	
	[Attribute("0")]
	float m_fVerticalOffset;
	
	[ForestGeneratorGroupIndexAttribute()]
	int m_iGroupIndex;
	
	float m_fScale = 2;
	TreeType m_Type;
	
	void AdjustScale();
	
	//-----------------------------------------------------------------------
	void CopyValues(notnull SCR_ForestGeneratorTreeBase newObject)
	{
		newObject.m_fMinScale = m_fMinScale;
		newObject.m_fMaxScale = m_fMaxScale;
		newObject.m_iGroupIndex = m_iGroupIndex;
		newObject.m_Prefab = m_Prefab;
		newObject.m_fBotDistance = m_fBotDistance;
		newObject.m_fScale = m_fScale;
		newObject.m_Type = m_Type;
		newObject.m_fVerticalOffset = m_fVerticalOffset;
		
		newObject.m_fRandomPitchAngle = m_fRandomPitchAngle;
		newObject.m_fRandomRollAngle = m_fRandomRollAngle;
	}
	
	//-----------------------------------------------------------------------
	ForestGeneratorTreeBase Copy()
	{
		SCR_ForestGeneratorTreeBase newObject = new ForestGeneratorTree();
		CopyValues(newObject);
		return newObject;
	}
};

//-----------------------------------------------------------------------
class ForestGeneratorShapeImportData : Managed
{
	IEntitySource source;
	IEntity entity;
	int id;
	ref AAB bbox;
	ref array<vector> points = new ref array<vector>();
	
	void GenerateAAB()
	{
		bbox = AAB.MakeFromPoints(points);
	}
	
	void ~ForestGeneratorShapeImportData()
	{
		points = null;
		bbox = null;
	}
};