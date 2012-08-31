#ifndef CPPTL_FORWARDS_H_INCLUDED
# define CPPTL_FORWARDS_H_INCLUDED

# include <cpptl/config.h>

namespace CppTL {

// any.h
class Any;
class AnyBadCast;

// atomiccounter.h
class AtomicCounter;

// conststring.h
class ConstCharView;
class ConstString;
class StringBuffer;
class StringConcatenator;

// enumerator.h
template<class ValueType>
class AnyEnumerator;

// functor.h
class Functor0;
template<class ReturnType> 
class Functor0R;

// intrusiveptr.h
class IntrusiveCount;
template<class PointeeType>
class IntrusivePtr;

// reflection.h
class Attribut;
class Class;
class Method;

// scopedptr.h
template<class PointeeType>
class ScopedPtr;
template<class PointeeType>
class ScopedArray;

// sharedptr.h
template<class PointeeType>
class SharedPtr;

// _stlimpl.h
template<class T>
class MallocAllocator;

template<class T>
struct LessPred;

} // namespace CppTL



#endif // CPPTL_FORWARDS_H_INCLUDED
