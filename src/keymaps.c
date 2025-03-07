#include "../include/uni-void.h"

Key decode_key(int ch) {
  switch (ch) {
    case 's' : case 'j' : case KEY_DOWN : return key_down;
    case 'w' : case 'k' : case KEY_UP : return key_up;
    case 'd' : case 'l' : case KEY_RIGHT : return key_right;
    case 'a' : case 'h' : case KEY_LEFT : return key_left;
    case KEY_ENTER : case '\n' :  return key_return;
    case 'u' : return key_undo;
    case 'r' : return key_redo;
    case 'q' : return key_exit;
    case 'Q' : return key_force_quit;
    case KEY_RESIZE : return key_resize;
    case '?' : return key_usage;
  }
  return key_invalid;
}

