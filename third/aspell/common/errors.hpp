/* Automatically generated file.  Do not edit directly. */

/* This file is part of The New Aspell
 * Copyright (C) 2001-2002 by Kevin Atkinson under the GNU LGPL
 * license version 2.0 or 2.1.  You should have received a copy of the
 * LGPL license along with this library if you did not you can find it
 * at http://www.gnu.org/.                                              */

#ifndef ASPELL_ERRORS__HPP
#define ASPELL_ERRORS__HPP


namespace acommon {

struct ErrorInfo;

extern "C" const ErrorInfo * const aerror_other;
extern "C" const ErrorInfo * const aerror_operation_not_supported;
extern "C" const ErrorInfo * const   aerror_cant_copy;
extern "C" const ErrorInfo * const aerror_file;
extern "C" const ErrorInfo * const   aerror_cant_open_file;
extern "C" const ErrorInfo * const     aerror_cant_read_file;
extern "C" const ErrorInfo * const     aerror_cant_write_file;
extern "C" const ErrorInfo * const   aerror_invalid_name;
extern "C" const ErrorInfo * const   aerror_bad_file_format;
extern "C" const ErrorInfo * const aerror_dir;
extern "C" const ErrorInfo * const   aerror_cant_read_dir;
extern "C" const ErrorInfo * const aerror_config;
extern "C" const ErrorInfo * const   aerror_unknown_key;
extern "C" const ErrorInfo * const   aerror_cant_change_value;
extern "C" const ErrorInfo * const   aerror_bad_key;
extern "C" const ErrorInfo * const   aerror_bad_value;
extern "C" const ErrorInfo * const   aerror_duplicate;
extern "C" const ErrorInfo * const aerror_language_related;
extern "C" const ErrorInfo * const   aerror_unknown_language;
extern "C" const ErrorInfo * const   aerror_unknown_soundslike;
extern "C" const ErrorInfo * const   aerror_language_not_supported;
extern "C" const ErrorInfo * const   aerror_no_wordlist_for_lang;
extern "C" const ErrorInfo * const   aerror_mismatched_language;
extern "C" const ErrorInfo * const aerror_encoding;
extern "C" const ErrorInfo * const   aerror_unknown_encoding;
extern "C" const ErrorInfo * const   aerror_encoding_not_supported;
extern "C" const ErrorInfo * const   aerror_conversion_not_supported;
extern "C" const ErrorInfo * const aerror_pipe;
extern "C" const ErrorInfo * const   aerror_cant_create_pipe;
extern "C" const ErrorInfo * const   aerror_process_died;
extern "C" const ErrorInfo * const aerror_bad_input;
extern "C" const ErrorInfo * const   aerror_invalid_word;
extern "C" const ErrorInfo * const   aerror_word_list_flags;
extern "C" const ErrorInfo * const     aerror_invalid_flag;
extern "C" const ErrorInfo * const     aerror_conflicting_flags;


static const ErrorInfo * const other_error = aerror_other;
static const ErrorInfo * const operation_not_supported_error = aerror_operation_not_supported;
static const ErrorInfo * const   cant_copy = aerror_cant_copy;
static const ErrorInfo * const file_error = aerror_file;
static const ErrorInfo * const   cant_open_file_error = aerror_cant_open_file;
static const ErrorInfo * const     cant_read_file = aerror_cant_read_file;
static const ErrorInfo * const     cant_write_file = aerror_cant_write_file;
static const ErrorInfo * const   invalid_name = aerror_invalid_name;
static const ErrorInfo * const   bad_file_format = aerror_bad_file_format;
static const ErrorInfo * const dir_error = aerror_dir;
static const ErrorInfo * const   cant_read_dir = aerror_cant_read_dir;
static const ErrorInfo * const config_error = aerror_config;
static const ErrorInfo * const   unknown_key = aerror_unknown_key;
static const ErrorInfo * const   cant_change_value = aerror_cant_change_value;
static const ErrorInfo * const   bad_key = aerror_bad_key;
static const ErrorInfo * const   bad_value = aerror_bad_value;
static const ErrorInfo * const   duplicate = aerror_duplicate;
static const ErrorInfo * const language_related_error = aerror_language_related;
static const ErrorInfo * const   unknown_language = aerror_unknown_language;
static const ErrorInfo * const   unknown_soundslike = aerror_unknown_soundslike;
static const ErrorInfo * const   language_not_supported = aerror_language_not_supported;
static const ErrorInfo * const   no_wordlist_for_lang = aerror_no_wordlist_for_lang;
static const ErrorInfo * const   mismatched_language = aerror_mismatched_language;
static const ErrorInfo * const encoding_error = aerror_encoding;
static const ErrorInfo * const   unknown_encoding = aerror_unknown_encoding;
static const ErrorInfo * const   encoding_not_supported = aerror_encoding_not_supported;
static const ErrorInfo * const   conversion_not_supported = aerror_conversion_not_supported;
static const ErrorInfo * const pipe_error = aerror_pipe;
static const ErrorInfo * const   cant_create_pipe = aerror_cant_create_pipe;
static const ErrorInfo * const   process_died = aerror_process_died;
static const ErrorInfo * const bad_input_error = aerror_bad_input;
static const ErrorInfo * const   invalid_word = aerror_invalid_word;
static const ErrorInfo * const   word_list_flags_error = aerror_word_list_flags;
static const ErrorInfo * const     invalid_flag = aerror_invalid_flag;
static const ErrorInfo * const     conflicting_flags = aerror_conflicting_flags;


}

#endif /* ASPELL_ERRORS__HPP */
