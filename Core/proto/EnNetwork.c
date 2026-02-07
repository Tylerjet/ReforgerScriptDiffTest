/*
// Example: Custom codec functions.
// This allows using CustomClass instances as arguments of RPCs or as replicated properties marked with
// RplProp attribute.
class CustomClass
{
	//! Takes snapshot and encodes it into packet using as few bits as possible.
	static void Encode(SSnapSerializerBase snapshot, ScriptCtx ctx, ScriptBitSerializer packet);

	//! Takes packet and decodes it into snapshot. Returns true on success or false when an error occurs.
	static bool Decode(ScriptBitSerializer packet, ScriptCtx ctx, SSnapSerializerBase snapshot);

	//! Compares two snapshots. Returns true when they match or false otherwise.
	static bool SnapCompare(SSnapSerializerBase lhs, SSnapSerializerBase rhs, ScriptCtx ctx);

	//! Compares instance against snapshot. Returns true when they match or false otherwise.
	static bool PropCompare(CustomClass instance, SSnapSerializerBase snapshot, ScriptCtx ctx);

	//! Writes data from an instance into snapshot. Opposite of Inject().
	static bool Extract(CustomClass instance, ScriptCtx ctx, SSnapSerializerBase snapshot);

	//! Writes data from snapshot into instance. Opposite of Extract().
	static bool Inject(SSnapSerializerBase snapshot, ScriptCtx ctx, CustomClass instance);
}
*/
typedef int RplIdentity;
typedef int RplId;

/*! Replication connection identity. */
class RplIdentity
{
	proto native static RplIdentity Local();
	proto native static RplIdentity Invalid();

	proto native bool IsValid();
};

/*! Replication item identifier */
class RplId
{
	proto native static RplId Invalid();

	proto native bool IsValid();
};

/*!
Communication channel. Reliable is guaranteed to be delivered. Unreliable not.
*/
enum RplChannel
{
	Reliable,
	Unreliable
};


/*!
Target of the RPC call. Delivery depends on the ownership of caller and networking mode.

Mode = SERVER
|              | Server | Owner  | Broadcast   |
|--------------|--------|--------|-------------|
| Owner        | self   | self   | all clients |
| Not Owner    | self   | client | all clients |

Mode = CLIENT
|              | Server | Owner  | Broadcast   |
|--------------|--------|--------|-------------|
| Owner        | server | self   | self        |
| Not Owner    | drop   | drop   | self        |

*/
enum RplRcver
{
	Server,
	Owner,
	Broadcast
};

/*!
Entity networking role.
Proxy = receive props only
Authority = send props and send RPCS
*/
enum RplRole
{
	Authority,
	Proxy
};


/*!
Property groups that allow for replication of only part of the object.
*/
enum RplGroup
{
	Mandatory,
	Group_1,
	Group_2,
	Group_3,
	Group_4,
	Group_5,
	Group_6
};

/*!
Conditional replication rule.
*/
enum RplCondition
{
	None,
	OwnerOnly,
	NoOwner,
	Custom
};

class ScriptCtx : Managed
{
};

/*!
Networking base class.
*/
class Replication
{
	/*! Common invalid id. Used for invalid RplIds, ConnectionIds... */
	const RplId INVALID_ID = 0xFFFFFFFF;
	const RplIdentity INVALID_IDENTITY = 0xFFFFFFFF;

	/*!
	Looks for RplId assigned by the Replication layer.
	Warning: It is a lookup! Don't use in tight loops!
	\returns id or INVALID_ID if entity is not part of the replication process.
	*/
	proto native static RplId FindId(Managed entity);

	/*!
	Lookds for Item of the provided id.
	\returns Item or null when no item with provided id exists.
	*/
	proto native static Managed FindItem(RplId id);

	/*!
	Tries to look for the owner of provided replication id. When successful
	returns its identity. Failure returns INVALID_ID constant.
	*/
	proto native static int FindOwner(RplId id);

	/*! Should be called before Run() to setup the mode of replication. */
	//proto native static void SetMode(int mode);

	/*!
	Master time for all reaplication instances.
	(Exponentialy smoothed, is dependent on connection quality)
	*/
	proto native static float Time();

	/*! Tells whenever the replication is active. */
	proto native static bool IsRunning();

	/*! The replication is currently in runtime mode. The inserted nodes will
	 * receive a dynamic IDs they will be automatically replicated to relevant
	 * clients. */
	proto native static bool Runtime();

	/*! The replication is currently in state of world loading. The inserted nodes
	 * will receive a static IDs and have to be spawned/present on both (Server
	 * and Client). */
	proto native static bool Loadtime();

	/*!
	Removes the Entity from replication process immediately. (crash prevention)
	Usage of this method should be the last resort for preventing crashes and
	should be omitted when possible.
	#ToBeRemoved
	*/
	proto native static void RemoveEntity(IEntity entity);

	/*!
	Notifies replication systems about changes in your object and queues him for replication in near future.
	*/
	proto native static void BumpMe();

	/*!
	WARNING! INTERNAL PROFILING FEATURE ONLY!
	Sets the number of virtual connections which are just generating load for the master.
	*/
	proto native static void SetNumVirtualConnections(int num);

	/*!
	WARNING! Using the hardcoded IsServer will result in code thats not modular
	and won't adapt when running in different environment consider using the
	replication roles and ownership instead.
	*/
	proto native static bool IsServer();

	/*!
	WARNING! Using the hardcoded IsClient will result in code thats not modular
	and won't adapt when running in different environment consider using the
	replication roles and ownership instead.
	*/
	proto native static bool IsClient();
};


/*!
Property annotation attribute. Use to enable property replication on Entities and components.
*/
class RplProp
{
	RplGroup		 m_Group;
	RplCondition m_Condition;
	string			 m_CustomCondName;
	string			 m_OnRplName;
	ScriptCtx		 m_pCtx;

	void RplProp(RplGroup			group = RplGroup.Mandatory,
							 string				onRplName = "",
							 ScriptCtx		ctx = NULL,
							 RplCondition condition = RplCondition.None,
							 string				customConditionName = "")
	{
		m_Group = group;
		m_OnRplName = onRplName;
		m_pCtx = ctx;
		m_Condition = condition;
		m_CustomCondName = customConditionName;
	}
};


/*!
RPC annotation attribute. Use to mark method as RPC method.
Remote call is executed via Rpc(Method, Arg1, Arg2, ...).
*/
class RplRpc
{
	RplChannel	 m_Channel;
	RplRcver		 m_Rcver;
	RplCondition m_Condition;
	string			 m_CustomCondName;

	void RplRpc(RplChannel	 channel,
							RplRcver		 rcver,
							RplCondition condition = RplCondition.None,
							string			 customConditionName = "")
	{
		m_Channel = channel;
		m_Rcver = rcver;
		m_Condition = condition;
		m_CustomCondName = customConditionName;
	}
};


/*!
OnReplication annotation attribute. Annotate method that will get called when properties are replicated.

[NetOnInjected()]
void OnInjected(int mask) {}
*/
class OnRpl
{
};

class RplBeforeInjected
{
};

class RplBeforeExtracted
{
};

class ScriptBitWriter : Managed
{
	/*!
	Writes sizeInBits bits of data to internal storage.
	*/
	proto void Write(void src, int sizeInBits);
	/*!
	Compresses a RplId and writes it to internal storage.
	The resulting data is about 1 to 4 bytes in size.
	*/
	proto void WriteRplId(RplId val);
	/*!
	Compresses a EntityID and writes it to internal storage.
	The resulting data is about 1 to 8 bytes in size.
	*/
	proto void WriteEntityId(EntityID val);
	/*!
	Compresses an integer and writes it to internal storage.
	Compression goes down to the byte level. Therefore, the resulting data is
	about 1 to 4 bytes in size.
	
	Warning:
	If you know the range is constant use WriteCompressed_IntRange instead
	*/
	proto void WriteInt(int val);
	/*!
	Compresses an integer and writes it to internal storage.
	Compression uses only as many bits as necessary taking the range defined by {min..max}
	into account.
	
	Warning:
	Use this only when you know the range is constant!
	*/
	proto void WriteIntRange(int val, int min, int max);
	/*!
	Compresses a float into a half and writes it to internal storage.
	The resulting data is 2 bytes in size.
	
	Warning:
	Half only gives you about 64k +/- or range for the decimal part.
	*/
	proto void WriteHalf(float val);
	/*!
	Compresses a float into <0;1> range and writes it to internal storage.
	The resulting data is 1 byte in size.
	
	Warning:
	Half only gives you about 64k +/- or range for the decimal part.
	*/
	proto void WriteFloat01(float val);
	/*!
	Compresses an angle in radians and writes it to internal storage.
	The resulting data is 1 byte in size.
	*/
	proto void WriteRadian(float val);
	/*!
	Compresses a quaternion and writes it to internal storage.
	The resulting data is 4 byte in size.
	*/
	proto void WriteQuaternion(float val[4]);
	/*!
	Writes a ResourceName to internal storage.
	The resulting data is 8 byte in size.
	*/
	proto void WriteResourceName(ResourceName val);
	/*!
	Writes a string to internal storage.
	*/
	proto void WriteString(string val);
	/*!
	Returns the current position in internal storage in bits.
	*/
	proto native int Tell();

	//----------------------------------------------------------------------------
	// Helper functions for built-in types.
	// They are safe defaults but they might use more bits than necessary.

	void WriteBool(bool val) { Write(val, 1); }	
	void WriteFloat(float val) { Write(val, 32); }
	void WriteVector(vector val)
	{
		// 3 values * 32-bits each = 96
		Write(val, 96);
	}
}

class ScriptBitReader : Managed
{
	/*!
	Reads sizeInBits bits of data from internal storage.
	*/
	proto bool Read(out void dst, int sizeInBits);
	/*!
	Decompresses and returns a RplId from internal storage.
	*/
	proto bool ReadRplId(out RplId val);
	/*!
	Decompresses and returns a EntityID from internal storage.
	*/
	proto bool ReadEntityId(out EntityID val);
	/*!
	Decompresses and returns an integer from internal storage.
	*/
	proto bool ReadInt(out int val);
	/*!
	Decompresses and returns an integer from internal storage.
	*/
	proto bool ReadIntRange(out int val, int min, int max);
	/*!
	Decompresses and returns a float from internal storage.
	*/
	proto bool ReadHalf(out float val);
	/*!
	Decompresses and returns a float from internal storage.
	*/
	proto bool ReadFloat01(out float val);
	/*!
	Decompresses and returns a float from internal storage.
	*/
	proto bool ReadRadian(out float val);
	/*!
	Decompresses and returns a quaternion from internal storage.
	*/
	proto bool ReadQuaternion(out float val[4]);
	/*!
	Reads and returns a ResourceName from internal storage.
	*/
	proto bool ReadResourceName(out ResourceName val);
	/*!
	Reads a string from internal storage.
	*/
	proto bool ReadString(out string val);
	/*!
	Returns the current position in internal storage in bits.
	*/
	proto native int Tell();

	//----------------------------------------------------------------------------
	// Helper functions for built-in types.
	// They are safe defaults but they might use more bits than necessary.

	bool ReadBool(out bool val) { return Read(val, 1); }
	bool ReadFloat(out float val) { return Read(val, 32); }
	bool ReadVector(out vector val)
	{
		// 3 values * 32-bits each = 96
		return Read(val, 96);
	}
}

class ScriptBitSerializer : Managed
{
	//! Serializes the data pointer. The size is the amount of bits serialized.
	proto bool Serialize(inout void data, int sizeInBits);
	proto bool SerializeRplId(inout RplId val);
	proto bool SerializeEntityId(inout EntityID val);
	proto bool SerializeInt(inout int val);
	proto bool SerializeIntRange(inout int val, int min, int max);
	proto bool SerializeHalf(inout float val);
	proto bool SerializeFloat01(inout float val);
	proto bool SerializeRadian(inout float val);
	proto bool SerializeQuaternion(inout float val[4]);
	proto bool SerializeResourceName(inout ResourceName val);
	proto bool SerializeString(inout string val);
	//! Returns the current position in the buffer with bit precision.
	proto native int Tell();

	//----------------------------------------------------------------------------
	// Helper functions for built-in types.
	// They are safe defaults but they might use more bits than necessary.

	bool SerializeBool(inout bool val) { return Serialize(val, 1); }
	bool SerializeFloat(inout float val) { return Serialize(val, 32); }
	bool SerializeVector(inout vector val)
	{
		// 3 values * 32-bits each = 96
		return Serialize(val, 96);
	}
}

class SSnapSerializerBase: Managed
{
	//! Serializes the data pointer. The size is the amount of bytes serialized.
	proto void SerializeBytes(inout void data, int sizeInBytes);

	void SerializeBool(inout bool val) { SerializeBytes(val, 4); }
	void SerializeInt(inout int val) { SerializeBytes(val, 4); }
	void SerializeFloat(inout float val) { SerializeBytes(val, 4); }
	void SerializeVector(inout vector val) { SerializeBytes(val, 12); }

	void EncodeBool(ScriptBitSerializer packet)
	{
		bool val;
		this.SerializeBytes(val, 4);
		packet.Serialize(val, 1);
	}
	void DecodeBool(ScriptBitSerializer packet)
	{
		bool val;
		packet.Serialize(val, 1);
		this.SerializeBytes(val, 4);
	}

	void EncodeInt(ScriptBitSerializer packet)
	{
		int val;
		this.SerializeBytes(val, 4);
		packet.Serialize(val, 32);
	}
	void DecodeInt(ScriptBitSerializer packet)
	{
		int val;
		packet.Serialize(val, 32);
		this.SerializeBytes(val, 4);
	}

	void EncodeFloat(ScriptBitSerializer packet)
	{
		float val;
		this.SerializeBytes(val, 4);
		packet.Serialize(val, 32);
	}
	void DecodeFloat(ScriptBitSerializer packet)
	{
		float val;
		packet.Serialize(val, 32);
		this.SerializeBytes(val, 4);
	}

	void EncodeVector(ScriptBitSerializer packet)
	{
		vector val;
		this.SerializeBytes(val, 12);
		packet.Serialize(val, 96);
	}
	void DecodeVector(ScriptBitSerializer packet)
	{
		vector val;
		packet.Serialize(val, 96);
		this.SerializeBytes(val, 12);
	}

	//! Serialization of the BitSerializer type. The size is the amount of bytes
	//! written/read from the bit serializer.
	proto native bool Serialize(ScriptBitSerializer serializer, int sizeInBytes);

	//! Returns the current position within the buffer with byte precision.
	proto native int Tell();

	//! Compares the insides of the buffer with provided pointer (bitwise). Size
	//! is the amount of read bytes from the data.
	proto bool Compare(void data, int sizeInBytes);
	bool CompareBool(bool val) { return Compare(val, 4); }
	bool CompareInt(int val) { return Compare(val, 4); }
	bool CompareFloat(float val) { return Compare(val, 4); }
	bool CompareVector(vector val) { return Compare(val, 12); }

	//! Compares the contents of two SnapSerialiers. The size is amount of bytes
	//! used.
	proto native bool CompareSnapshots(SSnapSerializerBase snapshot, int sizeInBytes);

	private void SSnapSerializerBase();
	private void ~SSnapSerializerBase();
}

//! Binary data container used in conjuction with a serializer.
class SSnapshot : Managed
{
	void SSnapshot(int sizeInBytes);
}

//! Snapshot serializer utility.
class SSnapSerializer : SSnapSerializerBase
{
	//! Creates a writer only serializer for given snapshot.
	proto static ref SSnapSerializer MakeWriter(SSnapshot snap);
	//! Creates a read only serializer for given snapshot.
	proto static ref SSnapSerializer MakeReader(SSnapshot snap);

	//! Sets current position within the buffer with byte precision.
	proto native int Seek(int posInBytes);
}