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

# --- Start: Asset Copying ---
# This is a compatible way to copy a directory during the build process.
# It uses run_command() at configure time to find all asset files.
# NOTE: If you add or remove files from the 'assets' directory, you must
# re-run meson configure (e.g., 'meson setup --reconfigure builddir').

# At configuration time, find all files in the assets directory using the 'find' command.
# This command is run once when you set up the build directory.
asset_list_cmd = run_command('find', 'assets', '-type', 'f', check: true)
# The output of the command is split into a list of file paths.
asset_files = asset_list_cmd.stdout().strip().split('\n')

# Then, we define the custom target.
copy_assets = custom_target('copy_assets',
                            # The command will be triggered if any file in the `input` list changes.
                            input : asset_files,
                            # This defines the output directory name within the build directory.
                            output : 'assets',
                            # The command to execute. We use a shell command for a robust copy.
                            # 1. `rm -rf @OUTPUT@`: Removes the old assets directory to ensure a clean state.
                            # 2. `cp -r ...`: Copies the source assets directory to the output location.
                            #    `@SOURCE_ROOT@` is the path to your project's root source directory.
                            #    The '/.' at the end of 'assets/.' is important: it copies the *contents*
                            #    of the assets directory, not the directory itself, preventing `build/assets/assets`.
                            command : ['sh', '-c', 'rm -rf @OUTPUT@ && cp -r @SOURCE_ROOT@/assets/. @OUTPUT@'],
                            # This ensures the target is built as part of the default build process (e.g., when you run `meson compile`).
                            build_by_default : true,
)
# --- End: Asset Copying ---


src = files(
    'src' / 'glad.c',
    'src' / 'graphics.c',
    'src' / 'assets.c',
    'src' / 'gui.c',
    'src' / 'ecs.c',
    'src' / 'input.c',
    'src' / 'camera.c'
)

inc = include_directories('include')

executable(
    'aurion',
    # The custom_target 'copy_assets' is added to the list of sources.
    # This tells Meson that 'copy_assets' must be built before 'aurion'.
    [src, 'src' / 'sandbox.c', copy_assets],
    dependencies: [sdl_dep, cglm_dep, flecs_dep],
    include_directories: inc,
    install: false,
)
e = executable('run_test', 'src' / 'test_ecs.c', include_directories: inc, dependencies: [flecs_dep,cglm_dep])
test('test', e)
