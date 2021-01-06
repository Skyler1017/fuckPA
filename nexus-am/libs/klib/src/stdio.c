#include "klib.h"
#include <stdarg.h>

// pa2.2
#define MAX_BUF 2000
// 判断是否是数字
#define ISNUM(x) (((x) >= '0') && ((x) <= '9'))
// char to digital
#define CHAR2NUM(x) ((x) - '0')

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int print_string(char *buf, char *s, int length, int ladjust) {
    int i;
    int len = 0;
    char *s1 = s;
    while (*s1++)
        len++;
    if (length < len)
        length = len;

    if (ladjust) {
        for (i = 0; i < len; i++)
            buf[i] = s[i];
        for (i = len; i < length; i++)
            buf[i] = ' ';
    } else {
        for (i = 0; i < length - len; i++)
            buf[i] = ' ';
        for (i = length - len; i < length; i++)
            buf[i] = s[i - length + len];
    }
    return length;
}

int print_char(char *buf, char c, int length, int ladjust) {
    int i;

    if (length < 1)
        length = 1;
    if (ladjust) {
        *buf = c;
        for (i = 1; i < length; i++)
            buf[i] = ' ';
    } else {
        for (i = 0; i < length - 1; i++)
            buf[i] = ' ';
        buf[length - 1] = c;
    }
    return length;
}

int print_num(char *buf, unsigned long u, int base, int negFlag,
              int length, int ladjust, char padc, int upcase) {
    /* algorithm :
       *  1. prints the number from left to right in reverse form.
       *  2. fill the remaining spaces with padc if length is longer than
       *     the actual length
       *     TRICKY : if left adjusted, no "0" padding.
       *		    if negtive, insert  "0" padding between "0" and number.
       *  3. if (!ladjust) we reverse the whole string including paddings
       *  4. otherwise we only reverse the actual string representing the num.
       */

    int actualLength = 0;
    char *p = buf;
    int i;

    do {
        int tmp = u % base;
        if (tmp <= 9) {
            *p++ = '0' + tmp;
        } else if (upcase) {
            *p++ = 'A' + tmp - 10;
        } else {
            *p++ = 'a' + tmp - 10;
        }
        u /= base;
    } while (u != 0);

    if (negFlag) {
        *p++ = '-';
    }

    // finally, we will reverse the whole string
    /* figure out actual length and adjust the maximum length */
    actualLength = p - buf;
    if (length < actualLength)
        length = actualLength;

    /* add padding */
    if (ladjust) {
        padc = ' ';
    }
    if (negFlag && !ladjust && (padc == '0')) {
        for (i = actualLength - 1; i < length - 1; i++)
            buf[i] = padc;
        buf[length - 1] = '-';
    } else {
        for (i = actualLength; i < length; i++)
            buf[i] = padc;
    }

    /* prepare to reverse the string */
    {
        int begin = 0;
        int end;
        if (ladjust) {
            //left alogn，just reverse the numbers (including the negFlag:'-' )
            end = actualLength - 1;
        } else {
            end = length - 1;
        }

        while (end > begin) {
            char tmp = buf[begin];
            buf[begin] = buf[end];
            buf[end] = tmp;
            begin++;
            end--;
        }
    }

    /* adjust the string pointer */
    return length;
}

int printf(const char *fmt, ...) {
    return 0;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
    char buf[MAX_BUF];
    int cur = 0;
    char c;
    char *s;
    long int num;
    int longFlag;
    int negFlag;
    int width;   // output width
    int prec;    // the precision of decimal
    int ladjust; // left align
    char padc;   // the character used to fill extra positions

    int length;

    while (1) {
        // 查找%
        while ((*fmt) && *fmt != '%') {
            out[cur++] = *fmt;
            fmt++;
        }

        // 是否到末尾
        if ((*fmt) == '\0') {
            break;
        }
        fmt++;
        // 标志位
        longFlag = 0;
        negFlag = 0;
        width = 0;
        ladjust = 0;
        prec = 0;
        padc = ' ';
        //对其标志
        if (*fmt == '-') {
            ladjust = 1;
            fmt++;
        } else if (*fmt == '0') {
            padc = '0';
            fmt++;
        }

        //如果是读取数字
        while (ISNUM(*fmt)) {
            width = width * 10 + CHAR2NUM(*fmt);
            fmt++;
        }

        //检查精度
        if (*fmt == '.') {
            fmt++;
            while (ISNUM(*fmt)) {
                prec = prec * 10 + CHAR2NUM(*fmt);
                fmt++;
            }
        }

        // 长整数
        if (*fmt == 'l') {
            longFlag = 1;
            fmt++;
        }

        length = 0;
        /* check format flag */
        negFlag = 0;
        switch (*fmt) {
            case 'b':
                if (longFlag) {
                    num = va_arg(ap, long int);
                } else {
                    num = va_arg(ap, int);
                }
                length = print_num(buf, num, 2, 0, width, ladjust, padc, 0);
                break;

            case 'd':
            case 'D':
                if (longFlag) {
                    num = va_arg(ap, long int);
                } else {
                    num = va_arg(ap, int);
                }
                if (num < 0) {
                    num = -num;
                    negFlag = 1;
                }
                length = print_num(buf, num, 10, negFlag, width, ladjust, padc, 0);
                break;

            case 'o':
            case 'O':
                if (longFlag) {
                    num = va_arg(ap, long int);
                } else {
                    num = va_arg(ap, int);
                }
                length = print_num(buf, num, 8, 0, width, ladjust, padc, 0);
                break;

            case 'u':
            case 'U':
                if (longFlag) {
                    num = va_arg(ap, long int);
                } else {
                    num = va_arg(ap, int);
                }
                length = print_num(buf, num, 10, 0, width, ladjust, padc, 0);
                break;
            case 'p':
            case 'x':
                if (longFlag) {
                    num = va_arg(ap, long int);
                } else {
                    num = va_arg(ap, int);
                }
                length = print_num(buf, num, 16, 0, width, ladjust, padc, 0);
                break;

            case 'X':
                if (longFlag) {
                    num = va_arg(ap, long int);
                } else {
                    num = va_arg(ap, int);
                }
                length = print_num(buf, num, 16, 0, width, ladjust, padc, 1);
                break;

            case 'c':
                c = (char) va_arg(ap, int);
                length = print_char(buf, c, width, ladjust);
                break;

            case 's':
                s = (char *) va_arg(ap, char *);
                length = print_string(buf, s, width, ladjust);
                break;

            case '\0':
                fmt--;
                break;

            default:
                /* output this char as it is */
                buf[length++] = *fmt;
        } /* switch (*fmt) */

        fmt++;
        int i;
        for (i = 0; i < length; ++i) {
            out[cur++] = buf[i];
        }
    }

    /* special termination call */
    out[cur] = '\0';
    return cur;
}

int sprintf(char *out, const char *fmt, ...) {
    va_list ap;
    //  void va_start(va_list arg_ptr, prev_param);
    // 功能：以固定参数的地址为起点确定变参的内存起始地址，
    //      获取第一个参数的首地址
    va_start(ap, fmt);
    int len = vsprintf(out, fmt, ap);
    // int len = 0;
    va_end(ap);
    return len;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
    return 0;
}

#endif
