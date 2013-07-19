enum color{
  c_black,
  c_red,
  c_green,
  c_yellow,
  c_blue,
  c_magenta,
  c_cyan,
  c_white
};
#define DEFAULT_LEN   4 /* length of \e[0m */
#define COLOR_LEN     5  /* length of \e[3Xm */
#define DEFAULT_COLOR "\033[0m"

char *foreground_colorize(enum color c, char *s, int len);
char *background_colorize(enum color c, char *s, int len);
