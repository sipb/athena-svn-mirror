pixmapdir = $(datadir)/pixmaps

pixmap_DATA = gnomemeeting-logo-icon.png gnomemeeting-logo.png gnomemeeting-splash.png 


EMOTICONS_FILES = \
	emoticon-face1.png \
	emoticon-face2.png \
	emoticon-face3.png \
	emoticon-face4.png \
	emoticon-face5.png \
	emoticon-face6.png \
	emoticon-face7.png \
	emoticon-face8.png \
	emoticon-face9.png \
	emoticon-face10.png \
	emoticon-face11.png \
	emoticon-face12.png \
	emoticon-face13.png \
	emoticon-face14.png \
	emoticon-face15.png \
	emoticon-face16.png \
	emoticon-face17.png \
	emoticon-face18.png \
	emoticon-face19.png \
	emoticon-dead-happy.png \
	emoticon-dead.png \
	emoticon-dead-sad.png \
	emoticon-happy-devil.png \
	emoticon-happy-nose-glasses.png \
	emoticon-sad-devil.png \
	emoticon-sad-glasses.png \
	emoticon-sad-nose-glasses.png 

IMAGES_FILES= \
	addressbook-24.png \
	addressbook-16.png \
	text-chat.png \
	control-panel.png \
	connect.png \
	disconnect.png \
	volume.png \
	microphone.png \
	video-preview.png \
	audio-mute.png \
	speaker-phone.png \
	video-mute.png \
	status-available-16.png \
	status-in-a-call-16.png \
	status-free-for-chat-16.png \
	status-ringing-16.png \
	status-busy-16.png \
	status-forward-16.png \
	druid_audio.png \
	druid_video.png \
	druid_ixj.png \
	druid_personal.png \
	druid_connection.png \
	local_contact.png \
	remote_contact.png \
	whiteness.png \
	brightness.png \
	colourness.png \
	contrast.png \
	connect-16.png \
	disconnect-16.png \
	calls-history.png


IMAGES= \
	gm_addressbook_24_stock_data addressbook-24.png \
	gm_addressbook_16_stock_data addressbook-16.png \
	gm_text_chat_stock_data text-chat.png \
	gm_control_panel_stock_data control-panel.png \
	gm_connect_stock_data connect.png \
	gm_disconnect_stock_data disconnect.png \
	gm_volume_stock_data volume.png \
	gm_microphone_stock_data microphone.png \
	gm_video_preview_stock_data video-preview.png \
	gm_audio_mute_stock_data audio-mute.png \
	gm_speaker_phone_stock_data speaker-phone.png \
	gm_video_mute_stock_data video-mute.png \
	gm_status_available_stock_data status-available-16.png \
	gm_status_ringing_stock_data status-ringing-16.png \
	gm_status_in_a_call_stock_data status-in-a-call-16.png \
	gm_status_forward_stock_data status-forward-16.png \
	gm_status_busy_stock_data status-busy-16.png \
	gm_status_free_for_chat_stock_data status-free-for-chat-16.png \
	gm_druid_audio_stock_data druid_audio.png \
	gm_druid_video_stock_data druid_video.png \
	gm_druid_ixj_stock_data druid_ixj.png \
	gm_druid_personal_stock_data druid_personal.png \
	gm_druid_connection_stock_data druid_connection.png \
	gm_remote_contact_stock_data remote_contact.png \
	gm_local_contact_stock_data local_contact.png \
	gm_whiteness_stock_data whiteness.png \
	gm_brightness_stock_data brightness.png \
	gm_colourness_stock_data colourness.png \
	gm_contrast_stock_data contrast.png \
	gm_connect_16_stock_data connect-16.png \
	gm_disconnect_16_stock_data disconnect-16.png \
	gm_calls_history_stock_data calls-history.png


EMOTICONS= \
	gm_emoticon_face1 emoticon-face1.png \
	gm_emoticon_face2 emoticon-face2.png \
	gm_emoticon_face3 emoticon-face3.png \
	gm_emoticon_face4 emoticon-face4.png \
	gm_emoticon_face5 emoticon-face5.png \
	gm_emoticon_face6 emoticon-face6.png \
	gm_emoticon_face7 emoticon-face7.png \
	gm_emoticon_face8 emoticon-face8.png \
	gm_emoticon_face9 emoticon-face9.png \
	gm_emoticon_face10 emoticon-face10.png \
	gm_emoticon_face11 emoticon-face11.png \
	gm_emoticon_face12 emoticon-face12.png \
	gm_emoticon_face13 emoticon-face13.png \
	gm_emoticon_face14 emoticon-face14.png \
	gm_emoticon_face15 emoticon-face15.png \
	gm_emoticon_face16 emoticon-face16.png \
	gm_emoticon_face17 emoticon-face17.png \
	gm_emoticon_face18 emoticon-face18.png \
	gm_emoticon_face19 emoticon-face19.png \
	gm_emoticon_dead_happy emoticon-dead-happy.png \
	gm_emoticon_dead emoticon-dead.png \
	gm_emoticon_dead_sad emoticon-dead-sad.png \
	gm_emoticon_happy_devil emoticon-happy-devil.png \
	gm_emoticon_nose_glasses emoticon-happy-nose-glasses.png \
	gm_emoticon_sad_devil emoticon-sad-devil.png \
	gm_emoticon_sad_glasses emoticon-sad-glasses.png \
	gm_emoticon_sad_nose_glasses emoticon-sad-nose-glasses.png \
	gm_emoticon_happy_nose_glasses emoticon-happy-nose-glasses.png 


noinst_DATA = inlines.h inline_emoticons.h

EXTRA_DIST= $(pixmap_DATA) $(EMOTICONS_FILES) $(IMAGES_FILES) text_logo.xpm

inlines.h: $(IMAGE_FILES) Makefile.am
	gdk-pixbuf-csource --raw --build-list $(IMAGES) > inlines.h

inline_emoticons.h: $(EMOTICONS_FILES) Makefile.am
	gdk-pixbuf-csource --raw --build-list $(EMOTICONS) > inline_emoticons.h
