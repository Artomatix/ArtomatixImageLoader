SET(LIBRARY_PATHS
    HINTS
    ${CMAKE_INSTALL_PREFIX}
    PATH_SUFFIXES lib64 lib
    PATHS
    ${OPENEXR_ROOT}
    /usr/
    /usr/local/
    /opt/local/
)

FIND_PATH(OPENEXR_INCLUDE_DIR half.h
    HINTS
    ${CMAKE_INSTALL_PREFIX}
    PATH_SUFFIXES /OpenEXR
    PATHS
	${OPENEXR_ROOT}/include
    /usr/include       
    /usr/local/include  
    /opt/local/include
)

FIND_LIBRARY(OPENEXR_HALF_LIBRARY       NAMES Half      PATHS ${LIBRARY_PATHS})
FIND_LIBRARY(OPENEXR_IEX_LIBRARY        NAMES Iex       PATHS ${LIBRARY_PATHS})
FIND_LIBRARY(OPENEXR_IMATH_LIBRARY      NAMES Imath     PATHS ${LIBRARY_PATHS})
FIND_LIBRARY(OPENEXR_ILMIMF_LIBRARY      NAMES IlmImf     PATHS ${LIBRARY_PATHS})
FIND_LIBRARY(OPENEXR_ILMTHREAD_LIBRARY  NAMES IlmThread PATHS ${LIBRARY_PATHS})

FIND_LIBRARY(OPENEXR_HALF_LIBRARYD       NAMES Halfd      PATHS ${LIBRARY_PATHS})
FIND_LIBRARY(OPENEXR_IEX_LIBRARYD        NAMES Iexd       PATHS ${LIBRARY_PATHS})
FIND_LIBRARY(OPENEXR_IMATH_LIBRARYD      NAMES Imathd     PATHS ${LIBRARY_PATHS})
FIND_LIBRARY(OPENEXR_ILMIMF_LIBRARYD      NAMES IlmImfd     PATHS ${LIBRARY_PATHS})
FIND_LIBRARY(OPENEXR_ILMTHREAD_LIBRARYD  NAMES IlmThreadd PATHS ${LIBRARY_PATHS})

SET(OPENEXR_INCLUDE_DIRS ${OPENEXR_INCLUDE_DIR})
SET(OPENEXR_LIBRARIESO optimized ${OPENEXR_IMATH_LIBRARY} optimized ${OPENEXR_ILMIMF_LIBRARY} optimized ${OPENEXR_ILMTHREAD_LIBRARY} optimized ${OPENEXR_HALF_LIBRARY} optimized ${OPENEXR_IEX_LIBRARY})
SET(OPENEXR_LIBRARIESD debug ${OPENEXR_IMATH_LIBRARYD} debug ${OPENEXR_ILMIMF_LIBRARYD} debug ${OPENEXR_ILMTHREAD_LIBRARYD} debug ${OPENEXR_HALF_LIBRARYD} debug ${OPENEXR_IEX_LIBRARYD})

hunter_add_package(ZLIB)
find_package(ZLIB)
SET(OPENEXR_LIBRARIES ${OPENEXR_LIBRARIESO} ${OPENEXR_LIBRARIESD} ${ZLIB_LIBRARIES})

if(NOT WIN32)
    set(OPENEXR_LIBRARIES ${OPENEXR_LIBRARIES} pthread)
endif()

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(OPENEXR DEFAULT_MSG OPENEXR_INCLUDE_DIR OPENEXR_HALF_LIBRARY OPENEXR_IEX_LIBRARY OPENEXR_IMATH_LIBRARY OPENEXR_ILMTHREAD_LIBRARY)
