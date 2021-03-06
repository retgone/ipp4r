#ifndef __IPP4R_METATYPE_H__
#define __IPP4R_METATYPE_H__

#include "arx/Preprocessor.h"
#include "ipp4r_macro.h"

/**
 * @file
 * 
 * This header contains support macros and functions for metatype infrastructure.
 * The name "meta" has no special meaning and was chosen arbitrarily, without any prior analysis. <p>
 *
 * The metatype infrastructure works on two different levels:
 * <ul>
 * <li> Preprocessor level (pp-time)
 * <li> Compile-time level
 * <li> Run-time level
 * </ul>
 * <p>
 *
 * At pp-time we define a special preprocessor type METATYPE, which is composed of DATATYPE and CHANNELS.
 * DATATYPE and CHANNELS parts of METATYPE can be acessed using acessor macros at pp-time. <p>
 *
 * At compile-time we have: 
 * <ul>
 * <li> an enum for each METATYPE 
 * <li> c-type and an enum for each DATATYPE
 * <li> an enum and channel count value for each CHANNELS
 * </ul>
 * pp-time abstractions can be converted to their compile-time equivalents using conversion macros. 
 * Also at compile-time we have a special number type - IppMetaNumber, which is a supertype for all data types. 
 * The corresponding pp-time DATATYPE for IppMetaNumber is D_METANUM. <p>
 *
 * At run-time we have METACALL macros which perform a call basing on the value of METATYPE enum, and converters to and from IppMetaNumber type. <p>
 */

#ifdef __cplusplus
extern "C" {
#endif

// -------------------------------------------------------------------------- //
// pp-time definitions
// -------------------------------------------------------------------------- //
#define D_METANUM 32f

#define D_SUPPORTED (3, (8u,     16u,      32f))
#define D_ALL       (4, (8u,     16u,      32f,    32s))
#define D_SCALE_MAP (4, (255.0f, 65535.0f, 1.0f,   2147483647.0f))
#define D_MAX_MAP   (4, (255,    65535,    1.0f,   2147483647))

#define C_SUPPORTED (3, (C1, C3, AC4))
#define C_ALL       (4, (C1, C3, AC4, C4))
#define C_COUNT_MAP (4, (1,  3,  4,   4))
#define C_MAX_COUNT 4

#define M_SUPPORTED    (9,  (8u_C1, 8u_C3, 8u_AC4, 16u_C1, 16u_C3, 16u_AC4, 32f_C1, 32f_C3, 32f_AC4))
#define M_ALL          (16, (8u_C1, 8u_C3, 8u_AC4, 16u_C1, 16u_C3, 16u_AC4, 32f_C1, 32f_C3, 32f_AC4, 8u_C4, 16u_C4, 32f_C4, 32s_C1, 32s_C3, 32s_AC4, 32s_C4)) 
#define M_DATATYPE_MAP (16, (8u,    8u,    8u,     16u,    16u,    16u,     32f,    32f,    32f,     8u,    16u,    32f,    32s,    32s,    32s,     32s   ))
#define M_CHANNELS_MAP (16, (   C1,    C3,    AC4,     C1,     C3,     AC4,     C1,     C3,     AC4,    C4,     C4,     C4,     C1,     C3,     AC4,     C4))


// -------------------------------------------------------------------------- //
// pp-time maps
// -------------------------------------------------------------------------- //
#define M_INDEX(METATYPE) M_INDEX_I(METATYPE)
#define M_INDEX_I(METATYPE) M_INDEX_II(M_INDEX_ ## METATYPE)
#define M_INDEX_II(res) res
#define M_INDEX_8u_C1   0
#define M_INDEX_8u_C3   1
#define M_INDEX_8u_AC4  2
#define M_INDEX_16u_C1  3
#define M_INDEX_16u_C3  4
#define M_INDEX_16u_AC4 5
#define M_INDEX_32f_C1  6
#define M_INDEX_32f_C3  7
#define M_INDEX_32f_AC4 8
#define M_INDEX_8u_C4   9
#define M_INDEX_16u_C4  10
#define M_INDEX_32f_C4  11
#define M_INDEX_32s_C1  12
#define M_INDEX_32s_C3  13
#define M_INDEX_32s_AC4 14
#define M_INDEX_32s_C4  15

#define D_INDEX(DATATYPE) D_INDEX_I(DATATYPE)
#define D_INDEX_I(DATATYPE) D_INDEX_II(D_INDEX_ ## DATATYPE)
#define D_INDEX_II(res) res
#define D_INDEX_8u   0
#define D_INDEX_16u  1
#define D_INDEX_32f  2
#define D_INDEX_32s  3

#define C_INDEX(CHANNELS) C_INDEX_I(CHANNELS)
#define C_INDEX_I(CHANNELS) C_INDEX_II(C_INDEX_ ## CHANNELS)
#define C_INDEX_II(res) res
#define C_INDEX_C1   0
#define C_INDEX_C3   1
#define C_INDEX_AC4  2
#define C_INDEX_C4   3


// -------------------------------------------------------------------------- //
// pp-time functions
// -------------------------------------------------------------------------- //
#define M_DATATYPE(METATYPE) ARX_ARRAY_ELEM(M_INDEX(METATYPE), M_DATATYPE_MAP)
#define M_CHANNELS(METATYPE) ARX_ARRAY_ELEM(M_INDEX(METATYPE), M_CHANNELS_MAP)
#define M_CREATE(DATATYPE, CHANNELS) ARX_JOIN_3(DATATYPE, _, CHANNELS)

#define M_REPLACE_D(METATYPE, DATATYPE) M_CREATE(DATATYPE, M_CHANNELS(METATYPE))
#define M_REPLACE_C(METATYPE, CHANNELS) M_CREATE(M_DATATYPE(METATYPE), CHANNELS)

#define M_REPLACE_D_IF_D(METATYPE, OLD_DATATYPE, NEW_DATATYPE) IF_M_IS_D(METATYPE, OLD_DATATYPE, M_REPLACE_D(METATYPE, NEW_DATATYPE), METATYPE)
#define M_REPLACE_C_IF_C(METATYPE, OLD_CHANNELS, NEW_CHANNELS) IF_M_IS_C(METATYPE, OLD_CHANNELS, M_REPLACE_C(METATYPE, NEW_CHANNELS), METATYPE)


// -------------------------------------------------------------------------- //
// pp-time to compile-time converters
// -------------------------------------------------------------------------- //
#define D_CTYPE(DATATYPE) ARX_JOIN(Ipp, DATATYPE)
#define D_CENUM(DATATYPE) ARX_JOIN(ipp, DATATYPE)
#define D_SCALE(DATATYPE) ARX_ARRAY_ELEM(D_INDEX(DATATYPE), D_SCALE_MAP)
#define D_MAX(DATATYPE) ARX_ARRAY_ELEM(D_INDEX(DATATYPE), D_MAX_MAP)

#define C_CENUM(CHANNELS) ARX_JOIN(ipp, CHANNELS)
#define C_CNUMB(CHANNELS) ARX_ARRAY_ELEM(C_INDEX(CHANNELS), C_COUNT_MAP)

#define M_CENUM(METATYPE) ARX_JOIN(ipp, METATYPE)


// -------------------------------------------------------------------------- //
// METATYPE support macros
// -------------------------------------------------------------------------- //
#define M_EQ_M(METATYPE_0, METATYPE_1) ARX_EQUAL(M_INDEX(METATYPE_0), M_INDEX(METATYPE_1))
#define M_IS_C(METATYPE, CHANNELS) ARX_EQUAL(C_INDEX(M_CHANNELS(METATYPE)), C_INDEX(CHANNELS))
#define M_IS_D(METATYPE, DATATYPE) ARX_EQUAL(D_INDEX(M_DATATYPE(METATYPE)), D_INDEX(DATATYPE))

#define D_EQ_D(DATATYPE_0, DATATYPE_1) ARX_EQUAL(D_INDEX(DATATYPE_0), D_INDEX(DATATYPE_1))
#define C_EQ_C(CHANNELS_0, CHANNELS_1) ARX_EQUAL(C_INDEX(CHANNELS_0), C_INDEX(CHANNELS_1))

#define IF_M_EQ_M(METATYPE_0, METATYPE_1, T, F) ARX_IF(M_EQ_M(METATYPE_0, METATYPE_1), T, F)
#define IF_M_IS_C(METATYPE, DATATYPE, T, F) ARX_IF(M_IS_C(METATYPE, DATATYPE), T, F)
#define IF_M_IS_D(METATYPE, DATATYPE, T, F) ARX_IF(M_IS_D(METATYPE, DATATYPE), T, F)

#define IF_D_EQ_D(DATATYPE_0, DATATYPE_1, T, F) ARX_IF(D_EQ_D(DATATYPE_0, DATATYPE_1), T, F)
#define IF_C_EQ_C(CHANNELS_0, CHANNELS_1, T, F) ARX_IF(C_EQ_C(CHANNELS_0, CHANNELS_1), T, F)

#define D_IN_D_2(DATATYPE, DATATYPE_0, DATATYPE_1) ARX_OR(D_EQ_D(DATATYPE, DATATYPE_0), D_EQ_D(DATATYPE, DATATYPE_1))
#define C_IN_C_2(CHANNELS, CHANNELS_0, CHANNELS_1) ARX_OR(C_EQ_C(CHANNELS, CHANNELS_0), C_EQ_C(CHANNELS, CHANNELS_1))

#define DD_EQ_DD(DATATYPE_PAIR_0, DATATYPE_PAIR_1)                              \
  ARX_AND(D_EQ_D(ARX_TUPLE_ELEM(2, 0, DATATYPE_PAIR_0), ARX_TUPLE_ELEM(2, 0, DATATYPE_PAIR_1)), D_EQ_D(ARX_TUPLE_ELEM(2, 1, DATATYPE_PAIR_0), ARX_TUPLE_ELEM(2, 1, DATATYPE_PAIR_1)))
#define CC_EQ_CC(CHANNELS_PAIR_0, CHANNELS_PAIR_1)                              \
  ARX_AND(C_EQ_C(ARX_TUPLE_ELEM(2, 0, CHANNELS_PAIR_0), ARX_TUPLE_ELEM(2, 0, CHANNELS_PAIR_1)), C_EQ_C(ARX_TUPLE_ELEM(2, 1, CHANNELS_PAIR_0), ARX_TUPLE_ELEM(2, 1, CHANNELS_PAIR_1)))

#define IF_DD_EQ_DD(DATATYPE_PAIR_0, DATATYPE_PAIR_1, T, F)                     \
  ARX_IF(DD_EQ_DD(DATATYPE_PAIR_0, DATATYPE_PAIR_1), T, F)
#define IF_CC_EQ_CC(CHANNELS_PAIR_0, CHANNELS_PAIR_1, T, F)                     \
  ARX_IF(CC_EQ_CC(CHANNELS_PAIR_0, CHANNELS_PAIR_1), T, F)

#define DD_IN_DDA(DATATYPE_PAIR, DATATYPE_PAIR_ARRAY) ARX_ARRAY_EXISTS(DATATYPE_PAIR_ARRAY, DD_EQ_DD, DATATYPE_PAIR)
#define CC_IN_CCA(CHANNELS_PAIR, CHANNELS_PAIR_ARRAY) ARX_ARRAY_EXISTS(CHANNELS_PAIR_ARRAY, CC_EQ_CC, CHANNELS_PAIR)


// -------------------------------------------------------------------------- //
// run-time converters
// -------------------------------------------------------------------------- //
typedef D_CTYPE(D_METANUM) IppMetaNumber; /**< Supertype for all data types */

//#define R2C_METANUM(V) ((IppMetaType) R2C_DBL(V))
//#define C2R_METANUM(V) C2R_DBL(V)

#define M2C_NUMBER(METATYPE, METANUM)                                           \
  M2C_NUMBER_D(M_DATATYPE(METATYPE), (METANUM))
#define M2C_NUMBER_D(DATATYPE, METANUM)                                         \
  IF_D_EQ_D(DATATYPE, D_METANUM, (METANUM), (D_CTYPE(DATATYPE)) ((METANUM) * D_SCALE(DATATYPE)))

#define C2M_NUMBER(METATYPE, C_NUMBER)                                          \
  C2M_NUMBER_D(M_DATATYPE(METATYPE), (C_NUMBER))
#define C2M_NUMBER_D(DATATYPE, C_NUMBER)                                        \
  IF_D_EQ_D(DATATYPE, D_METANUM, (C_NUMBER), (IppMetaNumber) ((C_NUMBER) / D_SCALE(DATATYPE)))


// -------------------------------------------------------------------------- //
// IppMetaType enum
// -------------------------------------------------------------------------- //
typedef enum {
#define ENUM_METATYPE_I(METATYPE, ARG) M_CENUM(METATYPE),
  ARX_ARRAY_FOREACH(M_SUPPORTED, ENUM_METATYPE_I, ARX_EMPTY)
#undef ENUM_METATYPE_I
} IppMetaType;


// -------------------------------------------------------------------------- //
// METACALL
// -------------------------------------------------------------------------- //
#if 0
#  define IPPMETACALL(CRITERIA, LEFT_PART, CRITERIA_ARRAY, FUNCTION, ARGS, DEFAULT_ACTION, DEFAULT_VALUE)
#endif

#define IPPMETACALL ARX_JOIN(IPPMETACALL_, ARX_AUTO_REC(IPPMETACALL_P, 4))

#define IPPMETACALL_P(n) ARX_JOIN(IPPMETACALL_CHECK_, IPPMETACALL_ ## n(~, ~, (1, (IPPMETACALL_EMPTY)), ~, ~, ~, ~))

#define IPPMETACALL_CHECK_ARX_NIL 1
#define IPPMETACALL_CHECK_IPPMETACALL_1(A, B, C, D, E, F, G) 0
#define IPPMETACALL_CHECK_IPPMETACALL_2(A, B, C, D, E, F, G) 0
#define IPPMETACALL_CHECK_IPPMETACALL_3(A, B, C, D, E, F, G) 0

#define IPPMETACALL_EMPTY_HELPER_IPPMETACALL_EMPTY

#define IPPMETACALL_1(CRITERIA, LEFT_PART, CRITERIA_ARRAY, FUNCTION, ARGS, DEFAULT_ACTION, DEFAULT_VALUE) \
  ARX_IF(ARX_EQUAL(ARX_ARRAY_SIZE(CRITERIA_ARRAY), 1),                          \
    ARX_IF(ARX_IS_EMPTY(ARX_JOIN(IPPMETACALL_EMPTY_HELPER_, ARX_ARRAY_ELEM(0, CRITERIA_ARRAY))), \
      ARX_NIL ARX_TUPLE_EAT_7,                                                  \
      IPPMETACALL_1_I                                                           \
    ),                                                                          \
    IPPMETACALL_1_I                                                             \
  )(CRITERIA, LEFT_PART, CRITERIA_ARRAY, FUNCTION, ARGS, DEFAULT_ACTION, DEFAULT_VALUE)
#define IPPMETACALL_1_I(CRITERIA, LEFT_PART, CRITERIA_ARRAY, FUNCTION, ARGS, DEFAULT_ACTION, DEFAULT_VALUE) \
  switch(CRITERIA) {                                                            \
  ARX_ARRAY_FOREACH(CRITERIA_ARRAY, IPPMETACALL_1_II, (3, (LEFT_PART, FUNCTION, ARGS))) \
  default:                                                                      \
    DEFAULT_ACTION;                                                             \
    LEFT_PART DEFAULT_VALUE;                                                    \
  }
#define IPPMETACALL_1_II(METATYPE, ARGS)                                        \
  case M_CENUM(METATYPE):                                                       \
    ARX_ARRAY_ELEM(0, ARGS) ARX_ARRAY_ELEM(1, ARGS)(METATYPE, ARX_ARRAY_ELEM(2, ARGS)); \
    break;

#define IPPMETACALL_2(CRITERIA, LEFT_PART, CRITERIA_ARRAY, FUNCTION, ARGS, DEFAULT_ACTION, DEFAULT_VALUE) \
  ARX_IF(ARX_EQUAL(ARX_ARRAY_SIZE(CRITERIA_ARRAY), 1),                          \
    ARX_IF(ARX_IS_EMPTY(ARX_JOIN(IPPMETACALL_EMPTY_HELPER_, ARX_ARRAY_ELEM(0, CRITERIA_ARRAY))), \
      ARX_NIL ARX_TUPLE_EAT_7,                                                  \
      IPPMETACALL_2_I                                                           \
    ),                                                                          \
    IPPMETACALL_2_I                                                             \
  )(CRITERIA, LEFT_PART, CRITERIA_ARRAY, FUNCTION, ARGS, DEFAULT_ACTION, DEFAULT_VALUE)
#define IPPMETACALL_2_I(CRITERIA, LEFT_PART, CRITERIA_ARRAY, FUNCTION, ARGS, DEFAULT_ACTION, DEFAULT_VALUE) \
  switch(CRITERIA) {                                                            \
  ARX_ARRAY_FOREACH(CRITERIA_ARRAY, IPPMETACALL_2_II, (3, (LEFT_PART, FUNCTION, ARGS))) \
  default:                                                                      \
    DEFAULT_ACTION;                                                             \
    LEFT_PART DEFAULT_VALUE;                                                    \
  }
#define IPPMETACALL_2_II(METATYPE, ARGS)                                        \
  case M_CENUM(METATYPE):                                                       \
    ARX_ARRAY_ELEM(0, ARGS) ARX_ARRAY_ELEM(1, ARGS)(METATYPE, ARX_ARRAY_ELEM(2, ARGS)); \
    break;

#define IPPMETACALL_3(CRITERIA, LEFT_PART, CRITERIA_ARRAY, FUNCTION, ARGS, DEFAULT_ACTION, DEFAULT_VALUE) \
  ARX_IF(ARX_EQUAL(ARX_ARRAY_SIZE(CRITERIA_ARRAY), 1),                          \
    ARX_IF(ARX_IS_EMPTY(ARX_JOIN(IPPMETACALL_EMPTY_HELPER_, ARX_ARRAY_ELEM(0, CRITERIA_ARRAY))), \
      ARX_NIL ARX_TUPLE_EAT_7,                                                  \
      IPPMETACALL_3_I                                                           \
    ),                                                                          \
    IPPMETACALL_3_I                                                             \
  )(CRITERIA, LEFT_PART, CRITERIA_ARRAY, FUNCTION, ARGS, DEFAULT_ACTION, DEFAULT_VALUE)
#define IPPMETACALL_3_I(CRITERIA, LEFT_PART, CRITERIA_ARRAY, FUNCTION, ARGS, DEFAULT_ACTION, DEFAULT_VALUE) \
  switch(CRITERIA) {                                                            \
  ARX_ARRAY_FOREACH(CRITERIA_ARRAY, IPPMETACALL_3_II, (3, (LEFT_PART, FUNCTION, ARGS))) \
  default:                                                                      \
    DEFAULT_ACTION;                                                             \
    LEFT_PART DEFAULT_VALUE;                                                    \
  }
#define IPPMETACALL_3_II(METATYPE, ARGS)                                        \
  case M_CENUM(METATYPE):                                                       \
    ARX_ARRAY_ELEM(0, ARGS) ARX_ARRAY_ELEM(1, ARGS)(METATYPE, ARX_ARRAY_ELEM(2, ARGS)); \
    break;

#define IPPMETACALL_4(CRITERIA, LEFT_PART, CRITERIA_ARRAY, FUNCTION, ARGS, DEFAULT_ACTION, DEFAULT_VALUE) OMG_TEH_DRAMA


// -------------------------------------------------------------------------- //
// Standard metafunction for METACALL
// -------------------------------------------------------------------------- //
/**
 * @param ARGS (prefix, suffix, args)
 */
#define IPPMETAFUNC(METATYPE, ARGS)                                                \
  ARX_JOIN_3(ARX_TUPLE_ELEM(3, 0, ARGS), METATYPE, ARX_TUPLE_ELEM(3, 1, ARGS))ARX_TUPLE_ELEM(3, 2, ARGS)


/**
 * @param ARG value
 */
#define IPPMETAVAL(METATUPE, ARG) ARG


// -------------------------------------------------------------------------- //
// IppMetaType run-time accessors
// -------------------------------------------------------------------------- //
/**
 * @returns IppChannels encoded in the given IppMetaType
 */
IppChannels metatype_channels(IppMetaType metaType);


/**
 * @returns IppDataType encoded in the given IppMetaType
 */
IppDataType metatype_datatype(IppMetaType metaType);


/**
 * @returns IppMetaType composed of given IppDataType and IppChannels
 */
IppMetaType metatype_compose(IppDataType dataType, IppChannels channels);


/**
 * @return size in bytes of one pixel for given metatype
 */
int metatype_pixel_size(IppMetaType metaType);


// -------------------------------------------------------------------------- //
// is_supported functions
// -------------------------------------------------------------------------- //
/**
 * @returns TRUE if the given IppChannels enum is supported, FALSE otherwise
 */
int is_channels_supported(IppChannels channels);


/**
 * @returns TRUE if the given IppDataType is supported, FALSE otherwise
 */
int is_datatype_supported(IppDataType dataType);


/**
 * @returns TRUE if the given IppMetaType is supported, FALSE otherwise
 */
int is_metatype_supported(IppMetaType metaType);



#ifdef __cplusplus
}
#endif

#endif

