#include <gnome.h>


void
on_window1_realize                     (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_window1_remove                      (GtkContainer    *container,
                                        GtkWidget       *widget,
                                        gpointer         user_data);

gboolean
on_window1_delete_event                (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_window1_size_allocate               (GtkWidget       *widget,
                                        GtkAllocation   *allocation,
                                        gpointer         user_data);
void
on_window1_unrealize                   (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_window1_destroy                     (GtkObject       *object,
                                        gpointer         user_data);

gboolean
on_window1_button_press_event          (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data);

gboolean
on_editor_button_press_event          (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data);

gboolean
on_window1_client_event                (GtkWidget       *widget,
                                        GdkEventClient  *event,
                                        gpointer         user_data);

gboolean
on_window1_drag_motion                 (GtkWidget       *widget,
                                        GdkDragContext  *drag_context,
                                        gint             x,
                                        gint             y,
                                        guint            time,
                                        gpointer         user_data);

gboolean
on_window1_button_release_event        (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data);

gboolean
on_window1_button_toggle_event        (GtkWidget       *widget,
				       gpointer         user_data);

gboolean
on_window1_configure_event        (GtkWidget       *widget,
                                        GdkEventConfigure  *event,
                                        gpointer         user_data);


gboolean
on_window1_leave_notify_event         (GtkWidget        *widget,
				       GdkEventCrossing *event,
				       gpointer          user_data);

gboolean
on_window1_motion_notify_event         (GtkWidget       *widget,
                                        GdkEventMotion  *event,
                                        gpointer         user_data);

gboolean
on_window1_enter_notify_event         (GtkWidget        *widget,
				       GdkEventCrossing *event,
				       gpointer          user_data);

gboolean
on_window1_leave_notify_event         (GtkWidget        *widget,
				       GdkEventCrossing *event,
				       gpointer          user_data);

void
on_entryName_changed                   (GtkEditable     *editable,
                                        gpointer         user_data);
void
on_entry_input_device_changed          (GtkEditable     *editable,
                                        gpointer         user_data);

gboolean
on_dialogSettings_delete_event         (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

gboolean
on_dialogSettings_destroy_event        (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_dialogSettings_destroy              (GtkObject       *object,
                                        gpointer         user_data);

void
on_buttonKeySize_clicked               (GtkButton       *button,
                                        gpointer         user_data);

void
on_spinSpacing_changed                 (GtkEditable     *editable,
                                        gpointer         user_data);


void
on_spinWidth_changed                   (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_spinHeight_changed                  (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_spinWidth_changed                   (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_spinHeight_changed                  (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_spinSpacing_changed                 (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_radiobuttonTypeSwitch_toggled       (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_radiobuttonTypeMouseButton_toggled  (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_radiobuttonTypeDwell_toggled        (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_radiobuttonTypeMousePointer_toggled (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_entryActionName_changed               (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_buttonNewAction_clicked             (GtkButton       *button,
                                        gpointer         user_data);

void
on_checkWordCompletion_toggled         (GtkToggleButton *togglebutton,
                                        gpointer         user_data);


void
on_buttonNewAction_clicked             (GtkButton       *button,
                                        gpointer         user_data);

void
on_buttonSaveAction_clicked            (GtkButton       *button,
                                        gpointer         user_data);

void
on_buttonDeleteAction_clicked          (GtkButton       *button,
                                        gpointer         user_data);

void
on_spinSwitchDelay_changed             (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_buttonAccessMethodWizard_clicked    (GtkButton       *button,
                                        gpointer         user_data);

gboolean
on_comboActionNames_focus_out_event    (GtkWidget       *widget,
                                        GdkEventFocus   *event,
                                        gpointer         user_data);

gboolean
on_entryActionName_focus_out_event     (GtkWidget       *widget,
                                        GdkEventFocus   *event,
                                        gpointer         user_data);

gboolean
on_spinSwitchDelay_focus_in_event      (GtkWidget       *widget,
                                        GdkEventFocus   *event,
                                        gpointer         user_data);

gboolean
on_comboActionNames_focus_out_event    (GtkWidget       *widget,
                                        GdkEventFocus   *event,
                                        gpointer         user_data);

gboolean
on_entryActionName_leave_notify_event  (GtkWidget       *widget,
                                        GdkEventCrossing *event,
                                        gpointer         user_data);

void
on_buttonChangeName_clicked            (GtkButton       *button,
                                        gpointer         user_data);

void
on_radiobuttonSwitch1_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_spinSwitchDelay_changed             (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_radiobuttonSwitch2_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_radiobuttonSwitch3_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_radiobuttonSwitch4_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_radiobuttonSwitch5_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_radiobuttonSwitchPress_toggled      (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_radiobuttonSwitchRelease_toggled    (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_radiobuttonMouseButton0_toggled     (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_radiobuttonMouseButton1_toggled     (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_radiobuttonMouseButton2_toggled     (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_radiobuttonMouseButton3_toggled     (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_radiobuttonMouseButton4_toggled     (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_radiobuttonButtonPress_toggled      (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_radiobuttonButtonRelease_toggled    (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_radiobuttonButtonClick_toggled      (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_radiobuttonButtonDoubleClick_toggled
                                        (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_spinButtonDelay_changed             (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_spinDwellRate_changed               (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_checkKeyAverage_toggled             (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_spinKeyAverage_changed              (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_file1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_new1_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_open1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_save1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_save_as1_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_exit1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_help1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_gok_editor_help1_activate           (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_about1_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

gboolean
on_windowEditor_destroy_event          (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

gboolean
on_windowEditor_delete_event           (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_spinbutton47_changed                (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_buttonNext_clicked                  (GtkButton       *button,
                                        gpointer         user_data);

void
on_buttonPrevious_clicked              (GtkButton       *button,
                                        gpointer         user_data);

void
on_buttonAddNewKey_clicked             (GtkButton       *button,
                                        gpointer         user_data);

void
on_buttonDeleteKey_clicked             (GtkButton       *button,
                                        gpointer         user_data);

void
on_buttonDuplicate_clicked             (GtkButton       *button,
                                        gpointer         user_data);

void
on_spinLeft_changed                    (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_spinLeft_insert_text                (GtkEditable     *editable,
                                        gchar           *new_text,
                                        gint             new_text_length,
                                        gint            *position,
                                        gpointer         user_data);

void
on_spinRight_insert_text               (GtkEditable     *editable,
                                        gchar           *new_text,
                                        gint             new_text_length,
                                        gint            *position,
                                        gpointer         user_data);

void
on_spinTop_insert_text                 (GtkEditable     *editable,
                                        gchar           *new_text,
                                        gint             new_text_length,
                                        gint            *position,
                                        gpointer         user_data);

void
on_spinBottom_insert_text              (GtkEditable     *editable,
                                        gchar           *new_text,
                                        gint             new_text_length,
                                        gint            *position,
                                        gpointer         user_data);


void
on_dialogSettings_show                 (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_buttonAddFeedback_clicked           (GtkButton       *button,
                                        gpointer         user_data);

void
on_buttonDeleteFeedback_clicked        (GtkButton       *button,
                                        gpointer         user_data);

void
on_buttonChangeFeedbackName_clicked    (GtkButton       *button,
                                        gpointer         user_data);

void
on_buttonFeedbackSoundFile_clicked     (GtkButton       *button,
                                        gpointer         user_data);

void
on_entry63_changed                     (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_entryFeedback_changed               (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_checkKeyFlashing_clicked            (GtkButton       *button,
                                        gpointer         user_data);

void
on_spinKeyFlashing_changed             (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_checkSoundOn_clicked                (GtkButton       *button,
                                        gpointer         user_data);

void
on_entrySoundName_changed              (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_notebook2_switch_page               (GtkNotebook     *notebook,
                                        GtkNotebookPage *page,
                                        gint             page_num,
                                        gpointer         user_data);

gboolean
on_notebook2_destroy_event             (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

gboolean
on_notebook2_delete_event              (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_spinWidth_changed                   (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_spinHeight_changed                  (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_spinSpacing_changed                 (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_entryActionName_changed               (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_buttonNewAction_clicked             (GtkButton       *button,
                                        gpointer         user_data);

void
on_buttonDeleteAction_clicked          (GtkButton       *button,
                                        gpointer         user_data);

void
on_buttonChangeName_clicked            (GtkButton       *button,
                                        gpointer         user_data);

void
on_radiobuttonTypeSwitch_toggled       (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_radiobuttonTypeMouseButton_toggled  (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_radiobuttonTypeMousePointer_toggled (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_radiobuttonSwitch1_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_radiobuttonSwitch2_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_radiobuttonSwitch3_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_radiobuttonSwitch4_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_radiobuttonSwitch5_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_radiobuttonSwitchPress_toggled      (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_radiobutton1SwitchRelease_toggled   (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_spinSwitchDelay_changed             (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_radiobutton12_toggled               (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_radiobutton13_toggled               (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_radiobutton14_toggled               (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_radiobutton15_toggled               (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_radiobutton16_toggled               (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_radiobutton17_toggled               (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_radiobutton18_toggled               (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_radiobutton19_toggled               (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_radiobutton20_toggled               (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_spinDwellRate_changed               (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_checkKeyAverate_toggled             (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_spinKeyAverage_changed              (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_buttonAddFeedback_clicked           (GtkButton       *button,
                                        gpointer         user_data);

void
on_buttonDeleteFeedback_clicked        (GtkButton       *button,
                                        gpointer         user_data);

void
on_checkKeyFlashing_clicked            (GtkButton       *button,
                                        gpointer         user_data);

void
on_entrySoundName_changed              (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_buttonFeedbackSoundFile_clicked     (GtkButton       *button,
                                        gpointer         user_data);

void
on_buttonChangeFeedbackName_clicked    (GtkButton       *button,
                                        gpointer         user_data);

void
on_radiobuttonButtonPressed_toggled    (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_radiobuttonButtonReleased_toggled   (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_radiobuttonButtonClicked_toggled    (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_radiobuttonButtonPress_toggled      (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_radiobuttonButtonRelease_toggled    (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_radiobuttonButtonClick_toggled      (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_radiobuttonSwitchPress_toggled      (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_radiobutton1SwitchRelease_toggled   (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_radiobuttonSwitchRelease_toggled    (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_checkKeyAverage_toggled             (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_entryName_changed                   (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_new1_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_open1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_save1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_save_as1_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_quit1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_about1_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_buttonNext_clicked                  (GtkButton       *button,
                                        gpointer         user_data);

void
on_buttonPrevious_clicked              (GtkButton       *button,
                                        gpointer         user_data);

void
on_buttonAddNewKey_clicked             (GtkButton       *button,
                                        gpointer         user_data);

void
on_buttonDeleteKey_clicked             (GtkButton       *button,
                                        gpointer         user_data);

void
on_buttonDuplicate_clicked             (GtkButton       *button,
                                        gpointer         user_data);

void
on_spinLeft_insert_text                (GtkEditable     *editable,
                                        gchar           *new_text,
                                        gint             new_text_length,
                                        gint            *position,
                                        gpointer         user_data);

void
on_spinRight_insert_text               (GtkEditable     *editable,
                                        gchar           *new_text,
                                        gint             new_text_length,
                                        gint            *position,
                                        gpointer         user_data);

void
on_spinTop_insert_text                 (GtkEditable     *editable,
                                        gchar           *new_text,
                                        gint             new_text_length,
                                        gint            *position,
                                        gpointer         user_data);

void
on_spinBottom_insert_text              (GtkEditable     *editable,
                                        gchar           *new_text,
                                        gint             new_text_length,
                                        gint            *position,
                                        gpointer         user_data);

void
on_exit1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

gboolean
on_dialogSettings_delete_event         (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_spinDwellRate_changed               (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_checkKeyAverage_toggled             (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_spinKeyAverage_changed              (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_radiobuttonTypeValuator_toggled     (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_action_type_notebook_change_current_page
                                        (GtkNotebook     *notebook,
                                        gint             offset,
                                        gpointer         user_data);

void
on_2d_valuator_radiobutton_toggled     (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_1d_radiobutton_toggled              (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_axis_selection_spinbutton_changed   (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_activate_on_enter_button_toggled    (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_activate_on_dwell_button_toggled    (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_activate_on_move_button_toggled     (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_pointer_delay_spinbutton_changed    (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_key_averaging_button_toggled        (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_core_pointer_button_toggled         (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_xinput_device_button_toggled        (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_joystick_button_toggled             (GtkToggleButton *togglebutton,
                                        gpointer         user_data);


void
on_buttonNewAction_clicked             (GtkButton       *button,
                                        gpointer         user_data);

void
on_buttonDeleteAction_clicked          (GtkButton       *button,
                                        gpointer         user_data);


void
on_checkExtraWordList_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_buttonHelp_clicked                  (GtkButton       *button,
                                        gpointer         user_data);

void
on_buttonApply_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_buttonRevert_clicked                (GtkButton       *button,
                                        gpointer         user_data);

void
on_buttonCancel_clicked                (GtkButton       *button,
                                        gpointer         user_data);

void
on_buttonOK_clicked                    (GtkButton       *button,
                                        gpointer         user_data);

void
on_buttonHelp_clicked                  (GtkButton       *button,
                                        gpointer         user_data);

void
on_buttonApply_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_buttonRevert_clicked                (GtkButton       *button,
                                        gpointer         user_data);

void
on_buttonCancel_clicked                (GtkButton       *button,
                                        gpointer         user_data);

void
on_buttonOK_clicked                    (GtkButton       *button,
                                        gpointer         user_data);
