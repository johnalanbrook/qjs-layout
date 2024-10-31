project('layout', 'c')

quickjs = dependency('quickjs', fallback:['quickjs', 'quickjs_dep'])

layout_lib = library('layout',
  'qjs_layout.c',
  dependencies: [quickjs],
  c_shared_args: '-DJS_SHARED_LIBRARY',
)

qjs_layout_dep = declare_dependency(
  link_with: layout_lib,
)
