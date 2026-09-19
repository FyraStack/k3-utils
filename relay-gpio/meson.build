project(
  'relay-gpio',
  'cpp',
  version: '0.1.0',
  default_options: [
    'cpp_std=c++17',
    'warning_level=3',
  ],
)

mold = find_program('mold', required: false)
if mold.found()
  add_project_link_arguments('-fuse-ld=mold', language: 'cpp')
endif

gpiodcxx = dependency('libgpiodcxx', method: 'pkg-config')

executable(
  'relay-gpio',
  'main.cpp',
    'configuration.cpp',
      'relay_controller.cpp',
    'http_server.cpp',
  dependencies: gpiodcxx,
  install: true,
)

install_subdir(
  'public',
  install_dir: get_option('datadir') / 'relay-gpio',
)
