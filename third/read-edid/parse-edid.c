#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char byte;
/* byte must be 8 bits */

/* int must be at least 16 bits */

/* long must be at least 32 bits */



#define DIE_MSG( x ) \
        { MSG( x ); exit( 1 ); }


#define UPPER_NIBBLE( x ) \
        (((128|64|32|16) & (x)) >> 4)

#define LOWER_NIBBLE( x ) \
        ((1|2|4|8) & (x))

#define COMBINE_HI_8LO( hi, lo ) \
        ( (((unsigned)hi) << 8) | (unsigned)lo )

#define COMBINE_HI_4LO( hi, lo ) \
        ( (((unsigned)hi) << 4) | (unsigned)lo )

const byte edid_v1_header[] = { 0x00, 0xff, 0xff, 0xff,
                                0xff, 0xff, 0xff, 0x00 };

const byte edid_v1_descriptor_flag[] = { 0x00, 0x00 };


#define EDID_LENGTH                             0x80

#define EDID_HEADER                             0x00
#define EDID_HEADER_END                         0x07

#define ID_MANUFACTURER_NAME                    0x08
#define ID_MANUFACTURER_NAME_END                0x09

#define EDID_STRUCT_VERSION                     0x12
#define EDID_STRUCT_REVISION                    0x13

#define ESTABLISHED_TIMING_1                    0x23
#define ESTABLISHED_TIMING_2                    0x24
#define MANUFACTURERS_TIMINGS                   0x25

#define DETAILED_TIMING_DESCRIPTIONS_START      0x36
#define DETAILED_TIMING_DESCRIPTION_SIZE        18
#define NO_DETAILED_TIMING_DESCRIPTIONS         4



#define DETAILED_TIMING_DESCRIPTION_1           0x36
#define DETAILED_TIMING_DESCRIPTION_2           0x48
#define DETAILED_TIMING_DESCRIPTION_3           0x5a
#define DETAILED_TIMING_DESCRIPTION_4           0x6c



#define PIXEL_CLOCK_LO     (unsigned)dtd[ 0 ]
#define PIXEL_CLOCK_HI     (unsigned)dtd[ 1 ]
#define PIXEL_CLOCK        (COMBINE_HI_8LO( PIXEL_CLOCK_HI,PIXEL_CLOCK_LO )*10000)

#define H_ACTIVE_LO        (unsigned)dtd[ 2 ]

#define H_BLANKING_LO      (unsigned)dtd[ 3 ]

#define H_ACTIVE_HI        UPPER_NIBBLE( (unsigned)dtd[ 4 ] )

#define H_ACTIVE           COMBINE_HI_8LO( H_ACTIVE_HI, H_ACTIVE_LO )

#define H_BLANKING_HI      LOWER_NIBBLE( (unsigned)dtd[ 4 ] )

#define H_BLANKING         COMBINE_HI_8LO( H_BLANKING_HI, H_BLANKING_LO )




#define V_ACTIVE_LO        (unsigned)dtd[ 5 ]

#define V_BLANKING_LO      (unsigned)dtd[ 6 ]

#define V_ACTIVE_HI        UPPER_NIBBLE( (unsigned)dtd[ 7 ] )

#define V_ACTIVE           COMBINE_HI_8LO( V_ACTIVE_HI, V_ACTIVE_LO )

#define V_BLANKING_HI      LOWER_NIBBLE( (unsigned)dtd[ 7 ] )

#define V_BLANKING         COMBINE_HI_8LO( V_BLANKING_HI, V_BLANKING_LO )



#define H_SYNC_OFFSET_LO   (unsigned)dtd[ 8 ]
#define H_SYNC_WIDTH_LO    (unsigned)dtd[ 9 ]

#define V_SYNC_OFFSET_LO   UPPER_NIBBLE( (unsigned)dtd[ 10 ] )
#define V_SYNC_WIDTH_LO    LOWER_NIBBLE( (unsigned)dtd[ 10 ] )

#define V_SYNC_WIDTH_HI    ((unsigned)dtd[ 11 ] & (1|2))
#define V_SYNC_OFFSET_HI   (((unsigned)dtd[ 11 ] & (4|8)) >> 2)

#define H_SYNC_WIDTH_HI    (((unsigned)dtd[ 11 ] & (16|32)) >> 4)
#define H_SYNC_OFFSET_HI   (((unsigned)dtd[ 11 ] & (64|128)) >> 6)


#define V_SYNC_WIDTH       COMBINE_HI_4LO( V_SYNC_WIDTH_HI, V_SYNC_WIDTH_LO )
#define V_SYNC_OFFSET      COMBINE_HI_4LO( V_SYNC_OFFSET_HI, V_SYNC_OFFSET_LO )

#define H_SYNC_WIDTH       COMBINE_HI_4LO( H_SYNC_WIDTH_HI, H_SYNC_WIDTH_LO )
#define H_SYNC_OFFSET      COMBINE_HI_4LO( H_SYNC_OFFSET_HI, H_SYNC_OFFSET_LO )

#define H_SIZE_LO          (unsigned)dtd[ 12 ]
#define V_SIZE_LO          (unsigned)dtd[ 13 ]

#define H_SIZE_HI          UPPER_NIBBLE( (unsigned)dtd[ 14 ] )
#define V_SIZE_HI          LOWER_NIBBLE( (unsigned)dtd[ 14 ] )

#define H_SIZE             COMBINE_HI_8LO( H_SIZE_HI, H_SIZE_LO )
#define V_SIZE             COMBINE_HI_8LO( V_SIZE_HI, V_SIZE_LO )

#define H_BORDER           (unsigned)dtd[ 15 ]
#define V_BORDER           (unsigned)dtd[ 16 ]

#define FLAGS              (unsigned)dtd[ 17 ]

#define INTERLACED         (FLAGS&128)


#define MONITOR_NAME            0xfc
#define MONITOR_LIMITS          0xfd
#define UNKNOWN_DESCRIPTOR      -1
#define DETAILED_TIMING_BLOCK   -2


#define DESCRIPTOR_DATA         5
#define V_MIN_RATE              block[ 5 ]
#define V_MAX_RATE              block[ 6 ]
#define H_MIN_RATE              block[ 7 ]
#define H_MAX_RATE              block[ 8 ]

#define MAX_PIXEL_CLOCK         (((int)block[ 9 ]) * 10)
#define GTF_SUPPORT		block[10]

char* myname;

void MSG( const char* x )
{
  fprintf( stderr, "%s: %s\n", myname, x ); 
}


int
parse_edid( byte* edid );


int
parse_timing_description( byte* dtd );


int
parse_monitor_limits( byte* block );

int
block_type( byte* block );

char*
get_monitor_name( byte const*  block );


int
main( int argc, char** argv )
{
  byte edid[ EDID_LENGTH ];
  FILE* edid_file;

  myname = argv[ 0 ];
  fprintf( stderr, "%s: parse-edid version 1.3.7\n", myname );
  
  if ( argc > 2 )
    {
      DIE_MSG( "syntax: [input EDID file]" );
    }
  else
    {
      if ( argc == 2 )
	{
	  edid_file = fopen( argv[ 1 ], "rb" );
	  if ( !edid_file )
	    DIE_MSG( "unable to open file for input\n" );
	}
      
      else
	edid_file = stdin;
    }

  if ( fread( edid, sizeof( byte ), EDID_LENGTH, edid_file )
       != EDID_LENGTH )

    {
      DIE_MSG( "IO error reading EDID" );
    }

  fclose( edid_file );

  return parse_edid( edid );
}

int
parse_edid( byte* edid )
{
  unsigned i;
  byte* block;
  char* monitor_name = "Unknown";
  byte checksum = 0;
  
  for( i = 0; i < EDID_LENGTH; i++ )
    checksum += edid[ i ];

  if (  checksum != 0  )
      MSG( "EDID checksum failed - data is corrupt. Continuing anyway." );
  else
      MSG( "EDID checksum passed." );
  

  if ( strncmp( edid+EDID_HEADER, edid_v1_header, EDID_HEADER_END+1 ) )
    {
      MSG( "first bytes don't match EDID version 1 header" );
      MSG( "do not trust output (if any)." );
    }

  printf( "\n# EDID version %d revision %d\n", (int)edid[EDID_STRUCT_VERSION],(int)edid[EDID_STRUCT_REVISION] );
  

  printf( "Section \"Monitor\"\n" );

  block = edid + DETAILED_TIMING_DESCRIPTIONS_START;

  for( i = 0; i < NO_DETAILED_TIMING_DESCRIPTIONS; i++,
	 block += DETAILED_TIMING_DESCRIPTION_SIZE )
    {

      if ( block_type( block ) == MONITOR_NAME )
	{
	  monitor_name = get_monitor_name( block );
	  break;
	}
    }

  printf( "\tIdentifier \"%s\"\n", monitor_name );
  printf( "\tVendorName \"Unknown\"\n" );
  printf( "\tModelName \"%s\"\n", monitor_name );


  block = edid + DETAILED_TIMING_DESCRIPTIONS_START;

  for( i = 0; i < NO_DETAILED_TIMING_DESCRIPTIONS; i++,
	 block += DETAILED_TIMING_DESCRIPTION_SIZE )
    {

      if ( block_type( block ) == MONITOR_LIMITS )
	parse_monitor_limits( block );
    }


  block = edid + DETAILED_TIMING_DESCRIPTIONS_START;

  for( i = 0; i < NO_DETAILED_TIMING_DESCRIPTIONS; i++,
	 block += DETAILED_TIMING_DESCRIPTION_SIZE )
    {

      if ( block_type( block ) == DETAILED_TIMING_BLOCK )
	parse_timing_description( block );
    }


  printf( "EndSection\n" );

  return 0;
}


int
parse_timing_description( byte* dtd )
{
  printf( "\tMode \t\"%dx%d\"\n", H_ACTIVE, V_ACTIVE );

  printf( "\t\tDotClock\t%f\n", (double)PIXEL_CLOCK/1000000.0 );

  printf( "\t\tHTimings\t%u %u %u %u\n", H_ACTIVE,
	  H_ACTIVE+H_SYNC_OFFSET,
	  H_ACTIVE+H_SYNC_OFFSET+H_SYNC_WIDTH,
	  H_ACTIVE+H_BLANKING );

  printf( "\t\tVTimings\t%u %u %u %u\n", V_ACTIVE,
	  V_ACTIVE+V_SYNC_OFFSET,
	  V_ACTIVE+V_SYNC_OFFSET+V_SYNC_WIDTH,
	  V_ACTIVE+V_BLANKING );

  if ( INTERLACED )
    printf( "\t\tFlags\t\"Interlace\"\n" );

  printf( "\tEndMode\n" );

  return 0;
}


int
block_type( byte* block )
{

  if ( !strncmp( edid_v1_descriptor_flag, block, 2 ) )
    {

      /* descriptor */

      if ( block[ 2 ] != 0 )
	return UNKNOWN_DESCRIPTOR;


      return block[ 3 ];
    } else {

      /* detailed timing block */

      return DETAILED_TIMING_BLOCK;
    }
}

char*
get_monitor_name( byte const* block )
{
  static char name[ 13 ];
  unsigned i;
  byte const* ptr = block + DESCRIPTOR_DATA;


  for( i = 0; i < 13; i++, ptr++ )
    {

      if ( *ptr == 0xa )
	{
	  name[ i ] = 0;
	  return name;
	}

      name[ i ] = *ptr;
    }


  return name;
}

int
parse_monitor_limits( byte* block )
{
  printf( "\tHorizSync %u-%u\n", H_MIN_RATE, H_MAX_RATE );
  printf( "\tVertRefresh %u-%u\n", V_MIN_RATE, V_MAX_RATE );
  if ( MAX_PIXEL_CLOCK == 10*0xff )
    printf( "# Max dot clock not given\n" );
  else
    printf( "# Max dot clock (video bandwidth) %u MHz\n", (int)MAX_PIXEL_CLOCK );

  if ( GTF_SUPPORT )
    {
      printf( "# EDID version 3 GTF given: contact author\n" );
    }
  
  return 0;
}

