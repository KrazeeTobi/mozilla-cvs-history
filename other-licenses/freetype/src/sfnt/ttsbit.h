/***************************************************************************/
/*                                                                         */
/*  ttsbit.h                                                               */
/*                                                                         */
/*    TrueType and OpenType embedded bitmap support (specification).       */
/*                                                                         */
/*  Copyright 1996-2001 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef __TTSBIT_H__
#define __TTSBIT_H__


#include <ft2build.h>
#include "ttload.h"


FT_BEGIN_HEADER


  FT_LOCAL FT_Error
  TT_Load_SBit_Strikes( TT_Face    face,
                        FT_Stream  stream );

  FT_LOCAL void
  TT_Free_SBit_Strikes( TT_Face  face );


  FT_LOCAL FT_Error
  TT_Set_SBit_Strike( TT_Face    face,
                      FT_Int     x_ppem,
                      FT_Int     y_ppem,
                      FT_ULong  *astrike_index );

  FT_LOCAL FT_Error
  TT_Load_SBit_Image( TT_Face           face,
                      FT_ULong          strike_index,
                      FT_UInt           glyph_index,
                      FT_UInt           load_flags,
                      FT_Stream         stream,
                      FT_Bitmap        *map,
                      TT_SBit_Metrics  *metrics );


FT_END_HEADER

#endif /* __TTSBIT_H__ */


/* END */
