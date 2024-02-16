#
# Cmake package for including the Agar-Core library in a cmake project.
#
if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/ag_coreTargets.cmake")
  include("${CMAKE_CURRENT_LIST_DIR}/ag_coreTargets.cmake")
endif()
if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/ag_coreStaticTargets.cmake")
  include("${CMAKE_CURRENT_LIST_DIR}/ag_coreStaticTargets.cmake")
endif()

if(NOT TARGET ag_core::ag_core AND TARGET ag_core::ag_core-static)
  if(CMAKE_VERSION VERSION_LESS "3.18")
      set_target_properties(ag_core::ag_core-static PROPERTIES IMPORTED_GLOBAL TRUE)
  endif()
  add_library(ag_core::ag_core ALIAS ag_core::ag_core-static)
endif()

get_target_property(AGAR_CORE_INCLUDE_DIRS ag_core::ag_core INTERFACE_INCLUDE_DIRECTORIES)

set(relprops
    IMPORTED_IMPLIB_RELEASE IMPORTED_IMPLIB_NOCONFIG IMPORTED_IMPLIB IMPORTED_IMPLIB_MINSIZEREL
    IMPORTED_IMPLIB_RELWITHDEBINFO IMPORTED_LOCATION_RELEASE IMPORTED_LOCATION_NOCONFIG
    IMPORTED_LOCATION IMPORTED_LOCATION_MINSIZEREL IMPORTED_LOCATION_RELWITHDEBINFO)

set(dbgprops IMPORTED_IMPLIB_DEBUG IMPORTED_LOCATION_DEBUG)

foreach(prop ${relprops})
	get_target_property(ag_coreimplib ag_core::ag_core ${prop})
	if(ag_coreimplib)
		message("set ag_coreimplib from ${prop}")
		break()
	endif()
endforeach()

foreach(prop ${dbgprops})
	get_target_property(ag_coreimplibdbg ag_core::ag_core ${prop})
	if(ag_coreimplibdbg)
		message("set ag_coreimplibdbg from ${prop}")
		break()
	endif()
endforeach()

if(ag_coreimplib AND ag_coreimplibdbg)
	set(AGAR_CORE_LIBRARIES $<IF:$<CONFIG:Debug>,${ag_coreimplibdbg},${ag_coreimplib}>)
else()
	if((NOT ag_coreimplib) AND ag_coreimplibdbg)
		set(ag_coreimplib ${ag_coreimplibdbg})
	endif()
	if(ag_coreimplib)
		set(AGAR_CORE_LIBRARIES ${ag_coreimplib})
	else()
		message(FATAL_ERROR, "ag_core::ag_core has no lib to link against in IMPORTED_IMPLIB* or IMPORTED_LOCATION*")
	endif()
endif()

get_target_property(ag_deplib ag_core::ag_core INTERFACE_LINK_LIBRARIES)
if(ag_deplib)
	set(AGAR_CORE_LIBRARIES ${AGAR_CORE_LIBRARIES} ${ag_deplib})
endif()

get_filename_component(AGAR_CORE_LIBDIR ${ag_coreimplib} PATH)

# XXX
get_filename_component(AGAR_CORE_EXEC_PREFIX ${AGAR_CORE_LIBDIR} PATH)
set(AGAR_CORE_PREFIX ${AGAR_CORE_EXEC_PREFIX})

unset(ag_coreimplib)
unset(ag_coreimplibdbg)
unset(ag_deplib)
unset(relprops)
unset(dbgprops)
