#include "quickjs.h"
#include <stdint.h>
#include <math.h>

#define LAY_FLOAT 1
#define LAY_IMPLEMENTATION
#include "layout.h"

static JSClassID js_layout_class_id;

#define GETLAY \
lay_context *lay = JS_GetOpaque2(js, self, js_layout_class_id); \
if (!lay) return JS_EXCEPTION;

#define GETITEM(VAR, ARG) \
lay_id VAR; \
if (JS_ToUint32(js, &VAR, ARG)) return JS_EXCEPTION; \

static JSValue js_layout_context_new(JSContext *js, JSValueConst self, int argc, JSValueConst *argv) {
  lay_context *lay = js_malloc(js, sizeof(*lay));
  lay_init_context(lay);
  JSValue obj = JS_NewObjectClass(js, js_layout_class_id);
  JS_SetOpaque(obj, lay);
  return obj;
}

static JSValue js_layout_item(JSContext *js, JSValueConst self, int argc, JSValueConst *argv) {
  GETLAY
  lay_id item_id = lay_item(lay);
  return JS_NewUint32(js,item_id);
}

static JSValue js_layout_set_size(JSContext *js, JSValueConst self, int argc, JSValueConst *argv)
{
  if (argc < 2) return JS_EXCEPTION;
  
  GETLAY
  GETITEM(id, argv[0])
  double size[2];
  JSValue width_val = JS_GetPropertyUint32(js, argv[1], 0);
  JSValue height_val = JS_GetPropertyUint32(js, argv[1], 1);
  JS_ToFloat64(js, &size[0], width_val);
  JS_ToFloat64(js, &size[1], height_val);
  JS_FreeValue(js, width_val);
  JS_FreeValue(js, height_val);
  if (isnan(size[0])) size[0] = 0;
  if (isnan(size[1])) size[1] = 0;
  lay_set_size_xy(lay, id, size[0], size[1]);
  
  return JS_UNDEFINED;
}

static JSValue js_layout_set_behave(JSContext *js, JSValueConst self, int argc, JSValueConst *argv)
{
  GETLAY
  GETITEM(id, argv[0])
  uint32_t behave_flags;
  JS_ToUint32(js, &behave_flags, argv[1]);
  lay_set_behave(lay, id, behave_flags);
  return JS_UNDEFINED;
}

static JSValue js_layout_set_contain(JSContext *js, JSValueConst self, int argc, JSValueConst *argv)
{
  GETLAY
  GETITEM(id, argv[0])
  uint32_t contain_flags;
  JS_ToUint32(js, &contain_flags, argv[1]);
  lay_set_contain(lay, id, contain_flags);
  return JS_UNDEFINED;
}

#define PULLMARGIN(DIR,MARGIN) \
JSValue js_##DIR = JS_GetPropertyStr(js, argv[1], #DIR); \
if (!JS_IsUndefined(js_##DIR)) { \
  JS_ToFloat64(js, &margins[MARGIN], js_##DIR); \
  if (isnan(margins[MARGIN])) margins[MARGIN] = 0; \
  JS_FreeValue(js, js_##DIR); \
}\

static JSValue js_layout_set_margins(JSContext *js, JSValueConst self, int argc, JSValueConst *argv)
{
  GETLAY
  GETITEM(id, argv[0])
  double margins[4] = {0};
  PULLMARGIN(l,0)
  PULLMARGIN(r,2)
  PULLMARGIN(t,1)
  PULLMARGIN(b,3)
  lay_set_margins_ltrb(lay, id, margins[0], margins[1], margins[2], margins[3]);
  return JS_UNDEFINED;
}

static JSValue js_layout_insert(JSContext *js, JSValueConst self, int argc, JSValueConst *argv) {
  GETLAY
  GETITEM(parent, argv[0])
  GETITEM(child, argv[1])
  lay_insert(lay, parent, child);
  return JS_UNDEFINED;
}

static JSValue js_layout_append(JSContext *js, JSValueConst self, int argc, JSValueConst *argv) {
  GETLAY
  GETITEM(earlier, argv[0])
  GETITEM(later, argv[1])
  lay_append(lay, earlier, later);
  return JS_UNDEFINED;
}

static JSValue js_layout_push(JSContext *js, JSValueConst self, int argc, JSValueConst *argv) {
  GETLAY
  GETITEM(parent, argv[0])
  GETITEM(child, argv[1])
  lay_push(lay, parent, child);
  return JS_UNDEFINED;
}

static JSValue js_layout_run(JSContext *js, JSValueConst self, int argc, JSValueConst *argv) {
  GETLAY
  lay_run_context(lay);
  return JS_UNDEFINED;
}

static JSValue js_layout_get_rect(JSContext *js, JSValueConst self, int argc, JSValueConst *argv) {
  GETLAY
  GETITEM(id, argv[0])
  lay_vec4 rect = lay_get_rect(lay, id);
  JSValue ret = JS_NewObject(js);
  JS_SetPropertyStr(js, ret, "x", JS_NewFloat64(js, rect[0]));
  JS_SetPropertyStr(js, ret, "y", JS_NewFloat64(js, rect[1]));
  JS_SetPropertyStr(js, ret, "width", JS_NewFloat64(js, rect[2]));
  JS_SetPropertyStr(js, ret, "height", JS_NewFloat64(js, rect[3]));    

  return ret;
}

static JSValue js_layout_reset(JSContext *js, JSValueConst self, int argc, JSValueConst *argv) {
  GETLAY
  lay_reset_context(lay);
  return JS_UNDEFINED;
}

static void js_layout_finalizer(JSRuntime *rt, JSValue val) {
  lay_context *ctx = JS_GetOpaque(val, js_layout_class_id);
  lay_destroy_context(ctx);
  js_free_rt(rt, ctx);
}

static JSClassDef js_layout_class = {
  "layout_context",
  .finalizer = js_layout_finalizer,
};

static const JSCFunctionListEntry js_layout_proto_funcs[] = {
  JS_CFUNC_DEF("item", 1, js_layout_item),
  JS_CFUNC_DEF("insert", 2, js_layout_insert),
  JS_CFUNC_DEF("append", 2, js_layout_append),
  JS_CFUNC_DEF("push", 2, js_layout_push),
  JS_CFUNC_DEF("run", 0, js_layout_run),
  JS_CFUNC_DEF("get_rect", 1, js_layout_get_rect),
  JS_CFUNC_DEF("reset", 0, js_layout_reset),
  JS_CFUNC_DEF("set_margins", 2, js_layout_set_margins),
  JS_CFUNC_DEF("set_size", 2, js_layout_set_size),
  JS_CFUNC_DEF("set_contain", 2, js_layout_set_contain),
  JS_CFUNC_DEF("set_behave", 2, js_layout_set_behave),
};

static const JSCFunctionListEntry js_layout_funcs[] = {
  JS_CFUNC_DEF("make_context", 0, js_layout_context_new)
};

JSValue js_layout_use(JSContext *js)
{
  JS_NewClassID(&js_layout_class_id);
  JS_NewClass(JS_GetRuntime(js), js_layout_class_id, &js_layout_class);

  JSValue proto = JS_NewObject(js);
  JS_SetPropertyFunctionList(js, proto, js_layout_proto_funcs, sizeof(js_layout_proto_funcs) / sizeof(JSCFunctionListEntry));
  JS_SetClassProto(js, js_layout_class_id, proto);

  JSValue contain = JS_NewObject(js);
  JSValue behave = JS_NewObject(js);
  JS_SetPropertyStr(js, contain, "row", JS_NewUint32(js, LAY_ROW));
  JS_SetPropertyStr(js, contain, "column", JS_NewUint32(js, LAY_COLUMN));
  JS_SetPropertyStr(js, contain, "layout", JS_NewUint32(js, LAY_LAYOUT));
  JS_SetPropertyStr(js, contain, "flex", JS_NewUint32(js, LAY_FLEX));
  JS_SetPropertyStr(js, contain, "wrap", JS_NewUint32(js, LAY_WRAP));
  JS_SetPropertyStr(js, contain, "nowrap", JS_NewUint32(js, LAY_NOWRAP));
    
  JS_SetPropertyStr(js, contain, "start", JS_NewUint32(js, LAY_START));
  JS_SetPropertyStr(js, contain, "middle", JS_NewUint32(js, LAY_MIDDLE));    
  JS_SetPropertyStr(js, contain, "end", JS_NewUint32(js, LAY_END));
  JS_SetPropertyStr(js, contain, "justify", JS_NewUint32(js, LAY_JUSTIFY));
    
  JS_SetPropertyStr(js, behave, "left", JS_NewUint32(js, LAY_LEFT));
  JS_SetPropertyStr(js, behave, "top", JS_NewUint32(js, LAY_TOP));
  JS_SetPropertyStr(js, behave, "right", JS_NewUint32(js, LAY_RIGHT));
  JS_SetPropertyStr(js, behave, "bottom", JS_NewUint32(js, LAY_BOTTOM));
  JS_SetPropertyStr(js, behave, "hfill", JS_NewUint32(js, LAY_HFILL));
  JS_SetPropertyStr(js, behave, "vfill", JS_NewUint32(js, LAY_VFILL));
  JS_SetPropertyStr(js, behave, "hcenter", JS_NewUint32(js, LAY_HCENTER));
  JS_SetPropertyStr(js, behave, "vcenter", JS_NewUint32(js, LAY_VCENTER));
  JS_SetPropertyStr(js, behave, "center", JS_NewUint32(js, LAY_CENTER));    
  JS_SetPropertyStr(js, behave, "fill", JS_NewUint32(js, LAY_FILL));
  JS_SetPropertyStr(js, behave, "break", JS_NewUint32(js, LAY_BREAK));

  JSValue export = JS_NewObject(js);
  JS_SetPropertyFunctionList(js, export, js_layout_funcs, sizeof(js_layout_funcs)/sizeof(JSCFunctionListEntry));
  JS_SetPropertyStr(js, export, "behave", behave);
  JS_SetPropertyStr(js, export, "contain", contain);  

  return export;
}

static int js_layout_init(JSContext *js, JSModuleDef *m)
{
  JSValue export =js_layout_use(js);
  JS_SetModuleExport(js, m, "default", export);
  return 0;
}

#ifdef JS_SHARED_LIBRARY
#define JS_INIT_MODULE js_init_module
#else
#define JS_INIT_MODULE js_init_module_layout
#endif

JSModuleDef *JS_INIT_MODULE(JSContext *js, const char *module_name) {
    JSModuleDef *m = JS_NewCModule(js, module_name, js_layout_init);
    if (!m) return NULL;
    JS_AddModuleExport(js, m, "default");

    return m;
}
