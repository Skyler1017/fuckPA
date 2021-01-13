#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#include <stdlib.h>

uint32_t isa_reg_str2val(const char *s, bool *success);

enum {
    TK_NOTYPE = 256,
    TK_EQ,
    TK_SUB,
    TK_ADD,
    TK_MULTI,
    TK_DIV,  // 261
    TK_NUM,
    TK_LEFT,
    TK_RIGHT,
    TK_REG,
    TK_HEX,
    TK_AND,
    TK_OR,
    TK_DEREF
};
#define max_token_num 65536

typedef struct paren {
    int left, right;
} Paren;

Paren parens[max_token_num / 2];
int nr_paren;
static struct rule {
    char *regex;
    int token_type;
} rules[] = {
        {" +",                                                         TK_NOTYPE},             // spaces
        {"0[Xx][0-9a-fA-F]+",                                          TK_HEX}, // hex (要在数字前面：TK_NUM)

        {"^[1-9][0-9]*",                                               TK_NUM}, // digital
        {"\\+",                                                        TK_ADD},          // plus
        {"-",                                                          TK_SUB},            // sub
        {"\\*",                                                        TK_MULTI},        // sub
        {"/",                                                          TK_DIV},            // div
        {"==",                                                         TK_EQ},            // equal
        {"\\(",                                                        TK_LEFT},
        {"\\)",                                                        TK_RIGHT},
        {"&&",                                                         TK_AND},    // AND
        {"\\|\\|",                                                     TK_OR}, // OR
        {"\\$[a-dA-D][hlHL]|\\$[eEpP]?(ax|dx|cx|bx|bp|si|di|sp|ip|c)", TK_REG}
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]))

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
    int i;
    char error_msg[128];
    int ret;

    for (i = 0; i < NR_REGEX; i++) {
        ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
        if (ret != 0) {
            regerror(ret, &re[i], error_msg, 128);
            panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
        }
    }
}

typedef struct token {
    // token 类型
    int type;
    // token 的值，如数字类型，需要存储其值
    char str[32];
} Token;
#define NR_TOKEN 256
// __attribute__((used)) 表示未被引用也需要被保留
static Token tokens[NR_TOKEN] __attribute__((used)) = {};
// nr_token记录解析的token数量
static int nr_token __attribute__((used)) = 0;

static bool make_token(char *e) {
    int position = 0;
    int i;
    regmatch_t pmatch;

    nr_token = 0;
    while (e[position] != '\0') {
        /* Try all rules one by one. */
        for (i = 0; i < NR_REGEX; i++) {
            if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
                char *substr_start = e + position;
                int substr_len = pmatch.rm_eo;
                position += substr_len;

                if (substr_len >= 32) {
                    printf("%.*s length of the substring is too long.\n", substr_len, substr_start);
                    return false;
                }
                if (nr_token >= NR_TOKEN) {
                    printf("too many tokens\n");
                    return false;
                }
                switch (rules[i].token_type) {
                    case TK_NOTYPE:
                        break;
                    case TK_REG:
                    case TK_HEX:
                    case TK_NUM:
                        strncpy(tokens[nr_token].str, substr_start, substr_len);
                        tokens[nr_token].str[substr_len] = '\0';
                    default:
                        tokens[nr_token++].type = rules[i].token_type;
                        break;
                }
                //Log("tokes type : %d", rules[i].token_type);
                break;
            }
        }

        if (i == NR_REGEX) {
            printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
            return false;
        }
    }
    //匹配括号优化算法
    // int stack[max_token_num];
    // int top = 0;
    // nr_paren = 0;
    // for (int i = 0; i < nr_token; i++)
    // {
    //   if (tokens[i].type == TK_LEFT)
    //   {
    //     stack[top++] = i;
    //   }
    //   if (tokens[i].type == TK_RIGHT)
    //   {
    //     assert(top >= 0);
    //     if (top == 0)
    //     {
    //       printf("unmatched right paren at %d\n", i);
    //       return false;
    //     }
    //     parens[nr_paren].left = stack[top - 1];
    //     parens[nr_paren].right = i;
    //     nr_paren++;
    //     stack[--top] = -1;
    //   }
    // }
    // if (top != 0)
    // {
    //   printf("number of left paren and right paren not equal\n");
    //   return false;
    // }

    return true;
}

int check_parentheses(int p, int q) {
    int i, tag = 0;
    bool bound = false;
    if (tokens[p].type == TK_LEFT && tokens[q].type == TK_RIGHT) {
        bound = true;
    }
    for (i = p; i <= q; i++) {
        if (tokens[i].type == TK_LEFT) {
            tag++;
        } else if (tokens[i].type == TK_RIGHT) {
            tag--;
        }
        if (tag < 0) {
            return -1;
        }
    }
    if (tag != 0) {
        return -1;
    }
    if (!bound) {
        return 0;
    }
    // 括号包围
    for (i = p + 1; i <= q - 1; ++i) {
        if (tokens[i].type == TK_LEFT) {
            ++tag;
        } else if (tokens[i].type == TK_RIGHT) {
            --tag;
        }
        if (tag < 0) {
            return 0;
        }
    }
    return 1; // top must be zero
}

int op_pri(int op) {
    int pri;
    switch (op) {
        case TK_OR:
            pri = 0;
            break;
        case TK_AND:
            pri = 1;
            break;
        case TK_EQ:
            pri = 2;
            break;
        case TK_ADD:
        case TK_SUB:
            pri = 3;
            break;
        case TK_MULTI:
        case TK_DIV:
            pri = 4;
            break;
        case TK_DEREF:
            pri = 5;
            break;
        default:
            pri = 10;
    }
    return pri;
}

bool is_op(int ch) {
    return ch == TK_AND || ch == TK_SUB || ch == TK_DEREF || ch == TK_ADD ||
           ch == TK_DIV || ch == TK_OR || ch == TK_EQ || ch == TK_MULTI;
}

int compare(int i, int j) {
    int p1 = op_pri(tokens[i].type);
    int p2 = op_pri(tokens[j].type);
    return p1 < p2 ? -1 : (p1 == p2 ? 0 : 1);
}

int get_main_op(int p, int q) {
    int count = 0, i, pos = -1;
    for (i = p; i <= q; ++i) {
        int type = tokens[i].type;
        // 不在括号中
        if (!count && is_op(type)) {
            if (pos == -1)
                pos = i;
            else if (compare(i, pos) <= 0)
                pos = i;
        } else if (type == TK_LEFT)
            count++;
        else if (type == TK_RIGHT)
            count--;
    }
//    Log("main op pos %d ", pos);
    return pos;
}

uint32_t log_invalid_token(int p, int q) {
    printf("Invalid tokens: (%d, %d)\n", p, q);
//    for (; p <= q; ++p) {
//        int type = tokens[p].type;
//        printf("%d: %d,%s", p, type, tokens[p].str);
//        if (type == TK_NUM || type == TK_HEX || type == TK_REG)
//            printf(" - %s", tokens[p].str);
//        printf("\n");
//    }
    return 0;
}

uint32_t eval(int p, int q, bool *success) {
    if (p > q) {
        return log_invalid_token(p, q);
    }
        //一个token的情况
    else if (p == q) {
        uint32_t val = 0;
        int type = tokens[p].type;
        if (type == TK_NUM || type == TK_HEX) {
//            Log("parse : %s", tokens[p].str);
            return strtoul(tokens[p].str, NULL, 0);
        } else if (type == TK_REG) {
            val = isa_reg_str2val(tokens[p].str + 1, success);
            if (*success) {
                return val;
            }
            printf("Unknown register: %s\n", tokens[p].str);
            return 0;
        }
    }

    int ret = check_parentheses(p, q);
    if (ret == -1) {
        return log_invalid_token(p, q);
    }
    if (ret == 1) {
        return eval(p + 1, q - 1, success);
    }
    int pos = get_main_op(p, q);
    if (pos == -1) {
        return log_invalid_token(p, q);
    }

    //计算
    uint32_t val1 = 0, val2 = 0, val = 0;
    if (tokens[pos].type != TK_DEREF) {
        val1 = eval(p, pos - 1, success);
    }
    if (*success == false) {
        return 0;
    }
    val2 = eval(pos + 1, q, success);
    if (*success == false) {
        return 0;
    }
    //Log("val1 %u,val2 %u", val1, val2);
    switch (tokens[pos].type) {
        case TK_ADD:
            val = val1 + val2;
            break;
        case TK_SUB:
            val = val1 - val2;
            break;
        case TK_MULTI:
            val = val1 * val2;
            break;
        case TK_DIV:
            if (val2 == 0) {
                printf("Divide 0 error at [%d, %d]", p, q);
                return *success = false;
            }
            val = val1 / val2;
            break;
        case TK_AND:
            val = val1 && val2;
            break;
        case TK_OR:
            val = val1 || val2;
            break;
        case TK_EQ:
            val = val1 == val2;
            break;
        case TK_DEREF:
            val = vaddr_read(val2, 4);
            break;
        default:
            printf("Unknown token type: %d\n", tokens[pos].type);
            return *success = false;
    }
//    Log("exec val: %u", val);
    return val;
}

/* erase the index of the tokens array between [p, p + cnt - 1] */
void erase(int p, int cnt) {
    int i;
    for (i = p; i + cnt < nr_token; ++i) {
        tokens[i] = tokens[i + cnt];
    }
    nr_token -= cnt;
}

uint32_t expr(char *e, bool *success) {
    if (!make_token(e)) {
        *success = false;
        return 0;
    }

    int i, type;
    for (i = 0; i < nr_token; ++i) {
        type = tokens[i].type;
        if (type == TK_ADD || type == TK_SUB) {
            int j = i;
            int flag = 1;
            while (j < nr_token && (type == TK_ADD || type == TK_SUB)) {
                flag *= (type == TK_ADD ? 1 : -1);
                type = tokens[++j].type;
            }
            if (j - i > 1) {
                tokens[i].type = (flag == 1 ? TK_ADD : TK_SUB);
                erase(i + 1, j - i - 1);
            }
        }
    }

    // 判断指针
    for (i = 0; i < nr_token; ++i) {
        if (tokens[i].type == TK_MULTI &&
            (i == 0 || is_op(tokens[i - 1].type) || tokens[i - 1].type == TK_LEFT)) {
            printf("指针 at %d\n", i);
            if (i == 0 || tokens[i - 1].type != TK_DEREF)
                tokens[i].type = TK_DEREF;
        }
    }

    uint32_t ret = eval(0, nr_token - 1, success);
    return ret;
}
