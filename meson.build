project('layout', 'c')

quickjs = dependency('quickjs')

layout_lib = library('layout',
  'qjs_layout.c',
  dependencies: [quickjs],
  c_shared_args: '-DJS_SHARED_LIBRARY',
)

qjs_layout_dep = declare_dependency(
  link_with: layout_lib,
)

lay_bench = executable('bench',
  'benchmark_layout.c'
)

lay_test = executable('lay_test',
  'test_layout.c'
)

test('test suite', lay_test)
benchmark('standard bench', lay_bench)