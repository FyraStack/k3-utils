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

gpiodcxx = dependency('libgpiodcxx', method: 'pkg-config', version: '>= 2.0.0')

sources = files(
  'main.cpp',
  'configuration.cpp',
  'relay_controller.cpp',
  'http_server.cpp',
)

executable(
  'relay-gpio',
  sources,
  dependencies: gpiodcxx,
  install: true,
)

clang_format = find_program('clang-format', required: false)
if clang_format.found()
  run_target(
    'format',
    command: [clang_format, '-i'] + sources,
  )
endif

clang_tidy = find_program('clang-tidy', required: false)
python3 = find_program('python3', required: false)
if clang_tidy.found()
  run_target(
    'clang-tidy',
    command: [clang_tidy, '-p', meson.current_build_dir(), '--warnings-as-errors=*'] + sources,
  )
  if python3.found()
    run_target(
      'clang-tidy-sarif',
      command: [
        python3,
        files('clang_tidy_sarif.py'),
        '--clang-tidy', clang_tidy,
        '--build-dir', meson.current_build_dir(),
        '--source-root', meson.current_source_dir(),
        '--output', meson.current_build_dir() / 'clang-tidy.sarif',
      ] + sources,
    )
  endif
endif
