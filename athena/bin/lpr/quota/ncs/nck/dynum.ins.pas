{ dynum.ins.pas molson 26 June 87 
  Insert file for dialog dynamic enums package.  Your .dps file should look something like:
    APPLICATION_INTERFACE
        dynum_t := null:
            end
    USER_INTERFACE
        dynum_m := menu:
            task = dynum_t;
            entries = ();
            end;
}
{ Changes:
    26-jun-87 molson    original.
}
        
%IFDEF dynum_ins_pas %THEN
    %EXIT
%ELSE
    %VAR dynum_ins_pas
%ENDIF

CONST                             
        { since this isn't a real module these are just baby error codes - and ones you won't get }
        { from the system.                                                                        }
        dynum_$no_addr_space = 01;
        dynum_$no_such_task  = 02;
        dynum_$no_such_entry = 03;
        dynum_$no_entry_set  = 04;
TYPE 
    dynum_$entry_ptr_t = ^dynum_$entry_t;

    dynum_$entry_t = RECORD   
        id            : integer;                  { the id associated with this entry - static                }
        choice        : integer;                  { the dialog value associated with this entry - dynamic     }
        flagged       : boolean;                  { entry is flagged                                          }
        dp_string     : ARRAY [1..80] OF CHAR;    { string displayed in menu for this  entry                  }
        dp_str_len    : integer;                  { length of string                                          }
        ud_size       : integer;                  { size of the user data                                     }
        user_data     : univ_ptr;                 { pointer to user data                                      }
        user_set_data : boolean;                  { allocating and deallocating user data is user's problem.  }
        next_entry    : dynum_$entry_ptr_t;       { pointer to next entry                                     }
        END;

PROCEDURE dynum_$sort_entries(
    IN task_id  : dp_$task_id;
   OUT status   : status_$t
    ); EXTERN;
            
{ like dp_$enum_get_entry }
PROCEDURE dynum_$get_entry(
    IN task_id  : dp_$task_id;
   OUT entry_id : integer;                      { dynum_$entry_t.id                                    }
   OUT status   : status_$t
    ); EXTERN;

{ like dp_$enum_set_entry }
PROCEDURE dynum_$set_entry(
    IN task_id  : dp_$task_id;
    IN entry_id : integer;                      { dynum_$entry_t.id                                    }
   OUT status   : status_$t
    ); EXTERN;

{ like dp_$enum_enable_choice }
PROCEDURE dynum_$enable_entry(   
    IN task_id  : dp_$task_id;
    IN entry_id : integer;                      { dynum_$entry_t.id }
    IN onoff    : boolean;
   OUT status   : status_$t
    ); EXTERN;

{ like dp_$enum_flag_choice }                          
PROCEDURE dynum_$flag_entry(
    IN task_id    : dp_$task_id;
    IN entry_id   : integer;                    { dynum_$entry_t.id }
    IN flag_value : boolean;
   OUT status     : status_$t 
    ); EXTERN;

{ like dp_$enum_add_new_choice }
PROCEDURE dynum_$add_entry(
    IN task_id        : dp_$task_id ;
    IN new_string     : UNIV dp_$string_t ;
    IN new_string_len : integer ;
    IN ud_size        : integer;
   OUT entry_id       : integer;                { dynum_$entry_t.id                                    }
   OUT status         : status_$t
    ); EXTERN;

PROCEDURE dynum_$add_entry_sorted(
    IN task_id        : dp_$task_id ;
    IN new_string     : UNIV dp_$string_t ;
    IN new_string_len : integer ;
    IN ud_size        : integer;
   OUT entry_id       : integer;                { dynum_$entry_t.id                                    }
   OUT status         : status_$t
    ); EXTERN;

PROCEDURE dynum_$delete_entry(
    IN task_id      : dp_$task_id;
    IN entry_id     : integer;                  { dynum_$entry_t.id                                    }
   OUT status       : status_$t
    ); EXTERN;
                 
PROCEDURE dynum_$delete_all_entries(
    IN task_id : dp_$task_id;
   OUT status  : status_$t
   ); EXTERN;

{ change the string associated with and entry }
PROCEDURE dynum_$modify_entry_string(
    IN task_id      : dp_$task_id;
    IN entry_id     : integer;                  { dynum_$entry_t.id }
    IN new_string   : UNIV dp_$string_t;
    IN new_string_l : integer;
   OUT status       : status_$t
    ); EXTERN;
                    
{ gets the user data associated with an entry_id   }
PROCEDURE dynum_$get_user_data(
    IN  task_id     : dp_$task_id;
    IN  entry_id    : integer;                  { dynum_$entry_t.id }
   OUT  user_data   : univ_ptr;
   OUT  status      : status_$t
    ); EXTERN;
                 
PROCEDURE dynum_$set_user_data(
    IN  task_id     : dp_$task_id;
    IN  entry_id    : integer;                  { dynum_$entry_t.id }
    IN user_data   : univ_ptr;
   OUT  status      : status_$t
    ); EXTERN; 

{ returns the number of entries currently associated with this task }
PROCEDURE dynum_$get_high_entry(
    IN  task_id     : dp_$task_id;
   OUT  n_entries   : integer;
   OUT  status      : status_$t
    ); EXTERN;                             

{ returns a text string associated with a dynum error }
PROCEDURE dynum_$get_error_string(
    IN err          : status_$t;
   OUT errstring    : UNIV string;
   OUT status       : status_$t
   ); EXTERN;
