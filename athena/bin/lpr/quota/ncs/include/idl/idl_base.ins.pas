{ idl_base.ins.pas, /us/idl}

{*
 *  --------------------------------------------------------------------
 * |THE FOLLOWING PROGRAMS ARE THE SOLE PROPERTY OF APOLLO COMPUTER INC.|
 * |      AND CONTAIN ITS PROPRIETARY AND CONFIDENTIAL INFORMATION.     |
 *  --------------------------------------------------------------------
 *}

{*
 * This file is %INCLUDE'd by ALL Pascal files emitted by the interface compiler
 *}

%IFDEF idl_base %THEN
%EXIT
%ELSE
%VAR idl_base
%ENDIF

type
    char_ptr   = ^char;
    integer8   = -128..127;

    unsigned8  = 0..255;
    unsigned16 = 0..65535;
    unsigned   = unsigned16;
    unsigned32 = 0..2147483647; { only 2^31 because of Pascal complier limits }

    integer64  = record
                    high:   integer32;
                    low:    unsigned32;
                 end;

    unsigned64  = record
                    high:   unsigned32;
                    low:    unsigned32;
                 end;

    handle_rec_t =
        record
            data_offset:        unsigned;
            end;

    handle_t  = ^handle_rec_t;

    rpc_$drep_t = 
        packed record 
            int_rep:    0 .. 15;
            char_rep:   0 .. 15;
            float_rep:  0 .. 255;
            reserved:   unsigned16;
        end;

