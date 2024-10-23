project('layout', 'c')

quickjs = dependency('quickjs', fallback:['quickjs', 'quickjs_dep'])

libtype = get_option('default_library')
if libtype == 'static'
  add_project_arguments('-DJS_SHARED_LIBRARY', language:'c')
endif

layout_lib = library('layout',
  'qjs_layout.c',
  dependencies: [quickjs]
)

qjs_layout_dep = declare_dependency(
  link_with: layout_lib,
)
