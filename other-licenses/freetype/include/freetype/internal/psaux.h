/***************************************************************************/
/*                                                                         */
/*  psaux.h                                                                */
/*                                                                         */
/*    Auxiliary functions and data structures related to PostScript fonts  */
/*    (specification).                                                     */
/*                                                                         */
/*  Copyright 1996-2001, 2002 by                                           */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef __PSAUX_H__
#define __PSAUX_H__


#include <ft2build.h>
#include FT_INTERNAL_OBJECTS_H
#include FT_INTERNAL_TYPE1_TYPES_H


FT_BEGIN_HEADER


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                             T1_TABLE                          *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/


  typedef struct PS_TableRec_*              PS_Table;
  typedef const struct PS_Table_FuncsRec_*  PS_Table_Funcs;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    PS_Table_FuncsRec                                                  */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A set of function pointers to manage PS_Table objects.             */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    table_init    :: Used to initialize a table.                       */
  /*                                                                       */
  /*    table_done    :: Finalizes resp. destroy a given table.            */
  /*                                                                       */
  /*    table_add     :: Adds a new object to a table.                     */
  /*                                                                       */
  /*    table_release :: Releases table data, then finalizes it.           */
  /*                                                                       */
  typedef struct  PS_Table_FuncsRec_
  {
    FT_Error
    (*init)( PS_Table   table,
             FT_Int     count,
             FT_Memory  memory );

    void
    (*done)( PS_Table  table );

    FT_Error
    (*add)( PS_Table   table,
            FT_Int     index,
            void*      object,
            FT_Int     length );

    void
    (*release)( PS_Table  table );

  } PS_Table_FuncsRec;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    PS_TableRec                                                        */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A PS_Table is a simple object used to store an array of objects in */
  /*    a single memory block.                                             */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    block     :: The address in memory of the growheap's block.  This  */
  /*                 can change between two object adds, due to            */
  /*                 reallocation.                                         */
  /*                                                                       */
  /*    cursor    :: The current top of the grow heap within its block.    */
  /*                                                                       */
  /*    capacity  :: The current size of the heap block.  Increments by    */
  /*                 1kByte chunks.                                        */
  /*                                                                       */
  /*    max_elems :: The maximum number of elements in table.              */
  /*                                                                       */
  /*    num_elems :: The current number of elements in table.              */
  /*                                                                       */
  /*    elements  :: A table of element addresses within the block.        */
  /*                                                                       */
  /*    lengths   :: A table of element sizes within the block.            */
  /*                                                                       */
  /*    memory    :: The object used for memory operations                 */
  /*                 (alloc/realloc).                                      */
  /*                                                                       */
  /*    funcs     :: A table of method pointers for this object.           */
  /*                                                                       */
  typedef struct  PS_TableRec_
  {
    FT_Byte*           block;          /* current memory block           */
    FT_Offset          cursor;         /* current cursor in memory block */
    FT_Offset          capacity;       /* current size of memory block   */
    FT_Long            init;

    FT_Int             max_elems;
    FT_Int             num_elems;
    FT_Byte**          elements;       /* addresses of table elements */
    FT_Int*            lengths;        /* lengths of table elements   */

    FT_Memory          memory;
    PS_Table_FuncsRec  funcs;

  } PS_TableRec;


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                       T1 FIELDS & TOKENS                      *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  typedef struct PS_ParserRec_*  PS_Parser;

  typedef struct T1_TokenRec_*   T1_Token;
  
  typedef struct T1_FieldRec_*   T1_Field;

  
  /* simple enumeration type used to identify token types */
  typedef enum  T1_TokenType_
  {
    T1_TOKEN_TYPE_NONE = 0,
    T1_TOKEN_TYPE_ANY,
    T1_TOKEN_TYPE_STRING,
    T1_TOKEN_TYPE_ARRAY,

    /* do not remove */
    T1_TOKEN_TYPE_MAX

  } T1_TokenType;


  /* a simple structure used to identify tokens */
  typedef struct  T1_TokenRec_
  {
    FT_Byte*      start;   /* first character of token in input stream */
    FT_Byte*      limit;   /* first character after the token          */
    T1_TokenType  type;    /* type of token                            */

  } T1_TokenRec;


  /* enumeration type used to identify object fields */
  typedef enum  T1_FieldType_
  {
    T1_FIELD_TYPE_NONE = 0,
    T1_FIELD_TYPE_BOOL,
    T1_FIELD_TYPE_INTEGER,
    T1_FIELD_TYPE_FIXED,
    T1_FIELD_TYPE_STRING,
    T1_FIELD_TYPE_INTEGER_ARRAY,
    T1_FIELD_TYPE_FIXED_ARRAY,
    T1_FIELD_TYPE_CALLBACK,

    /* do not remove */
    T1_FIELD_TYPE_MAX

  } T1_FieldType;


  typedef enum  T1_FieldLocation_
  {
    T1_FIELD_LOCATION_CID_INFO,
    T1_FIELD_LOCATION_FONT_DICT,
    T1_FIELD_LOCATION_FONT_INFO,
    T1_FIELD_LOCATION_PRIVATE,

    /* do not remove */
    T1_FIELD_LOCATION_MAX

  } T1_FieldLocation;


  typedef void
  (*T1_Field_ParseFunc)( FT_Face     face,
                         FT_Pointer  parser );


  /* structure type used to model object fields */
  typedef struct  T1_FieldRec_
  {
    const char*         ident;        /* field identifier               */
    T1_FieldLocation    location;
    T1_FieldType        type;         /* type of field                  */
    T1_Field_ParseFunc  reader;
    FT_UInt             offset;       /* offset of field in object      */
    FT_Byte             size;         /* size of field in bytes         */
    FT_UInt             array_max;    /* maximal number of elements for */
                                      /* array                          */
    FT_UInt             count_offset; /* offset of element count for    */
                                      /* arrays                         */
  } T1_FieldRec;


#define T1_NEW_SIMPLE_FIELD( _ident, _type, _fname ) \
          {                                          \
            _ident, T1CODE, _type,                   \
            0,                                       \
            FT_FIELD_OFFSET( _fname ),               \
            FT_FIELD_SIZE( _fname ),                 \
            0, 0                                     \
          },

#define T1_NEW_CALLBACK_FIELD( _ident, _reader )    \
          {                                         \
            _ident, T1CODE, T1_FIELD_TYPE_CALLBACK, \
            (T1_Field_ParseFunc)_reader,            \
            0, 0,                                   \
            0, 0                                    \
          },

#define T1_NEW_TABLE_FIELD( _ident, _type, _fname, _max ) \
          {                                               \
            _ident, T1CODE, _type,                        \
            0,                                            \
            FT_FIELD_OFFSET( _fname ),                    \
            FT_FIELD_SIZE_DELTA( _fname ),                \
            _max,                                         \
            FT_FIELD_OFFSET( num_ ## _fname )             \
          },

#define T1_NEW_TABLE_FIELD2( _ident, _type, _fname, _max ) \
          {                                                \
            _ident, T1CODE, _type,                         \
            0,                                             \
            FT_FIELD_OFFSET( _fname ),                     \
            FT_FIELD_SIZE_DELTA( _fname ),                 \
            _max, 0                                        \
          },


#define T1_FIELD_TYPE_BOOL( _ident, _fname )                        \
          T1_NEW_SIMPLE_FIELD( _ident, T1_FIELD_TYPE_BOOL, _fname )

#define T1_FIELD_NUM( _ident, _fname )                                 \
          T1_NEW_SIMPLE_FIELD( _ident, T1_FIELD_TYPE_INTEGER, _fname )

#define T1_FIELD_FIXED( _ident, _fname )                             \
          T1_NEW_SIMPLE_FIELD( _ident, T1_FIELD_TYPE_FIXED, _fname )

#define T1_FIELD_STRING( _ident, _fname )                             \
          T1_NEW_SIMPLE_FIELD( _ident, T1_FIELD_TYPE_STRING, _fname )

#define T1_FIELD_NUM_TABLE( _ident, _fname, _fmax )                \
          T1_NEW_TABLE_FIELD( _ident, T1_FIELD_TYPE_INTEGER_ARRAY, \
                              _fname, _fmax )

#define T1_FIELD_FIXED_TABLE( _ident, _fname, _fmax )            \
          T1_NEW_TABLE_FIELD( _ident, T1_FIELD_TYPE_FIXED_ARRAY, \
                              _fname, _fmax )

#define T1_FIELD_NUM_TABLE2( _ident, _fname, _fmax )                \
          T1_NEW_TABLE_FIELD2( _ident, T1_FIELD_TYPE_INTEGER_ARRAY, \
                               _fname, _fmax )

#define T1_FIELD_FIXED_TABLE2( _ident, _fname, _fmax )            \
          T1_NEW_TABLE_FIELD2( _ident, T1_FIELD_TYPE_FIXED_ARRAY, \
                               _fname, _fmax )

#define T1_FIELD_CALLBACK( _ident, _name )       \
          T1_NEW_CALLBACK_FIELD( _ident, _name )


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                            T1 PARSER                          *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  typedef const struct PS_Parser_FuncsRec_*  PS_Parser_Funcs;

  typedef struct  PS_Parser_FuncsRec_
  {
    void
    (*init)( PS_Parser  parser,
             FT_Byte*   base,
             FT_Byte*   limit,
             FT_Memory  memory );

    void
    (*done)( PS_Parser  parser );

    void
    (*skip_spaces)( PS_Parser  parser );
    void
    (*skip_alpha)( PS_Parser  parser );

    FT_Long
    (*to_int)( PS_Parser  parser );
    FT_Fixed
    (*to_fixed)( PS_Parser  parser,
                 FT_Int     power_ten );
    FT_Int
    (*to_coord_array)( PS_Parser  parser,
                       FT_Int     max_coords,
                       FT_Short*  coords );
    FT_Int
    (*to_fixed_array)( PS_Parser  parser,
                       FT_Int     max_values,
                       FT_Fixed*  values,
                       FT_Int     power_ten );

    void
    (*to_token)( PS_Parser  parser,
                 T1_Token   token );
    void
    (*to_token_array)( PS_Parser  parser,
                       T1_Token   tokens,
                       FT_UInt    max_tokens,
                       FT_Int*    pnum_tokens );

    FT_Error
    (*load_field)( PS_Parser       parser,
                   const T1_Field  field,
                   void**          objects,
                   FT_UInt         max_objects,
                   FT_ULong*       pflags );

    FT_Error
    (*load_field_table)( PS_Parser       parser,
                         const T1_Field  field,
                         void**          objects,
                         FT_UInt         max_objects,
                         FT_ULong*       pflags );

  } PS_Parser_FuncsRec;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    PS_ParserRec                                                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A PS_Parser is an object used to parse a Type 1 font very quickly. */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    cursor :: The current position in the text.                        */
  /*                                                                       */
  /*    base   :: Start of the processed text.                             */
  /*                                                                       */
  /*    limit  :: End of the processed text.                               */
  /*                                                                       */
  /*    error  :: The last error returned.                                 */
  /*                                                                       */
  /*    memory :: The object used for memory operations (alloc/realloc).   */
  /*                                                                       */
  /*    funcs  :: A table of functions for the parser.                     */
  /*                                                                       */
  typedef struct  PS_ParserRec_
  {
    FT_Byte*   cursor;
    FT_Byte*   base;
    FT_Byte*   limit;
    FT_Error   error;
    FT_Memory  memory;

    PS_Parser_FuncsRec  funcs;
    
  } PS_ParserRec;


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                         T1 BUILDER                            *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/


  typedef struct T1_BuilderRec_*  T1_Builder;


  typedef FT_Error
  (*T1_Builder_Check_Points_Func)( T1_Builder  builder,
                                   FT_Int      count );

  typedef void
  (*T1_Builder_Add_Point_Func)( T1_Builder  builder,
                                FT_Pos      x,
                                FT_Pos      y,
                                FT_Byte     flag );

  typedef FT_Error
  (*T1_Builder_Add_Point1_Func)( T1_Builder  builder,
                                 FT_Pos      x,
                                 FT_Pos      y );

  typedef FT_Error
  (*T1_Builder_Add_Contour_Func)( T1_Builder  builder );

  typedef FT_Error
  (*T1_Builder_Start_Point_Func)( T1_Builder  builder,
                                  FT_Pos      x,
                                  FT_Pos      y );

  typedef void
  (*T1_Builder_Close_Contour_Func)( T1_Builder  builder );


  typedef const struct T1_Builder_FuncsRec_*  T1_Builder_Funcs;

  typedef struct  T1_Builder_FuncsRec_
  {
    void
    (*init)( T1_Builder    builder,
             FT_Face       face,
             FT_Size       size,
             FT_GlyphSlot  slot,
             FT_Bool       hinting );

    void
    (*done)( T1_Builder   builder );

    T1_Builder_Check_Points_Func   check_points;
    T1_Builder_Add_Point_Func      add_point;
    T1_Builder_Add_Point1_Func     add_point1;
    T1_Builder_Add_Contour_Func    add_contour;
    T1_Builder_Start_Point_Func    start_point;
    T1_Builder_Close_Contour_Func  close_contour;

  } T1_Builder_FuncsRec;


  /*************************************************************************/
  /*                                                                       */
  /* <Structure>                                                           */
  /*    T1_BuilderRec                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*     A structure used during glyph loading to store its outline.       */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    memory       :: The current memory object.                         */
  /*                                                                       */
  /*    face         :: The current face object.                           */
  /*                                                                       */
  /*    glyph        :: The current glyph slot.                            */
  /*                                                                       */
  /*    loader       :: XXX                                                */
  /*                                                                       */
  /*    base         :: The base glyph outline.                            */
  /*                                                                       */
  /*    current      :: The current glyph outline.                         */
  /*                                                                       */
  /*    max_points   :: maximum points in builder outline                  */
  /*                                                                       */
  /*    max_contours :: Maximal number of contours in builder outline.     */
  /*                                                                       */
  /*    last         :: The last point position.                           */
  /*                                                                       */
  /*    scale_x      :: The horizontal scale (FUnits to sub-pixels).       */
  /*                                                                       */
  /*    scale_y      :: The vertical scale (FUnits to sub-pixels).         */
  /*                                                                       */
  /*    pos_x        :: The horizontal translation (if composite glyph).   */
  /*                                                                       */
  /*    pos_y        :: The vertical translation (if composite glyph).     */
  /*                                                                       */
  /*    left_bearing :: The left side bearing point.                       */
  /*                                                                       */
  /*    advance      :: The horizontal advance vector.                     */
  /*                                                                       */
  /*    bbox         :: Unused.                                            */
  /*                                                                       */
  /*    path_begun   :: A flag which indicates that a new path has begun.  */
  /*                                                                       */
  /*    load_points  :: If this flag is not set, no points are loaded.     */
  /*                                                                       */
  /*    no_recurse   :: Set but not used.                                  */
  /*                                                                       */
  /*    error        :: An error code that is only used to report memory   */
  /*                    allocation problems.                               */
  /*                                                                       */
  /*    metrics_only :: A boolean indicating that we only want to compute  */
  /*                    the metrics of a given glyph, not load all of its  */
  /*                    points.                                            */
  /*                                                                       */
  /*    funcs        :: An array of function pointers for the builder.     */
  /*                                                                       */
  typedef struct  T1_BuilderRec_
  {
    FT_Memory       memory;
    FT_Face         face;
    FT_GlyphSlot    glyph;
    FT_GlyphLoader  loader;
    FT_Outline*     base;
    FT_Outline*     current;

    FT_Vector       last;

    FT_Fixed        scale_x;
    FT_Fixed        scale_y;

    FT_Pos          pos_x;
    FT_Pos          pos_y;

    FT_Vector       left_bearing;
    FT_Vector       advance;

    FT_BBox         bbox;          /* bounding box */
    FT_Bool         path_begun;
    FT_Bool         load_points;
    FT_Bool         no_recurse;
    FT_Bool         shift;

    FT_Error        error;         /* only used for memory errors */
    FT_Bool         metrics_only;

    void*           hints_funcs;    /* hinter-specific */
    void*           hints_globals;  /* hinter-specific */

    T1_Builder_FuncsRec  funcs;
    
  } T1_BuilderRec;


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                         T1 DECODER                            *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

#if 0

  /*************************************************************************/
  /*                                                                       */
  /* T1_MAX_SUBRS_CALLS details the maximum number of nested sub-routine   */
  /* calls during glyph loading.                                           */
  /*                                                                       */
#define T1_MAX_SUBRS_CALLS  8


  /*************************************************************************/
  /*                                                                       */
  /* T1_MAX_CHARSTRING_OPERANDS is the charstring stack's capacity.  A     */
  /* minimum of 16 is required.                                            */
  /*                                                                       */
#define T1_MAX_CHARSTRINGS_OPERANDS  32

#endif /* 0 */


  typedef struct  T1_Decoder_ZoneRec_
  {
    FT_Byte*  cursor;
    FT_Byte*  base;
    FT_Byte*  limit;

  } T1_Decoder_ZoneRec, *T1_Decoder_Zone;


  typedef struct T1_DecoderRec_*              T1_Decoder;
  typedef const struct T1_Decoder_FuncsRec_*  T1_Decoder_Funcs;


  typedef FT_Error
  (*T1_Decoder_Callback)( T1_Decoder  decoder,
                          FT_UInt     glyph_index );


  typedef struct  T1_Decoder_FuncsRec_
  {
    FT_Error
    (*init)( T1_Decoder           decoder,
             FT_Face              face,
             FT_Size              size,
             FT_GlyphSlot         slot,
             FT_Byte**            glyph_names,
             PS_Blend             blend,
             FT_Bool              hinting,
             T1_Decoder_Callback  callback );

    void
    (*done)( T1_Decoder  decoder );

    FT_Error
    (*parse_charstrings)( T1_Decoder  decoder,
                          FT_Byte*    base,
                          FT_UInt     len );
                          
  } T1_Decoder_FuncsRec;


  typedef struct  T1_DecoderRec_
  {
    T1_BuilderRec        builder;

    FT_Long              stack[T1_MAX_CHARSTRINGS_OPERANDS];
    FT_Long*             top;

    T1_Decoder_ZoneRec   zones[T1_MAX_SUBRS_CALLS + 1];
    T1_Decoder_Zone      zone;

    PSNames_Service      psnames;      /* for seac */
    FT_UInt              num_glyphs;
    FT_Byte**            glyph_names;

    FT_Int               lenIV;        /* internal for sub routine calls */
    FT_UInt              num_subrs;
    FT_Byte**            subrs;
    FT_Int*              subrs_len;    /* array of subrs length (optional) */

    FT_Matrix            font_matrix;
    FT_Vector            font_offset;

    FT_Int               flex_state;
    FT_Int               num_flex_vectors;
    FT_Vector            flex_vectors[7];

    PS_Blend             blend;       /* for multiple master support */

    T1_Decoder_Callback  parse_callback;
    T1_Decoder_FuncsRec  funcs;
    
  } T1_DecoderRec;


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                     TYPE1 CHARMAPS                            *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  typedef const struct T1_CMap_ClassesRec_*  T1_CMap_Classes;

  typedef struct T1_CMap_ClassesRec_
  {
    FT_CMap_Class  standard;
    FT_CMap_Class  expert;
    FT_CMap_Class  custom;
    FT_CMap_Class  unicode;
  
  } T1_CMap_ClassesRec;
  

  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                        PSAux Module Interface                 *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  typedef struct  PSAux_ServiceRec_
  {
    /* don't use `PS_Table_Funcs' and friends to avoid compiler warnings */
    const PS_Table_FuncsRec*    ps_table_funcs;
    const PS_Parser_FuncsRec*   ps_parser_funcs;
    const T1_Builder_FuncsRec*  t1_builder_funcs;
    const T1_Decoder_FuncsRec*  t1_decoder_funcs;

    void
    (*t1_decrypt)( FT_Byte*   buffer,
                   FT_Offset  length,
                   FT_UShort  seed );

    T1_CMap_Classes  t1_cmap_classes;

  } PSAux_ServiceRec, *PSAux_Service;

  /* backwards-compatible type definition */
  typedef PSAux_ServiceRec   PSAux_Interface;

FT_END_HEADER

#endif /* __PSAUX_H__ */


/* END */
