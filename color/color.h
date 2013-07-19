enum color{
  color_default,
  color_red,
  color_green,
  color_blue,
  color_yellow,
  color_black,
  color_white
};
char *foreground_colorize(enum color c, char *s, int len);
char *background_colorize(enum color c, char *s, int len);
