/* gok-log-reader.c
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

#include <sys/types.h>
#include "gok-log-reader.h"

/*
  private variables
*/
static int char_size = sizeof(char);


/*
  structures
*/
struct Gok_Log_Reader_Struct {
  off_t _file_size;
  char *_buffer;
  int _current_char;
};


/*
  private function declarations
*/
static void read_file_into_buffer(Gok_Log_Reader reader, FILE *file);
static char *separate_command(Gok_Log_Reader reader);
static Boolean line_break(Gok_Log_Reader reader);
static Boolean start_of_comment(Gok_Log_Reader reader);
static Boolean check_buffer(Gok_Log_Reader reader, const char character);
static Boolean end_of_buffer(Gok_Log_Reader reader);
static void skip_comment(Gok_Log_Reader reader);


void gok_log_reader_free(Gok_Log_Reader reader)
{
  free(reader->_buffer);
  free(reader);
}

Gok_Log_Reader gok_log_reader_create(const char *filename)
{
  Gok_Log_Reader reader;
  FILE *file;
  struct stat inode_info_buffer;

  file = fopen(filename, "r");
  if (NULL == file)
    {
      return NULL;
    }
#ifdef GOK_DEBUG
  reader = malloc(sizeof(struct Gok_Log_Reader_Struct));
#else
  reader = checked_malloc(sizeof(struct Gok_Log_Reader_Struct));
#endif
  stat(filename, &inode_info_buffer);
  reader->_file_size = inode_info_buffer.st_size;
  read_file_into_buffer(reader, file);
  fclose(file);
  reader->_current_char = 0;

  return reader;
}

static void read_file_into_buffer(Gok_Log_Reader reader, FILE *file)
{
  int file_size = reader->_file_size;
  int chars_read;

#ifdef GOK_DEBUG
  reader->_buffer = malloc(char_size * file_size);
#else
  reader->_buffer = checked_malloc(char_size * file_size);
#endif
  chars_read = fread(reader->_buffer, char_size, file_size, file);
  if (chars_read != file_size)
    {
      fclose(file);
      gok_log_reader_free(reader);
      fprintf(stderr, "\nUnable to read the log properly.  The file is corrupt.");
      fflush(stderr);
      exit(EXIT_FAILURE);
    }
}

char *gok_log_reader_get_next_command(Gok_Log_Reader reader)
{
  while (!end_of_buffer(reader) && start_of_comment(reader))
    {
      skip_comment(reader);
    }
  if (end_of_buffer(reader))
    {
      return NULL;
    }
  return separate_command(reader);
}

static char *separate_command(Gok_Log_Reader reader)
{
  int start_pos;
  int command_size;
  char *command;

  start_pos = reader->_current_char;
  do {
    reader->_current_char++;
  } while (!end_of_buffer(reader) && !start_of_comment(reader) && !line_break(reader));
  command_size = reader->_current_char - start_pos;
#ifdef GOK_DEBUG
  command = malloc(char_size * command_size);
#else
  command = checked_malloc(char_size * command_size);
#endif
  memcpy(command, &(reader->_buffer[start_pos]), command_size);
  command[command_size] = '\0';
  reader->_current_char++;

  return command;
}

static Boolean line_break(Gok_Log_Reader reader)
{
  return check_buffer(reader, '\n');
}

static Boolean start_of_comment(Gok_Log_Reader reader)
{
  return check_buffer(reader, '#');
}

static Boolean check_buffer(Gok_Log_Reader reader, const char character)
{
  return (reader->_buffer[reader->_current_char] == character);
}

static Boolean end_of_buffer(Gok_Log_Reader reader)
{
  return (reader->_current_char >= reader->_file_size);
}

static void skip_comment(Gok_Log_Reader reader)
{
  do {
    reader->_current_char++;
  } while (!line_break(reader) && !end_of_buffer(reader));
  reader->_current_char++;
}

/* Command_Vertex gok_log_reader_create_command_vertex(char *line) */
/* { */
/*   if (NULL == line) */
/*     { */
/*       return NULL; */
/*     } */
/*   return command_vertex_create(command_vertex_data_create_from_log(line)); */
/* } */
