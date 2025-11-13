#include "preprocesser.h"

char *preprocess(char *source) {

}



char *escape_char_for_output(char c) {
    switch (c) {
      case '\n': return "\\n";
      case '\t': return "\\t";
      case '\0': return "\\0";
      case '\\': return "\\\\";
      case '\'': return "\\'"
      default:  
          break;
    }
  
    char *charPtr;
    charPtr = (char *)malloc(sizeof(char) * 2);
    charPtr[0] = c;
    charPtr[1] = '\0';
    return;
      
}
