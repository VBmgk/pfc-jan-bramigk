#include "suggestions.h"
#include "utils.h"

#include <stdio.h>
#include <utility>

int add_suggestion(Suggestions &suggestions) {
  if (suggestions.tables_count < MAX_SUGGESTIONS) {
    suggestions.tables[suggestions.tables_count].spots_count = 0;
    suggestions.tables_count++;
    return suggestions.tables_count;
  } else {
    return -1;
  }
}

int del_suggestion(Suggestions & suggestions, int index) {
  FOR_RANGE(i, index, suggestions.tables_count - 1) {
    suggestions.tables[i] = std::move(suggestions.tables[i + 1]);
  }
  suggestions.tables_count--;
  return suggestions.tables_count;
}

void save_suggestions(Suggestions &suggestions, const char *filename);

void load_suggestions(Suggestions &suggestions, const char *filename);
