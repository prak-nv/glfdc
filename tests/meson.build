catch2_dep = dependency('catch2')

unittest_srcs = [
  'test_build.cc',
  'test_eval.cc',
  'test_stack.cc',
]

unittest_srcs += ['unit_runner.cc']

glfdc_tests_exe = executable('glfdc_unittests',
  unittest_srcs,
  include_directories: ['../'],
  dependencies: [catch2_dep],
  link_with: libglfdc)

test('glfd_unittests', glfdc_tests_exe)

