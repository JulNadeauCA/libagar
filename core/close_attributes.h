/*	Public domain	*/

/*
 * Undo compiler-specific attributes and annotations
 * (included by <core/close.h>, <gui/close.h>, etc.)
 */

#undef HAVE_ALIGNED_ATTRIBUTE
#undef HAVE_CONST_ATTRIBUTE
#undef HAVE_DEPRECATED_ATTRIBUTE
#undef HAVE_FORMAT_ATTRIBUTE
#undef HAVE_MALLOC_ATTRIBUTE
#undef HAVE_NORETURN_ATTRIBUTE
#undef HAVE_PACKED_ATTRIBUTE
#undef HAVE_PURE_ATTRIBUTE
#undef HAVE_WARN_UNUSED_RESULT_ATTRIBUTE
#undef HAVE_UNUSED_VARIABLE_ATTRIBUTE

#if !defined(_USE_AGAR_ATTRIBUTES) && !defined(_AGAR_INTERNAL)

# undef _Aligned_Attribute
# undef _Alloc_Align_Attribute
# undef _Alloc_Size_Attribute
# undef _Alloc_Size2_Attribute
# undef _Const_Attribute
# undef DEPRECATED_ATTRIBUTE
# undef FORMAT_ATTRIBUTE
# undef _Malloc_Like_Attribute
# undef _Noreturn_Attribute
# undef _Packed_Attribute
# undef _Pure_Attribute
# undef _Pure_Attribute_If_Unthreaded
# undef _Section_Attribute
# undef _Unused_Variable_Attribute
# undef _Used_Variable_Attribute
# undef _Warn_Unused_Result
# undef _Weak_Attribute

#endif /* !_USE_AGAR_ATTRIBUTES and !_AGAR_INTERNAL */
