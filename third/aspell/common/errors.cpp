/* Automatically generated file.  Do not edit directly. */

/* This file is part of The New Aspell
 * Copyright (C) 2001-2002 by Kevin Atkinson under the GNU LGPL
 * license version 2.0 or 2.1.  You should have received a copy of the
 * LGPL license along with this library if you did not you can find it
 * at http://www.gnu.org/.                                              */

#include "error.hpp"
#include "errors.hpp"

namespace acommon {


static const ErrorInfo aerror_other_obj = {
  0, // isa
  0, // mesg
  0, // num_parms
  {} // parms
};
extern "C" const ErrorInfo * const aerror_other = &aerror_other_obj;

static const ErrorInfo aerror_operation_not_supported_obj = {
  0, // isa
  0, // mesg
  0, // num_parms
  {} // parms
};
extern "C" const ErrorInfo * const aerror_operation_not_supported = &aerror_operation_not_supported_obj;

static const ErrorInfo aerror_cant_copy_obj = {
  aerror_operation_not_supported, // isa
  0, // mesg
  0, // num_parms
  {} // parms
};
extern "C" const ErrorInfo * const aerror_cant_copy = &aerror_cant_copy_obj;

static const ErrorInfo aerror_file_obj = {
  0, // isa
  "%file:1:", // mesg
  1, // num_parms
  {"file"} // parms
};
extern "C" const ErrorInfo * const aerror_file = &aerror_file_obj;

static const ErrorInfo aerror_cant_open_file_obj = {
  aerror_file, // isa
  "The file \"%file:1\" can not be opened", // mesg
  1, // num_parms
  {"file"} // parms
};
extern "C" const ErrorInfo * const aerror_cant_open_file = &aerror_cant_open_file_obj;

static const ErrorInfo aerror_cant_read_file_obj = {
  aerror_cant_open_file, // isa
  "The file \"%file:1\" can not be opened for reading.", // mesg
  1, // num_parms
  {"file"} // parms
};
extern "C" const ErrorInfo * const aerror_cant_read_file = &aerror_cant_read_file_obj;

static const ErrorInfo aerror_cant_write_file_obj = {
  aerror_cant_open_file, // isa
  "The file \"%file:1\" can not be opened for writing.", // mesg
  1, // num_parms
  {"file"} // parms
};
extern "C" const ErrorInfo * const aerror_cant_write_file = &aerror_cant_write_file_obj;

static const ErrorInfo aerror_invalid_name_obj = {
  aerror_file, // isa
  "The file name \"%file:1\" is invalid.", // mesg
  1, // num_parms
  {"file"} // parms
};
extern "C" const ErrorInfo * const aerror_invalid_name = &aerror_invalid_name_obj;

static const ErrorInfo aerror_bad_file_format_obj = {
  aerror_file, // isa
  "The file \"%file:1\" is not in the proper format.", // mesg
  1, // num_parms
  {"file"} // parms
};
extern "C" const ErrorInfo * const aerror_bad_file_format = &aerror_bad_file_format_obj;

static const ErrorInfo aerror_dir_obj = {
  0, // isa
  0, // mesg
  1, // num_parms
  {"dir"} // parms
};
extern "C" const ErrorInfo * const aerror_dir = &aerror_dir_obj;

static const ErrorInfo aerror_cant_read_dir_obj = {
  aerror_dir, // isa
  "The directory \"%dir:1\" can not be opened for reading.", // mesg
  1, // num_parms
  {"dir"} // parms
};
extern "C" const ErrorInfo * const aerror_cant_read_dir = &aerror_cant_read_dir_obj;

static const ErrorInfo aerror_config_obj = {
  0, // isa
  0, // mesg
  1, // num_parms
  {"key"} // parms
};
extern "C" const ErrorInfo * const aerror_config = &aerror_config_obj;

static const ErrorInfo aerror_unknown_key_obj = {
  aerror_config, // isa
  "The key \"%key:1\" is unknown.", // mesg
  1, // num_parms
  {"key"} // parms
};
extern "C" const ErrorInfo * const aerror_unknown_key = &aerror_unknown_key_obj;

static const ErrorInfo aerror_cant_change_value_obj = {
  aerror_config, // isa
  "The value for option \"%key:1\" can not be changed.", // mesg
  1, // num_parms
  {"key"} // parms
};
extern "C" const ErrorInfo * const aerror_cant_change_value = &aerror_cant_change_value_obj;

static const ErrorInfo aerror_bad_key_obj = {
  aerror_config, // isa
  "The key \"%key:1\" is not %accepted:2 and is thus invalid.", // mesg
  2, // num_parms
  {"key", "accepted"} // parms
};
extern "C" const ErrorInfo * const aerror_bad_key = &aerror_bad_key_obj;

static const ErrorInfo aerror_bad_value_obj = {
  aerror_config, // isa
  "The value \"%value:2\" is not %accepted:3 and is thus invalid for the key \"%key:1\".", // mesg
  3, // num_parms
  {"key", "value", "accepted"} // parms
};
extern "C" const ErrorInfo * const aerror_bad_value = &aerror_bad_value_obj;

static const ErrorInfo aerror_duplicate_obj = {
  aerror_config, // isa
  0, // mesg
  1, // num_parms
  {"key"} // parms
};
extern "C" const ErrorInfo * const aerror_duplicate = &aerror_duplicate_obj;

static const ErrorInfo aerror_language_related_obj = {
  0, // isa
  0, // mesg
  1, // num_parms
  {"lang"} // parms
};
extern "C" const ErrorInfo * const aerror_language_related = &aerror_language_related_obj;

static const ErrorInfo aerror_unknown_language_obj = {
  aerror_language_related, // isa
  "The language \"%lang:1\" is not known.", // mesg
  1, // num_parms
  {"lang"} // parms
};
extern "C" const ErrorInfo * const aerror_unknown_language = &aerror_unknown_language_obj;

static const ErrorInfo aerror_unknown_soundslike_obj = {
  aerror_language_related, // isa
  "The soundslike \"%sl:2\" is not known.", // mesg
  2, // num_parms
  {"lang", "sl"} // parms
};
extern "C" const ErrorInfo * const aerror_unknown_soundslike = &aerror_unknown_soundslike_obj;

static const ErrorInfo aerror_language_not_supported_obj = {
  aerror_language_related, // isa
  "The language \"%lang:1\" is not supported.", // mesg
  1, // num_parms
  {"lang"} // parms
};
extern "C" const ErrorInfo * const aerror_language_not_supported = &aerror_language_not_supported_obj;

static const ErrorInfo aerror_no_wordlist_for_lang_obj = {
  aerror_language_related, // isa
  "No word lists can be found for the language \"%lang:1\".", // mesg
  1, // num_parms
  {"lang"} // parms
};
extern "C" const ErrorInfo * const aerror_no_wordlist_for_lang = &aerror_no_wordlist_for_lang_obj;

static const ErrorInfo aerror_mismatched_language_obj = {
  aerror_language_related, // isa
  "Expected language \"%lang:1\" but got \"%prev:2\".", // mesg
  2, // num_parms
  {"lang", "prev"} // parms
};
extern "C" const ErrorInfo * const aerror_mismatched_language = &aerror_mismatched_language_obj;

static const ErrorInfo aerror_encoding_obj = {
  0, // isa
  0, // mesg
  1, // num_parms
  {"encod"} // parms
};
extern "C" const ErrorInfo * const aerror_encoding = &aerror_encoding_obj;

static const ErrorInfo aerror_unknown_encoding_obj = {
  aerror_encoding, // isa
  "The encoding \"%encod:1\" is not known.", // mesg
  1, // num_parms
  {"encod"} // parms
};
extern "C" const ErrorInfo * const aerror_unknown_encoding = &aerror_unknown_encoding_obj;

static const ErrorInfo aerror_encoding_not_supported_obj = {
  aerror_encoding, // isa
  "The encoding \"%encod:1\" is not supported.", // mesg
  1, // num_parms
  {"encod"} // parms
};
extern "C" const ErrorInfo * const aerror_encoding_not_supported = &aerror_encoding_not_supported_obj;

static const ErrorInfo aerror_conversion_not_supported_obj = {
  aerror_encoding, // isa
  "The conversion from \"%encod:1\" to \"%encod2:2\" is not supported.", // mesg
  2, // num_parms
  {"encod", "encod2"} // parms
};
extern "C" const ErrorInfo * const aerror_conversion_not_supported = &aerror_conversion_not_supported_obj;

static const ErrorInfo aerror_pipe_obj = {
  0, // isa
  0, // mesg
  0, // num_parms
  {} // parms
};
extern "C" const ErrorInfo * const aerror_pipe = &aerror_pipe_obj;

static const ErrorInfo aerror_cant_create_pipe_obj = {
  aerror_pipe, // isa
  0, // mesg
  0, // num_parms
  {} // parms
};
extern "C" const ErrorInfo * const aerror_cant_create_pipe = &aerror_cant_create_pipe_obj;

static const ErrorInfo aerror_process_died_obj = {
  aerror_pipe, // isa
  0, // mesg
  0, // num_parms
  {} // parms
};
extern "C" const ErrorInfo * const aerror_process_died = &aerror_process_died_obj;

static const ErrorInfo aerror_bad_input_obj = {
  0, // isa
  0, // mesg
  0, // num_parms
  {} // parms
};
extern "C" const ErrorInfo * const aerror_bad_input = &aerror_bad_input_obj;

static const ErrorInfo aerror_invalid_word_obj = {
  aerror_bad_input, // isa
  "The word \"%word:1\" is invalid.", // mesg
  1, // num_parms
  {"word"} // parms
};
extern "C" const ErrorInfo * const aerror_invalid_word = &aerror_invalid_word_obj;

static const ErrorInfo aerror_word_list_flags_obj = {
  aerror_bad_input, // isa
  0, // mesg
  0, // num_parms
  {} // parms
};
extern "C" const ErrorInfo * const aerror_word_list_flags = &aerror_word_list_flags_obj;

static const ErrorInfo aerror_invalid_flag_obj = {
  aerror_word_list_flags, // isa
  0, // mesg
  0, // num_parms
  {} // parms
};
extern "C" const ErrorInfo * const aerror_invalid_flag = &aerror_invalid_flag_obj;

static const ErrorInfo aerror_conflicting_flags_obj = {
  aerror_word_list_flags, // isa
  0, // mesg
  0, // num_parms
  {} // parms
};
extern "C" const ErrorInfo * const aerror_conflicting_flags = &aerror_conflicting_flags_obj;



}

