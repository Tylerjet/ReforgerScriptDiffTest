/**
 * \defgroup Types
 * \desc Enforce script essentials
 * @{
 */

//!Helper for passing string expression to functions with void parameter. Example: Print(String("Hello " + var));
string String(string s)
{
	return s;
}

/*!
\brief Vector constructor from components
    \param x \p float x component
	\param y \p float y component
	\param z \p float z component
	\return \p vector resulting vector
	@code
		Print( Vector(1, 2, 3) );

		>> <1,2,3>
	@endcode
*/
proto native vector Vector(float x, float y, float z);

 //! Super root of all classes in Enforce script
class Class
{
	/**
	\brief Returns true when instance is of the type, or inherited one.
	\param type Class type
	\returns \p bool true when 'clType' is the same as 'type', or inherited one.
	@code
		if (inst && inst.IsInherited(Widget))
		{
			Print("inst is inherited from Widget class!");		
		}
	@endcode
	*/
	proto native external bool IsInherited(typename type);
	
	//! Get actual size of instance including size of all referenced objects
	proto native external int GetSizeOf();
	
	/**
	\brief Returns name of class-type
		\param inst Class
		\returns \p string class-type
		@code
			Man player = g_Game.GetPlayer();
			string className = player.ClassName();
			Print(className);

			>> className = 'Man'
		@endcode
	*/
	proto native owned external string ClassName();
	
	/**
	\brief Returns typename of object's class
		\returns \p typename class-type
		@code
			Man player = g_Game.GetPlayer();
			typename type = player.Type();
			Print(type.ToString());

			>> 'Man'
		@endcode
	*/
	proto native external typename Type();
	
	proto external string ToString();
	
	//! Get actual count of references holding this instance. If instance is not managed, zero is returned.
	proto native external int GetRefCount();
	
	/**
	\brief Try to safely down-cast base class to child class.
		\returns down-casted 'from' pointer when cast is successfull (classes are related), or null if casting is invalid
		@code
			// assume that Man inheites from Object
			Object obj = g_Game.GetPlayer();
			Man player = Man.Cast(obj);
			
			if (player)
			{
				// horay!
			}
		@endcode
	*/		
	proto static Class Cast(Class from);
	
	//! This function is for internal script usage
	private proto static bool SafeCastType(out typename type, out Class to, Class from);
};

class Managed
{
};

//! Base class for classes which combine enf::Class and enf::BaseClass (in C++), all its ancestors present in script will be automatically registered as config classes
class ScriptAndConfig: Managed
{
}

//! Plain C++ pointer, no weak pointers, no mem management
class pointer
{
	proto string ToString();
};

class func
{
	//! For internal usage of VM 
	private proto void SetInstance(Managed inst);
};

//! script representation for C++ RTTI types
class TypeID: pointer
{
};

class array<Class T>: Managed
{
	/*!
	O(1) complexity.
	\return Number of elements of the array
	*/
	proto native int Count();

	/*!
	\return True if the array size is 0, false otherwise
	*/
	proto native bool IsEmpty();
	
	/*!
	Destroyes all elements of the array and sets the Count to 0.
	The underlying memory of the array is not freed.
	*/
	proto native void Clear();

	/*!
	Frees any underlying memory which is not used.
	For example if the array allocated enough memory for 100 items but only 1 us used
	(Count() is 1) this frees the memory taken by the remaining 99 items.
	\warning
	Memory allocation and deallocation are expensive so only use this function if you
	know you won't be adding new items to the array on frame-by-frame basis or when
	the memory consumption is of upmost importance.
	*/
	proto native void Compact();

	/*!
	Sets n-th element to given value.
	*/
	proto void Set(int n, T value);

	/*!
	Tries to find the first occurance of given value in the array.
	\return Index of the first occurance of `value` if found, -1 otherwise
	\note
	This method has complexity O(n)
	*/
	proto int Find(T value);
	
	/*!
	Return if value is in array or not. 
	\note
	This method has complexity O(n)
	*/
	proto bool Contains(T value);

	/*!
	\return Element at the index `n`
	*/
	proto T Get(int n);
	
	/*!
	Inserts element at the end of array.
	\param value
	Element to be inserted
	\return
	Position at which element is inserted
	*/
	proto int Insert(T value);
	
	/*!
	Inserts element at certain position and moves all elements behind
	this position by one.
	\param value
	Element to be inserted
	\param index
	Position at which element is inserted. Must be less than Array::GetCardinality()
	\return
	Number of elements after insertion
	*/
	proto int InsertAt(T value, int index);
	
	/*!
	\brief Inserts all elements from array
		\param from \p array<T> array from which all elements will be added
		@code
			TStringArray arr1 = new TStringArray;
			arr1.Insert( "Dave" );
			arr1.Insert( "Mark" );
			arr1.Insert( "John" );
			
			TStringArray arr2 = new TStringArray;
			arr2.Insert( "Sarah" );
			arr2.Insert( "Cate" );
			
			arr1.InsertAll(arr2);
			
			for ( int i = 0; i < arr1.Count(); i++ )
			{
				Print( arr1.Get(i) );
			}
			
			>> "Dave"
			>> "Mark"
			>> "John"
			>> "Sarah"
			>> "Cate"
		@endcode
	*/
	void InsertAll(notnull array<T> from)
	{
		for ( int i = 0; i < from.Count(); i++ )
		{
			Insert( from.Get(i) );
		}
	}

	/*!
	Removes element from array. The empty position is replaced by
	last element, so removal is quite fast but do not retain order.
	\param index
	Index of element to be removed
	*/
	proto native void Remove(int index);

	/*!
	Removes element from array, but retain all elements ordered. It's
	slower than Remove
	\param index
	Index of element to be removed
	*/
	proto native void RemoveOrdered(int index);

	/*!
	Resizes the array to given size.
	If the `newSize` is lower than current Count overflowing objects are destroyed.
	If the `newSize` is higher than current Count missing elements are initialized to zero (null).
	*/
	proto native void Resize(int newSize);

	/*!
	Swaps the contents of this and `other` arrays.
	Does not involve copying of the elements.
	*/
	proto native void Swap(notnull array<T> other);

	/*!
	Sorts elements of array, depends on underlaying type.
	*/
	proto native void Sort(bool reverse = false);
	
	/*!
	Copes contents of `from` array to this array.
	\return How many elements were copied
	*/
	proto int Copy(notnull array<T> from);

	proto int Init(T init[]);

	void RemoveItem(T value)
	{
		int remove_index = Find(value);

		if ( remove_index >= 0 )
		{
			Remove(remove_index);
		}
	}

	void RemoveItemOrdered(T value)
    {
        int remove_index = Find( value );
        
        if ( remove_index >= 0 )
        {
            RemoveOrdered( remove_index );
        }
    }
	
	/**
	\brief Print all elements in array
		\return \p void
		@code
			my_array.Debug();

			>> "One"
			>> "Two"
			>> "Three"
		@endcode
	*/
	void Debug()
	{
		PrintFormat( "Array count: %1", Count());
		
		for ( int i = 0; i < Count(); i++ )
		{
			T item = Get(i);

			if ( item )
			{
				PrintFormat("[%1] => %2", i, string.ToString(item));
			}
		}
	}

	/**
	\brief Returns a random index of array. If Count is 0, return index is -1 .
		\return \p int Random index of array
		@code
			Print( my_array.GetRandomIndex() );

			>> 2
		@endcode
	*/
	int GetRandomIndex()
	{
		if ( Count() > 0 )
		{
			return Math.RandomInt(0, Count());
		}
		
		return -1;	
	}

	/**
	\brief Returns a random element of array
		\return \p int Random element of array
		@code
			Print( my_array.GetRandomElement() );

			>> "Three"
		@endcode
	*/
	T GetRandomElement()
	{
		return Get(GetRandomIndex());
	}

	void SwapItems(int item1_index, int item2_index)
	{
		T item1 = Get(item1_index);
		Set(item1_index, Get(item2_index));
		Set(item2_index, item1);
	}
};

//force these to compile so we can link C++ methods to them
typedef array<string> TStringArray;
typedef array<float> TFloatArray;
typedef array<int> TIntArray;
typedef array<bool> TBoolArray;
typedef array<Class> TClassArray;
typedef array<Managed> TManagedArray;
typedef array<ref Managed> TManagedRefArray;
typedef array<vector> TVectorArray;
typedef array<pointer> TPointerArray;
typedef array<ResourceName> TResourceNameArray;

class set<Class T>: Managed
{
	proto native int Count();
	/*!
	\return True if the set size is 0, false otherwise
	*/
	proto native bool IsEmpty();
	/*!
	Destroyes all elements of the array and sets the Count to 0.
	The underlying memory of the array is not freed.
	*/
	proto native void Clear();
	/*!
	Frees any underlying memory which is not used.
	For example if the set allocated enough memory for 100 items but only 1 us used
	(Count() is 1) this frees the memory taken by the remaining 99 items.
	\warning
	Memory allocation and deallocation are expensive so only use this function if you
	know you won't be adding new items to the array on frame-by-frame basis or when
	the memory consumption is of upmost importance.
	*/
	proto native void Compact();
	/*!
	Tries to find the first occurance of given value in the array.
	\return Index of the first occurance of `value` if found, -1 otherwise
	\note
	This method has complexity O(log n)
	*/
	proto int Find(T value);
	/*!
	Return if value is in array or not. 
	\note
	This method has complexity O(log n)
	*/
	proto bool Contains(T value);
	proto T Get(int n);
	/*!
	Inserts element at the end of array.
	\param value
	Element to be inserted
	\return
	true when element was inserted, false when it is already in set
	*/
	proto bool Insert(T value);
	/*!
	Removes element from array, but retain all elements ordered.
	\param index
	Index of element to be removed
	*/
	proto native void Remove(int index);
	proto int Copy(set<T> from);
	proto native void Swap(set<T> other);
	proto int Init(T init[]);
};

//force these to compile so we can link C++ methods to them
typedef set<string> TStringSet;
typedef set<float> TFloatSet;
typedef set<int> TIntSet;
typedef set<Class> TClassSet;
typedef set<Managed> TManagedSet;
typedef set<ref Managed> TManagedRefSet;
typedef set<pointer> TPointerSet;

typedef int MapIterator;
/**
 \brief Associative array template
 \n usage:
 @code
 map<string, int> prg_count = new map<string, int>;

 // fill
 prg_count.Insert("hello", 10);
 prg_count.Insert("world", 20);
 prg_count.Insert("!", 1);

 Print(prg_count.Get("world")); // prints '20'

 @endcode

 */
class map<Class TKey,Class TValue>: Managed
{
	/*!
		\return
		The number of elements in the hashmap.
	*/
	proto native int Count();

	/*!
	\return True if the map size is 0, false otherwise
	*/
	proto native bool IsEmpty();

	/*!
		Clears the hash map.
	*/
	proto native void Clear();
	/*!
		Search for an element with the given key.
			
		\param key
		The key of the element to find
		\return
		Pointer to element data if found, NULL otherwise.
	*/
	proto TValue Get(TKey key);
	/*!
		Search for an element with the given key.
			
		\param key
		The key of the element to find
		\param val
		result is stored to val
		\return
		returns True if given key exist.
	*/
	proto bool Find(TKey key, out TValue val);
	/*!
	Return the i:th element in the map.
	Note: This operation is O(n) complexity. Use with care!

	\param index
	The position of the element in the map
	\return
	The element on the i:th position
	*/
	proto TValue GetElement(int index);
	/*!
	Return the i:th element key in the map.
	Note: This operation is O(n) complexity. Use with care!

	\param i
	The position of the element key in the map
	\return
	Return key of i-th element
	*/
	proto TKey GetKey(int i);
	/*!
	Sets value of element with given key. If element with key not exists, it is created.
	Note: creating new elements is faster using Insert function.
	*/
	proto void Set(TKey key, TValue value);
	/*!
	Removes element with given key.
	*/
	proto void Remove(TKey key);
	/*!
	Removes i:th element with given key.
	Note: This operation is O(n) complexity. Use with care!
	\param i
	The position of the element key in the map
	*/
	proto void RemoveElement(int i);
	/*!
	Returns if map contains element with given key.
	*/
	proto bool Contains(TKey key);
	/*!
	Insert new element into hash map.
		
	\param key
	Key of element to be inserted.
	\param value
	Data of element to be inserted.
	*/
	proto bool Insert(TKey key, TValue value);
	proto int Copy(map<TKey,TValue> from);
	
	bool ReplaceKey(TKey old_key, TKey new_key)
	{
		if (Contains(old_key))
		{
			Set(new_key, Get(old_key));
			Remove(old_key);
			return true;
		}	
		return false;
	}
	
	TKey GetKeyByValue(TValue value)
	{
		TKey ret;
		for (int i = 0; i < Count(); i++)
		{
			if (GetElement(i) == value) 
			{
				ret = GetKey(i);
				break;
			}
		}

		return ret;
	}

	proto native MapIterator Begin();
	proto native MapIterator End();
	proto native MapIterator Next(MapIterator it);
	proto TKey GetIteratorKey(MapIterator it);
	proto TValue GetIteratorElement(MapIterator it);
};

typedef map<int, float>				TIntFloatMap;
typedef map<int, int>					TIntIntMap;
typedef map<int, string>			TIntStringMap;
typedef map<int, Class>				TIntClassMap;
typedef map<int, Managed>			TIntManagedMap;
typedef map<int, ref Managed>	TIntManagedRefMap;
typedef map<int, pointer>		TIntPointerMap;
typedef map<int, vector>			TIntVectorMap;

typedef map<string, float>		TStringFloatMap;
typedef map<string, int>			TStringIntMap;
typedef map<string, string>		TStringStringMap;
typedef map<string, Class>		TStringClassMap;
typedef map<string, Managed>	TStringManagedMap;
typedef map<string, ref Managed>	TStringManagedRefMap;
typedef map<string, pointer>	TStringPointerMap;
typedef map<string, vector>		TStringVectorMap;

typedef map<Class, float>		TClassFloatMap;
typedef map<Class, int>			TClassIntMap;
typedef map<Class, string>		TClassStringMap;
typedef map<Class, Class>		TClassClassMap;
typedef map<Class, Managed>		TClassManagedMap;
typedef map<Class, ref Managed>		TClassManagedRefMap;	
typedef map<Class, pointer>	TClassPointerMap;
typedef map<Class, vector>		TClassVectorMap;

typedef map<pointer, float>		TPointerFloatMap;
typedef map<pointer, int>			TPointerIntMap;
typedef map<pointer, string>		TPointerStringMap;
typedef map<pointer, Class>		TPointerClassMap;
typedef map<pointer, Managed>		TPointerManagedMap;
typedef map<pointer, ref Managed>	TPointerManagedRefMap;
typedef map<pointer, pointer>		TPointerPointerMap;
typedef map<pointer, vector>		TPointerVectorMap;

typedef map<Managed, float>		TManagedFloatMap;
typedef map<Managed, int>		TManagedIntMap;
typedef map<Managed, string>	TManagedStringMap;
typedef map<Managed, Class>		TManagedClassMap;
typedef map<Managed, Managed>	TManagedManagedMap;
typedef map<Managed, ref Managed>	TManagedManagedRefMap;
typedef map<Managed, pointer>	TManagedPointerMap;
typedef map<Managed, vector>	TManagedVectorMap;

typedef map<ref Managed, float>	TManagedRefFloatMap;
typedef map<ref Managed, int>		TManagedRefIntMap;
typedef map<ref Managed, string>	TManagedRefStringMap;
typedef map<ref Managed, Class>		TManagedRefClassMap;
typedef map<ref Managed, Managed>	TManagedRefManagedMap;
typedef map<ref Managed, ref Managed>	TManagedRefManagedRefMap;
typedef map<ref Managed, pointer>	TManagedRefPointerMap;
typedef map<ref Managed, vector>	TManagedRefVectorMap;
//@}