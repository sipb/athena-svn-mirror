/* command-vertex-data.c 
* 
* Copyright 2002 Sun Microsystems, Inc.,
* Copyright 2002 University Of Toronto 
* 
* This library is free software; you can redistribute it and/or 
* modify it under the terms of the GNU Library General Public 
* License as published by the Free Software Foundation; either 
* version 2 of the License, or (at your option) any later version. 
* 
* This library is distributed in the hope that it will be useful, 
* but WITHOUT ANY WARRANTY; without even the implied warranty of 
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
* Library General Public License for more details. 
* 
* You should have received a copy of the GNU Library General Public 
* License along with this library; if not, write to the 
* Free Software Foundation, Inc., 59 Temple Place - Suite 330, 
* Boston, MA 02111-1307, USA. 
*/ 

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <strings.h>
#include <string.h>
#include "command-vertex-data.h"
#include "gok-log.h"

/*
  constants
*/
#define COMMAND_INFO_ITEMS 2
#define KEYBOARD_ID 0
#define KEY_ID 1

static const size_t char_size = sizeof(char);
static const size_t short_size = sizeof(short);
static const size_t float_size = sizeof(float);


/*
  structures
*/
struct Command_Vertex_Data_Struct {
  char *_keyboard_id;
  char *_key_id;
  float _weight;
};


/*
  private function declarations
*/
static void set_key_id(const Command_Vertex_Data data, const char *key_id);
static void set_keyboard_id(const Command_Vertex_Data data, const char *keyboard_id);
static void extract_command(char *line, char **command_info);
static char *find_next_space(const char *string);


const Boolean command_vertex_data_equals(const Object data_1, const Object data_2)
{
  return (string_equals(command_vertex_data_get_keyboard_id((Command_Vertex_Data)data_1),
			command_vertex_data_get_keyboard_id((Command_Vertex_Data)data_2)) &&
	  string_equals(command_vertex_data_get_key_id((Command_Vertex_Data)data_1),
			command_vertex_data_get_key_id((Command_Vertex_Data)data_2)));
}

void command_vertex_data_free(const Command_Vertex_Data data)
{
  free(data->_keyboard_id);
  free(data->_key_id);
  free(data);
}

Command_Vertex_Data command_vertex_data_create(const char *keyboard_id, const char *key_id, const float weight)
{
  Command_Vertex_Data data;

  if ((keyboard_id == NULL) || (key_id == NULL) || string_empty(keyboard_id) || string_empty(key_id))
    {
      return NULL;
    }
#ifdef GOK_DEBUG
  data = malloc(sizeof(struct Command_Vertex_Data_Struct));
#else
  data = checked_malloc(sizeof(struct Command_Vertex_Data_Struct));
#endif
  set_keyboard_id(data, keyboard_id);
  set_key_id(data, key_id);
  command_vertex_data_set_weight(data, weight);

  return data;
}

Command_Vertex_Data command_vertex_data_create_from_log(char *line)
{
  char *command_info[COMMAND_INFO_ITEMS];
  Command_Vertex_Data vertex_data;

  extract_command(line, command_info);
  vertex_data = command_vertex_data_create(command_info[KEYBOARD_ID],
					   command_info[KEY_ID],
					   COMMAND_VERTEX_DATA_DEFAULT_WEIGHT);
  free(line);
  return vertex_data;
}

static void set_key_id(const Command_Vertex_Data data, const char *key_id)
{
  int key_id_length;

  key_id_length = strlen(key_id) + 1;
#ifdef GOK_DEBUG
  data->_key_id = malloc(char_size * key_id_length);
#else
  data->_key_id = checked_malloc(char_size * key_id_length);
#endif
  memcpy(data->_key_id, key_id, key_id_length);
}

static void set_keyboard_id(const Command_Vertex_Data data, const char *keyboard_id)
{
  int keyboard_id_length;

  keyboard_id_length = strlen(keyboard_id) + 1;
#ifdef GOK_DEBUG
  data->_keyboard_id = malloc(char_size * keyboard_id_length);
#else
  data->_keyboard_id = checked_malloc(char_size * keyboard_id_length);
#endif
  memcpy(data->_keyboard_id, keyboard_id, keyboard_id_length);
}

const char *command_vertex_data_get_keyboard_id(const Command_Vertex_Data data)
{
  return data->_keyboard_id;
}

const char *command_vertex_data_get_key_id(const Command_Vertex_Data data)
{
  return data->_key_id;
}

void command_vertex_data_set_weight(const Command_Vertex_Data data, const float weight)
{
  data->_weight = weight;
}

float command_vertex_data_get_weight(const Command_Vertex_Data data)
{
  return data->_weight;
}

static void extract_command(char *line, char **command_info)
{
  char *end_of_token;

  command_info[KEYBOARD_ID] = line;
  end_of_token = find_next_space(line);
  *end_of_token = '\0';
  command_info[KEY_ID] = end_of_token + 1;
/*   end_of_token++; */
/*   command_info[KEY_ID] = end_of_token; */
/*   end_of_token = find_next_space(end_of_token); */
/*   *end_of_token = '\0'; */
/*   command_info[TIMESTAMP] = end_of_token + 1; */
}

static char *find_next_space(const char *string)
{
  return index(string, ' ');
}

void command_vertex_data_dump_text(const Command_Vertex_Data data, FILE *stream)
{
  fprintf(stream, "%s %s Vertex Weight: %f",
	  command_vertex_data_get_keyboard_id(data),
	  command_vertex_data_get_key_id(data),
	  command_vertex_data_get_weight(data));
}

void command_vertex_data_dump_binary(const Command_Vertex_Data data, FILE *stream)
{
  const char *keyboard_id = command_vertex_data_get_keyboard_id(data);
  const short keyboard_id_length = (short)strlen(keyboard_id);
  const char *key_id = command_vertex_data_get_key_id(data);
  const short key_id_length = (short)strlen(key_id);
  const float weight = command_vertex_data_get_weight(data);

  fwrite(&keyboard_id_length, short_size, 1, stream);
  fwrite(&key_id_length, short_size, 1, stream);
  fwrite(keyboard_id, char_size, keyboard_id_length, stream);
  fwrite(key_id, char_size, key_id_length, stream);
  fwrite(&weight, float_size, 1, stream);
}
