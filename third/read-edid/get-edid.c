/* (c) 2000 John Fremlin */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

typedef unsigned char byte;
typedef unsigned uint32;



#if defined linux

#include "lrmi/lrmi.h"

typedef struct LRMI_regs reg_frame;
typedef byte* real_ptr;

#define access_register(reg_frame,reg) (reg_frame . reg)
#define access_ptr_register(reg_frame,reg) (reg_frame -> reg)
#define access_seg_register(reg_frame,es) reg_frame.es
#define real_mode_int(interrupt,reg_frame_ptr) !LRMI_int(interrupt,reg_frame_ptr)
#define alloc_real(size) LRMI_alloc_real(size)
#define free_real(block) LRMI_free_real(block)
#define dosmemput(buffer,length,offset) memcpy(offset,buffer,length)

#else





#if defined __DJGPP__

#include <dos.h>
#include <dpmi.h>
#include <sys/movedata.h>

typedef __dpmi_regs reg_frame;
typedef int real_ptr;

#define access_register(reg_frame,reg) (reg_frame .d. reg)
#define access_ptr_register(reg_frame,reg) (reg_frame ->d. reg)
#define access_seg_register(reg_frame,es) reg_frame .x. es

#define real_mode_int(interrupt,reg_frame_ptr) __dpmi_int(interrupt,reg_frame_ptr)

#define alloc_real(size) \
	(__dpmi_allocate_dos_memory( (size+15)/16,&dummy )*16)
int dummy;

#define memcpy_from_real(buffer,block,length) \
	dosmemget( block, length, buffer )
#define free_real(block) /* too much hassle */


#else

#error No support for your compiler environment!

#endif // __DJGPP__

#endif // linux


real_ptr far_ptr_to_real_ptr( uint32 farptr )
{
  unsigned segment = (farptr & 0xffff0000)>>16;
  unsigned offset = (farptr & 0xffff);
  return (real_ptr)(segment*16+offset);
}




#define MAGIC   0x13

#define EDID_BLOCK_SIZE	128
#define EDID_V1_BLOCKS_TO_GO_OFFSET 126

#define SERVICE_REPORT_DDC	0
#define SERVICE_READ_EDID	1
#define SERVICE_LAST		1  // Read VDIF has been removed from the spec.



const char* ddc1_support_message[] =
{
  "Monitor and video card combination does not support DDC1 transfers",
  "Monitor and video card combination supports DDC1 transfers"
}
;
const char* ddc2_support_message[] =
{
  "Monitor and video card combination does not support DDC2 transfers",
  "Monitor and video card combination supports DDC2 transfers"
}
;
const char* screen_blanked_message[] =
{
  "Screen is not blanked during DDC transfer",
  "Screen is blanked during DDC transfer"
}
;


const char* supported_message[] =
{
  "Function unsupported",
  "Function supported"
}
;
const char* call_successful_message[] =
{
  "Call failed",
  "Call successful"
}
;

const char* vbe_service_message[] =
{
  "Report DDC capabilities",
  "Read EDID"
}
;

void
contact_author()
{
  fprintf( stderr,"\n\n*********** Something special has happened!\n" );
  fprintf( stderr,"Please contact the author, John Fremlin\n" );
  fprintf( stderr,"E-mail: one of vii@altern.org,vii@mailcc.com,vii@mailandnews.com\n" );
  fprintf( stderr,
	   "Please include full output from this program (especially that to stderr)\n\n\n\n" );
}


int
read_edid(  unsigned controller, FILE* output );

int do_vbe_ddc_service(unsigned BX,reg_frame* regs);

int
report_ddc_capabilities( unsigned controller );

int
report_vcontroller();
     

int
main( int argc, char** argv )
{
  unsigned controller = 0;
  int error;
  FILE* output;

  fprintf( stderr, "%s: get-edid version 1.3.7\n", argv[0] );
  
#if defined linux  
  if( !LRMI_init() )
    {
      fprintf( stderr, "%s: error initialising realmode interface\n", argv[0] );
      fprintf( stderr, "%s: do you have full superuser (root) permissions?\n", argv[0] );
      return 10;
    }
  
  ioperm(0, 0x400 , 1);
  iopl(3);
#endif
  

  if ( argc == 1 )
    output = stdout;
  else
    {
      if ( argc > 3 )
	{
	  fprintf( stderr, "%s: syntax %s [output-filename] [controller]\n",
		   argv[0],argv[0] );
	  return 3;
	}
      output = fopen( argv[1], "wb" );
      if ( !output )
	{
	  fprintf( stderr, "%s: error opening file \"%s\" for binary output\n",
		   argv[0],argv[1] );
	  return 4;
	}
      if ( argc == 3 )
	controller = strtol( argv[2],0,0 );
    }
  
  report_vcontroller();
  
  error = report_ddc_capabilities(controller);
  
  if (read_edid(controller,output))
    error = 1;

  fclose(output);
  
  return error;
}

int do_vbe_service(unsigned AX,unsigned BX,reg_frame* regs)
{
  const unsigned interrupt = 0x10;
  unsigned function_sup;
  unsigned success;
  int error = 0;

  access_ptr_register(regs,eax) = AX;
  access_ptr_register(regs,ebx) = BX;

  fprintf( stderr, "\n\tPerforming real mode VBE call\n" );

  fprintf( stderr, "\tInterrupt 0x%x ax=0x%x bx=0x%x cx=0x%x\n",
	   interrupt, AX, BX, (unsigned)access_ptr_register(regs,ecx) );
  
  if( real_mode_int(interrupt, regs) )
    {
      fprintf( stderr, "Error: something went wrong performing real mode interrupt\n" );
      error = 1;
    }
  
  AX = access_ptr_register(regs,eax);

  function_sup = ((AX & 0xff) == 0x4f);
  success = ((AX & 0xff00) == 0);
  
  fprintf( stderr, "\t%s\n", supported_message[ function_sup ] );
  fprintf( stderr, "\t%s\n\n", call_successful_message[ success ] );

  if (!success)
    error=1;
  if (!function_sup)
    error=2;
  
  return error;
}
  
int do_vbe_ddc_service(unsigned BX,reg_frame* regs)
{
  unsigned service = BX&0xff;
  unsigned AX = 0x4f15;


  fprintf( stderr, "\nVBE/DDC service about to be called\n" );
  if ( service > SERVICE_LAST )
    {
      fprintf( stderr, "\tUnknown VBE/DDC service\n" );
    }
  else
    {
      fprintf( stderr, "\t%s\n", vbe_service_message[ service ] );
    }  
  return do_vbe_service(AX,BX,regs);
}


int
report_ddc_capabilities( unsigned controller )
{
  reg_frame regs;
  int error;
  unsigned seconds_per_edid_block;
  unsigned ddc1_support;
  unsigned ddc2_support;
  unsigned screen_blanked_during_transfer;
  
  memset(&regs, 0, sizeof(regs));
  
  access_register(regs,ecx) = controller;
  
  error = do_vbe_ddc_service( SERVICE_REPORT_DDC,&regs );

  if ( !error )
    {
      seconds_per_edid_block = (access_register(regs,ebx) & 0xff00)>>16;
      ddc1_support = (access_register(regs,ebx) & 0x1)?1:0;
      ddc2_support = (access_register(regs,ebx) & 0x2)?1:0;
      screen_blanked_during_transfer = (access_register(regs,ebx) & 0x4)?1:0;

      fprintf( stderr, "\t%s\n", ddc1_support_message[ddc1_support] );
      fprintf( stderr, "\t%s\n", ddc2_support_message[ddc2_support] );
      fprintf( stderr, "\t%u seconds per 128 byte EDID block transfer\n",
	       seconds_per_edid_block );
      fprintf( stderr, "\t%s\n\n", screen_blanked_message[screen_blanked_during_transfer] );
    }
  
  return error;
}


int
read_edid(  unsigned controller, FILE* output )
{
  reg_frame regs;
 
  real_ptr block;
  byte* buffer;
  byte* pointer;
  unsigned blocks_left = 1;
  unsigned last_reported = 0;

  block = alloc_real( EDID_BLOCK_SIZE );
  
  if ( !block )
    {
      fprintf( stderr, "Error: can't allocate %x bytes of DOS memory for output block\n",
	       EDID_BLOCK_SIZE );
      return 2;
    }

#if defined __DJGPP__
  buffer = (byte*)malloc( EDID_BLOCK_SIZE );
  if ( !buffer )
    {
      fprintf( stderr, "Error: can't allocate %x bytes of memory for output block\n",
	       EDID_BLOCK_SIZE );
      return 2;
    }
#else
  buffer = block;
#endif
  
  memset(&regs, 0, sizeof(regs));

  access_seg_register(regs,es) = ((unsigned)block)/16;
  access_register(regs,edi) = 0;
  access_register(regs,ecx) = controller;
  
  
  do
    {
      unsigned counter;

      fprintf( stderr, "Reading next EDID block\n" );
#if defined linux      
      memset( block, MAGIC, EDID_BLOCK_SIZE );
#endif
      access_register(regs,edx)=blocks_left;

      if ( do_vbe_ddc_service( SERVICE_READ_EDID, &regs ) )
	{
	  fprintf( stderr,
		   "The EDID data should not be trusted as the VBE call failed\n" );
	}
      
#if defined linux
      for( pointer=block, counter=EDID_BLOCK_SIZE; counter; counter--,pointer++ )
	{
	  if ( *pointer != MAGIC )
	    goto block_ok;
	}
      fprintf( stderr, "Error: output block unchanged\n" );
      break;
      
    block_ok:
#else
      memcpy_from_real( buffer,block,EDID_BLOCK_SIZE );
#endif
		       
      blocks_left--;
      if ( buffer[ EDID_V1_BLOCKS_TO_GO_OFFSET ] > blocks_left )
	{
	  blocks_left = buffer[ EDID_V1_BLOCKS_TO_GO_OFFSET ];
	  
	  fprintf( stderr, "EDID claims %u more blocks left\n", blocks_left );
	  if ( blocks_left == MAGIC )
	    {
	      fprintf( stderr, "Possibly the EDID call did not work properly?\n" );
	    }
	  else if ( last_reported == blocks_left )
	    {
	      fprintf( stderr,
		       "EDID keeps on claiming same number of blocks left. Corruption?\n" );
	    }
	  else
	    contact_author();

	  last_reported = blocks_left;
	  return 1;
	}

      if ( EDID_BLOCK_SIZE != fwrite( buffer, sizeof( byte ), EDID_BLOCK_SIZE, output ) )
	{
	  fprintf( stderr, "\nError: problem writing output\n" );
	  return 1;
	}
      
    } while( blocks_left );

  
  free_real( block );
	
  return 0;
}

int
report_vcontroller()
{
  const unsigned info_block_length = 0x200;
  real_ptr block;
  real_ptr oem_string;
  reg_frame regs;
  byte* buffer;
  uint32 oem_string_ptr;
  byte next;
  unsigned vbe_version=0;

  memset(&regs, 0, sizeof(regs));

  block = alloc_real( info_block_length );
  if ( !block )
    {
      fprintf( stderr,
	       "Error: can't allocate %x bytes of DOS memory for videocard info block\n",
	       info_block_length );
      return 2;
    }
  
#if defined __DJGPP__
  buffer = malloc( info_block_length);
  if ( !buffer )
    {
      
      fprintf( stderr,
	       "Error: can't allocate %x bytes of normal memory for videocard info block\n",
	       info_block_length );
      return 2;
    }
#else
  buffer = block;
#endif
  

  dosmemput("VBE2",4,block);

  access_seg_register(regs,es) = ((unsigned)block)/16;
  access_register(regs,edi) = 0;
  
  do_vbe_service(0x4f00,0,&regs);

#if defined __DJGPP__
  memcpy_from_real( buffer,block,info_block_length );
#endif
  
  memcpy( &vbe_version, buffer+4, 2 );
  memcpy( &oem_string_ptr, buffer+6, 4 );

  oem_string = far_ptr_to_real_ptr( oem_string_ptr );

  fprintf( stderr, "\tVBE version %x\n", vbe_version );
  fprintf( stderr, "\tVBE string at 0x%x \"", (unsigned)oem_string );
  
  for(;;)
    {
      
#ifdef  __DJGPP__
      _dosmemgetb(oem_string, 1, &next);
#else
      next = *oem_string;
#endif
      if ( !next )
	break;
      fputc( next,stderr );

      oem_string++;
    }
  
  fprintf( stderr,"\"\n" );
  return 1;
}

