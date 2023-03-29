/*	Public domain	*/

/*
 * Unicode character ranges.
 * Our table is based on Unicode 14.0.0.
 * The OS/2 ranges are based on the OpenType 1.9 specification.
 */

const AG_UnicodeRange agUnicodeRanges[] = {
 { N_("Basic Latin"), AGSI_BASIC_LATIN_BEGIN, AGSI_BASIC_LATIN_END, NULL },
 { N_("Latin-1 Supplement"), AGSI_LATIN1_SUPPL_BEGIN, AGSI_LATIN1_SUPPL_END, NULL },
 { N_("Latin Extended A"), AGSI_LATIN_EXT_A_BEGIN, AGSI_LATIN_EXT_A_END, NULL },
 { N_("Latin Extended B"), AGSI_LATIN_EXT_B_BEGIN, AGSI_LATIN_EXT_B_END, NULL },
 { N_("IPA Extensions"), AGSI_IPA_EXTENSIONS_BEGIN, AGSI_IPA_EXTENSIONS_END, NULL },
 { N_("Spacing Modifier Letters"), AGSI_SP_MOD_LETTERS_BEGIN, AGSI_SP_MOD_LETTERS_END, NULL },
 { N_("Combining Diacritical Marks"), AGSI_COMB_DIACRIT_MARKS_BEGIN, AGSI_COMB_DIACRIT_MARKS_END, NULL },
 { N_("Greek and Coptic"), AGSI_GREEK_AND_COPTIC_BEGIN, AGSI_GREEK_AND_COPTIC_END, NULL },
 { N_("Cyrillic"), AGSI_CYRILLIC_BEGIN, AGSI_CYRILLIC_END, NULL },
 { N_("Cyrillic Supplement"), AGSI_CYRILLIC_SUPPL_BEGIN, AGSI_CYRILLIC_SUPPL_END, NULL },
 { N_("Armenian"), AGSI_ARMENIAN_BEGIN, AGSI_ARMENIAN_END, NULL },
 { N_("Hebrew"), AGSI_HEBREW_BEGIN, AGSI_HEBREW_END, NULL },
 { N_("Arabic"), AGSI_ARABIC_BEGIN, AGSI_ARABIC_END, NULL },
 { N_("Syriac"), AGSI_SYRIAC_BEGIN, AGSI_SYRIAC_END, NULL },
 { N_("Arabic Supplement"), AGSI_ARABIC_SUPPL_BEGIN, AGSI_ARABIC_SUPPL_END, NULL },
 { N_("Thaana"), AGSI_THAANA_BEGIN, AGSI_THAANA_END, NULL },
 { N_("Nko"), AGSI_NKO_BEGIN, AGSI_NKO_END, NULL },
 { N_("Samaritan"), AGSI_SAMARITAN_BEGIN, AGSI_SAMARITAN_END, NULL },
 { N_("Mandaic"), AGSI_MANDAIC_BEGIN, AGSI_MANDAIC_END, NULL },
 { N_("Syriac Supplement"), AGSI_SYRIAC_SUPPL_BEGIN, AGSI_SYRIAC_SUPPL_END, NULL },
 { N_("Arabic Extended B"), AGSI_ARABIC_EXT_B_BEGIN, AGSI_ARABIC_EXT_B_END, NULL },
 { N_("Arabic Extended A"), AGSI_ARABIC_EXT_A_BEGIN, AGSI_ARABIC_EXT_A_END, NULL },
 { N_("Devanagari"), AGSI_DEVANAGARI_BEGIN, AGSI_DEVANAGARI_END, NULL },
 { N_("Bengali"), AGSI_BENGALI_BEGIN, AGSI_BENGALI_END, NULL },
 { N_("Gurmukhi"), AGSI_GURMUKHI_BEGIN, AGSI_GURMUKHI_END, NULL },
 { N_("Gujarati"), AGSI_GUJARATI_BEGIN, AGSI_GUJARATI_END, NULL },
 { N_("Oriya"), AGSI_ORIYA_BEGIN, AGSI_ORIYA_END, NULL },
 { N_("Tamil"), AGSI_TAMIL_BEGIN, AGSI_TAMIL_END, NULL },
 { N_("Telugu"), AGSI_TELUGU_BEGIN, AGSI_TELUGU_END, NULL },
 { N_("Kannada"), AGSI_KANNADA_BEGIN, AGSI_KANNADA_END, NULL },
 { N_("Malayalam"), AGSI_MALAYALAM_BEGIN, AGSI_MALAYALAM_END, NULL },
 { N_("Sinhala"), AGSI_SINHALA_BEGIN, AGSI_SINHALA_END, NULL },
 { N_("Thai"), AGSI_THAI_BEGIN, AGSI_THAI_END, NULL },
 { N_("Lao"), AGSI_LAO_BEGIN, AGSI_LAO_END, NULL },
 { N_("Tibetan"), AGSI_TIBETAN_BEGIN, AGSI_TIBETAN_END, NULL },
 { N_("Myanmar"), AGSI_MYANMAR_BEGIN, AGSI_MYANMAR_END, NULL },
 { N_("Georgian"), AGSI_GEORGIAN_BEGIN, AGSI_GEORGIAN_END, NULL },
 { N_("Hangul Jamo"), AGSI_HANGUL_JAMO_BEGIN, AGSI_HANGUL_JAMO_END, NULL },
 { N_("Ethiopic"), AGSI_ETHIOPIC_BEGIN, AGSI_ETHIOPIC_END, NULL },
 { N_("Ethiopic Supplement"), AGSI_ETHIOPIC_SUPPL_BEGIN, AGSI_ETHIOPIC_SUPPL_END, NULL },
 { N_("Cherokee"), AGSI_CHEROKEE_BEGIN, AGSI_CHEROKEE_END, NULL },
 { N_("Unified Canadian Aboriginal Syllabics"), AGSI_UNI_CA_ABORIG_SYLL_BEGIN, AGSI_UNI_CA_ABORIG_SYLL_END, NULL },
 { N_("Ogham"), AGSI_OGHAM_BEGIN, AGSI_OGHAM_END, NULL },
 { N_("Runic"), AGSI_RUNIC_BEGIN, AGSI_RUNIC_END, NULL },
 { N_("Tagalog"), AGSI_TAGALOG_BEGIN, AGSI_TAGALOG_END, NULL },
 { N_("Hanunoo"), AGSI_HANUNOO_BEGIN, AGSI_HANUNOO_END, NULL },
 { N_("Buhid"), AGSI_BUHID_BEGIN, AGSI_BUHID_END, NULL },
 { N_("Tagbanwa"), AGSI_TAGBANWA_BEGIN, AGSI_TAGBANWA_END, NULL },
 { N_("Khmer"), AGSI_KHMER_BEGIN, AGSI_KHMER_END, NULL },
 { N_("Mongolian"), AGSI_MONGOLIAN_BEGIN, AGSI_MONGOLIAN_END, NULL },
 { N_("Unified Canadian Aboriginal Syllabics Extended"), AGSI_UNI_CA_ABORIG_SYLL_EXT_BEGIN, AGSI_UNI_CA_ABORIG_SYLL_EXT_END, NULL },
 { N_("Limbu"), AGSI_LIMBU_BEGIN, AGSI_LIMBU_END, NULL },
 { N_("Tai Le"), AGSI_TAI_LE_BEGIN, AGSI_TAI_LE_END, NULL },
 { N_("New Tai Lue"), AGSI_NEW_TAI_LUE_BEGIN, AGSI_NEW_TAI_LUE_END, NULL },
 { N_("Khmer Symbols"), AGSI_KHMER_SYM_BEGIN, AGSI_KHMER_SYM_END, NULL },
 { N_("Buginese"), AGSI_BUGINESE_BEGIN, AGSI_BUGINESE_END, NULL },
 { N_("Tai Tham"), AGSI_TAI_THAM_BEGIN, AGSI_TAI_THAM_END, NULL },
 { N_("Combined Diacritical Marks Extended"), AGSI_COMB_DIACRIT_MARKS_EXT_BEGIN, AGSI_COMB_DIACRIT_MARKS_EXT_END, NULL },
 { N_("Balinese"), AGSI_BALINESE_BEGIN, AGSI_BALINESE_END, NULL },
 { N_("Sundanese"), AGSI_SUNDANESE_BEGIN, AGSI_SUNDANESE_END, NULL },
 { N_("Batak"), AGSI_BATAK_BEGIN, AGSI_BATAK_END, NULL },
 { N_("Lepcha"), AGSI_LEPCHA_BEGIN, AGSI_LEPCHA_END, NULL },
 { N_("Ol Chiki"), AGSI_OL_CHIKI_BEGIN, AGSI_OL_CHIKI_END, NULL },
 { N_("Cyrillic Extended C"), AGSI_CYRILLIC_EXT_C_BEGIN, AGSI_CYRILLIC_EXT_C_END, NULL },
 { N_("Georgian Extended"), AGSI_GEORGIAN_EXT_BEGIN, AGSI_GEORGIAN_EXT_END, NULL },
 { N_("Sundanese Supplement"), AGSI_SUNDANESE_SUPPL_BEGIN, AGSI_SUNDANESE_SUPPL_END, NULL },
 { N_("Vedic Extensions"), AGSI_VEDIC_EXTENSIONS_BEGIN, AGSI_VEDIC_EXTENSIONS_END, NULL },
 { N_("Phonetic Extensions"), AGSI_PHONETIC_EXTENSIONS_BEGIN, AGSI_PHONETIC_EXTENSIONS_END, NULL },
 { N_("Combining Diacritical Marks Supplement"), AGSI_COMB_DIACRIT_MARKS_SUPPL_BEGIN, AGSI_COMB_DIACRIT_MARKS_SUPPL_END, NULL },
 { N_("Latin Extended Additional"), AGSI_LATIN_EXT_ADDITIONAL_BEGIN, AGSI_LATIN_EXT_ADDITIONAL_END, NULL },
 { N_("Greek Extended"), AGSI_GREEK_EXT_BEGIN, AGSI_GREEK_EXT_END, NULL },
 { N_("General Punctuation"), AGSI_GENERAL_PUNCTUATION_BEGIN, AGSI_GENERAL_PUNCTUATION_END, NULL },
 { N_("Superscripts and Subscripts"), AGSI_SUPERSCRIPTS_AND_SUBSCRIPTS_BEGIN, AGSI_SUPERSCRIPTS_AND_SUBSCRIPTS_END, NULL },
 { N_("Currency Symbols"), AGSI_CURRENCY_SYM_BEGIN, AGSI_CURRENCY_SYM_END, NULL },
 { N_("Combining Diacritical Marks For Symbols"), AGSI_COMB_DIACRIT_MARKS_FOR_SYM_BEGIN, AGSI_COMB_DIACRIT_MARKS_FOR_SYM_END, NULL },
 { N_("Letterlike Symbols"), AGSI_LETTERLIKE_SYM_BEGIN, AGSI_LETTERLIKE_SYM_END, NULL },
 { N_("Number Forms"), AGSI_NUMBER_FORMS_BEGIN, AGSI_NUMBER_FORMS_END, NULL },
 { N_("Arrows"), AGSI_ARROWS_BEGIN, AGSI_ARROWS_END, NULL },
 { N_("Mathematical Operators"), AGSI_MATH_OPERATORS_BEGIN, AGSI_MATH_OPERATORS_END, NULL },
 { N_("Miscellaneous Technical"), AGSI_MISC_TECH_BEGIN, AGSI_MISC_TECH_END, NULL },
 { N_("Control Pictures"), AGSI_CONTROL_PICTURES_BEGIN, AGSI_CONTROL_PICTURES_END, NULL },
 { N_("Optical Character Recognition"), AGSI_OCR_BEGIN, AGSI_OCR_END, NULL },
 { N_("Enclosed Alphanumerics"), AGSI_ENCLOSED_ALPHANUMERICS_BEGIN, AGSI_ENCLOSED_ALPHANUMERICS_END, NULL },
 { N_("Box Drawing"), AGSI_BOX_DRAWING_BEGIN, AGSI_BOX_DRAWING_END, NULL },
 { N_("Block Elements"), AGSI_BLOCK_ELEMENTS_BEGIN, AGSI_BLOCK_ELEMENTS_END, NULL },
 { N_("Geometric Shapes"), AGSI_GEOMETRIC_SHAPES_BEGIN, AGSI_GEOMETRIC_SHAPES_END, NULL },
 { N_("Miscellaneous Symbols"), AGSI_MISC_SYM_BEGIN, AGSI_MISC_SYM_END, NULL },
 { N_("Dingbats"), AGSI_DINGBATS_BEGIN, AGSI_DINGBATS_END, NULL },
 { N_("Miscellaneous Mathematical Symbols A"), AGSI_MISC_MATH_SYM_A_BEGIN, AGSI_MISC_MATH_SYM_A_END, NULL },
 { N_("Supplemental Arrows A"), AGSI_SUPPL_ARROWS_A_BEGIN, AGSI_SUPPL_ARROWS_A_END, NULL },
 { N_("Braille Patterns"), AGSI_BRAILLE_PATTERNS_BEGIN, AGSI_BRAILLE_PATTERNS_END, NULL },
 { N_("Supplemental Arrows B"), AGSI_SUPPL_ARROWS_B_BEGIN, AGSI_SUPPL_ARROWS_B_END, NULL },
 { N_("Miscellaneous Mathematical Symbols B"), AGSI_MISC_MATH_SYM_B_BEGIN, AGSI_MISC_MATH_SYM_B_END, NULL },
 { N_("Supplemental Mathematical Operators"), AGSI_SUPPL_MATH_OPERATORS_BEGIN, AGSI_SUPPL_MATH_OPERATORS_END, NULL },
 { N_("Miscellaneous Symbols and Arrows"), AGSI_MISC_SYM_AND_ARROWS_BEGIN, AGSI_MISC_SYM_AND_ARROWS_END, NULL },
 { N_("Glagolitic"), AGSI_GLAGOLITIC_BEGIN, AGSI_GLAGOLITIC_END, NULL },
 { N_("Latin Extended C"), AGSI_LATIN_EXT_C_BEGIN, AGSI_LATIN_EXT_C_END, NULL },
 { N_("Coptic"), AGSI_COPTIC_BEGIN, AGSI_COPTIC_END, NULL },
 { N_("Georgian Supplement"), AGSI_GEORGIAN_SUPPL_BEGIN, AGSI_GEORGIAN_SUPPL_END, NULL },
 { N_("Tifinagh"), AGSI_TIFINAGH_BEGIN, AGSI_TIFINAGH_END, NULL },
 { N_("Ethiopic Extended"), AGSI_ETHIOPIC_EXT_BEGIN, AGSI_ETHIOPIC_EXT_END, NULL },
 { N_("Cyrillic Extended A"), AGSI_CYRILLIC_EXT_A_BEGIN, AGSI_CYRILLIC_EXT_A_END, NULL },
 { N_("Supplemental Punctuation"), AGSI_SUPPL_PUNCTUATION_BEGIN, AGSI_SUPPL_PUNCTUATION_END, NULL },
 { N_("CJK Radicals Supplement"), AGSI_CJK_RADICALS_SUPPL_BEGIN, AGSI_CJK_RADICALS_SUPPL_END, NULL },
 { N_("Kangxi Radicals"), AGSI_KANGXI_RADICALS_BEGIN, AGSI_KANGXI_RADICALS_END, NULL },
 { N_("Ideographic Description Characters"), AGSI_IDEO_DESCR_CHARS_BEGIN, AGSI_IDEO_DESCR_CHARS_END, NULL },
 { N_("CJK Symbols and Punctuation"), AGSI_CJK_SYM_AND_PUNCT_BEGIN, AGSI_CJK_SYM_AND_PUNCT_END, NULL },
 { N_("Hiragana"), AGSI_HIRAGANA_BEGIN, AGSI_HIRAGANA_END, NULL },
 { N_("Katakana"), AGSI_KATAKANA_BEGIN, AGSI_KATAKANA_END, NULL },
 { N_("Bopomofo"), AGSI_BOPOMOFO_BEGIN, AGSI_BOPOMOFO_END, NULL },
 { N_("Hangul Compatibility Jamo"), AGSI_HANGUL_COMPAT_JAMO_BEGIN, AGSI_HANGUL_COMPAT_JAMO_END, NULL },
 { N_("Kanbun"), AGSI_KANBUN_BEGIN, AGSI_KANBUN_END, NULL },
 { N_("Bopomofo Extended"), AGSI_BOPOMOFO_EXT_BEGIN, AGSI_BOPOMOFO_EXT_END, NULL },
 { N_("CJK Strokes"), AGSI_CJK_STROKES_BEGIN, AGSI_CJK_STROKES_END, NULL },
 { N_("Katakana Phonetic Extensions"), AGSI_KATAKANA_PHONETIC_EXTS_BEGIN, AGSI_KATAKANA_PHONETIC_EXTS_END, NULL },
 { N_("Enclosed CJK Letters and Months"), AGSI_ENCL_CJK_LETTERS_MONTHS_BEGIN, AGSI_ENCL_CJK_LETTERS_MONTHS_END, NULL },
 { N_("CJK Compatibility"), AGSI_CJK_COMPAT_BEGIN, AGSI_CJK_COMPAT_END, NULL },
 { N_("CJK Unified Ideographs Extension A"), AGSI_CJK_UNI_IDEO_EXT_A_BEGIN, AGSI_CJK_UNI_IDEO_EXT_A_END, NULL },
 { N_("Yijing Hexagram Symbols"), AGSI_YIJING_HEXAGRAM_SYM_BEGIN, AGSI_YIJING_HEXAGRAM_SYM_END, NULL },
 { N_("CJK Unified Ideographs"), AGSI_CJK_UNI_IDEO_BEGIN, AGSI_CJK_UNI_IDEO_END, NULL },
 { N_("Yi Syllables"), AGSI_YI_SYLLABLES_BEGIN, AGSI_YI_SYLLABLES_END, NULL },
 { N_("Yi Radicals"), AGSI_YI_RADICALS_BEGIN, AGSI_YI_RADICALS_END, NULL },
 { N_("Lisu"), AGSI_LISU_BEGIN, AGSI_LISU_END, NULL },
 { N_("Vai"), AGSI_VAI_BEGIN, AGSI_VAI_END, NULL },
 { N_("Cyrillic Extended B"), AGSI_CYRILLIC_EXT_B_BEGIN, AGSI_CYRILLIC_EXT_B_END, NULL },
 { N_("Bamum"), AGSI_BAMUM_BEGIN, AGSI_BAMUM_END, NULL },
 { N_("Modifier Tone Letters"), AGSI_MOD_TONE_LETTERS_BEGIN, AGSI_MOD_TONE_LETTERS_END, NULL },
 { N_("Latin Extended D"), AGSI_LATIN_EXT_D_BEGIN, AGSI_LATIN_EXT_D_END, NULL },
 { N_("Syloti Nagri"), AGSI_SYLOTI_NAGRI_BEGIN, AGSI_SYLOTI_NAGRI_END, NULL },
 { N_("Common Indic Number Forms"), AGSI_COMMON_INDIC_NUMBER_FORMS_BEGIN, AGSI_COMMON_INDIC_NUMBER_FORMS_END, NULL },
 { N_("Phags-pa"), AGSI_PHAGS_PA_BEGIN, AGSI_PHAGS_PA_END, NULL },
 { N_("Saurashtra"), AGSI_SAURASHTRA_BEGIN, AGSI_SAURASHTRA_END, NULL },
 { N_("Devanagari"), AGSI_DEVANAGARI_EXT_BEGIN, AGSI_DEVANAGARI_EXT_END, NULL },
 { N_("Kayah Li"), AGSI_KAYAH_LI_BEGIN, AGSI_KAYAH_LI_END, NULL },
 { N_("Rejang"), AGSI_REJANG_BEGIN, AGSI_REJANG_END, NULL },
 { N_("Hangul Jamo Extended A"), AGSI_HANGUL_JAMO_EXT_A_BEGIN, AGSI_HANGUL_JAMO_EXT_A_END, NULL },
 { N_("Javanese"), AGSI_JAVANESE_BEGIN, AGSI_JAVANESE_END, NULL },
 { N_("Myanmar Extended B"), AGSI_MYANMAR_EXT_B_BEGIN, AGSI_MYANMAR_EXT_B_END, NULL },
 { N_("Cham"), AGSI_CHAM_BEGIN, AGSI_CHAM_END, NULL },
 { N_("Myanmar Extended A"), AGSI_MYANMAR_EXT_A_BEGIN, AGSI_MYANMAR_EXT_A_END, NULL },
 { N_("Tai Viet"), AGSI_TAI_VIET_BEGIN, AGSI_TAI_VIET_END, NULL },
 { N_("Meetei Mayek Extensions"), AGSI_MEETEI_MAYEK_EXTENSIONS_BEGIN, AGSI_MEETEI_MAYEK_EXTENSIONS_END, NULL },
 { N_("Ethiopic Extended A"), AGSI_ETHIOPIC_EXT_A_BEGIN, AGSI_ETHIOPIC_EXT_A_END, NULL },
 { N_("Latin Extended E"), AGSI_LATIN_EXT_E_BEGIN, AGSI_LATIN_EXT_E_END, NULL },
 { N_("Cherokee Supplement"), AGSI_CHEROKEE_SUPPL_BEGIN, AGSI_CHEROKEE_SUPPL_END, NULL },
 { N_("Meetei Mayek"), AGSI_MEETEI_MAYEK_BEGIN, AGSI_MEETEI_MAYEK_END, NULL },
 { N_("Hangul Syllables"), AGSI_HANGUL_SYLLABLES_BEGIN, AGSI_HANGUL_SYLLABLES_END, NULL },
 { N_("Hangul Jamo Extended B"), AGSI_HANGUL_JAMO_EXT_B_BEGIN, AGSI_HANGUL_JAMO_EXT_B_END, NULL },
 { N_("High Surrogate Area"), AGSI_HIGH_SURROGATE_AREA_BEGIN, AGSI_HIGH_SURROGATE_AREA_END, NULL },
 { N_("Low Surrogate Area"), AGSI_LOW_SURROGATE_AREA_BEGIN, AGSI_LOW_SURROGATE_AREA_END, NULL },
 { N_("Private Use Area"), AGSI_PRIVATE_USE_AREA_BEGIN, AGSI_PRIVATE_USE_AREA_END, NULL },
 { N_("CJK Compatibility Ideographs"), AGSI_CJK_COMPAT_IDEO_BEGIN, AGSI_CJK_COMPAT_IDEO_END, NULL },
 { N_("Alphabetic Presentation Forms"), AGSI_ALPHA_PRES_FORMS_BEGIN, AGSI_ALPHA_PRES_FORMS_END, NULL },
 { N_("Arabic Presentation Forms A"), AGSI_ARABIC_PRES_FORMS_A_BEGIN, AGSI_ARABIC_PRES_FORMS_A_END, NULL },
 { N_("Variation Selectors"), AGSI_VARIATION_SELECTORS_BEGIN, AGSI_VARIATION_SELECTORS_END, NULL },
 { N_("Vertical Forms"), AGSI_VERTICAL_FORMS_BEGIN, AGSI_VERTICAL_FORMS_END, NULL },
 { N_("Combining Half Marks"), AGSI_COMBINING_HALF_MARKS_BEGIN, AGSI_COMBINING_HALF_MARKS_END, NULL },
 { N_("CJK Compatibility Forms"), AGSI_CJK_COMPAT_FORMS_BEGIN, AGSI_CJK_COMPAT_FORMS_END, NULL },
 { N_("Small Form Variants"), AGSI_SMALL_FORM_VARIANTS_BEGIN, AGSI_SMALL_FORM_VARIANTS_END, NULL },
 { N_("Arabic Presentation Forms B"), AGSI_ARABIC_PRES_FORMS_B_BEGIN, AGSI_ARABIC_PRES_FORMS_B_END, NULL },
 { N_("Halfwidth and Fullwidth Forms"), AGSI_HALFWIDTH_FULLWIDTH_FORMS_BEGIN, AGSI_HALFWIDTH_FULLWIDTH_FORMS_END, NULL },
 { N_("Specials"), AGSI_SPECIALS_BEGIN, AGSI_SPECIALS_END, NULL },
 { N_("Linear B Syllabary"), AGSI_LINEAR_B_SYLLABARY_BEGIN, AGSI_LINEAR_B_SYLLABARY_END, NULL },
 { N_("Linear B Ideograms"), AGSI_LINEAR_B_IDEOGRAMS_BEGIN, AGSI_LINEAR_B_IDEOGRAMS_END, NULL },
 { N_("Aegean Numbers"), AGSI_AEGEAN_NUMBERS_BEGIN, AGSI_AEGEAN_NUMBERS_END, NULL },
 { N_("Ancient Greek Numbers"), AGSI_ANCIENT_GREEK_NUMBERS_BEGIN, AGSI_ANCIENT_GREEK_NUMBERS_END, NULL },
 { N_("Ancient Symbols"), AGSI_ANCIENT_SYM_BEGIN, AGSI_ANCIENT_SYM_END, NULL },
 { N_("Phaistos Disc"), AGSI_PHAISTOS_DISC_BEGIN, AGSI_PHAISTOS_DISC_END, NULL },
 { N_("Lycian"), AGSI_LYCIAN_BEGIN, AGSI_LYCIAN_END, NULL },
 { N_("Carian"), AGSI_CARIAN_BEGIN, AGSI_CARIAN_END, NULL },
 { N_("Coptic Epact Numbers"), AGSI_COPTIC_EPACT_NUMBERS_BEGIN, AGSI_COPTIC_EPACT_NUMBERS_END, NULL },
 { N_("Old Italic"), AGSI_OLD_ITALIC_BEGIN, AGSI_OLD_ITALIC_END, NULL },
 { N_("Gothic"), AGSI_GOTHIC_BEGIN, AGSI_GOTHIC_END, NULL },
 { N_("Old Permic"), AGSI_OLD_PERMIC_BEGIN, AGSI_OLD_PERMIC_END, NULL },
 { N_("Ugaritic"), AGSI_UGARITIC_BEGIN, AGSI_UGARITIC_END, NULL },
 { N_("Old Persian"), AGSI_OLD_PERSIAN_BEGIN, AGSI_OLD_PERSIAN_END, NULL },
 { N_("Deseret"), AGSI_DESERET_BEGIN, AGSI_DESERET_END, NULL },
 { N_("Shavian"), AGSI_SHAVIAN_BEGIN, AGSI_SHAVIAN_END, NULL },
 { N_("Osmanya"), AGSI_OSMANYA_BEGIN, AGSI_OSMANYA_END, NULL },
 { N_("Osage"), AGSI_OSAGE_BEGIN, AGSI_OSAGE_END, NULL },
 { N_("Elbasan"), AGSI_ELBASAN_BEGIN, AGSI_ELBASAN_END, NULL },
 { N_("Caucasian Albanian"), AGSI_CAUCASIAN_ALBANIAN_BEGIN, AGSI_CAUCASIAN_ALBANIAN_END, NULL },
 { N_("Vithkuqi"), AGSI_VITHKUQI_BEGIN, AGSI_VITHKUQI_END, NULL },
 { N_("Linear A"), AGSI_LINEAR_A_BEGIN, AGSI_LINEAR_A_END, NULL },
 { N_("Latin Extended F"), AGSI_LATIN_EXT_F_BEGIN, AGSI_LATIN_EXT_F_END, NULL },
 { N_("Cypriot Syllabary"), AGSI_CYPRIOT_SYLLABARY_BEGIN, AGSI_CYPRIOT_SYLLABARY_END, NULL },
 { N_("Imperial Aramaic"), AGSI_IMPERIAL_ARAMAIC_BEGIN, AGSI_IMPERIAL_ARAMAIC_END, NULL },
 { N_("Palmyrene"), AGSI_PALMYRENE_BEGIN, AGSI_PALMYRENE_END, NULL },
 { N_("Nabataean"), AGSI_NABATAEAN_BEGIN, AGSI_NABATAEAN_END, NULL },
 { N_("Hatran"), AGSI_HATRAN_BEGIN, AGSI_HATRAN_END, NULL },
 { N_("Phoenician"), AGSI_PHOENICIAN_BEGIN, AGSI_PHOENICIAN_END, NULL },
 { N_("Lydian"), AGSI_LYDIAN_BEGIN, AGSI_LYDIAN_END, NULL },
 { N_("Meroitic Hieroglyphs"), AGSI_MEROITIC_HIEROGLYPHS_BEGIN, AGSI_MEROITIC_HIEROGLYPHS_END, NULL },
 { N_("Meroitic Cursive"), AGSI_MEROITIC_CURSIVE_BEGIN, AGSI_MEROITIC_CURSIVE_END, NULL },
 { N_("Kharoshthi"), AGSI_KHAROSHTHI_BEGIN, AGSI_KHAROSHTHI_END, NULL },
 { N_("Old South Arabian"), AGSI_OLD_SOUTH_ARABIAN_BEGIN, AGSI_OLD_SOUTH_ARABIAN_END, NULL },
 { N_("Old North Arabian"), AGSI_OLD_NORTH_ARABIAN_BEGIN, AGSI_OLD_NORTH_ARABIAN_END, NULL },
 { N_("Manichaean"), AGSI_MANICHAEAN_BEGIN, AGSI_MANICHAEAN_END, NULL },
 { N_("Avestan"), AGSI_AVESTAN_BEGIN, AGSI_AVESTAN_END, NULL },
 { N_("Inscriptional Parthian"), AGSI_INSCRIPTIONAL_PARTHIAN_BEGIN, AGSI_INSCRIPTIONAL_PARTHIAN_END, NULL },
 { N_("Inscriptional Pahlavi"), AGSI_INSCRIPTIONAL_PAHLAVI_BEGIN, AGSI_INSCRIPTIONAL_PAHLAVI_END, NULL },
 { N_("Psalter Pahlavi"), AGSI_PSALTER_PAHLAVI_BEGIN, AGSI_PSALTER_PAHLAVI_END, NULL },
 { N_("Old Turkic"), AGSI_OLD_TURKIC_BEGIN, AGSI_OLD_TURKIC_END, NULL },
 { N_("Old Hungarian"), AGSI_OLD_HUNGARIAN_BEGIN, AGSI_OLD_HUNGARIAN_END, NULL },
 { N_("Hanifi Rohingya"), AGSI_HANIFI_ROHINGYA_BEGIN, AGSI_HANIFI_ROHINGYA_END, NULL },
 { N_("Rumi Numeral Symbols"), AGSI_RUMI_NUMERAL_SYM_BEGIN, AGSI_RUMI_NUMERAL_SYM_END, NULL },
 { N_("Yezidi"), AGSI_YEZIDI_BEGIN, AGSI_YEZIDI_END, NULL },
 { N_("Old Sogdian"), AGSI_OLD_SOGDIAN_BEGIN, AGSI_OLD_SOGDIAN_END, NULL },
 { N_("Sogdian"), AGSI_SOGDIAN_BEGIN, AGSI_SOGDIAN_END, NULL },
 { N_("Old Uyghur"), AGSI_OLD_UYGHUR_BEGIN, AGSI_OLD_UYGHUR_END, NULL },
 { N_("Chorasmian"), AGSI_CHORASMIAN_BEGIN, AGSI_CHORASMIAN_END, NULL },
 { N_("Elmaic"), AGSI_ELMAIC_BEGIN, AGSI_ELMAIC_END, NULL },
 { N_("Brahmi"), AGSI_BRAHMI_BEGIN, AGSI_BRAHMI_END, NULL },
 { N_("Kaithi"), AGSI_KAITHI_BEGIN, AGSI_KAITHI_END, NULL },
 { N_("Sora Sompeng"), AGSI_SORA_SOMPENG_BEGIN, AGSI_SORA_SOMPENG_END, NULL },
 { N_("Chakma"), AGSI_CHAKMA_BEGIN, AGSI_CHAKMA_END, NULL },
 { N_("Mahajani"), AGSI_MAHAJANI_BEGIN, AGSI_MAHAJANI_END, NULL },
 { N_("Sharada"), AGSI_SHARADA_BEGIN, AGSI_SHARADA_END, NULL },
 { N_("Sinhala"), AGSI_SINHALA_ARCHAIC_NUMBERS_BEGIN, AGSI_SINHALA_ARCHAIC_NUMBERS_END, NULL },
 { N_("Khojki"), AGSI_KHOJKI_BEGIN, AGSI_KHOJKI_END, NULL },
 { N_("Multani"), AGSI_MULTANI_BEGIN, AGSI_MULTANI_END, NULL },
 { N_("Khudawadi"), AGSI_KHUDAWADI_BEGIN, AGSI_KHUDAWADI_END, NULL },
 { N_("Grantha"), AGSI_GRANTHA_BEGIN, AGSI_GRANTHA_END, NULL },
 { N_("Newa"), AGSI_NEWA_BEGIN, AGSI_NEWA_END, NULL },
 { N_("Tirhuta"), AGSI_TIRHUTA_BEGIN, AGSI_TIRHUTA_END, NULL },
 { N_("Siddham"), AGSI_SIDDHAM_BEGIN, AGSI_SIDDHAM_END, NULL },
 { N_("Modi"), AGSI_MODI_BEGIN, AGSI_MODI_END, NULL },
 { N_("Mongolian Supplement"), AGSI_MONGOLIAN_SUPPL_BEGIN, AGSI_MONGOLIAN_SUPPL_END, NULL },
 { N_("Takri"), AGSI_TAKRI_BEGIN, AGSI_TAKRI_END, NULL },
 { N_("Ahom"), AGSI_AHOM_BEGIN, AGSI_AHOM_END, NULL },
 { N_("Dogra"), AGSI_DOGRA_BEGIN, AGSI_DOGRA_END, NULL },
 { N_("Warang Citi"), AGSI_WARANG_CITI_BEGIN, AGSI_WARANG_CITI_END, NULL },
 { N_("Dives Akuru"), AGSI_DIVES_AKURU_BEGIN, AGSI_DIVES_AKURU_END, NULL },
 { N_("Nandinagari"), AGSI_NANDINAGARI_BEGIN, AGSI_NANDINAGARI_END, NULL },
 { N_("Zanabazar Square"), AGSI_ZANABAZAR_SQUARE_BEGIN, AGSI_ZANABAZAR_SQUARE_END, NULL },
 { N_("Soyombo"), AGSI_SOYOMBO_BEGIN, AGSI_SOYOMBO_END, NULL },
 { N_("Unified Canadian Aboriginal Syllabics Extended A"), AGSI_UNI_CA_ABORIG_SYLL_EXT_A_BEGIN, AGSI_UNI_CA_ABORIG_SYLL_EXT_A_END, NULL },
 { N_("Pau Cin Hau"), AGSI_PAU_CIN_HAU_BEGIN, AGSI_PAU_CIN_HAU_END, NULL },
 { N_("Bhaiksuki"), AGSI_BHAIKSUKI_BEGIN, AGSI_BHAIKSUKI_END, NULL },
 { N_("Marchen"), AGSI_MARCHEN_BEGIN, AGSI_MARCHEN_END, NULL },
 { N_("Masaram Gondi"), AGSI_MASARAM_GONDI_BEGIN, AGSI_MASARAM_GONDI_END, NULL },
 { N_("Gunjala Gondi"), AGSI_GUNJALA_GONDI_BEGIN, AGSI_GUNJALA_GONDI_END, NULL },
 { N_("Makasar"), AGSI_MAKASAR_BEGIN, AGSI_MAKASAR_END, NULL },
 { N_("Lisu Supplement"), AGSI_LISU_SUPPL_BEGIN, AGSI_LISU_SUPPL_END, NULL },
 { N_("Tamil Supplement"), AGSI_TAMIL_SUPPL_BEGIN, AGSI_TAMIL_SUPPL_END, NULL },
 { N_("Cuneiform"), AGSI_CUNEIFORM_BEGIN, AGSI_CUNEIFORM_END, NULL },
 { N_("Cuneiform Numbers and Punctuation"), AGSI_CUNEIFORM_NUMBERS_AND_PUNCT_BEGIN, AGSI_CUNEIFORM_NUMBERS_AND_PUNCT_END, NULL },
 { N_("Early Dynastic Cuneiform"), AGSI_EARLY_DYNASTIC_CUNEIFORM_BEGIN, AGSI_EARLY_DYNASTIC_CUNEIFORM_END, NULL },
 { N_("Cypro-Minoan"), AGSI_CYPRO_MINOAN_BEGIN, AGSI_CYPRO_MINOAN_END, NULL },
 { N_("Egyptian Hieroglyphs"), AGSI_EGYPTIAN_HIEROGLYPHS_BEGIN, AGSI_EGYPTIAN_HIEROGLYPHS_END, NULL },
 { N_("Egyptian Hieroglyph Format Controls"), AGSI_EGYPTIAN_HIEROGLYPHS_FMT_BEGIN, AGSI_EGYPTIAN_HIEROGLYPHS_FMT_END, NULL },
 { N_("Anatolian Hieroglyphs"), AGSI_ANATOLIAN_HIEROGLYPHS_BEGIN, AGSI_ANATOLIAN_HIEROGLYPHS_END, NULL },
 { N_("Bamum Supplement"), AGSI_BAMUM_SUPPL_BEGIN, AGSI_BAMUM_SUPPL_END, NULL },
 { N_("Mro"), AGSI_MRO_BEGIN, AGSI_MRO_END, NULL },
 { N_("Tangsa"), AGSI_TANGSA_BEGIN, AGSI_TANGSA_END, NULL },
 { N_("Bassa Vah"), AGSI_BASSA_VAH_BEGIN, AGSI_BASSA_VAH_END, NULL },
 { N_("Pahawh Hmong"), AGSI_PAHAWH_HMONG_BEGIN, AGSI_PAHAWH_HMONG_END, NULL },
 { N_("Medefaidrin"), AGSI_MEDEFAIDRIN_BEGIN, AGSI_MEDEFAIDRIN_END, NULL },
 { N_("Miao"), AGSI_MIAO_BEGIN, AGSI_MIAO_END, NULL },
 { N_("Ideographic Symbols and Punctuation"), AGSI_IDEO_SYM_AND_PUNCT_BEGIN, AGSI_IDEO_SYM_AND_PUNCT_END, NULL },
 { N_("Tangut"), AGSI_TANGUT_BEGIN, AGSI_TANGUT_END, NULL },
 { N_("Tangut Components"), AGSI_TANGUT_COMPONENTS_BEGIN, AGSI_TANGUT_COMPONENTS_END, NULL },
 { N_("Khitan Small Script"), AGSI_KHITAN_SMALL_SCRIPT_BEGIN, AGSI_KHITAN_SMALL_SCRIPT_END, NULL },
 { N_("Tangut Supplement"), AGSI_TANGUT_SUPPL_BEGIN, AGSI_TANGUT_SUPPL_END, NULL },
 { N_("Kana Extended B"), AGSI_KANA_EXT_B_BEGIN, AGSI_KANA_EXT_B_END, NULL },
 { N_("Kana Supplement"), AGSI_KANA_SUPPL_BEGIN, AGSI_KANA_SUPPL_END, NULL },
 { N_("Kana Extended A"), AGSI_KANA_EXT_A_BEGIN, AGSI_KANA_EXT_A_END, NULL },
 { N_("Small Kana Extension"), AGSI_SMALL_KANA_EXT_BEGIN, AGSI_SMALL_KANA_EXT_END, NULL },
 { N_("Nushu"), AGSI_NUSHU_BEGIN, AGSI_NUSHU_END, NULL },
 { N_("Duployan"), AGSI_DUPLOYAN_BEGIN, AGSI_DUPLOYAN_END, NULL },
 { N_("Shorthand Format Controls"), AGSI_SHORTHAND_FORMAT_CONTROLS_BEGIN, AGSI_SHORTHAND_FORMAT_CONTROLS_END, NULL },
 { N_("Znamenny Musical Notation"), AGSI_ZNAMENNY_MUSICAL_NOT_BEGIN, AGSI_ZNAMENNY_MUSICAL_NOT_END, NULL },
 { N_("Byzantine Musical Symbols"), AGSI_BYZANTINE_MUSICAL_SYM_BEGIN, AGSI_BYZANTINE_MUSICAL_SYM_END, NULL },
 { N_("Musical Symbols"), AGSI_MUSICAL_SYM_BEGIN, AGSI_MUSICAL_SYM_END, NULL },
 { N_("Ancient Greek Musical Notation"), AGSI_ANCIENT_GREEK_MUSICAL_NOT_BEGIN, AGSI_ANCIENT_GREEK_MUSICAL_NOT_END, NULL },
 { N_("Mayan Numerals"), AGSI_MAYAN_NUMERALS_BEGIN, AGSI_MAYAN_NUMERALS_END, NULL },
 { N_("Tai Xuan Jing Symbols"), AGSI_TAI_XUAN_JING_SYM_BEGIN, AGSI_TAI_XUAN_JING_SYM_END, NULL },
 { N_("Counting Rod Numerals"), AGSI_COUNTING_ROD_NUMERALS_BEGIN, AGSI_COUNTING_ROD_NUMERALS_END, NULL },
 { N_("Mathematical Alphanumeric Symbols"), AGSI_MATH_ALPHANUMERIC_SYM_BEGIN, AGSI_MATH_ALPHANUMERIC_SYM_END, NULL },
 { N_("Sutton Sign Writing"), AGSI_SUTTON_SIGNWRITING_BEGIN, AGSI_SUTTON_SIGNWRITING_END, NULL },
 { N_("Latin Extended G"), AGSI_LATIN_EXT_G_BEGIN, AGSI_LATIN_EXT_G_END, NULL },
 { N_("Glagolitic Supplement"), AGSI_GLAGOLITIC_SUPPL_BEGIN, AGSI_GLAGOLITIC_SUPPL_END, NULL },
 { N_("Nyiakeng Puachue Hmong"), AGSI_NYIAKENG_PUACHUE_HMONG_BEGIN, AGSI_NYIAKENG_PUACHUE_HMONG_END, NULL },
 { N_("Toto"), AGSI_TOTO_BEGIN, AGSI_TOTO_END, NULL },
 { N_("Wancho"), AGSI_WANCHO_BEGIN, AGSI_WANCHO_END, NULL },
 { N_("Ethiopic Extended B"), AGSI_ETHIOPIC_EXT_B_BEGIN, AGSI_ETHIOPIC_EXT_B_END, NULL },
 { N_("Mende Kikakui"), AGSI_MENDE_KIKAKUI_BEGIN, AGSI_MENDE_KIKAKUI_END, NULL },
 { N_("Adlam"), AGSI_ADLAM_BEGIN, AGSI_ADLAM_END, NULL },
 { N_("Indic Siyaq Numbers"), AGSI_INDIC_SIYAQ_NUMBERS_BEGIN, AGSI_INDIC_SIYAQ_NUMBERS_END, NULL },
 { N_("Ottoman Siyan Numbers"), AGSI_OTTOMAN_SIYAQ_NUMBERS_BEGIN, AGSI_OTTOMAN_SIYAQ_NUMBERS_END, NULL },
 { N_("Arabic Mathematical Alphabetic Symbols"), AGSI_ARABIC_MATH_ALPHABETIC_SYM_BEGIN, AGSI_ARABIC_MATH_ALPHABETIC_SYM_END, NULL },
 { N_("Mahjong Tiles"), AGSI_MAHJONG_TILES_BEGIN, AGSI_MAHJONG_TILES_END, NULL },
 { N_("Domino Tiles"), AGSI_DOMINO_TILES_BEGIN, AGSI_DOMINO_TILES_END, NULL },
 { N_("Playing Cards"), AGSI_PLAYING_CARDS_BEGIN, AGSI_PLAYING_CARDS_END, NULL },
 { N_("Enclosed Alphanumeric Supplement"), AGSI_ENCL_ALPHANUMERIC_SUPPL_BEGIN, AGSI_ENCL_ALPHANUMERIC_SUPPL_END, NULL },
 { N_("Enclosed Ideographic Supplement"), AGSI_ENCL_IDEOGRAPHIC_SUPPL_BEGIN, AGSI_ENCL_IDEOGRAPHIC_SUPPL_END, NULL },
 { N_("Miscellaneous Symbols and Pictographs"), AGSI_MISC_SYM_AND_PIC_BEGIN, AGSI_MISC_SYM_AND_PIC_END, NULL },
 { N_("Emoticons"), AGSI_EMOTICONS_BEGIN, AGSI_EMOTICONS_END, NULL },
 { N_("Ornamental Dingbats"), AGSI_ORNAMENTAL_DINGBATS_BEGIN, AGSI_ORNAMENTAL_DINGBATS_END, NULL },
 { N_("Transport and Map Symbols"), AGSI_TRANSPORT_AND_MAP_SYM_BEGIN, AGSI_TRANSPORT_AND_MAP_SYM_END, NULL },
 { N_("Alchemical Symbols"), AGSI_ALCHEMICAL_SYM_BEGIN, AGSI_ALCHEMICAL_SYM_END, NULL },
 { N_("Geometric Shapes Extended"), AGSI_GEOMETRIC_SHAPES_EXT_BEGIN, AGSI_GEOMETRIC_SHAPES_EXT_END, NULL },
 { N_("Supplemental Arrows C"), AGSI_SUPPL_ARROWS_C_BEGIN, AGSI_SUPPL_ARROWS_C_END, NULL },
 { N_("Supplemental Symbols and Pictographs"), AGSI_SUPPL_SYM_AND_PIC_BEGIN, AGSI_SUPPL_SYM_AND_PIC_END, NULL },
 { N_("Chess Symbols"), AGSI_CHESS_SYM_BEGIN, AGSI_CHESS_SYM_END, NULL },
 { N_("Symbols and Pictographs Extended A"), AGSI_SYM_AND_PIC_EXT_A_BEGIN, AGSI_SYM_AND_PIC_EXT_A_END, NULL },
 { N_("Symbols for Legacy Computing"), AGSI_SYM_FOR_LEGACY_COMPUTING_BEGIN, AGSI_SYM_FOR_LEGACY_COMPUTING_END, NULL },
 { N_("CJK Unified Ideographs Extension B"), AGSI_CJK_UNI_IDEO_EXT_B_BEGIN, AGSI_CJK_UNI_IDEO_EXT_B_END, NULL },
 { N_("CJK Unified Ideographs Extension C"), AGSI_CJK_UNI_IDEO_EXT_C_BEGIN, AGSI_CJK_UNI_IDEO_EXT_C_END, NULL },
 { N_("CJK Unified Ideographs Extension D"), AGSI_CJK_UNI_IDEO_EXT_D_BEGIN, AGSI_CJK_UNI_IDEO_EXT_D_END, NULL },
 { N_("CJK Unified Ideographs Extension E"), AGSI_CJK_UNI_IDEO_EXT_E_BEGIN, AGSI_CJK_UNI_IDEO_EXT_E_END, NULL },
 { N_("CJK Unified Ideographs Extension F"), AGSI_CJK_UNI_IDEO_EXT_F_BEGIN, AGSI_CJK_UNI_IDEO_EXT_F_END, NULL },
 { N_("CJK Compatibility Ideographs Supplement"), AGSI_CJK_COMPAT_IDEO_SUPPL_BEGIN, AGSI_CJK_COMPAT_IDEO_SUPPL_END, NULL },
 { N_("CJK Unified Ideographs Extension G"), AGSI_CJK_UNI_IDEO_EXT_G_BEGIN, AGSI_CJK_UNI_IDEO_EXT_G_END, NULL },
 { N_("Tags"), AGSI_TAGS_BEGIN, AGSI_TAGS_END, NULL },
 { N_("Variation Selectors Supplement"), AGSI_VARIATION_SELECTORS_SUPPL_BEGIN, AGSI_VARIATION_SELECTORS_SUPPL_END, NULL },
 { N_("Supplemental Private Use Area A"), AGSI_SUPPL_PRIVATE_USE_AREA_A_BEGIN, AGSI_SUPPL_PRIVATE_USE_AREA_A_END, NULL },
 { N_("Supplemental Private Use Area B"), AGSI_SUPPL_PRIVATE_USE_AREA_B_BEGIN, AGSI_SUPPL_PRIVATE_USE_AREA_B_END, NULL },
};
const int agUnicodeRangeCount = sizeof(agUnicodeRanges) / sizeof(agUnicodeRanges[0]);

/*
 * Map an OS/2 Unicode range bit to one or more entries in our table.
 * Based on OpenType 1.9 specification.
 */
const int agUnicodeRangeFromOS2[][8] = {
	{ 0, -1 },              /* 0) Basic Latin */
	{ 1, -1 },              /* 1) Latin-1 Supplement */
	{ 2, -1 },              /* 2) Latin Extended-A */
	{ 3, -1 },              /* 3) Latin Extended-B */
	{ 4, 67, -1 },          /* 4) IPA Extensions + Phonetic Extensions */
	{ 5, 126, -1 },         /* 5) Spacing Modifier Letters + Modifier Tone Letters */
	{ 6, 68, -1 },          /* 6) Combining Diacritical Marks + Supplement */
	{ 7, -1 },              /* 7) Greek and Coptic */
	{ 8, -1 },              /* 8) Coptic */
	{ 9, 101, 124, -1 },    /* 9) Cyrillic + Supplement + Ext A/B */
	{ 10, -1 },             /* 10) Armenian */
	{ 11, -1 },             /* 11) Hebrew */
	{ 123, -1 },            /* 12) Vai */
	{ 12, 14, -1 },         /* 13) Arabic + Supplement */
	{ 16, -1 },             /* 14) Nko */
	{ 22, -1 },             /* 15) Devanagari */
	{ 23, -1 },             /* 16) Bengali */
	{ 24, -1 },             /* 17) Gurmukhi */
	{ 25, -1 },             /* 18) Gujarati */
	{ 26, -1 },             /* 19) Oriya */
	{ 27, -1 },             /* 20) Tamil */
	{ 28, -1 },             /* 21) Telugu */
	{ 29, -1 },             /* 22) Kannada */
	{ 30, -1 },             /* 23) Malayalam */
	{ 32, -1 },             /* 24) Thai */
	{ 33, -1 },             /* 25) Lao */
	{ 36, 98, -1 },         /* 26) Georgian + Supplement */
	{ 58, -1 },             /* 27) Balinese */
	{ 37, -1 },             /* 28) Hangul Jamo */
	{ 69, 96, 127, -1 },    /* 29) Latin Extended Additional + Ext C/D */
	{ 70, -1 },             /* 30) Greek Extended */
	{ 71, 102, -1 },        /* 31) General Punctuation + Supplement */
	{ 72, -1 },             /* 32) Superscripts and Subscripts */
	{ 73, -1 },             /* 33) Currency Symbols */
	{ 74, -1 },             /* 34) Combining Diacritical Marks For Symbols */
	{ 75, -1 },             /* 35) Letterlike Symbols */
	{ 76, -1 },             /* 36) Number Forms */
	{ 77, 89, 91, 94, -1 }, /* 37) Arrows + Supplements A/B + Misc Symbols and Arrows */
	{ 78, 93, 88, 92, -1 }, /* 38) Math Operators + Supplement + Misc Math Symbols A/B */
	{ 79, -1 },             /* 39) Miscellaneous Technical */
	{ 80, -1 },             /* 40) Control Pictures */
	{ 81, -1 },             /* 41) Optical Character Recognition */
	{ 82, -1 },             /* 42) Enclosed Alphanumerics */
	{ 83, -1 },             /* 43) Box Drawing */
	{ 84, -1 },             /* 44) Block Elements */
	{ 85, -1 },             /* 45) Geometric Shapes */
	{ 86, -1 },             /* 46) Miscellaneous Symbols */
	{ 87, -1 },             /* 47) Dingbats */
	{ 106, -1 },            /* 48) CJK Symbols and Punctuation */
	{ 107, -1 },            /* 49) Hiragana */
	{ 108, 114, -1 },       /* 50) Katakana + Phonetic Extensions */
	{ 109, 112, -1 },       /* 51) Bopomofo + Bopomofo Extended */
	{ 110, -1 },            /* 52) Hangul Compat Jamo */
	{ 130, -1 },            /* 53) Phags-pa */
	{ 115, -1 },            /* 54) Enclosed CJK Letters and Months */
	{ 116, -1 },            /* 55) CJK Compatibility */
	{ 146, -1 },            /* 56) Hangul Syllables */
	{ -1 },                 /* 57) Non-Plane 0 */
	{ 190, -1 },            /* 58) Phoenician */
	{ 119, 103, 104,        /* 59) CJK Unified Ideographs + CJK Radicals Supplement + */
	  105, 117, 307,        /*     Kangxi Radicals + Ideographic Description Chars + */
	  111, -1 },            /*     CJK Unified Ideographs Ext A/B + Kanbun */
	{ 150, -1 },            /* 60) Private Use Area (Plane 0) */
	{ 113, 151, 312, -1 },  /* 61) CJK Strokes + CJK Compat Ideographs + Supplement */
	{ 152, -1 },            /* 62) Alphabetic Presentation Forms */
	{ 153, -1 },            /* 63) Arabic Presentation Forms A */
	{ 156, -1 },            /* 64) Combining Half Marks */
	{ 155, 157, -1 },       /* 65) Vertical Forms + CJK Compat Forms */
	{ 158, -1 },            /* 66) Small Form Variants */
	{ 159, -1 },            /* 67) Arabic Presentation Forms B */
	{ 160, -1 },            /* 68) Halfwidth and Fullwidth Forms */
	{ 161, -1 },            /* 69) Specials */
	{ 34, -1 },             /* 70) Tibetan */
	{ 13, -1 },             /* 71) Syriac */
	{ 15, -1 },             /* 72) Thaana */
	{ 31, -1 },             /* 73) Sinhala */
	{ 35, -1 },             /* 74) Myanmar */
	{ 38, 39, 100, -1 },    /* 75) Ethiopic + Supplement + Extended */
	{ 40, -1 },             /* 76) Cherokee */
	{ 41, -1 },             /* 77) Unified Canadian Aboriginal Syllabics */
	{ 42, -1 },             /* 78) Ogham */
	{ 43, -1 },             /* 79) Runic */
	{ 48, 54, -1 },         /* 80) Khmer + Symbols */
	{ 49, -1 },             /* 81) Mongolian */
	{ 90, -1 },             /* 82) Braille Patterns */
	{ 120, 121, -1 },       /* 83) Yi Syllables + Radicals */
	{ 44, 45, 46, 47, -1 }, /* 84) Tagalog + Hanunoo + Buhid + Tagbanwa */
	{ 171, -1 },            /* 85) Old Italic */
	{ 172, -1 },            /* 86) Gothic */
	{ 176, -1 },            /* 87) Deseret */
	{ 272, 273, 274, -1 },  /* 88) Byzantine Musical + Musical Symbols + Ancient Greek Musical Notation */
	{ 278, -1 },            /* 89) Mathematical Alphanumeric Symbols */
	{ 316, 317, -1 },       /* 90) Supplemental Private Use Area A/B */
	{ 154, 315, -1 },       /* 91) Variation Selectors + Supplement */
	{ 314, -1 },            /* 92) Tags */
	{ 51, -1 },             /* 93) Limbu */
	{ 52, -1 },             /* 94) Tai Le */
	{ 53, -1 },             /* 95) New Tai Lue */
	{ 55, -1 },             /* 96) Buginese */
	{ 95, -1 },             /* 97) Glagolitic */
	{ 99, -1 },             /* 98) Tifinagh */
	{ 118, -1 },            /* 99) Yijing Hexagram Symbols */
	{ 128, -1 },            /* 100) Syloti Nagri */
	{ 162, 163, 164, -1 },  /* 101) Linear B Syllabary + Ideograms + Aegean Numbers */
	{ 165, -1 },            /* 102) Ancient Greek Numbers */
	{ 174, -1 },            /* 103) Ugaritic */
	{ 175, -1 },            /* 104) Old Persian */
	{ 177, -1 },            /* 105) Shavian */
	{ 178, -1 },            /* 106) Osmanya */
	{ 185, -1 },            /* 107) Cypriot Syllabary */
	{ 194, -1 },            /* 108) Kharoshthi */
	{ 276, -1 },            /* 109) Tai Xuan Jing Symbols */
	{ 245, 246, -1 },       /* 110) Cuneiform + Numbers and Punctuation */
	{ 277, -1 },            /* 111) Counting Rod Numerals */
	{ 59, -1 },             /* 112) Sundanese */
	{ 61, -1 },             /* 113) Lepcha */
	{ 62, -1 },             /* 114) Ol Chiki */
	{ 131, -1 },            /* 115) Saurashtra */
	{ 133, -1 },            /* 116) Kayah Li */
	{ 134, -1 },            /* 117) Rejang */
	{ 138, -1 },            /* 118) Cham */
	{ 166, -1 },            /* 119) Ancient Symbols */
	{ 167, -1 },            /* 120) Phaistos Disc */
	{ 169, 168, 191, -1 },  /* 121) Carian + Lycian + Lydian */
	{ 292, 291, -1 }        /* 122) Domino Tiles + Mahjong Tiles */
};

