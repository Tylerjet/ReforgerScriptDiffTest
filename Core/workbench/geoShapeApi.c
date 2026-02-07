#ifdef WORKBENCH

////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                        //
//            Unified API for reading geo shape files (such as ESRI Shapefiles)           //
//                                                                                        //
////////////////////////////////////////////////////////////////////////////////////////////

//! GeoShape type. Set for every shape. Helps with shape type-casting.
enum GeoShapeType
{
	POINT,			//!< single point
	MULTI_POINT,	//!< set of points
	POLYLINE,		//!< line string
	POLYGON,		//!< polygon
	UNKNOWN			//!< unknown / unsupported shape type, can contain attributes at least
}

//! Shape attribute data type.
enum GeoAttribType
{
	INT,		//!< integer
	FLT,		//!< float
	STR,		//!< string
	UNKNOWN		//!< unknown / unsupported data type, attribute value cannot be retrieved
}

/*!
GeoShape's attribute set.
Note: For ESRI Shapefiles, all shapes in a Shapefile will have the same attributes,
i.e. Count(), GetName(), GetType(), GetIndexByName() and HasAttrib() will always
return the same value for all shapes in the file for a given arguments.
THIS MAY NOT BE TRUE FOR OTHER FORMATS (e.g. GeoJSON).
*/
class GeoAttribCollection
{
	//! Total number of attributes in the collection.
	proto native int			Count();

	/*!
	Gets attribute's name.
	\param	index	attribute index [0 .. count-1]
	*/
	proto native string	GetName(int index);

	/*!
	Gets attribute's data type. Determines which Get*(int index) method should
	be use to read the value.
	\param	index	attribute index [0 .. count-1]
	*/
	proto native GeoAttribType	GetType(int index);

	/*!
	True iff the attribute has a value (is not null).
	Note: This may not be reliable with ESRI shapefiles as they don't officially
	support null values.
	\param	index	attribute index [0 .. count-1]
	*/
	proto native bool			IsAttribSet(int index);

	/*!
	Gets an attribute's value as int.
	\param	index	attribute index [0 .. count-1]
	\return	Original value if attrib's type is INT, floored value if attrib's
			type is FLT, 0 otherwise.
	*/
	proto native int			GetInt(int index);

	/*!
	Gets an attribute's value as float.
	\param	index	attribute index [0 .. count-1]
	\return	Original value if attrib's type is FLT, type-casted value if
			attrib's type is INT, 0 otherwise.
	*/
	proto native float			GetFloat(int index);

	/*!
	Gets an attribute's value as string.
	\param	index	attribute index [0 .. count-1]
	\return	Original value if attrib's type is STR, otherwise it yields in
			undefined behavior.
	*/
	proto native string	GetString(int index);

	/*!
	Gets an attribute's index by its name.
	\param	name	attribute's name
	\return	Attribute's index or -1 if there is no such attribute.
	*/
	proto native int			GetIndexByName(string name);

	/*!
	\return	True iff a given attribute exists in a collection. (Still, it may
			not be set: may have no value, be null).
	\param	name	attribute's name
	*/
	proto native bool			HasAttrib(string name);

	/*!
	True iff the attribute exists and has a value (is not null).
	Note: All *ByName(string) method versions are slower than their counterparts
	using attribute's index.
	\param	name	attribute's name
	\see	IsAttribSet(int index)
	*/
	proto native bool			IsAttribSetByName(string name);

	/*!
	Gets an attribute's value as int.
	Note: All *ByName(string) method versions are slower than their counterparts
	using attribute's index.
	\param	name	attribute's name
	\return	Original value if attrib's type is INT, floored value if attrib's
			type is FLT, 0 if the attribute does not exist or is of another type.
	*/
	proto native int			GetIntByName(string name);

	/*!
	Gets an attribute's value as float.
	Note: All *ByName(string) method versions are slower than their counterparts
	using attribute's index.
	\param	name	attribute's name
	\return	Original value if attrib's type is FLT, type-casted value if
			attrib's type is INT, 0 if the attribute does not exist or is of
			another type.
	*/
	proto native float			GetFloatByName(string name);

	/*!
	Gets an attribute's value as string.
	Note: All *ByName(string) method versions are slower than their counterparts
	using attribute's index.
	\param      name  attribute's name
	\return     Original value if attrib's type is STR, undefined behavior
	            if the attribute does not exist or is of another type.
	*/
	proto native string			GetStringByName(string name);
}

//! Collection of vertices (points). Can be treated like read-only array.
class GeoVertexCollection
{
	proto native int	Count();
	proto native vector Get(int index);
}

//! Base class for all shapes.
class GeoShape
{
	proto native GeoShapeType			GetType(); //!< Shape's type
	proto native GeoAttribCollection	GetAttributes(); //!< Shape's attributes
}

//! Single point.
class GeoPoint extends GeoShape
{
	proto native vector	GetCoords(); //!< Coordinates of the point.
}

//! Set of points.
class GeoMultiPoint extends GeoShape
{
	proto native GeoVertexCollection GetPoints(); //!< Coordinates of all points in the set.
}

//! Line string.
class GeoPolyline extends GeoShape
{
	proto native GeoVertexCollection GetVertices(); //!< Coordinates of all vertices of the line string.
}

//! Polygon (can have multiple parts).
class GeoPolygon extends GeoShape
{
	// TODO: comments
	proto native int				PartsCount();
	proto ref GeoVertexCollection	GetPart(int index);
}

/*!
Collection of shapes, i.e. (loaded) geo shape file interface. Can be treated
like read-only array.
*/
class GeoShapeCollection
{
	proto native int	Count();
	proto ref GeoShape	Get(int index);
}

//! API entry point - static class for loading geo shape files.
class GeoShapeLoader
{
	proto native static GeoShapeCollection LoadShapeFile(string fileName);
}

#endif