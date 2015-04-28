#ifndef SUGGESTIONS_H
#define SUGGESTIONS_H

#include "suggestion_table.h"

struct Suggestions {
  SuggestionTable tables[MAX_SUGGESTIONS];
  int tables_count = 0;
  int last_used = -1;
};

// allocate new suggestion, return the index or -1 on failure
int add_suggestion(Suggestions &suggestions);

// delete given suggestion, return new size or -1 on failure
int del_suggestion(Suggestions &suggestions, int index);

void save_suggestions(Suggestions &suggestions, const char *filename);

void load_suggestions(Suggestions &suggestions, const char *filename);

#endif
