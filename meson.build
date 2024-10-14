project(
  'aurion',
  ['c'],
  version: '0.1',
  default_options: ['warning_level=3', 'c_std=c2x'],
)

sdl_dep = dependency('SDL2')
flecs_dep = dependency('flecs')
cglm_dep = dependency('cglm')

unity_subproject = subproject('Unity')
unity_dependency = unity_subproject.get_variable('unity_dep')
unity_gen_runner = unity_subproject.get_variable('gen_test_runner')

src = files(
  'src' / 'glad.c',
  'src' / 'graphics.c',
  'src' / 'assets.c',
      'src' / 'gui.c',
      'src' / 'world.c',
      'src' / 'input.c',
      'src' / 'camera.c'
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
  [src, 'src' / 'sandbox.c'],
  dependencies: [sdl_dep,cglm_dep,flecs_dep],
  include_directories: inc,
  install: false,
)
e = executable('run_test', 'src' / 'test_ecs.c', include_directories: inc, dependencies: [flecs_dep,cglm_dep])
test('test', e)

