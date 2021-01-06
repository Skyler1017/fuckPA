#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);

void isa_reg_display();

void print_wp();

WP *new_wp();

int free_wp(int);

static int cmd_help(char *args);

static int cmd_c(char *args);

static int cmd_q(char *args);

static int cmd_si(char *args);

static int cmd_info(char *args);

static int cmd_x(char *args);

static int cmd_p(char *args);

static int cmd_w(char *args);

static int cmd_d(char *args);

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char *rl_gets() {
    static char *line_read = NULL;

    if (line_read) {
        free(line_read);
        line_read = NULL;
    }

    line_read = readline("(nemu) ");

    if (line_read && *line_read) {
        add_history(line_read);
    }

    return line_read;
}

static struct {
    char *name;
    char *description;

    int (*handler)(char *);
} cmd_table[] = {
        {"help", "Display informations about all supported commands", cmd_help},
        {"c",    "Continue the execution of the program",             cmd_c},
        {"q",    "Exit NEMU",                                         cmd_q},
        {"si",   "Continue the execution of the program in N steps",  cmd_si},
        {"info", "Print all registers",                               cmd_info},
        {"x",    "Scan the memory",                                   cmd_x},
        {"p",    "Calculate the value of expression",                 cmd_p},
        {"w",    "Add a watchpoint",                                  cmd_w},
        {"d",    "Delete a watchpoint",                               cmd_d}
};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
        if (strcmp(cmd, cmd_table[i].name) == 0) {
            if (cmd_table[i].handler(args) < 0) { return; }
            break;
        }
    }

      if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

static int cmd_help(char *args) {
    /* extract the first argument */
    char *arg = strtok(NULL, " ");
    int i;

    if (arg == NULL) {
        /* no argument given */
        for (i = 0; i < NR_CMD; i++) {
            printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        }
    } else {
        for (i = 0; i < NR_CMD; i++) {
            if (strcmp(arg, cmd_table[i].name) == 0) {
                printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
                return 0;
            }
        }
        printf("Unknown command '%s'\n", arg);
    }
    return 0;
}

static int cmd_c(char *args) {
    cpu_exec(-1);
    return 0;
}

static int cmd_q(char *args) {
    return -1;
}

static int cmd_si(char *args) {
    char *arg = strtok(args, " ");
    if (arg == NULL) {
        printf("too few arguments. \n");
        return 1;
    }
    int num = atoi(arg);
    cpu_exec(num);
    printf("ok");
    return 0;
}

static int cmd_info(char *args) {
    char *arg = strtok(args, " ");
    if (arg == NULL) {
        printf("Too few argument for info cmd. Usage info [r|w] \n");
        return 0;
    }
    // register info
    if (strcmp(arg, "r") == 0) {
        // printf("eax is %x\n", cpu.eax);
        // printf("ecx is %x\n", cpu.ecx);
        // printf("edx is %x\n", cpu.edx);
        // printf("ebx is %x\n", cpu.ebx);
        // printf("esp is %x\n", cpu.esp);
        // printf("ebp is %x\n", cpu.ebp);
        // printf("esi is %x\n", cpu.esi);
        // printf("edi is %x\n", cpu.edi);
        // printf("---------------------------\n");
        isa_reg_display();
    } else if (strcmp(arg, "w") == 0) {
        print_wp();
    }

    return 0;
}

static int cmd_x(char *args) {
    //获取内存起始地址和扫描长度。
    if (args == NULL) {
        printf("too few parameter! \n");
        return 1;
    }

    char *arg = strtok(args, " ");
    if (arg == NULL) {
        printf("too few parameter! \n");
        return 1;
    }
    int n = atoi(arg);
    char *EXPR = strtok(NULL, " ");
    if (EXPR == NULL) {
        printf("too few parameter! \n");
        return 1;
    }
    if (strtok(NULL, " ") != NULL) {
        printf("too many parameter! \n");
        return 1;
    }
    bool success = true;
    //vaddr_t addr = expr(EXPR , &success);
    if (success != true) {
        printf("ERRO!!\n");
        return 1;
    }
    char *str;
    // vaddr_t addr = atoi(EXPR);
    vaddr_t addr = strtol(EXPR, &str, 16);
    // printf("%#lX\n",ad);
    //进行内存扫描,每次四个字节;
    for (int i = 0; i < n; i++) {
        uint32_t data = vaddr_read(addr + i * 4, 4);
        printf("0x%08x  ", addr + i * 4);
        for (int j = 0; j < 4; j++) {
            printf("0x%02x ", data & 0xff);
            data = data >> 8;
        }
        printf("\n");
    }
    return 0;
}

static int cmd_p(char *args) {
    if (args == NULL)
        return 0;
    bool success = true;
    uint32_t value = expr(args, &success);
    if (success) {
        printf("%u(0x%08x)\n", value, value);
    }
    return 0;
}

static int cmd_w(char *args) {
    char *arg = args;
    if (arg == NULL) {
        printf("One argument should be provided\n");
        return 0;
    }
    WP *wp = new_wp();
    memset(wp->expr, 0, sizeof(wp->expr));
    strcpy(wp->expr, arg);
    bool success = true;
    wp->value = expr(arg, &success);
    if (!success) {
        printf("set watchpoint failed. Please check your exprssion!\n");
        free_wp(wp->NO);
    }
    return 0;
}

static int cmd_d(char *args) {
    char *arg = strtok(NULL, " ");
    if (arg == NULL)
        return 0;
    int n = 0;
    sscanf(arg, "%d", &n);
    if (free_wp(n) != 0)
        printf("Delete failed: %d is not exist!\n", n);
    else
        printf("Delete success!\n");
    return 0;
}