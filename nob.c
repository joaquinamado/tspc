#include <stddef.h>
#define NOB_IMPLEMENTATION
#include "nob.h"
#define FLAG_IMPLEMENTATION
#include "flag.h"

Cmd cmd = {0};

static void usage(void) {
  fprintf(stderr, "Usage: %s [<FLAGS>] [--] [<program args>]\n",
          flag_program_name());
  fprintf(stderr, "FLAGS:\n");
  flag_print_options(stderr);
}

int main(int argc, char **argv) {
  GO_REBUILD_URSELF(argc, argv);

  bool run = false;
  bool help = false;
  flag_bool_var(&run, "run", false, "Run the program after compilation.");
  flag_bool_var(&help, "help", false, "Print this help message.");

  if (!flag_parse(argc, argv)) {
    usage();
    flag_print_error(stderr);
    return 1;
  }

  if (help) {
    usage();
    return 0;
  }

  Nob_File_Paths src_files = {0};
  if (!nob_read_entire_dir("src", &src_files))
    return 1;

  for (size_t i = 0; i < src_files.count; i++) {
    const char *src = src_files.items[i];
    if (!nob_sv_end_with(nob_sv_from_cstr(src), ".c"))
      continue;

    const char *src_path = nob_temp_sprintf("src/%s", src);
    const char *obj_path =
        nob_temp_sprintf("src/%.*s.o", (int)(strlen(src) - 2), src);

    cmd.count = 0;
    cmd_append(&cmd, "cc");
    cmd_append(&cmd, "-Wall", "-Wextra", "-ggdb");
    cmd_append(&cmd, "-I./raylib-5.5_linux_amd64/include/");
    cmd_append(&cmd, "-I./src/");
    cmd_append(&cmd, "-c", src_path);
    cmd_append(&cmd, "-o", obj_path);
    if (!cmd_run(&cmd))
      return 1;
  }

  cmd.count = 0;
  cmd_append(&cmd, "cc");
  cmd_append(&cmd, "-ggdb");
  cmd_append(&cmd, "-Wall");
  cmd_append(&cmd, "-Wextra");
  cmd_append(&cmd, "-I./raylib-5.5_linux_amd64/include/");

  Nob_File_Paths obj_files = {0};
  if (!nob_read_entire_dir("src", &obj_files))
    return 1;
  for (size_t i = 0; i < obj_files.count; i++) {
    const char *f = obj_files.items[i];
    if (!nob_sv_end_with(nob_sv_from_cstr(f), ".o"))
      continue;
    cmd_append(&cmd, nob_temp_sprintf("src/%s", f));
  }

  cmd_append(&cmd, "-o", "./main", "main.c");
  cmd_append(&cmd, "-L./raylib-5.5_linux_amd64/lib/");
  cmd_append(&cmd, "-l:libraylib.a");
  cmd_append(&cmd, "-lm");

  if (!cmd_run(&cmd))
    return 1;

  if (run) {
    cmd_append(&cmd, "./main");
    da_append_many(&cmd, argv, argc);
    if (!cmd_run(&cmd))
      return 1;
  }

  return 0;
}
