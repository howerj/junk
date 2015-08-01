#!/usr/bin/awk -f
# a brainfuck to C translator.
# Needs a recent version of gawk, if on OS X,
# try using Fink's version.
# <http://awk.info/?doc/bfc.html>
# steve jenson

BEGIN {
  print "#include <stdio.h>\n";
  print "int main() {";
  print "  int c = 0;";
  print "  static int b[30000];\n";
}

/[\[\]\+\-<>\.\,]/ {
  #Note: the order in which these are
  #substituted is very important.
  gsub(/\]/, "  }\n");
  gsub(/\[/, "  while(b[c] != 0) {\n");
  gsub(/\+/, "  ++b[c];\n");
  gsub(/\-/, "  --b[c];\n");
  gsub(/>/, "  ++c;\n");
  gsub(/</, "  --c;\n");
  gsub(/\./, "  putchar(b[c]);\n");
  gsub(/\,/, "  b[c] = getchar();\n");
  print $0
}

END {
  print "\n  return 0;";
  print "}";
}
