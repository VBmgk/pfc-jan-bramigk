#include "suggestions.h"

int add_suggestion(Suggestions &suggestions) {
  if (suggestions.tables_count < MAX_SUGGESTIONS) {
    suggestions.tables_count++;
    return suggestions.tables_count;
  } else {
    return -1;
  }
}

int del_suggestion(Suggestions & /*suggestions*/, int /*index*/) {
  // TODO: implement, has to move all subsequent suggestions, PITA
  return -1;
}
