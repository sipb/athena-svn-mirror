{*
 * ========================================================================== 
 * Copyright  1987 by Apollo Computer Inc., Chelmsford, Massachusetts

 * All Rights Reserved

 * All Apollo source code software programs, object code software programs,
 * documentation and copies thereof shall contain the copyright notice above
 * and this permission notice.  Apollo Computer Inc. reserves all rights,
 * title and interest with respect to copying, modification or the
 * distribution of such software programs and associated documentation,
 * except those rights specifically granted by Apollo in a Product Software
 * Program License or Source Code License between Apollo and Licensee.
 * Without this License, such software programs may not be used, copied,
 * modified or distributed in source or object code form.  Further, the
 * copyright notice must appear on the media, the supporting documentation
 * and packaging.  A Source Code License does not grant any rights to use
 * Apollo Computer's name or trademarks in advertising or publicity, with
 * respect to the distribution of the software programs without the specific
 * prior written permission of Apollo.  Trademark agreements may be obtained
 * in a separate Trademark License Agreement.

 * Apollo disclaims all warranties, express or implied, with respect to
 * the Software Programs including the implied warranties of merchantability
 * and fitness, for a particular purpose.  In no event shall Apollo be liable
 * for any special, indirect or consequential damages or any damages
 * whatsoever resulting from loss of use, data or profits whether in an
 * action of contract or tort, arising out of or in connection with the
 * use or performance of such software programs.
========================================================================== 
}
{ dynum.pas - package to manage dynamic enums }
{ Changes:

    26-jun-87   molson  original coding.
}

MODULE dynum;
        
DEFINE  
    dynum_$get_high_entry,
    dynum_$get_error_string,
    dynum_$get_user_data,   
    dynum_$set_user_data,
    dynum_$modify_entry_string,
    dynum_$flag_entry,
    dynum_$enable_entry,
    dynum_$get_entry,
    dynum_$set_entry,
    dynum_$delete_all_entries,
    dynum_$delete_entry,
    dynum_$add_entry,
    dynum_$sort_entries,
    dynum_$add_entry_sorted;
   
%include 'sys/ins/base.ins.pas'; 
%include 'sys/ins/rws.ins.pas';
%include 'sys/ins/gpr.ins.pas';

%include 'sys/ins/dialog.ins.pas';

%include '/sys/ins/pfm.ins.pas';   

%include 'dynum.ins.pas';

TYPE
    task_list_t = ^task_entry_t;

    task_entry_t = RECORD
        task_id     : dp_$task_id;              { dialog task           }
        current     : integer;                  { current id            }
        n_entries   : integer;                  { number of entries now }
        ids         : integer;                  { counter to generate unique ids }
        first_entry : dynum_$entry_ptr_t;       { first entry }
        last_entry  : dynum_$entry_ptr_t;       { last entry  }
        tnext       : task_list_t;
        END;

VAR

    first_task : task_list_t := NIL;
    last_task  : task_list_t := NIL;

{ I N T E R N A L }

FUNCTION string_greater (
    IN string1      : dp_$string_t;
    IN string2      : dp_$string_t;
    IN max_len      : integer
    ) : BOOLEAN;
    
VAR
    i       : integer;

BEGIN
    i := 1;
    WHILE (i <= max_len) DO BEGIN
        IF (string1[i] > string2[i]) THEN BEGIN 
            string_greater := TRUE;
            RETURN;
            END
        ELSE IF (string1[i] < string2[i]) THEN BEGIN
            string_greater := FALSE;
            RETURN;
            END;
        i := i + 1;
        END;

    IF (i = max_len) THEN string_greater := TRUE; { strings are really equal }
    END;

PROCEDURE dynum_get_entry(
    IN task_entry : task_list_t;
    IN entry_id   : integer;
   OUT entry      : dynum_$entry_ptr_t;
   OUT status     : status_$t);

BEGIN 

    { find it }
    entry := task_entry^.first_entry;
    IF entry = NIL THEN BEGIN
        status.all := dynum_$no_such_entry;
        RETURN;
        END;

    WHILE (entry^.id <> entry_id) DO BEGIN 
        entry := entry^.next_entry;           
        IF entry = NIL THEN BEGIN 
            status.all := dynum_$no_such_entry;
            RETURN;
            END;
        END;

    END;

PROCEDURE dynum_get_task(
    IN task_id  : dp_$task_id;
   OUT entry    : task_list_t;
   OUT new      : boolean;
   OUT status   : status_$t);
   
VAR
    tlp   : task_list_t;
    clrec : pfm_$cleanup_rec;
    tst   : status_$t;

BEGIN   
    status := pfm_$cleanup(clrec);
    IF status.all <> pfm_$cleanup_set THEN BEGIN 
        IF (new AND (tlp <> NIL)) THEN 
            rws_$release_heap_pool(tlp,rws_$std_pool,tst);
        entry := NIL;
        new := FALSE;
        RETURN;
        END;
    
    status.all := status_$ok;

    entry := NIL;

    new := FALSE;
  
    tlp := first_task;
    
    WHILE (tlp <> NIL) DO 
        IF tlp^.task_id <> task_id THEN 
            tlp := tlp^.tnext
        ELSE EXIT;
                
    { we didn't find this task }
    IF tlp = NIL THEN BEGIN                     
        new := TRUE;
        tlp := rws_$alloc_heap_pool(rws_$std_pool,sizeof(task_entry_t));
        IF tlp = NIL THEN BEGIN 
            status.all := dynum_$no_addr_space;
            RETURN;
            END;
        IF first_task = NIL THEN BEGIN
            first_task := tlp;
            last_task := tlp;
            END
        ELSE last_task^.tnext := tlp;
        last_task := tlp;
        tlp^.tnext := NIL;         
        tlp^.n_entries := 0;
        tlp^.current := 0;
        tlp^.task_id := task_id;
        tlp^.ids := 0;
        END;

    entry := tlp;
                    
    pfm_$rls_cleanup(clrec,status);

    END;

{ this is almost always called by cleanup }
PROCEDURE dynum_delete_task(
    IN  task_entry : task_list_t;
   OUT  status     : status_$t
    );

VAR          
    tlp : task_list_t;

BEGIN
    tlp := first_task;
       
    IF tlp = NIL THEN return;
                     
    IF task_entry = first_task THEN BEGIN 
        first_task := task_entry^.tnext;
        END
    ELSE BEGIN 
         IF tlp^.tnext <> NIL THEN 
            WHILE (tlp^.tnext <> task_entry) DO BEGIN 
                tlp := tlp^.tnext;
                IF tlp = NIL THEN EXIT;
                END;
    
        tlp^.tnext := task_entry^.tnext;
        END;            

    IF task_entry = last_task THEN BEGIN 
        last_task := tlp;
        last_task^.tnext := NIL;
        END;

    rws_$release_heap_pool(task_entry,rws_$std_pool,status);

    END;

PROCEDURE dynum_add_entry(
    IN task_id        : dp_$task_id ;
    IN new_string     : UNIV dp_$string_t ;
    IN new_string_len : integer ;
    IN ud_size        : integer;
    IN sorted         : boolean;
   OUT entry_id       : integer;         
   OUT status         : status_$t
    );

LABEL
    release,
    abort;

VAR
    task_entry : task_list_t;
    new        : boolean;
    ep         : dynum_$entry_ptr_t;
    te         : dynum_$entry_ptr_t;
    i          : integer;
    clrec : pfm_$cleanup_rec;
    tst   : status_$t;

BEGIN                                         

    status := pfm_$cleanup(clrec);
    IF status.all <> pfm_$cleanup_set THEN 
        GOTO abort;
   
    status.all := status_$ok;
    
    { get the task }
    dynum_get_task(task_id,task_entry,new,status);

    { create a new entry }      
    WITH task_entry^ DO BEGIN  

        { make an dynum entry }
        ep := rws_$alloc_heap_pool(rws_$std_pool,sizeof(dynum_$entry_t));
        IF ep = NIL THEN BEGIN 
            status.all := dynum_$no_addr_space;
            GOTO release;
            END;            
        ep^.user_data := NIL;

        IF (new) OR (first_entry = NIL) THEN
            first_entry := ep
        ELSE
            last_entry^.next_entry := ep;

        last_entry := ep;
        ep^.next_entry := NIL;
        n_entries := n_entries + 1;
                     
        { give this entry a unique id, bump the id counter }
        ids := ids + 1;
        ep^.id := ids;
                  
        { copy the string }
        FOR i := 1 TO new_string_len DO
            ep^.dp_string[i] := new_string[i];
                 
        ep^.dp_str_len := new_string_len;

        { make the user data }
        ep^.ud_size := ud_size;
        IF (ud_size > 0) THEN BEGIN 
            ep^.user_data := rws_$alloc_heap_pool(rws_$std_pool,ud_size);
            IF ep^.user_data = NIL THEN BEGIN 
                status.all := dynum_$no_addr_space;
                GOTO release;
                END;
            END;
        ep^.user_set_data := FALSE;

        IF (sorted) THEN
            dynum_$sort_entries(task_id,status)    { redrawa menu, so will add }
        ELSE        
            dp_$enum_add_new_choice(task_entry^.task_id,ep^.dp_string,ep^.dp_str_len,ep^.choice,status);
        IF status.all <> status_$ok THEN GOTO release;

        END; { with task_entry^ } 

    entry_id := ep^.id;
  
    pfm_$rls_cleanup(clrec,status);

    RETURN;

release:
    pfm_$rls_cleanup(clrec,status);
abort:     
    IF new THEN 
        dynum_delete_task(task_entry,tst)
    ELSE BEGIN 
        task_entry^.n_entries := task_entry^.n_entries - 1;
                     
        te := task_entry^.first_entry;
        WHILE ((te^.next_entry <> ep) AND (te^.next_entry <> NIL)) DO
            te := te^.next_entry;

        te^.next_entry := NIL;

        task_entry^.last_entry := te;

        END;
    IF ep <> NIL THEN BEGIN 
        IF ep^.user_data <> NIL THEN 
            rws_$release_heap_pool(ep^.user_data,rws_$std_pool,tst);

        rws_$release_heap_pool(ep,rws_$std_pool,tst);
        END;

    END;

PROCEDURE dynum_$add_entry_sorted(*
    IN task_id        : dp_$task_id ;
    IN new_string     : UNIV dp_$string_t ;
    IN new_string_len : integer ;
    IN ud_size        : integer;
   OUT entry_id       : integer;                { dynum_$entry_t.id                                    }
   OUT status         : status_$t
    *); 

BEGIN   
    dynum_add_entry(task_id,new_string,new_string_len,ud_size,TRUE,entry_id,status);
    END;

PROCEDURE dynum_$add_entry(*
    IN task_id        : dp_$task_id ;
    IN new_string     : UNIV dp_$string_t ;
    IN new_string_len : integer ;
    IN ud_size        : integer;
   OUT entry_id       : integer;                { dynum_$entry_t.id                                    }
   OUT status         : status_$t
    *); 

BEGIN   
    dynum_add_entry(task_id,new_string,new_string_len,ud_size,FALSE,entry_id,status);
    END;

PROCEDURE dynum_$sort_entries(*
    IN  task_id     : dp_$task_id;
   OUT  status      : status_$t
    *);
VAR
    dp_strings : dp_$string_array_t;
    te,ne      : dynum_$entry_ptr_t;
    pep        : dynum_$entry_ptr_t;
    task_entry : task_list_t;
    max_len    : integer;
    new        : boolean;
    tst        : status_$t;

BEGIN
    { find the task     }
    dynum_get_task(task_id,task_entry,new,status);
    IF status.all <> status_$ok THEN RETURN;
    
    IF new THEN BEGIN 
        status.all := dynum_$no_such_task;
        dynum_delete_task(task_entry,tst);
        RETURN;
        END;
 
    IF task_entry^.first_entry = NIL THEN RETURN;

    { sort the linked list }
    te := task_entry^.first_entry;      
    pep := NIL;
    WHILE (te^.next_entry <> NIL)  DO BEGIN
        IF te^.dp_str_len > te^.next_entry^.dp_str_len THEN
            max_len := te^.dp_str_len
        ELSE
            max_len := te^.next_entry^.dp_str_len;
        IF string_greater(te^.dp_string,te^.next_entry^.dp_string,max_len) THEN BEGIN
            { switch places }
            ne := te^.next_entry;
            IF pep = NIL THEN 
                task_entry^.first_entry := ne
            ELSE 
                pep^.next_entry := ne;
            te^.next_entry  := ne^.next_entry;
            ne^.next_entry := te;
            {  start over   }
            te  := task_entry^.first_entry;
            pep := NIL;
            END
        ELSE BEGIN 
            pep := te;
            te := te^.next_entry;
            END;
        END;

    { fix last entry! }
    task_entry^.last_entry := te;

    { redo the enum }
    dp_$enum_set_choices(task_entry^.task_id,1,0,dp_strings,TRUE,status);
    IF status.all <> status_$ok THEN RETURN;

    pep := task_entry^.first_entry;
    WHILE (pep <> NIL) DO BEGIN 
        dp_$enum_add_new_choice(task_entry^.task_id,pep^.dp_string,pep^.dp_str_len,pep^.choice,status);
        IF status.all <> status_$ok THEN RETURN;
        IF pep^.flagged THEN 
            dp_$enum_flag_choice(task_entry^.task_id,pep^.choice,pep^.flagged,status);
        pep := pep^.next_entry;
        END;

    END;

PROCEDURE dynum_$delete_entry(*
    IN task_id      : dp_$task_id;
    IN entry_id     : integer;                  { dynum_$entry_t.id                                    }
   OUT status       : status_$t
    *); 

VAR
    task_entry : task_list_t;
    new        : boolean;
    ep         : dynum_$entry_ptr_t;
    pep        : dynum_$entry_ptr_t;
    dp_strings : dp_$string_array_t;
    tst        : status_$t;

BEGIN   

    { find the task     }
    dynum_get_task(task_id,task_entry,new,status);
    IF status.all <> status_$ok THEN RETURN;
    
    IF new THEN BEGIN 
        status.all := dynum_$no_such_task;
        dynum_delete_task(task_entry,tst);
        RETURN;
        END;
 
    IF task_entry^.first_entry = NIL THEN RETURN;

    { search }
    ep := task_entry^.first_entry;
    pep := NIL;

    WHILE (ep^.id <> entry_id) DO BEGIN 
        IF ep^.next_entry = NIL THEN BEGIN 
            status.all := dynum_$no_such_entry;
            RETURN;
            END
        ELSE BEGIN 
            pep := ep;
            ep := ep^.next_entry;
            END;
        END;               
               
    IF (pep = NIL) THEN 
        task_entry^.first_entry := ep^.next_entry
    ELSE
        pep^.next_entry := ep^.next_entry;

    IF (task_entry^.last_entry^.id = entry_id) THEN
        task_entry^.last_entry := pep;

    task_entry^.n_entries := task_entry^.n_entries - 1;

    { now get it out of the enum, fixing the choices as we go }
    dp_$enum_set_choices(task_entry^.task_id,1,0,dp_strings,TRUE,status);
    IF status.all <> status_$ok THEN RETURN;

    pep := task_entry^.first_entry;
    WHILE (pep <> NIL) DO BEGIN 
        dp_$enum_add_new_choice(task_entry^.task_id,pep^.dp_string,pep^.dp_str_len,pep^.choice,status);
        IF status.all <> status_$ok THEN RETURN;
        IF pep^.flagged THEN 
            dp_$enum_flag_choice(task_entry^.task_id,pep^.choice,pep^.flagged,status);
        pep := pep^.next_entry;
        END;
    IF task_entry^.first_entry = NIL THEN task_entry^.ids := 0;
             

    { and release the space }
    IF (ep^.user_data <> NIL) AND (NOT ep^.user_set_data) THEN 
        rws_$release_heap_pool(ep^.user_data,rws_$std_pool,status);
            
    rws_$release_heap_pool(ep,rws_$std_pool,tst);
    IF status.all = status_$ok THEN status := tst;

    END;    
             
PROCEDURE dynum_$delete_all_entries(*
    IN task_id : dp_$task_id;
   OUT status  : status_$t*);

VAR
    task_entry : task_list_t;
    new        : boolean;
    pep        : dynum_$entry_ptr_t;
    dp_strings : dp_$string_array_t;
    tst        : status_$t;

BEGIN   

    { find the task     }
    dynum_get_task(task_id,task_entry,new,status);
    IF status.all <> status_$ok THEN RETURN;
    
    IF new THEN BEGIN 
        status.all := dynum_$no_such_task;
        dynum_delete_task(task_entry,tst);
        RETURN;
        END;

    { now get it out of the enum, fixing the choices as we go }
    dp_$enum_set_choices(task_entry^.task_id,1,0,dp_strings,TRUE,status);
    IF status.all <> status_$ok THEN RETURN;

    pep := task_entry^.first_entry;
    WHILE (pep <> NIL) DO BEGIN 
        IF (pep^.user_data <> NIL) AND (NOT pep^.user_set_data) THEN 
            rws_$release_heap_pool(pep^.user_data,rws_$std_pool,status);
        rws_$release_heap_pool(pep,rws_$std_pool,tst);
        pep := pep^.next_entry;
        END;
   
    task_entry^.first_entry := NIL;
    task_entry^.last_entry  := NIL;
                 
    task_entry^.ids := 0;
    task_entry^.n_entries := 0;
    task_entry^.current := 0;

    END;    

PROCEDURE dynum_$get_entry(*
    IN task_id  : dp_$task_id;
   OUT entry_id : integer;                      { dynum_$entry_t.id                                    }
   OUT status   : status_$t
    *); 
 
VAR
    task_entry : task_list_t;
    new        : boolean;
    result     : integer;
    ep         : dynum_$entry_ptr_t;
    tst        : status_$t;

BEGIN   

    { find the task     }
    dynum_get_task(task_id,task_entry,new,status);
    IF status.all <> status_$ok THEN RETURN;
    
    IF new THEN BEGIN 
        status.all := dynum_$no_such_task;
        dynum_delete_task(task_entry,tst);
        RETURN;
        END;

    { read the value    }
    dp_$enum_get_value(task_entry^.task_id,result,status);
    IF status.all <> status_$ok THEN RETURN;
                 
    IF (result = 0) THEN BEGIN 
        status.all := dynum_$no_entry_set;
        RETURN;
        END;

    { find it }
    ep := task_entry^.first_entry;
    WHILE (ep^.choice <> result) DO 
        ep := ep^.next_entry;

    task_entry^.current := ep^.id;
    entry_id            := ep^.id;

    END;
       
PROCEDURE dynum_$set_entry(*
    IN task_id  : dp_$task_id;
    IN entry_id : integer;                      { dynum_$entry_t.id                                    }
   OUT status   : status_$t
    *); 

VAR
    task_entry : task_list_t;
    new        : boolean;
    ep         : dynum_$entry_ptr_t;
    tst        : status_$t;

BEGIN   

    { find the task     }
    dynum_get_task(task_id,task_entry,new,status);
    IF status.all <> status_$ok THEN RETURN;     

    IF new THEN BEGIN 
        status.all := dynum_$no_such_task;
        dynum_delete_task(task_entry,tst);
        RETURN;
        END;
        
    dynum_get_entry(task_entry,entry_id,ep,status);
    IF status.all <> status_$ok THEN RETURN;

    { set the value    }
    dp_$enum_set_value(task_entry^.task_id,ep^.choice,status);
    IF status.all <> status_$ok THEN RETURN;

    task_entry^.current := ep^.id;

    END;

PROCEDURE dynum_$enable_entry(*
    IN task_id  : dp_$task_id;
    IN entry_id : integer;                      { dynum_$entry_t.id }
    IN onoff    : boolean;
   OUT status   : status_$t
    *);

VAR
    task_entry : task_list_t;
    new        : boolean;
    ep         : dynum_$entry_ptr_t;
    tst        : status_$t;

BEGIN     
    dynum_get_task(task_id,task_entry,new,status);

    IF status.all <> status_$ok THEN RETURN;     

    IF new THEN BEGIN 
        status.all := dynum_$no_such_task;
        dynum_delete_task(task_entry,tst);
        RETURN;
        END;
        
    dynum_get_entry(task_entry,entry_id,ep,status);
    IF status.all <> status_$ok THEN RETURN;

    dp_$enum_enable_choice(task_entry^.task_id,ep^.choice,onoff,status);

    END;

PROCEDURE dynum_$flag_entry(*
    IN task_id    : dp_$task_id;
    IN entry_id   : integer;                    { dynum_$entry_t.id }
    IN flag_value : boolean;
   OUT status     : status_$t 
    *); 

VAR
    task_entry : task_list_t;
    new        : boolean;
    ep         : dynum_$entry_ptr_t;
    tst        : status_$t;

BEGIN     
    dynum_get_task(task_id,task_entry,new,status);

    IF status.all <> status_$ok THEN RETURN;     

    IF new THEN BEGIN 
        status.all := dynum_$no_such_task;
        dynum_delete_task(task_entry,tst);
        RETURN;
        END;
        
    dynum_get_entry(task_entry,entry_id,ep,status);
    IF status.all <> status_$ok THEN RETURN;

    ep^.flagged := flag_value;
    
    dp_$enum_flag_choice(task_entry^.task_id,ep^.choice,flag_value,status);

    END;

PROCEDURE dynum_$modify_entry_string(*
    IN task_id      : dp_$task_id;
    IN entry_id     : integer;                  { dynum_$entry_t.id }
    IN new_string   : UNIV dp_$string_t;
    IN new_string_l : integer;
   OUT status       : status_$t
    *); 

VAR
    task_entry : task_list_t;
    new        : boolean;
    ep         : dynum_$entry_ptr_t;
    i          : integer;
    dp_strings : dp_$string_array_t;    
    tst        : status_$t;

BEGIN     
    dynum_get_task(task_id,task_entry,new,status);

    IF status.all <> status_$ok THEN RETURN;     

    IF new THEN BEGIN 
        status.all := dynum_$no_such_task;
        dynum_delete_task(task_entry,tst);
        RETURN;
        END;
        
    dynum_get_entry(task_entry,entry_id,ep,status);
    IF status.all <> status_$ok THEN RETURN;

    FOR i := 1 TO new_string_l DO
        ep^.dp_string[i] := new_string[i];

    ep^.dp_str_len := new_string_l;

    dp_$enum_set_choices(task_entry^.task_id,1,0,dp_strings,TRUE,status);
    IF status.all <> status_$ok THEN RETURN;

    ep := task_entry^.first_entry;
    WHILE (ep <> NIL) DO BEGIN 
        dp_$enum_add_new_choice(task_entry^.task_id,ep^.dp_string,ep^.dp_str_len,ep^.choice,status);
        IF status.all <> status_$ok THEN RETURN;
        IF ep^.flagged THEN 
            dp_$enum_flag_choice(task_entry^.task_id,ep^.choice,ep^.flagged,status);
        ep := ep^.next_entry;
        END;

    END;

PROCEDURE dynum_$get_user_data(*
    IN  task_id     : dp_$task_id;
    IN  entry_id    : integer;                  { dynum_$entry_t.id }
   OUT  user_data   : univ_ptr
   OUT  status      : status_$t;
    *); 

VAR
    task_entry : task_list_t;
    new        : boolean;
    ep         : dynum_$entry_ptr_t;
    tst        : status_$t;

BEGIN     
    dynum_get_task(task_id,task_entry,new,status);
    IF status.all <> status_$ok THEN RETURN;     

    IF new THEN BEGIN 
        status.all := dynum_$no_such_task;
        dynum_delete_task(task_entry,tst);
        RETURN;
        END;
        
    dynum_get_entry(task_entry,entry_id,ep,status);
    IF status.all <> status_$ok THEN RETURN;
        
    user_data := ep^.user_data;

    END;
                  
{ This should only be called if user data was NOT set by add_entry. }
{ User data added in here will not be removed when the entry is     }
{ deleted.                                                          }
PROCEDURE dynum_$set_user_data(*
    IN  task_id     : dp_$task_id;
    IN  entry_id    : integer;                  { dynum_$entry_t.id }
    IN user_data    : univ_ptr
   OUT  status      : status_$t;
    *); 

VAR
    task_entry : task_list_t;
    new        : boolean;
    ep         : dynum_$entry_ptr_t;
    tst        : status_$t;

BEGIN     
    dynum_get_task(task_id,task_entry,new,status);
    IF status.all <> status_$ok THEN RETURN;     

    IF new THEN BEGIN 
        status.all := dynum_$no_such_task;
        dynum_delete_task(task_entry,tst);
        RETURN;
        END;
        
    dynum_get_entry(task_entry,entry_id,ep,status);
    IF status.all <> status_$ok THEN RETURN;
        
    ep^.user_data := user_data;
    
    ep^.user_set_data := TRUE;

    END;

PROCEDURE dynum_$get_high_entry(*
    IN  task_id     : dp_$task_id;
   OUT  n_entries   : integer;
   OUT  status      : status_$t
    *);

VAR
    task_entry : task_list_t;
    new        : boolean;
    tst        : status_$t;

BEGIN            
    n_entries := 0;

    dynum_get_task(task_id,task_entry,new,status);
    IF status.all <> status_$ok THEN RETURN;     
                 
    IF new THEN BEGIN 
        status.all := dynum_$no_such_task;
        dynum_delete_task(task_entry,tst);
        RETURN;
        END;

    n_entries := task_entry^.ids;

    END;

PROCEDURE dynum_$get_error_string(*
    IN err          : status_$t;
   OUT errstring    : UNIV string;
   OUT status       : status_$t
   *); 

BEGIN          
    CASE err.all OF
        dynum_$no_addr_space : errstring := 'Insufficient memory';
        dynum_$no_such_task  : errstring := 'No such task';
        dynum_$no_such_entry : errstring := 'No such entry';
        END;
    END;
