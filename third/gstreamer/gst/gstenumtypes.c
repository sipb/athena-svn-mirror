
/* Generated data (by glib-mkenums) */

#include <gst/gst.h>

/* enumerations from "gstobject.h" */
GType
gst_object_flags_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GST_DESTROYED, "GST_DESTROYED", "destroyed" },
      { GST_FLOATING, "GST_FLOATING", "floating" },
      { GST_OBJECT_FLAG_LAST, "GST_OBJECT_FLAG_LAST", "object-flag-last" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GstObjectFlags", values);
  }
  return etype;
}


/* enumerations from "gsttypes.h" */
GType
gst_element_state_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GFlagsValue values[] = {
      { GST_STATE_VOID_PENDING, "GST_STATE_VOID_PENDING", "void-pending" },
      { GST_STATE_NULL, "GST_STATE_NULL", "null" },
      { GST_STATE_READY, "GST_STATE_READY", "ready" },
      { GST_STATE_PAUSED, "GST_STATE_PAUSED", "paused" },
      { GST_STATE_PLAYING, "GST_STATE_PLAYING", "playing" },
      { 0, NULL, NULL }
    };
    etype = g_flags_register_static ("GstElementState", values);
  }
  return etype;
}

GType
gst_element_state_return_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GST_STATE_FAILURE, "GST_STATE_FAILURE", "failure" },
      { GST_STATE_SUCCESS, "GST_STATE_SUCCESS", "success" },
      { GST_STATE_ASYNC, "GST_STATE_ASYNC", "async" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GstElementStateReturn", values);
  }
  return etype;
}

GType
gst_result_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GST_RESULT_OK, "GST_RESULT_OK", "ok" },
      { GST_RESULT_NOK, "GST_RESULT_NOK", "nok" },
      { GST_RESULT_NOT_IMPL, "GST_RESULT_NOT_IMPL", "not-impl" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GstResult", values);
  }
  return etype;
}


/* enumerations from "gstautoplug.h" */
GType
gst_autoplug_flags_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GST_AUTOPLUG_TO_CAPS, "GST_AUTOPLUG_TO_CAPS", "to-caps" },
      { GST_AUTOPLUG_TO_RENDERER, "GST_AUTOPLUG_TO_RENDERER", "to-renderer" },
      { GST_AUTOPLUG_FLAG_LAST, "GST_AUTOPLUG_FLAG_LAST", "flag-last" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GstAutoplugFlags", values);
  }
  return etype;
}


/* enumerations from "gstbin.h" */
GType
gst_bin_flags_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GST_BIN_FLAG_MANAGER, "GST_BIN_FLAG_MANAGER", "flag-manager" },
      { GST_BIN_SELF_SCHEDULABLE, "GST_BIN_SELF_SCHEDULABLE", "self-schedulable" },
      { GST_BIN_FLAG_PREFER_COTHREADS, "GST_BIN_FLAG_PREFER_COTHREADS", "flag-prefer-cothreads" },
      { GST_BIN_FLAG_FIXED_CLOCK, "GST_BIN_FLAG_FIXED_CLOCK", "flag-fixed-clock" },
      { GST_BIN_FLAG_LAST, "GST_BIN_FLAG_LAST", "flag-last" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GstBinFlags", values);
  }
  return etype;
}


/* enumerations from "gstbuffer.h" */
GType
gst_buffer_flag_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GST_BUFFER_READONLY, "GST_BUFFER_READONLY", "readonly" },
      { GST_BUFFER_SUBBUFFER, "GST_BUFFER_SUBBUFFER", "subbuffer" },
      { GST_BUFFER_ORIGINAL, "GST_BUFFER_ORIGINAL", "original" },
      { GST_BUFFER_DONTFREE, "GST_BUFFER_DONTFREE", "dontfree" },
      { GST_BUFFER_DISCONTINUOUS, "GST_BUFFER_DISCONTINUOUS", "discontinuous" },
      { GST_BUFFER_KEY_UNIT, "GST_BUFFER_KEY_UNIT", "key-unit" },
      { GST_BUFFER_PREROLL, "GST_BUFFER_PREROLL", "preroll" },
      { GST_BUFFER_FLAG_LAST, "GST_BUFFER_FLAG_LAST", "flag-last" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GstBufferFlag", values);
  }
  return etype;
}


/* enumerations from "gstcaps.h" */
GType
gst_caps_flags_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GFlagsValue values[] = {
      { GST_CAPS_FIXED, "GST_CAPS_FIXED", "fixed" },
      { GST_CAPS_FLOATING, "GST_CAPS_FLOATING", "floating" },
      { 0, NULL, NULL }
    };
    etype = g_flags_register_static ("GstCapsFlags", values);
  }
  return etype;
}


/* enumerations from "gstclock.h" */
GType
gst_clock_entry_status_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GST_CLOCK_ENTRY_OK, "GST_CLOCK_ENTRY_OK", "ok" },
      { GST_CLOCK_ENTRY_EARLY, "GST_CLOCK_ENTRY_EARLY", "early" },
      { GST_CLOCK_ENTRY_RESTART, "GST_CLOCK_ENTRY_RESTART", "restart" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GstClockEntryStatus", values);
  }
  return etype;
}

GType
gst_clock_entry_type_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GST_CLOCK_ENTRY_SINGLE, "GST_CLOCK_ENTRY_SINGLE", "single" },
      { GST_CLOCK_ENTRY_PERIODIC, "GST_CLOCK_ENTRY_PERIODIC", "periodic" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GstClockEntryType", values);
  }
  return etype;
}

GType
gst_clock_return_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GST_CLOCK_STOPPED, "GST_CLOCK_STOPPED", "stopped" },
      { GST_CLOCK_TIMEOUT, "GST_CLOCK_TIMEOUT", "timeout" },
      { GST_CLOCK_EARLY, "GST_CLOCK_EARLY", "early" },
      { GST_CLOCK_ERROR, "GST_CLOCK_ERROR", "error" },
      { GST_CLOCK_UNSUPPORTED, "GST_CLOCK_UNSUPPORTED", "unsupported" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GstClockReturn", values);
  }
  return etype;
}

GType
gst_clock_flags_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GFlagsValue values[] = {
      { GST_CLOCK_FLAG_CAN_DO_SINGLE_SYNC, "GST_CLOCK_FLAG_CAN_DO_SINGLE_SYNC", "do-single-sync" },
      { GST_CLOCK_FLAG_CAN_DO_SINGLE_ASYNC, "GST_CLOCK_FLAG_CAN_DO_SINGLE_ASYNC", "do-single-async" },
      { GST_CLOCK_FLAG_CAN_DO_PERIODIC_SYNC, "GST_CLOCK_FLAG_CAN_DO_PERIODIC_SYNC", "do-periodic-sync" },
      { GST_CLOCK_FLAG_CAN_DO_PERIODIC_ASYNC, "GST_CLOCK_FLAG_CAN_DO_PERIODIC_ASYNC", "do-periodic-async" },
      { GST_CLOCK_FLAG_CAN_SET_RESOLUTION, "GST_CLOCK_FLAG_CAN_SET_RESOLUTION", "set-resolution" },
      { GST_CLOCK_FLAG_CAN_SET_SPEED, "GST_CLOCK_FLAG_CAN_SET_SPEED", "set-speed" },
      { 0, NULL, NULL }
    };
    etype = g_flags_register_static ("GstClockFlags", values);
  }
  return etype;
}


/* enumerations from "gstcpu.h" */
GType
gst_cpu_flags_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GFlagsValue values[] = {
      { GST_CPU_FLAG_MMX, "GST_CPU_FLAG_MMX", "mmx" },
      { GST_CPU_FLAG_SSE, "GST_CPU_FLAG_SSE", "sse" },
      { GST_CPU_FLAG_MMXEXT, "GST_CPU_FLAG_MMXEXT", "mmxext" },
      { GST_CPU_FLAG_3DNOW, "GST_CPU_FLAG_3DNOW", "3dnow" },
      { 0, NULL, NULL }
    };
    etype = g_flags_register_static ("GstCPUFlags", values);
  }
  return etype;
}


/* enumerations from "gstdata.h" */
GType
gst_data_flags_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GST_DATA_READONLY, "GST_DATA_READONLY", "readonly" },
      { GST_DATA_FLAG_LAST, "GST_DATA_FLAG_LAST", "flag-last" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GstDataFlags", values);
  }
  return etype;
}


/* enumerations from "gstelement.h" */
GType
gst_element_flags_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GST_ELEMENT_COMPLEX, "GST_ELEMENT_COMPLEX", "complex" },
      { GST_ELEMENT_DECOUPLED, "GST_ELEMENT_DECOUPLED", "decoupled" },
      { GST_ELEMENT_THREAD_SUGGESTED, "GST_ELEMENT_THREAD_SUGGESTED", "thread-suggested" },
      { GST_ELEMENT_INFINITE_LOOP, "GST_ELEMENT_INFINITE_LOOP", "infinite-loop" },
      { GST_ELEMENT_NEW_LOOPFUNC, "GST_ELEMENT_NEW_LOOPFUNC", "new-loopfunc" },
      { GST_ELEMENT_EVENT_AWARE, "GST_ELEMENT_EVENT_AWARE", "event-aware" },
      { GST_ELEMENT_USE_THREADSAFE_PROPERTIES, "GST_ELEMENT_USE_THREADSAFE_PROPERTIES", "use-threadsafe-properties" },
      { GST_ELEMENT_SCHEDULER_PRIVATE1, "GST_ELEMENT_SCHEDULER_PRIVATE1", "scheduler-private1" },
      { GST_ELEMENT_SCHEDULER_PRIVATE2, "GST_ELEMENT_SCHEDULER_PRIVATE2", "scheduler-private2" },
      { GST_ELEMENT_FLAG_LAST, "GST_ELEMENT_FLAG_LAST", "flag-last" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GstElementFlags", values);
  }
  return etype;
}


/* enumerations from "gstevent.h" */
GType
gst_event_type_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GST_EVENT_UNKNOWN, "GST_EVENT_UNKNOWN", "unknown" },
      { GST_EVENT_EOS, "GST_EVENT_EOS", "eos" },
      { GST_EVENT_FLUSH, "GST_EVENT_FLUSH", "flush" },
      { GST_EVENT_EMPTY, "GST_EVENT_EMPTY", "empty" },
      { GST_EVENT_DISCONTINUOUS, "GST_EVENT_DISCONTINUOUS", "discontinuous" },
      { GST_EVENT_NEW_MEDIA, "GST_EVENT_NEW_MEDIA", "new-media" },
      { GST_EVENT_QOS, "GST_EVENT_QOS", "qos" },
      { GST_EVENT_SEEK, "GST_EVENT_SEEK", "seek" },
      { GST_EVENT_SEEK_SEGMENT, "GST_EVENT_SEEK_SEGMENT", "seek-segment" },
      { GST_EVENT_SEGMENT_DONE, "GST_EVENT_SEGMENT_DONE", "segment-done" },
      { GST_EVENT_SIZE, "GST_EVENT_SIZE", "size" },
      { GST_EVENT_RATE, "GST_EVENT_RATE", "rate" },
      { GST_EVENT_FILLER, "GST_EVENT_FILLER", "filler" },
      { GST_EVENT_TS_OFFSET, "GST_EVENT_TS_OFFSET", "ts-offset" },
      { GST_EVENT_INTERRUPT, "GST_EVENT_INTERRUPT", "interrupt" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GstEventType", values);
  }
  return etype;
}

GType
gst_event_flag_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GFlagsValue values[] = {
      { GST_EVENT_FLAG_NONE, "GST_EVENT_FLAG_NONE", "event-flag-none" },
      { GST_RATE_FLAG_NEGATIVE, "GST_RATE_FLAG_NEGATIVE", "rate-flag-negative" },
      { 0, NULL, NULL }
    };
    etype = g_flags_register_static ("GstEventFlag", values);
  }
  return etype;
}

GType
gst_seek_type_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GFlagsValue values[] = {
      { GST_SEEK_METHOD_CUR, "GST_SEEK_METHOD_CUR", "method-cur" },
      { GST_SEEK_METHOD_SET, "GST_SEEK_METHOD_SET", "method-set" },
      { GST_SEEK_METHOD_END, "GST_SEEK_METHOD_END", "method-end" },
      { GST_SEEK_FLAG_FLUSH, "GST_SEEK_FLAG_FLUSH", "flag-flush" },
      { GST_SEEK_FLAG_ACCURATE, "GST_SEEK_FLAG_ACCURATE", "flag-accurate" },
      { GST_SEEK_FLAG_KEY_UNIT, "GST_SEEK_FLAG_KEY_UNIT", "flag-key-unit" },
      { GST_SEEK_FLAG_SEGMENT_LOOP, "GST_SEEK_FLAG_SEGMENT_LOOP", "flag-segment-loop" },
      { 0, NULL, NULL }
    };
    etype = g_flags_register_static ("GstSeekType", values);
  }
  return etype;
}

GType
gst_seek_accuracy_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GST_SEEK_CERTAIN, "GST_SEEK_CERTAIN", "certain" },
      { GST_SEEK_FUZZY, "GST_SEEK_FUZZY", "fuzzy" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GstSeekAccuracy", values);
  }
  return etype;
}


/* enumerations from "gstformat.h" */
GType
gst_format_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GST_FORMAT_UNDEFINED, "GST_FORMAT_UNDEFINED", "undefined" },
      { GST_FORMAT_DEFAULT, "GST_FORMAT_DEFAULT", "default" },
      { GST_FORMAT_BYTES, "GST_FORMAT_BYTES", "bytes" },
      { GST_FORMAT_TIME, "GST_FORMAT_TIME", "time" },
      { GST_FORMAT_BUFFERS, "GST_FORMAT_BUFFERS", "buffers" },
      { GST_FORMAT_PERCENT, "GST_FORMAT_PERCENT", "percent" },
      { GST_FORMAT_UNITS, "GST_FORMAT_UNITS", "units" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GstFormat", values);
  }
  return etype;
}


/* enumerations from "gstindex.h" */
GType
gst_index_certainty_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GST_INDEX_UNKNOWN, "GST_INDEX_UNKNOWN", "unknown" },
      { GST_INDEX_CERTAIN, "GST_INDEX_CERTAIN", "certain" },
      { GST_INDEX_FUZZY, "GST_INDEX_FUZZY", "fuzzy" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GstIndexCertainty", values);
  }
  return etype;
}

GType
gst_index_entry_type_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GST_INDEX_ENTRY_ID, "GST_INDEX_ENTRY_ID", "id" },
      { GST_INDEX_ENTRY_ASSOCIATION, "GST_INDEX_ENTRY_ASSOCIATION", "association" },
      { GST_INDEX_ENTRY_OBJECT, "GST_INDEX_ENTRY_OBJECT", "object" },
      { GST_INDEX_ENTRY_FORMAT, "GST_INDEX_ENTRY_FORMAT", "format" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GstIndexEntryType", values);
  }
  return etype;
}

GType
gst_index_lookup_method_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GST_INDEX_LOOKUP_EXACT, "GST_INDEX_LOOKUP_EXACT", "exact" },
      { GST_INDEX_LOOKUP_BEFORE, "GST_INDEX_LOOKUP_BEFORE", "before" },
      { GST_INDEX_LOOKUP_AFTER, "GST_INDEX_LOOKUP_AFTER", "after" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GstIndexLookupMethod", values);
  }
  return etype;
}

GType
gst_assoc_flags_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GFlagsValue values[] = {
      { GST_ASSOCIATION_FLAG_NONE, "GST_ASSOCIATION_FLAG_NONE", "none" },
      { GST_ASSOCIATION_FLAG_KEY_UNIT, "GST_ASSOCIATION_FLAG_KEY_UNIT", "key-unit" },
      { GST_ASSOCIATION_FLAG_LAST, "GST_ASSOCIATION_FLAG_LAST", "last" },
      { 0, NULL, NULL }
    };
    etype = g_flags_register_static ("GstAssocFlags", values);
  }
  return etype;
}

GType
gst_index_resolver_method_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GST_INDEX_RESOLVER_CUSTOM, "GST_INDEX_RESOLVER_CUSTOM", "custom" },
      { GST_INDEX_RESOLVER_GTYPE, "GST_INDEX_RESOLVER_GTYPE", "gtype" },
      { GST_INDEX_RESOLVER_PATH, "GST_INDEX_RESOLVER_PATH", "path" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GstIndexResolverMethod", values);
  }
  return etype;
}

GType
gst_index_flags_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GST_INDEX_WRITABLE, "GST_INDEX_WRITABLE", "writable" },
      { GST_INDEX_READABLE, "GST_INDEX_READABLE", "readable" },
      { GST_INDEX_FLAG_LAST, "GST_INDEX_FLAG_LAST", "flag-last" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GstIndexFlags", values);
  }
  return etype;
}


/* enumerations from "gstpad.h" */
GType
gst_pad_link_return_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GST_PAD_LINK_REFUSED, "GST_PAD_LINK_REFUSED", "refused" },
      { GST_PAD_LINK_DELAYED, "GST_PAD_LINK_DELAYED", "delayed" },
      { GST_PAD_LINK_OK, "GST_PAD_LINK_OK", "ok" },
      { GST_PAD_LINK_DONE, "GST_PAD_LINK_DONE", "done" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GstPadLinkReturn", values);
  }
  return etype;
}

GType
gst_pad_direction_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GST_PAD_UNKNOWN, "GST_PAD_UNKNOWN", "unknown" },
      { GST_PAD_SRC, "GST_PAD_SRC", "src" },
      { GST_PAD_SINK, "GST_PAD_SINK", "sink" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GstPadDirection", values);
  }
  return etype;
}

GType
gst_pad_flags_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GST_PAD_DISABLED, "GST_PAD_DISABLED", "disabled" },
      { GST_PAD_NEGOTIATING, "GST_PAD_NEGOTIATING", "negotiating" },
      { GST_PAD_FLAG_LAST, "GST_PAD_FLAG_LAST", "flag-last" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GstPadFlags", values);
  }
  return etype;
}

GType
gst_pad_presence_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GST_PAD_ALWAYS, "GST_PAD_ALWAYS", "always" },
      { GST_PAD_SOMETIMES, "GST_PAD_SOMETIMES", "sometimes" },
      { GST_PAD_REQUEST, "GST_PAD_REQUEST", "request" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GstPadPresence", values);
  }
  return etype;
}

GType
gst_pad_template_flags_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GST_PAD_TEMPLATE_FIXED, "GST_PAD_TEMPLATE_FIXED", "fixed" },
      { GST_PAD_TEMPLATE_FLAG_LAST, "GST_PAD_TEMPLATE_FLAG_LAST", "flag-last" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GstPadTemplateFlags", values);
  }
  return etype;
}


/* enumerations from "gstplugin.h" */
GType
gst_plugin_error_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GST_PLUGIN_ERROR_MODULE, "GST_PLUGIN_ERROR_MODULE", "module" },
      { GST_PLUGIN_ERROR_DEPENDENCIES, "GST_PLUGIN_ERROR_DEPENDENCIES", "dependencies" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GstPluginError", values);
  }
  return etype;
}


/* enumerations from "gstprops.h" */
GType
gst_props_type_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GST_PROPS_END_TYPE, "GST_PROPS_END_TYPE", "end-type" },
      { GST_PROPS_INVALID_TYPE, "GST_PROPS_INVALID_TYPE", "invalid-type" },
      { GST_PROPS_INT_TYPE, "GST_PROPS_INT_TYPE", "int-type" },
      { GST_PROPS_FLOAT_TYPE, "GST_PROPS_FLOAT_TYPE", "float-type" },
      { GST_PROPS_FOURCC_TYPE, "GST_PROPS_FOURCC_TYPE", "fourcc-type" },
      { GST_PROPS_BOOLEAN_TYPE, "GST_PROPS_BOOLEAN_TYPE", "boolean-type" },
      { GST_PROPS_STRING_TYPE, "GST_PROPS_STRING_TYPE", "string-type" },
      { GST_PROPS_VAR_TYPE, "GST_PROPS_VAR_TYPE", "var-type" },
      { GST_PROPS_LIST_TYPE, "GST_PROPS_LIST_TYPE", "list-type" },
      { GST_PROPS_GLIST_TYPE, "GST_PROPS_GLIST_TYPE", "glist-type" },
      { GST_PROPS_FLOAT_RANGE_TYPE, "GST_PROPS_FLOAT_RANGE_TYPE", "float-range-type" },
      { GST_PROPS_INT_RANGE_TYPE, "GST_PROPS_INT_RANGE_TYPE", "int-range-type" },
      { GST_PROPS_LAST_TYPE, "GST_PROPS_LAST_TYPE", "last-type" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GstPropsType", values);
  }
  return etype;
}

GType
gst_props_flags_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GFlagsValue values[] = {
      { GST_PROPS_FIXED, "GST_PROPS_FIXED", "fixed" },
      { GST_PROPS_FLOATING, "GST_PROPS_FLOATING", "floating" },
      { 0, NULL, NULL }
    };
    etype = g_flags_register_static ("GstPropsFlags", values);
  }
  return etype;
}


/* enumerations from "gstquery.h" */
GType
gst_query_type_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GST_QUERY_NONE, "GST_QUERY_NONE", "none" },
      { GST_QUERY_TOTAL, "GST_QUERY_TOTAL", "total" },
      { GST_QUERY_POSITION, "GST_QUERY_POSITION", "position" },
      { GST_QUERY_LATENCY, "GST_QUERY_LATENCY", "latency" },
      { GST_QUERY_JITTER, "GST_QUERY_JITTER", "jitter" },
      { GST_QUERY_START, "GST_QUERY_START", "start" },
      { GST_QUERY_SEGMENT_END, "GST_QUERY_SEGMENT_END", "segment-end" },
      { GST_QUERY_RATE, "GST_QUERY_RATE", "rate" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GstQueryType", values);
  }
  return etype;
}


/* enumerations from "gstscheduler.h" */
GType
gst_scheduler_flags_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GST_SCHEDULER_FLAG_FIXED_CLOCK, "GST_SCHEDULER_FLAG_FIXED_CLOCK", "fixed-clock" },
      { GST_SCHEDULER_FLAG_LAST, "GST_SCHEDULER_FLAG_LAST", "last" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GstSchedulerFlags", values);
  }
  return etype;
}

GType
gst_scheduler_state_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GST_SCHEDULER_STATE_NONE, "GST_SCHEDULER_STATE_NONE", "none" },
      { GST_SCHEDULER_STATE_RUNNING, "GST_SCHEDULER_STATE_RUNNING", "running" },
      { GST_SCHEDULER_STATE_STOPPED, "GST_SCHEDULER_STATE_STOPPED", "stopped" },
      { GST_SCHEDULER_STATE_ERROR, "GST_SCHEDULER_STATE_ERROR", "error" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GstSchedulerState", values);
  }
  return etype;
}


/* enumerations from "gstthread.h" */
GType
gst_thread_state_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GST_THREAD_STATE_STARTED, "GST_THREAD_STATE_STARTED", "state-started" },
      { GST_THREAD_STATE_SPINNING, "GST_THREAD_STATE_SPINNING", "state-spinning" },
      { GST_THREAD_STATE_REAPING, "GST_THREAD_STATE_REAPING", "state-reaping" },
      { GST_THREAD_FLAG_LAST, "GST_THREAD_FLAG_LAST", "flag-last" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GstThreadState", values);
  }
  return etype;
}


/* enumerations from "gstregistry.h" */
GType
gst_registry_return_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GFlagsValue values[] = {
      { GST_REGISTRY_OK, "GST_REGISTRY_OK", "ok" },
      { GST_REGISTRY_LOAD_ERROR, "GST_REGISTRY_LOAD_ERROR", "load-error" },
      { GST_REGISTRY_SAVE_ERROR, "GST_REGISTRY_SAVE_ERROR", "save-error" },
      { GST_REGISTRY_PLUGIN_LOAD_ERROR, "GST_REGISTRY_PLUGIN_LOAD_ERROR", "plugin-load-error" },
      { GST_REGISTRY_PLUGIN_SIGNATURE_ERROR, "GST_REGISTRY_PLUGIN_SIGNATURE_ERROR", "plugin-signature-error" },
      { 0, NULL, NULL }
    };
    etype = g_flags_register_static ("GstRegistryReturn", values);
  }
  return etype;
}

GType
gst_registry_flags_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GFlagsValue values[] = {
      { GST_REGISTRY_READABLE, "GST_REGISTRY_READABLE", "readable" },
      { GST_REGISTRY_WRITABLE, "GST_REGISTRY_WRITABLE", "writable" },
      { GST_REGISTRY_EXISTS, "GST_REGISTRY_EXISTS", "exists" },
      { GST_REGISTRY_REMOTE, "GST_REGISTRY_REMOTE", "remote" },
      { GST_REGISTRY_DELAYED_LOADING, "GST_REGISTRY_DELAYED_LOADING", "delayed-loading" },
      { 0, NULL, NULL }
    };
    etype = g_flags_register_static ("GstRegistryFlags", values);
  }
  return etype;
}


/* enumerations from "gstparse.h" */
GType
gst_parse_error_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GST_PARSE_ERROR_SYNTAX, "GST_PARSE_ERROR_SYNTAX", "syntax" },
      { GST_PARSE_ERROR_NO_SUCH_ELEMENT, "GST_PARSE_ERROR_NO_SUCH_ELEMENT", "no-such-element" },
      { GST_PARSE_ERROR_NO_SUCH_PROPERTY, "GST_PARSE_ERROR_NO_SUCH_PROPERTY", "no-such-property" },
      { GST_PARSE_ERROR_LINK, "GST_PARSE_ERROR_LINK", "link" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GstParseError", values);
  }
  return etype;
}


/* Generated data ends here */

