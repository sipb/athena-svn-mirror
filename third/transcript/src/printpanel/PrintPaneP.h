/*
 * PrintPaneP.h
 *
 * Copyright (C) 1992 by Adobe Systems Incorporated.
 * All rights reserved.
 * $RCSID: $Header: /afs/dev.mit.edu/source/repository/third/transcript/src/printpanel/PrintPaneP.h,v 1.1.1.1 1996-10-07 20:25:54 ghudson Exp $
 */

#ifndef _PrintPanelP_H
#define _PrintPanelP_H

#include "PrintPanel.h"
#include "PrintPaneI.h"

#define PICKONE 0
#define PICKMANY 1
#define BOOL 2

typedef struct _SecondaryData {
    int which_filter;
    Widget which_widget;
} SecondaryData;

typedef struct _FileSBData {
    Widget head;
    Widget which_sb;
    Widget which_field;
    char **name;
} FileSBData;

typedef struct _NUpData {
    int rows;
    int columns;
    int gaudy;
    int rotated;
} NUpData;

typedef struct _RangeData {
    int begin;
    int end;
} RangeData;

typedef struct _PslprData {
    int showpage;
    int squeeze;
    int landscape;
    int overtranslate;
} PslprData;

typedef struct _PsdraftData {
    char *font;
    float size;
    float angle;
    float gray;
    float xpos;
    float ypos;
    char *draftstring;
    int outline;
} PsdraftData;

typedef struct _EnscriptData {
    char *font;
    float font_size;
    char *header_font;
    float header_font_size;
    char *header;
    int tab_width;
    int columns;
    int lines;
    int rotated;
    int norotate;
    int gaudy;
    int lpt_mode;
    int no_header;
    int truncate_lines;
    int ignore_binary;
    int report_missing_chars;
    int quiet_mode;
    int no_burst_page;
} EnscriptData;

typedef struct _RoffData {
    char *font;
    char *options;
    char *font_dir;
} RoffData;

typedef struct _PS630Data {
    char *body_font;
    char *bold_font;
    char *pitch;
} PS630Data;

typedef struct _PS4014Data {
    char *scale;
    char *left;
    char *bottom;
    char *width;
    char *height;
    int portrait;
    int cr_no_lf;
    int lf_no_cr;
    int margin_2;
} PS4014Data;

typedef struct _FeatureData {
    char *name;
    int num;
    char *value[20];
    char *tmpvalue[20];
} FeatureData;

typedef struct _FaxData {
    char key[100];
    char name[100];
    char phone[10];
    int sendps;
    char *passwd;
    int save;
    char *filename;
} FaxData;

typedef struct _FeatureValueData {
    Widget head;
    int which_feature;
    char *choice;
    int which_key;
    int which_option;
} FeatureValueData;

typedef struct _ConstraintData {
    struct _UIData *no_key;
    struct _OptionData *no_option;
} ConstraintData;

typedef struct _OptionData {
    char name[255];
    char name_tran[255];
    int gray;
    Widget imp;
    int num_constraints;
    ConstraintData constraints[20];
} OptionData;

typedef struct _UIData {
    int type;
    char keyword[255];
    char key_tran[255];
    int default_option;
    Widget imp;
    int display;
    int num_options;
    OptionData options[50];
} UIData;

typedef struct _PPDData {
    char format_version[10];
    long free_vm;
    char product[255];
    char ps_version[10];
    char model[255];
    char nickname[255];
    int language_level;
    int color_device;
    char default_resolution[20];
    int supports_fax;
    int num_uis;
    UIData uis[20];
} PPDData;

typedef struct _OptionWidgets {
    Widget child[MAXCHILDREN];
    int num_children;
} OptionWidgets;

typedef struct _FilterListData {
    char name[255];
    int index;
} FilterListData;


typedef struct {
    XtCallbackList ok_callback;
    XtCallbackList cancel_callback;
    String in_file_name;
    String filter_name;
    String printer_name;
    Widget panel_child;
    Widget ok_button_child;
    Widget apply_button_child;
    Widget reset_button_child;
    Widget cancel_button_child;
    Widget bottom_separator_child;
    Widget top_separator_child;
    Widget print_toggle_child;
    Widget preview_toggle_child;
    Widget save_toggle_child;
    Widget fax_toggle_child;
    Widget radio_box_child;
    Widget save_label_child;
    Widget save_text_child;
    Widget save_button_child;
    Widget fax_label_child;
    Widget fax_text_child;
    Widget fax_button_child;
    Widget printer_list_child;
    Widget printer_label_child;
    Widget printer_field_child;
    Widget filter_list_child;
    Widget filter_label_child;
    Widget filter_field_child;
    Widget printer_option_button_child;
    Widget filter_option_button_child;
    Widget open_button_child;
    Widget open_field_child;
    Widget special_button_child;
    Widget status_button_child;
    Widget copy_label_child;
    Widget copy_field_child;
    Widget page_label_child;
    Widget page_box_child;
    Widget all_toggle_child;
    Widget range_toggle_child;
    Widget begin_field_child;
    Widget end_field_child;
    Widget range_label_child;
    Widget range_button_child;
    Widget input_file_selection_child;
    Widget output_file_selection_child;
    Widget message_dialog_child;
    Widget special_form_child;
    Widget special_ok_button_child;
    Widget special_apply_button_child;
    Widget special_reset_button_child;
    Widget special_cancel_button_child;
    Widget special_separator_child;
    Widget special_nup_toggle_child;
    Widget special_nup_row_label_child;
    Widget special_nup_row_field_child;
    Widget special_nup_column_label_child;
    Widget special_nup_column_field_child;
    Widget special_gaudy_toggle_child;
    Widget special_rotate_toggle_child;
    Widget special_showpage_toggle_child;
    Widget special_landscape_toggle_child;
    Widget special_squeeze_toggle_child;
    Widget special_overtranslate_toggle_child;
    Widget special_draft_toggle_child;
    Widget special_draft_button_child;
    Widget draft_form_child;
    Widget draft_label_child;
    Widget draft_ok_button_child;
    Widget draft_apply_button_child;
    Widget draft_reset_button_child;
    Widget draft_cancel_button_child;
    Widget draft_separator_child;
    Widget draft_string_label_child;
    Widget draft_string_field_child;
    Widget draft_x_label_child;
    Widget draft_x_field_child;
    Widget draft_y_label_child;
    Widget draft_y_field_child;
    Widget draft_font_label_child;
    Widget draft_font_field_child;
    Widget draft_size_label_child;
    Widget draft_size_field_child;
    Widget draft_angle_label_child;
    Widget draft_angle_field_child;
    Widget draft_gray_label_child;
    Widget draft_gray_field_child;
    Widget draft_outline_toggle_child;
    Widget generic_panel_child;
    Widget generic_label_child;
    Widget generic_separator_child;
    Widget generic_ok_button_child;
    Widget generic_apply_button_child;
    Widget generic_reset_button_child;
    Widget generic_cancel_button_child;
    Widget fax_panel_child;
    Widget fax_panel_label_child;
    Widget fax_ok_button_child;
    Widget fax_apply_button_child;
    Widget fax_reset_button_child;
    Widget fax_cancel_button_child;
    Widget fax_separator_child;
    Widget fax_phone_button_child;
    Widget fax_add_button_child;
    Widget fax_extra_button_child;
    Widget fax_save_toggle_child;
    Widget fax_save_button_child;
    Widget fax_save_selection_child;
    Widget fax_rec_label_child;
    Widget fax_rec_field_child;
    Widget fax_num_label_child;
    Widget fax_num_field_child;
    Widget fax_format_radio_child;
    Widget fax_standard_toggle_child;
    Widget fax_fine_toggle_child;
    Widget fax_teleps_toggle_child;
    Widget fax_teleps_label_child;
    Widget fax_teleps_field_child;
    Widget fax_phone_panel_child;
    Widget fax_phone_ok_button_child;
    Widget fax_phone_apply_button_child;
    Widget fax_phone_reset_button_child;
    Widget fax_phone_cancel_button_child;
    Widget fax_phone_separator_child;
    Widget fax_phone_label_child;
    Widget fax_phone_list_child;
    Widget feature_window_child;
    Widget feature_panel_child;
    Widget feature_form_child;
    Widget feature_label_child;
    Widget feature_separator_child;
    Widget feature_ok_button_child;
    Widget feature_apply_button_child;
    Widget feature_reset_button_child;
    Widget feature_cancel_button_child;
    Widget pulldown_menu_child;
    Widget pulldown_buttons[3];
    Widget feature_children[100];
    int num_feature_children;
    Widget fax_phone_children[100];
    int fax_phone_num_children;
    OptionWidgets option_children[6];
    Widget plot_selection_box;

    /* Private */
    char *input_file;
    char *output_file;
    char current_printer_name[30];
    char *default_printer_name;
    char *default_filter_name;
    char *preview_path;
    char *fax_path;
    int filter;
    int old_filter;
    int use_pslpr;
    int use_psnup;
    int use_psdraft, tmp_psdraft;
    int tmp_psnup;
    int action;
    int range;
    int copies;
    NUpData nup_info, tmp_nup_info;
    RangeData page_ranges[10];
    PslprData pslpr_info, tmp_pslpr_info;
    PsdraftData psdraft_info, tmp_psdraft_info;
    FileSBData input_sb_data, output_sb_data;
    FileSBData plot_data;
    FileSBData fax_file_data;
    EnscriptData enscript_info, tmp_enscript_info;
    RoffData roff_info, tmp_roff_info;
    PS630Data ps630_info, tmp_ps630_info;
    char *plot_profile, *tmp_plot_profile;
    PS4014Data ps4014_info, tmp_ps4014_info;
    FeatureData feature_info[50];
    int num_features;
    FilterListData filterlist[20];
    int num_filters;
    Widget filter_widget;
    int error_fd;
    int status_fd;
    int save_fd;
    XmString err_list;
    XmString status_list;
    char builtup_line[1024];
    char tmp_line[1024];
    SecondaryData option_callback_data, special_call_data;
    SecondaryData feature_callback_data, fax_callback_data;
    FeatureValueData choice_callback_data[100];
    FaxData fax_data, tmp_fax_data;
    char *printers[MAXPRINTERS];
    int num_printers;
    char *keys[10];
    int num_keys;
    int max_keys;
    int error_pipes[10][2];
    int connection_pipes[10][2];
    PPDData features;
    char faxdb[255];
} PrintPanelPart;

typedef struct _PrintPanelRec {
    CorePart core;
    CompositePart composite;
    ConstraintPart constraint;
    XmManagerPart manager;
    PrintPanelPart printpanel;
} PrintPanelRec;

typedef struct {
    XtPointer extension;
} PrintPanelClassPart;

typedef struct _PrintPanelClassRec {
    CoreClassPart core_class;
    CompositeClassPart composite_class;
    ConstraintClassPart constraint_class;
    XmManagerClassPart manager_class;
    PrintPanelClassPart printpanel_class;
} PrintPanelClassRec, *PrintPanelWidgetClass;

#endif /* _PrintPanelP_H */
/* DO NOT ADD ANYTHING AFTER THIS ENDIF */
