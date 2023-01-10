" Vim syntax file
" Language:     LibAgar - Agar-AU C API
" URL:
" https://github.com/JulNadeauCA/libagar/blob/master/syntax/agarau.vim
" Maintainer:   Julien Nadeau Carriere <vedge@csoft.net>
" Last Change:  2023 January 09

if !exists("c_no_agar_au") || exists("c_agar_au_typedefs")
  syn keyword cType AU_Module AU_ModuleClass AU_Port AU_Source
  syn keyword cType AU_Format AU_Player AU_Buffer AU_Channel
  syn keyword cType AU_DevOut AU_DevOutClass
  syn keyword cType AU_DevOutFile AU_DevOutPA
  " au/au_dev_out.h
  syn keyword cConstant AU_MINBUFSIZ AU_DEV_OUT_THREADED AU_DEV_OUT_CLOSING
  syn keyword cConstant AU_DEV_OUT_ERROR
  " au/au_math.h
  syn keyword cConstant AU_PI
  " au/au_wave.h
  syn keyword cType AU_Wave
endif
