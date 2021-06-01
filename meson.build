project('glfdc', ['c', 'cpp'], default_options: ['cpp_std=c++17'])

cxx = meson.get_compiler('cpp')

m_dep = cxx.find_library('m', required: false)
rt_dep = cxx.find_library('rt', required: false)

libglfdc = library('glfdc', [
    'base26.cc',
    'bitvector.cc',
    'cfold.cc',
    'eval.cc',
    'expr.cc',
    'expr_builder.cc',
    'sexpr.cc',
    'sexpr_cmp.cc',
    'sparse_map.cc',
    'stack.cc'
  ],
  dependencies: [m_dep, rt_dep]
)

exe = executable('glfdc', ['glfdc.cc'], link_with: [libglfdc])

subdir('tests')
