#include "build.h"

char *dirname(char *path);

#ifdef DOS_HOST
#define PATHSEPARATOR '\\'
#define PATHSEPARATORSTR "\\"
#else
#ifdef WINDOWS_HOST
#define PATHSEPARATOR '\\'
#define PATHSEPARATORSTR "\\"
#else
#define PATHSEPARATOR '/'
#define PATHSEPARATORSTR "/"
#endif
#endif

// --== Configuration state (CLI + env overrides) ==--
static char cfg_sdk_path[1024] = "";
static char cfg_asm_path[1024] = "";
static char cfg_link_path[1024] = "";
static char cfg_lib_path[1024] = "";
static char cfg_output_dir[1024] = "";
static bool cfg_cli_verbose = false;
static bool cfg_cli_dump = false;
static bool cfg_cli_keep = false;
static include_path_t *cfg_cli_include_paths = NULL;

obj_t *_objs = NULL;
obj_t *_last_obj = NULL;
char *_path = "";

void remove_ext(char *filename)
{
    char *last_dot = strrchr(filename, '.');
    if (last_dot != NULL && last_dot != filename) {
        *last_dot = '\0';
    }
}

char *get_ext(char *filename)
{
    char *last_dot = strrchr(filename, '.');
    if (last_dot != NULL && last_dot != filename) {
        return last_dot;
    }
    return "";
}

static void append_include_path(const char *path)
{
    include_path_t *node = malloc(sizeof(include_path_t));
    strncpy(node->path, path, sizeof(node->path) - 1);
    node->path[sizeof(node->path) - 1] = '\0';
    node->next = cfg_cli_include_paths;
    cfg_cli_include_paths = node;
}

static void resolve_config(void)
{
    char *env;

    if (!cfg_sdk_path[0]) {
        env = getenv("HC_SDK_PATH");
        if (env) {
            strncpy(cfg_sdk_path, env, sizeof(cfg_sdk_path) - 1);
            cfg_sdk_path[sizeof(cfg_sdk_path) - 1] = '\0';
        } else {
            const char *prj = get_value("config", "", "sdk_path");
            strncpy(cfg_sdk_path, prj, sizeof(cfg_sdk_path) - 1);
            cfg_sdk_path[sizeof(cfg_sdk_path) - 1] = '\0';
        }
    }

    if (!cfg_asm_path[0] && cfg_sdk_path[0]) {
        strncpy(cfg_asm_path, cfg_sdk_path, sizeof(cfg_asm_path) - 1);
        cfg_asm_path[sizeof(cfg_asm_path) - 1] = '\0';
    }
    if (!cfg_link_path[0] && cfg_sdk_path[0]) {
        strncpy(cfg_link_path, cfg_sdk_path, sizeof(cfg_link_path) - 1);
        cfg_link_path[sizeof(cfg_link_path) - 1] = '\0';
    }
    if (!cfg_lib_path[0] && cfg_sdk_path[0]) {
        strncpy(cfg_lib_path, cfg_sdk_path, sizeof(cfg_lib_path) - 1);
        cfg_lib_path[sizeof(cfg_lib_path) - 1] = '\0';
    }

    if (!cfg_output_dir[0]) {
        env = getenv("HC_OUTPUT_DIR");
        if (env) {
            strncpy(cfg_output_dir, env, sizeof(cfg_output_dir) - 1);
            cfg_output_dir[sizeof(cfg_output_dir) - 1] = '\0';
        } else {
            const char *prj = get_value("config", "", "output_dir");
            strncpy(cfg_output_dir, prj, sizeof(cfg_output_dir) - 1);
            cfg_output_dir[sizeof(cfg_output_dir) - 1] = '\0';
        }
    }
}

static void cmd_puts(char *buf, size_t size, const char *s)
{
    strncat(buf, s, size - strlen(buf) - 1);
}

static void cmd_put_base(char *buf, size_t size, const char *base)
{
    if (base && base[0]) {
        cmd_puts(buf, size, base);
        size_t len = strlen(buf);
        if (len > 0 && buf[len-1] != PATHSEPARATOR && len + 1 < size) {
            buf[len] = PATHSEPARATOR;
            buf[len+1] = '\0';
        }
    }
}

static void cmd_put_tool(char *buf, size_t size, const char *s)
{
    cmd_puts(buf, size, s);
    #if defined(DOS_HOST) || defined(WINDOWS_HOST)
    cmd_puts(buf, size, ".exe");
    #endif
}

static void resolve_output_path(char *out, size_t out_size, const char *filename)
{
    if (filename[0] == PATHSEPARATOR) {
        strncpy(out, filename, out_size - 1);
        out[out_size - 1] = '\0';
        return;
    }
    const char *base = cfg_output_dir[0] ? cfg_output_dir : _path;
    int n = snprintf(out, out_size, "%s", base);
    if (n > 0 && out[n-1] != PATHSEPARATOR && (size_t)n + 1 < out_size) {
        out[n] = PATHSEPARATOR;
        out[n+1] = '\0';
    }
    strncat(out, filename, out_size - strlen(out) - 1);
}

static void resolve_source_path(char *out, size_t out_size, const char *filename)
{
    if (filename[0] != PATHSEPARATOR) {
        int n = snprintf(out, out_size, "%s%s%s",
            _path,
            (*_path && _path[strlen(_path)-1] != PATHSEPARATOR) ? PATHSEPARATORSTR : "",
            filename);
        if (n < 0 || (size_t)n >= out_size) {
            out[out_size - 1] = '\0';
        }
    } else {
        strncpy(out, filename, out_size - 1);
        out[out_size - 1] = '\0';
    }
}

void help(void)
{
    printf("HC Builder for Retro Computing v%d.%d R%d\n", VERSION, SUBVERSION, REVISION);
    printf("HC Software Development Kit for Retro Computing\n");
    printf("Copyright (c) 2025,2026 Humberto Costa dos Santos Junior\n\n");
    printf("Usage: hcbuild [options] <project.prj> <command> [config]\n");
    printf("Commands:\n");
    printf("  make           Build project configuration\n");
    printf("  clean          Remove project output files\n");
    printf("Options:\n");
    printf("  --sdk-path <dir>     Set SDK tools directory\n");
    printf("  --asm-path <dir>     Set assembler directory\n");
    printf("  --link-path <dir>    Set linker directory\n");
    printf("  --lib-path <dir>     Set librarian directory\n");
    printf("  --output-dir <dir>   Set output directory\n");
    printf("  -I <dir>             Add include path (RetroLang)\n");
    printf("  --verbose, -v        Enable verbose output\n");
    printf("  --dump               Generate assembly dump files\n");
    printf("  --keep               Keep intermediate files\n");
    printf("  --help, -h           Show this help\n");
    printf("\nEnvironment variables:\n");
    printf("  HC_SDK_PATH          SDK tools directory\n");
    printf("  HC_OUTPUT_DIR        Output directory\n");
    exit(1);
}

void make_files(section_t *section)
{
    int st;
    char cmd[24000];
    char source_name[2048];
    char temp_name[2048];
    char obj_name[2048];
    char dump_name[2048];
    bool ok = false;
    bool verbose = cfg_cli_verbose || get_value_bool("config", "", "verbose");
    bool dump = cfg_cli_dump || get_value_bool("config", "", "dump");
    bool keep = cfg_cli_keep || get_value_bool("config", "", "keep");
    const char *asm_dir = cfg_asm_path[0] ? cfg_asm_path : cfg_sdk_path;

    if (!section) return;
    keyvalue_t *kv = section->keys;
    while (kv) {
        ok = false;
        resolve_source_path(source_name, sizeof(source_name), kv->key);

        snprintf(obj_name, sizeof(obj_name), "%s", source_name);
        remove_ext(obj_name);
        strncat(obj_name, ".obj", sizeof(obj_name) - strlen(obj_name) - 1);

        snprintf(dump_name, sizeof(dump_name), "%s", source_name);
        remove_ext(dump_name);
        strncat(dump_name, ".dump", sizeof(dump_name) - strlen(dump_name) - 1);

        const char *ext = get_ext(source_name);
        if (!strcmp(ext, ".__s")) {
            fprintf(stderr, "error: invalid file extension: %s\n", source_name);
            exit(1);
        }

        if (!strcmp(ext, ".rl") || !strcmp(ext, ".RL")) {
            ok = true;
            snprintf(temp_name, sizeof(temp_name), "%s", source_name);
            remove_ext(temp_name);
            strncat(temp_name, ".__s", sizeof(temp_name) - strlen(temp_name) - 1);

            cmd[0] = '\0';
            cmd_put_base(cmd, sizeof(cmd), cfg_sdk_path);
            #ifdef DOS_HOST
            cmd_puts(cmd, sizeof(cmd), "rlang");
            if (section->subsection[0] == '8' && section->subsection[1] == '0')
                cmd_puts(cmd, sizeof(cmd), &section->subsection[2]);
            else
                cmd_puts(cmd, sizeof(cmd), section->subsection);
            cmd_put_tool(cmd, sizeof(cmd), "");
            #else
            cmd_puts(cmd, sizeof(cmd), "retrolang-");
            cmd_puts(cmd, sizeof(cmd), section->subsection);
            cmd_put_tool(cmd, sizeof(cmd), "");
            #endif

            for (include_path_t *inc = cfg_cli_include_paths; inc; inc = inc->next) {
                snprintf(cmd + strlen(cmd), sizeof(cmd) - strlen(cmd), " -I %s", inc->path);
            }
            section_t *rl_include_path = get_section("retrolang", "include_path");
            if (rl_include_path) {
                keyvalue_t *ikv = rl_include_path->keys;
                while (ikv) {
                    snprintf(cmd + strlen(cmd), sizeof(cmd) - strlen(cmd), " -I %s", ikv->key);
                    ikv = ikv->next;
                }
            }
            snprintf(cmd + strlen(cmd), sizeof(cmd) - strlen(cmd), " -o %s %s", temp_name, source_name);

            if (verbose) printf("%s\n", cmd);
            st = system(cmd);
            if (st) exit(-1);
            strncpy(source_name, temp_name, sizeof(source_name) - 1);
        }

        if (!strcmp(ext, ".b") || !strcmp(ext, ".B")) {
            ok = true;
            snprintf(temp_name, sizeof(temp_name), "%s", source_name);
            remove_ext(temp_name);
            strncat(temp_name, ".__s", sizeof(temp_name) - strlen(temp_name) - 1);

            cmd[0] = '\0';
            cmd_put_base(cmd, sizeof(cmd), asm_dir);
            #ifdef DOS_HOST
            cmd_puts(cmd, sizeof(cmd), "bcomp");
            if (section->subsection[0] == '8' && section->subsection[1] == '0')
                cmd_puts(cmd, sizeof(cmd), &section->subsection[2]);
            else
                cmd_puts(cmd, sizeof(cmd), section->subsection);
            cmd_put_tool(cmd, sizeof(cmd), "");
            #else
            cmd_puts(cmd, sizeof(cmd), "hcbcomp-");
            cmd_puts(cmd, sizeof(cmd), section->subsection);
            cmd_put_tool(cmd, sizeof(cmd), "");
            #endif

            snprintf(cmd + strlen(cmd), sizeof(cmd) - strlen(cmd), " -o %s %s", temp_name, source_name);

            if (verbose) printf("%s\n", cmd);
            st = system(cmd);
            if (st) exit(-1);
            strncpy(source_name, temp_name, sizeof(source_name) - 1);
        }

        ext = get_ext(source_name);
        if (!strcmp(ext, ".s") || !strcmp(ext, ".S") || !strcmp(ext, ".__s")) {
            ok = true;
            cmd[0] = '\0';
            cmd_put_base(cmd, sizeof(cmd), asm_dir);
            #ifdef DOS_HOST
            cmd_puts(cmd, sizeof(cmd), "hcasm");
            if (section->subsection[0] == '8' && section->subsection[1] == '0')
                cmd_puts(cmd, sizeof(cmd), &section->subsection[2]);
            else
                cmd_puts(cmd, sizeof(cmd), section->subsection);
            cmd_put_tool(cmd, sizeof(cmd), "");
            #else
            cmd_puts(cmd, sizeof(cmd), "hcasm-");
            cmd_puts(cmd, sizeof(cmd), section->subsection);
            cmd_put_tool(cmd, sizeof(cmd), "");
            #endif

            snprintf(cmd + strlen(cmd), sizeof(cmd) - strlen(cmd), " -o %s", obj_name);
            if (dump) {
                snprintf(cmd + strlen(cmd), sizeof(cmd) - strlen(cmd), " -dump %s", dump_name);
            }
            snprintf(cmd + strlen(cmd), sizeof(cmd) - strlen(cmd), " %s", source_name);

            if (verbose) printf("%s\n", cmd);
            st = system(cmd);
            if (st) exit(-1);

            obj_t *obj = malloc(sizeof(obj_t) + strlen(obj_name));
            strcpy(obj->name, obj_name);
            obj->next = NULL;
            if (_last_obj)
                _last_obj->next = obj;
            else
                _objs = obj;
            _last_obj = obj;
        }

        if (!strcmp(ext, ".__s")) {
            if (!keep) remove(source_name);
        }

        if (!ok) {
            fprintf(stderr, "error: extension not supported: %s\n", source_name);
            exit(1);
        }
        kv = kv->next;
    }
}

void clean_files(section_t *section)
{
    char obj_name[1024];
    char file_name[1024];
    if (!section) return;
    bool dump = cfg_cli_dump || get_value_bool("config", "", "dump");
    keyvalue_t *kv = section->keys;
    while (kv) {
        resolve_source_path(file_name, sizeof(file_name), kv->key);
        strncpy(obj_name, file_name, sizeof(obj_name) - 1);
        obj_name[sizeof(obj_name) - 1] = '\0';
        remove_ext(obj_name);
        strncat(obj_name, ".obj", sizeof(obj_name) - strlen(obj_name) - 1);
        remove(obj_name);
        if (dump) {
            remove_ext(obj_name);
            strncat(obj_name, ".dump", sizeof(obj_name) - strlen(obj_name) - 1);
            remove(obj_name);
        }
        kv = kv->next;
    }
}

void make_link(section_t *section, char *config)
{
    int st;
    char *cmd;
    char *out_file = NULL;
    char *sym_file = NULL;
    char *format = NULL;
    char *text_offset = NULL;
    char *data_offset = NULL;
    char *bss_offset = NULL;
    char *align = NULL;
    char *stack_size = NULL;
    char output_path[2048];
    const char *link_dir = cfg_link_path[0] ? cfg_link_path : cfg_sdk_path;
    const char *lib_dir = cfg_lib_path[0] ? cfg_lib_path : cfg_sdk_path;
    bool verbose = cfg_cli_verbose || get_value_bool("config", "", "verbose");

    if (!section) {
        fprintf(stderr, "error: link configuration not found: %s\n", config);
        exit(1);
    }

    out_file = get_value(section->name, section->subsection, "filename");
    sym_file = get_value(section->name, section->subsection, "symbols");
    format = get_value(section->name, section->subsection, "format");
    text_offset = get_value(section->name, section->subsection, "text");
    data_offset = get_value(section->name, section->subsection, "data");
    bss_offset = get_value(section->name, section->subsection, "bss");
    align = get_value(section->name, section->subsection, "align");
    stack_size = get_value(section->name, section->subsection, "stack");
    if (strlen(format) == 0) format = "bin";
    if (strlen(out_file) == 0) out_file = "a.out";

    resolve_output_path(output_path, sizeof(output_path), out_file);

    obj_t *obj = _objs;
    size_t cmd_size = 8192;
    cmd_size += strlen(output_path) + strlen(sym_file) + strlen(text_offset)
              + strlen(data_offset) + strlen(bss_offset) + strlen(align) + strlen(stack_size) + 256;
    while (obj) {
        cmd_size += strlen(obj->name) + 4;
        obj = obj->next;
    }
    cmd = malloc(cmd_size);

    if (!strcmp(format, "lib")) {
        cmd[0] = '\0';
        cmd_put_base(cmd, cmd_size, lib_dir);
        cmd_puts(cmd, cmd_size, "hclib");
        cmd_put_tool(cmd, cmd_size, "");
        cmd_puts(cmd, cmd_size, " ");
        cmd_puts(cmd, cmd_size, output_path);
    } else {
        cmd[0] = '\0';
        cmd_put_base(cmd, cmd_size, link_dir);
        #ifdef DOS_HOST
        cmd_puts(cmd, cmd_size, "hclnk");
        cmd_puts(cmd, cmd_size, format);
        cmd_put_tool(cmd, cmd_size, "");
        #else
        cmd_puts(cmd, cmd_size, "hclink-");
        cmd_puts(cmd, cmd_size, format);
        cmd_put_tool(cmd, cmd_size, "");
        #endif

        snprintf(cmd + strlen(cmd), cmd_size - strlen(cmd), " -o %s", output_path);

        if (strlen(text_offset)) {
            snprintf(cmd + strlen(cmd), cmd_size - strlen(cmd), " -text %s", text_offset);
        }
        if (strlen(data_offset)) {
            snprintf(cmd + strlen(cmd), cmd_size - strlen(cmd), " -data %s", data_offset);
        }
        if (strlen(bss_offset)) {
            snprintf(cmd + strlen(cmd), cmd_size - strlen(cmd), " -bss %s", bss_offset);
        }
        if (strlen(align)) {
            snprintf(cmd + strlen(cmd), cmd_size - strlen(cmd), " -align %s", align);
        }
        if (strlen(sym_file)) {
            snprintf(cmd + strlen(cmd), cmd_size - strlen(cmd), " -sym %s", sym_file);
        }
        if (strlen(stack_size)) {
            snprintf(cmd + strlen(cmd), cmd_size - strlen(cmd), " -stack %s", stack_size);
        }
    }

    obj = _objs;
    while (obj) {
        snprintf(cmd + strlen(cmd), cmd_size - strlen(cmd), " %s", obj->name);
        obj = obj->next;
    }

    if (verbose) printf("%s\n", cmd);
    st = system(cmd);
    if (st) exit(-1);
    free(cmd);
}

void clean_link(section_t *section, char *config)
{
    char *out_file = NULL;
    char output_path[2048];
    if (!section) {
        fprintf(stderr, "error: link configuration not found: %s\n", config);
        exit(1);
    }
    out_file = get_value(section->name, section->subsection, "filename");
    if (strlen(out_file) == 0) out_file = "a.out";
    resolve_output_path(output_path, sizeof(output_path), out_file);
    remove(output_path);

    out_file = get_value(section->name, section->subsection, "symbols");
    if (strlen(out_file)) {
        resolve_output_path(output_path, sizeof(output_path), out_file);
        remove(output_path);
    }
}

void make_libs(section_t *section)
{
    char obj_name[2048];
    if (!section) return;
    keyvalue_t *kv = section->keys;
    while (kv) {
        resolve_source_path(obj_name, sizeof(obj_name), kv->key);
        obj_t *obj = malloc(sizeof(obj_t) + strlen(obj_name));
        strcpy(obj->name, obj_name);
        obj->next = NULL;
        if (_last_obj)
            _last_obj->next = obj;
        else
            _objs = obj;
        _last_obj = obj;
        kv = kv->next;
    }
}

static void parse_args(int argc, char **argv, char **project_file, char **command, char **config)
{
    int i;
    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--sdk-path") || !strcmp(argv[i], "-s")) {
            if (++i >= argc) { fprintf(stderr, "error: --sdk-path requires a path\n"); help(); }
            strncpy(cfg_sdk_path, argv[i], sizeof(cfg_sdk_path) - 1);
            cfg_sdk_path[sizeof(cfg_sdk_path) - 1] = '\0';
        }
        else if (!strcmp(argv[i], "--asm-path")) {
            if (++i >= argc) { fprintf(stderr, "error: --asm-path requires a path\n"); help(); }
            strncpy(cfg_asm_path, argv[i], sizeof(cfg_asm_path) - 1);
            cfg_asm_path[sizeof(cfg_asm_path) - 1] = '\0';
        }
        else if (!strcmp(argv[i], "--link-path")) {
            if (++i >= argc) { fprintf(stderr, "error: --link-path requires a path\n"); help(); }
            strncpy(cfg_link_path, argv[i], sizeof(cfg_link_path) - 1);
            cfg_link_path[sizeof(cfg_link_path) - 1] = '\0';
        }
        else if (!strcmp(argv[i], "--lib-path")) {
            if (++i >= argc) { fprintf(stderr, "error: --lib-path requires a path\n"); help(); }
            strncpy(cfg_lib_path, argv[i], sizeof(cfg_lib_path) - 1);
            cfg_lib_path[sizeof(cfg_lib_path) - 1] = '\0';
        }
        else if (!strcmp(argv[i], "--output-dir")) {
            if (++i >= argc) { fprintf(stderr, "error: --output-dir requires a path\n"); help(); }
            strncpy(cfg_output_dir, argv[i], sizeof(cfg_output_dir) - 1);
            cfg_output_dir[sizeof(cfg_output_dir) - 1] = '\0';
        }
        else if (!strcmp(argv[i], "-I")) {
            if (++i >= argc) { fprintf(stderr, "error: -I requires a path\n"); help(); }
            append_include_path(argv[i]);
        }
        else if (!strcmp(argv[i], "--verbose") || !strcmp(argv[i], "-v")) {
            cfg_cli_verbose = true;
        }
        else if (!strcmp(argv[i], "--dump")) {
            cfg_cli_dump = true;
        }
        else if (!strcmp(argv[i], "--keep")) {
            cfg_cli_keep = true;
        }
        else if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h")) {
            help();
        }
        else if (argv[i][0] == '-') {
            fprintf(stderr, "error: unknown option: %s\n", argv[i]);
            help();
        }
        else {
            if (!*project_file) *project_file = argv[i];
            else if (!*command) *command = argv[i];
            else if (!*config) *config = argv[i];
            else {
                fprintf(stderr, "error: unexpected argument: %s\n", argv[i]);
                help();
            }
        }
    }
}

int main(int argc, char **argv)
{
    char *project_file = NULL;
    char *command = NULL;
    char *config = NULL;

    parse_args(argc, argv, &project_file, &command, &config);

    if (!project_file || !command) help();
    if (!config) config = "release";

    cfg_process(project_file);
    resolve_config();

    _path = strdup(project_file);
    _path = dirname(_path);

    if (!strcmp(command, "make")) {
        make_libs(get_section("lib", "start"));
        make_libs(get_section("libs", "start"));
        make_files(get_section("files", "8080"));
        make_files(get_section("files", "8085"));
        make_files(get_section("files", "8086"));
        make_files(get_section("files", "z80"));
        make_libs(get_section("lib", ""));
        make_libs(get_section("libs", ""));
        make_link(get_section("link", config), config);
    }
    else if (!strcmp(command, "clean")) {
        clean_files(get_section("files", "8080"));
        clean_files(get_section("files", "8085"));
        clean_files(get_section("files", "8086"));
        clean_files(get_section("files", "z80"));
        clean_link(get_section("link", config), config);
    }
    else {
        fprintf(stderr, "error: hcbuild command unknown: %s\n", command);
        return 1;
    }
    return 0;
}
