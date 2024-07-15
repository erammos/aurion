project(
  'aurion',
  ['c'],
  version: '0.1',
  default_options: ['warning_level=3', 'c_std=c23'],
)
sdl_dep = dependency('SDL2')
cglm_dep = dependency('cglm')
freetype_dep = dependency('freetype')

unity_subproject = subproject('Unity')
unity_dependency = unity_subproject.get_variable('unity_dep')
unity_gen_runner = unity_subproject.get_variable('gen_test_runner')

src = files(
  'src' / 'main.c',
  'src' / 'glad.c',
  'src' / 'graphics.c',
  'src' / 'assets.c'
)
assets_src = 'assets'       # Path to your assets folder
build_assets_dir = 'build/assets'  # Destination path in the build directory

# Define a custom command to copy assets during build
copy_assets_command = [
    'cp', '-rf', # Adjust options as needed
    meson.current_source_dir() + '/' + assets_src + '/',
    meson.current_build_dir() + '/' + assets_src + '/'
]
run_command(copy_assets_command,check:true)

inc = include_directories('include')

executable(
  'aurion',
  src,
  dependencies: [sdl_dep,cglm_dep,freetype_dep],
  include_directories: inc,
  install: true,
)

executable('text-atlas', 'src' / 'text_app.c',dependencies: [freetype_dep])
