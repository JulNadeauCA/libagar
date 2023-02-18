" Vim syntax file
" Language:     Agar GUI (LibAgar) Bitmap Font File.
" URL:
" https://github.com/JulNadeauCA/libagar/blob/master/syntax/agarbf.vim
" Maintainer:   Julien Nadeau Carriere <vedge@csoft.net>
" Last Change:  2023 February 17

" quit when a syntax file was already loaded
if !exists("main_syntax")
  if exists("b:current_syntax")
    finish
  endif
  let main_syntax = 'agarbf'
elseif exists("b:current_syntax") && b:current_syntax == "agarbf"
  finish
endif

let s:cpo_save = &cpo
set cpo&vim

" Add dash to allowed keyword characters.
syn iskeyword @,48-57,_,192-255,-
syn match agbfUnitDecorators /\(#\|-\|+\|%\|px\)/ contained
syn match agbfColor contained "#\x\{3,4\}\>" contains=agbfUnitDecorators
syn match agbfColor contained "#\x\{6\}\>" contains=agbfUnitDecorators
syn match agbfColor contained "#\x\{16\}\>" contains=agbfUnitDecorators

syn match agbfValueInteger contained "[-+]\=\d\+" contains=agbfUnitDecorators
syn match agbfValueNumber contained "[-+]\=\d\+\(\.\d*\)\=" contains=agbfUnitDecorators

syn match agbfDirective "^unicode$"
syn match agbfDirective "^.$"
syn match agbfDirective "^\(name\|author\|license\|colorize\|file\|size\|flags\|ascent\|advance\|lineskip\|underline-position\|underline-thickness\) "
syn keyword agbfColorizeMode all grays none

" Comment
syn region agbfComment start="^/\*" end="\*/$" contains=@Spell fold

syn match agbfUnicodeEscape "\\\x\{1,6}\s\?"
syn match agbfSpecialCharQQ +\\\\\|\\"+ contained
syn match agbfSpecialCharQ +\\\\\|\\'+ contained
syn region agbfStringQQ start=+"+ skip=+\\\\\|\\"+ end=+"+ contains=agbfUnicodeEscape,agbfSpecialCharQQ
syn region agbfStringQ start=+'+ skip=+\\\\\|\\'+ end=+'+ contains=agbfUnicodeEscape,agbfSpecialCharQ

if main_syntax == "agarbf"
  syn sync minlines=10
endif

" Define the default highlighting.
" Only when an item doesn't have highlighting yet

hi def link agbfComment Comment
hi def link agbfDirective Statement
hi def link agbfUnicodeEscape Special
hi def link agbfStringQQ String
hi def link agbfStringQ String
hi def link agbfValueInteger Number
hi def link agbfValueNumber Number
hi def link agbfUnitDecorators Number
hi def link agbfColorizeMode StorageClass

let b:current_syntax = "agarbf"

if main_syntax == 'agarbf'
  unlet main_syntax
endif

let &cpo = s:cpo_save
unlet s:cpo_save

" vim: ts=8
