/* command-graph.c 
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

#include "command-graph.h"

/*
  constants
*/
#define PREDICTIONS_FILENAME ".gok-command-prediction"
#define COMMAND_INFO_ITEMS 2
#define KEYBOARD_ID 0
#define KEY_ID 1

static const size_t char_size = sizeof(char);
static const size_t short_size = sizeof(short);
static const size_t float_size = sizeof(float);

static const gchar* m_full_file_name;

/*
  structures
*/
struct Command_Graph_Struct {
  Arraylist _vertices;
  int _total_commands;
  Command_Vertex _last_n_added_vertices[MAX_NUM_PREDICTIONS];
  Command_Vertex _last_added_vertex;
};

/*
  private function declarations
*/
static void remove_inbound_edges(const Arraylist vertices_list, const Command_Vertex the_vertex);
static const int command_vertex_weight_sort(const Object vertex_1, const Object vertex_2);
static const int command_vertex_keyboard_sort(const Object vertex_1, const Object vertex_2);
static void remove_vertices(const Command_Graph graph);
static void shift_command_vertex_history(const Command_Graph graph, const Command_Vertex vertex);
static void create_vertices(const Command_Graph graph, char *buffer_ptr, const char *end_of_buffer);
static void create_edges(const Command_Graph graph,
			 char *buffer_ptr,
			 const char *end_of_buffer,
			 const Command_Vertex vertex_for_edges);
static float extract_weight(char **buffer_ptr);
static void extract_command(char **buffer_ptr, char **command_info);
static char *read_file_into_buffer(FILE *file, const int file_size);


void command_graph_free(const Command_Graph graph)
{
  remove_vertices(graph);
  arraylist_free(graph->_vertices);
  free(graph);
}

static void remove_vertices(const Command_Graph graph)
{
  Arraylist vertices_list = command_graph_get_vertices(graph);
  int length = arraylist_size(vertices_list);
  int i;

  for (i = 0; i < length; i++)
    {
      command_vertex_free(arraylist_get(vertices_list, i));
    }
}

Command_Graph command_graph_create()
{
  Command_Graph graph;
  int i;

#ifdef GOK_DEBUG
  graph = malloc(sizeof(struct Command_Graph_Struct));
#else
  graph = checked_malloc(sizeof(struct Command_Graph_Struct));
#endif
  graph->_vertices = arraylist_create(command_vertex_equals);
  graph->_total_commands = 0;
  for (i = 0; i < MAX_NUM_PREDICTIONS; i++)
    {
      graph->_last_n_added_vertices[i] = NULL;
    }
  graph->_last_added_vertex = NULL;

  return graph;
}

void command_graph_add_vertex(const Command_Graph graph, const Command_Vertex vertex)
{
  Arraylist vertices_list = command_graph_get_vertices(graph);
  Command_Vertex actual_vertex;

  graph->_total_commands++;

  if (!arraylist_contains(vertices_list, vertex))
    {
      arraylist_add(vertices_list, vertex);
      actual_vertex = vertex;
    }
  else
    {
      Command_Vertex_Data actual_vertex_data;
      float vertex_weight;
      float actual_vertex_weight;

      vertex_weight = command_vertex_data_get_weight(command_vertex_get_vertex_data(vertex));
      actual_vertex = arraylist_get(vertices_list, arraylist_index_of(vertices_list, vertex));
      actual_vertex_data = command_vertex_get_vertex_data(actual_vertex);
      actual_vertex_weight = command_vertex_data_get_weight(actual_vertex_data);
      command_vertex_data_set_weight(actual_vertex_data, actual_vertex_weight + vertex_weight);
      command_vertex_free(vertex);
    }

  if (NULL != graph->_last_added_vertex)
    {
      command_vertex_add_edge(graph->_last_added_vertex,
			      command_edge_create(actual_vertex,
						  command_edge_data_create(COMMAND_EDGE_DATA_DEFAULT_WEIGHT)));
    }

  graph->_last_added_vertex = actual_vertex;
  shift_command_vertex_history(graph, actual_vertex);
}

static void shift_command_vertex_history(const Command_Graph graph, const Command_Vertex vertex)
{
  memmove(graph->_last_n_added_vertices + 1, graph->_last_n_added_vertices, sizeof(Command_Vertex) * (MAX_NUM_PREDICTIONS - 1));
  graph->_last_n_added_vertices[0] = vertex;
}

const Command_Vertex *command_graph_get_last_n_added_vertices(const Command_Graph graph)
{
  return graph->_last_n_added_vertices;
}

void command_graph_remove_vertex(const Command_Graph graph, const Command_Vertex the_vertex)
{
  Arraylist vertices_list = command_graph_get_vertices(graph);

  if (arraylist_contains(vertices_list, the_vertex))
    {
      remove_inbound_edges(vertices_list, the_vertex);
      arraylist_remove(vertices_list, the_vertex);
    }
  command_vertex_free(the_vertex);
}

static void remove_inbound_edges(const Arraylist vertices_list, const Command_Vertex the_vertex)
{
  int vertices_list_length = arraylist_size(vertices_list);
  Command_Edge_Data temp_vertex_edge_data = command_edge_data_create(COMMAND_EDGE_DATA_DEFAULT_WEIGHT);
  Command_Edge temp_vertex_edge = command_edge_create(the_vertex, temp_vertex_edge_data);
  int i;

  for (i = 0; i < vertices_list_length; i++)
    {
      Command_Vertex a_vertex = (Command_Vertex)arraylist_get(vertices_list, i);
      Command_Edge inbound_edge = command_vertex_get_edge(a_vertex, temp_vertex_edge);
      if (NULL != inbound_edge)
	{
	  /* NOTE:
	     This is another algorithm where all the outbound edges of the vertex being
	     removed is reassigned to the vertex that points to this soon to be deleted
	     vertex.  In essence, A->B->C then A->C when B is removed.
	  */
/* 	      for (j = 0; j < outbound_edges_length; j++) */
/* 		{ */
/* 		  outbound_edge = (Command_Edge)arraylist_get(outbound_edges, j); */
/* 		  command_vertex_add_edge(a_vertex, outbound_edge); */
/* 		} */
	  command_vertex_remove_edge(a_vertex, inbound_edge);
	}
    }

  command_edge_free(temp_vertex_edge);
  command_edge_data_free(temp_vertex_edge_data);
}

Command_Vertex command_graph_get_vertex(const Command_Graph graph, const Command_Vertex vertex)
{
  Arraylist vertices_list = command_graph_get_vertices(graph);
  int index = arraylist_index_of(vertices_list, vertex);

  if (index > -1)
    {
      return (Command_Vertex)arraylist_get(vertices_list, index);
    }
  return NULL;
}

int command_graph_size(const Command_Graph graph)
{
  return arraylist_size(command_graph_get_vertices(graph));
}

int command_graph_total_commands(const Command_Graph graph)
{
  return graph->_total_commands;
}

Arraylist command_graph_get_vertices(const Command_Graph graph)
{
  return graph->_vertices;
}

Command_Vertex command_graph_get_last_added_vertex(const Command_Graph graph)
{
  return graph->_last_added_vertex;
}

void command_graph_sort_by_stat(const Command_Graph graph)
{
  arraylist_sort(command_graph_get_vertices(graph), command_vertex_weight_sort);
}

static const int command_vertex_weight_sort(const Object vertex_1, const Object vertex_2)
{
  float vertex_weight_1;
  float vertex_weight_2;

  vertex_weight_1 = command_vertex_data_get_weight(command_vertex_get_vertex_data((Object)*(int*)vertex_1));
  vertex_weight_2 = command_vertex_data_get_weight(command_vertex_get_vertex_data((Object)*(int*)vertex_2));

  return (int)(vertex_weight_2 - vertex_weight_1);
}

void command_graph_sort_by_keyboard(const Command_Graph graph)
{
  arraylist_sort(command_graph_get_vertices(graph), command_vertex_keyboard_sort);
}

static const int command_vertex_keyboard_sort(const Object vertex_1, const Object vertex_2)
{
  const char *keyboard_id_1, *keyboard_id_2;

  keyboard_id_1 = command_vertex_data_get_keyboard_id(command_vertex_get_vertex_data((Object)*(int*)vertex_1));
  keyboard_id_2 = command_vertex_data_get_keyboard_id(command_vertex_get_vertex_data((Object)*(int*)vertex_2));

  return strcmp(keyboard_id_1, keyboard_id_2);
}

void command_graph_dump_to_text(const Command_Graph graph, FILE *stream)
{
  Arraylist vertices_list = command_graph_get_vertices(graph);
  int graph_size = command_graph_size(graph);
  int i, j;

  for (i = 0; i < graph_size; i++)
    {
      Command_Vertex a_vertex = (Command_Vertex)arraylist_get(vertices_list, i);
      Arraylist edges_list = command_vertex_get_edges(a_vertex);
      int edges_list_length = arraylist_size(edges_list);

      command_vertex_dump_text(a_vertex, stream);
      for (j = 0; j < edges_list_length; j++)
	{
	  Command_Edge an_edge = (Command_Edge)arraylist_get(edges_list, j);
	  fprintf(stream, "\n  ");
	  command_edge_dump_text(an_edge, stream);
	}
      fprintf(stream, "\n");
    }
  fflush(stream);
}

void command_graph_dump_to_binary(const Command_Graph graph)
{
  Arraylist vertices_list = command_graph_get_vertices(graph);
  int graph_size = command_graph_size(graph);
  FILE *file;
  int i, j;
  
  if (graph_size < 1)
  {
  	return;
  }

  file = fopen(m_full_file_name, "wb");
  if (NULL == file)
    {
      fprintf(stderr, "\nUnable to save session information.\n");
      fflush(stderr);
      exit(EXIT_FAILURE);
    }

  for (i = 0; i < graph_size; i++)
    {
      Command_Vertex a_vertex = (Command_Vertex)arraylist_get(vertices_list, i);
      Arraylist edges_list = command_vertex_get_edges(a_vertex);
      int edges_list_length = arraylist_size(edges_list);

      command_vertex_dump_binary(a_vertex, file);
      for (j = 0; j < edges_list_length; j++)
	{
	  Command_Edge an_edge = (Command_Edge)arraylist_get(edges_list, j);
	  command_edge_dump_binary(an_edge, file);
	}
    }
  fflush(file);
  fclose(file);
  
  /* TODO - if this function is called more than once during execution - need to change next line */
  g_free((gchar*)m_full_file_name);
}

Command_Graph command_graph_reconstruct_from_binary()
{
  FILE *file;
  char *buffer;
  char *end_of_buffer;
  char *buffer_ptr;
  struct stat inode_info_buffer;
  int file_size;
  Command_Graph graph;
  
   m_full_file_name = g_build_filename (g_get_home_dir (),PREDICTIONS_FILENAME, NULL);
   file = fopen(m_full_file_name, "rb");
  if (NULL == file)
    {
      return NULL;
    }

  stat(m_full_file_name, &inode_info_buffer);
  file_size = inode_info_buffer.st_size;
  buffer = read_file_into_buffer(file, file_size);
  fclose(file);

  graph = command_graph_create();

  end_of_buffer = buffer + file_size;
  buffer_ptr = buffer;
  create_vertices(graph, buffer_ptr, end_of_buffer);
  buffer_ptr = buffer;
  create_edges(graph, buffer_ptr, end_of_buffer, NULL);

  free(buffer);

  return graph;
}

static void create_vertices(const Command_Graph graph, char *buffer_ptr, const char *end_of_buffer)
{
  short type;

  if (buffer_ptr >= end_of_buffer)
    {
      return;
    }
  memcpy(&type, buffer_ptr, short_size);
  buffer_ptr += short_size;
  if (STORAGE_TYPE_COMMAND_EDGE == type)
    {
      short keyboard_id_length, key_id_length;
      memcpy(&keyboard_id_length, buffer_ptr, short_size);
      buffer_ptr += short_size;
      memcpy(&key_id_length, buffer_ptr, short_size);
      buffer_ptr += short_size;
      create_vertices(graph, buffer_ptr + keyboard_id_length + key_id_length + float_size, end_of_buffer);
    }
  else if (STORAGE_TYPE_COMMAND_VERTEX == type)
    {
      char *command_info[COMMAND_INFO_ITEMS];

      extract_command(&buffer_ptr, command_info);
      graph->_total_commands++;
      arraylist_add(command_graph_get_vertices(graph),
		    command_vertex_create(command_vertex_data_create(command_info[KEYBOARD_ID],
								     command_info[KEY_ID],
								     extract_weight(&buffer_ptr))));
      free(command_info[KEYBOARD_ID]);
      free(command_info[KEY_ID]);
      create_vertices(graph, buffer_ptr, end_of_buffer);
    }
  else
    {
      command_graph_free(graph);
      fprintf(stderr, "\nUnable to initialize properly.  The saved session file is corrupt.");
      fflush(stderr);
      exit(EXIT_FAILURE);
    }
}

static void create_edges(const Command_Graph graph,
			 char *buffer_ptr,
			 const char *end_of_buffer,
			 const Command_Vertex vertex_for_edges)
{
  short type;

  if (buffer_ptr >= end_of_buffer)
    {
      if (NULL != vertex_for_edges)
	{
	  command_vertex_free(vertex_for_edges);
	}
      return;
    }
  memcpy(&type, buffer_ptr, short_size);
  buffer_ptr += short_size;
  if (STORAGE_TYPE_COMMAND_VERTEX == type)
    {
      char *command_info[COMMAND_INFO_ITEMS];

      if (NULL != vertex_for_edges)
	{
	  command_vertex_free(vertex_for_edges);
	}
      extract_command(&buffer_ptr, command_info);
      create_edges(graph,
		   buffer_ptr + float_size,
		   end_of_buffer,
		   command_vertex_create(command_vertex_data_create(command_info[KEYBOARD_ID],
								    command_info[KEY_ID],
								    COMMAND_VERTEX_DATA_DEFAULT_WEIGHT)));
      free(command_info[KEYBOARD_ID]);
      free(command_info[KEY_ID]);
    }
  else if (STORAGE_TYPE_COMMAND_EDGE == type)
    {
      Command_Vertex next_vertex;
      char *command_info[COMMAND_INFO_ITEMS];

      extract_command(&buffer_ptr, command_info);
      next_vertex = command_vertex_create(command_vertex_data_create(command_info[KEYBOARD_ID],
								     command_info[KEY_ID],
								     COMMAND_VERTEX_DATA_DEFAULT_WEIGHT));
      command_vertex_add_edge(command_graph_get_vertex(graph, vertex_for_edges),
			      command_edge_create(command_graph_get_vertex(graph, next_vertex),
						  command_edge_data_create(extract_weight(&buffer_ptr))));
      command_vertex_free(next_vertex);
      free(command_info[KEYBOARD_ID]);
      free(command_info[KEY_ID]);
      create_edges(graph, buffer_ptr, end_of_buffer, vertex_for_edges);
    }
  else
    {
      command_graph_free(graph);
      fprintf(stderr, "\nUnable to initialize properly.  The saved session file is corrupt.");
      fflush(stderr);
      exit(EXIT_FAILURE);
    }
}

static float extract_weight(char **buffer_ptr)
{
  float weight;

  memcpy(&weight, *buffer_ptr, float_size);
  *buffer_ptr += float_size;
  return weight;
}

static void extract_command(char **buffer_ptr, char **command_info)
{
  short keyboard_id_length, key_id_length;

  memcpy(&keyboard_id_length, *buffer_ptr, short_size);
  *buffer_ptr += short_size;
  memcpy(&key_id_length, *buffer_ptr, short_size);
  *buffer_ptr += short_size;
#ifdef GOK_DEBUG
  command_info[KEYBOARD_ID] = malloc(char_size * (keyboard_id_length + 1));
  command_info[KEY_ID] = malloc(char_size * (key_id_length + 1));
#else
  command_info[KEYBOARD_ID] = checked_malloc(char_size * (keyboard_id_length + 1));
  command_info[KEY_ID] = checked_malloc(char_size * (key_id_length + 1));
#endif
  memcpy(command_info[KEYBOARD_ID], *buffer_ptr, keyboard_id_length);
  command_info[KEYBOARD_ID][keyboard_id_length] = '\0';
  *buffer_ptr += keyboard_id_length;
  memcpy(command_info[KEY_ID], *buffer_ptr, key_id_length);
  command_info[KEY_ID][key_id_length] = '\0';
  *buffer_ptr += key_id_length;
}

static char *read_file_into_buffer(FILE *file, const int file_size)
{
  char *buffer;
  int chars_read;
  short checksum;

#ifdef GOK_DEBUG
  buffer = malloc(char_size * file_size);
#else
  buffer = checked_malloc(char_size * file_size);
#endif
  chars_read = fread(buffer, char_size, file_size, file);
  memcpy(&checksum, buffer, short_size);
  if ((chars_read != file_size) || (STORAGE_TYPE_COMMAND_VERTEX != checksum))
    {
      fclose(file);
      free(buffer);
      fprintf(stderr, "\nUnable to initialize properly.  The saved session file is corrupt.");
      fflush(stderr);
      exit(EXIT_FAILURE);
    }
  return buffer;
}
