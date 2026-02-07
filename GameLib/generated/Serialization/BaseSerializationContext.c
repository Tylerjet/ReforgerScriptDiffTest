/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Serialization
\{
*/

/*!
Base class for serialization contexts
*/
class BaseSerializationContext: Managed
{
	proto external bool IsValid();
	/*!
	Type discriminator is used to add polymorph object instance support to the serializer. Type is written inside the object when saving. When the load happens, it creates the instance based on this type.
	*/
	proto external void EnableTypeDiscriminator(string fieldName = "$type");
	proto external bool IsTypeDiscriminatorEnabled();
	proto external bool StartObject(string name);
	proto external bool EndObject();
}

/*!
\}
*/
