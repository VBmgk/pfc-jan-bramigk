#include "suggestions.h"
#include "utils.h"

#include <stdio.h>
#include <string.h>
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

int del_suggestion(Suggestions &suggestions, int index) {
  FOR_RANGE(i, index, suggestions.tables_count - 1) {
    suggestions.tables[i] = std::move(suggestions.tables[i + 1]);
  }
  suggestions.tables_count--;
  return suggestions.tables_count;
}

#define SUGGS_FILE_HEADER "[AI suggestions version 1]"

void save_suggestions(Suggestions &suggestions, const char *filename) {
  auto file = fopen(filename, "w");
  if (!file) {
    perror("Could not save suggestions");
    return;
  }

  fprintf(file, SUGGS_FILE_HEADER "\n");

  int tables_count = suggestions.tables_count;
  fprintf(file, "tables_count = %i\n", tables_count);

  FOR_N(i, tables_count) {
    auto table = suggestions.tables[i];
    fprintf(file, "---\n");
    fprintf(file, "name = %s\n", table.name);
    int spots_count = table.spots_count;
    fprintf(file, "spots_count = %i\n", spots_count);
    FOR_N(j, spots_count) {
      auto spot = table.spots[j];
      fprintf(file, "%f, %f\n", spot.x, spot.y);
    }
  }
}

void load_suggestions(Suggestions &suggestions, const char *filename) {
  auto file = fopen(filename, "r");
  if (!file) {
    perror("Could not load suggestions");
    return;
  }

  char line[256];
  fgets(line, 256, file);
  if (strcmp(line, SUGGS_FILE_HEADER "\n")) {
    fprintf(stderr, "Incompatible header detected, maybe newer or invalid.\n");
  }

  int &tables_count = suggestions.tables_count;
  fgets(line, 256, file);
  sscanf(line, "tables_count = %i", &tables_count);
  if (tables_count > MAX_SUGGESTIONS) {
    fprintf(stderr, "Warning: too many suggestions (%i), truncating.",
            tables_count);
    tables_count = MAX_SUGGESTIONS;
  }

  FOR_N(i, tables_count) {
    auto &table = suggestions.tables[i];
    fgets(line, 256, file); // "---"
    // fscanf(file, "---");
    fgets(line, 256, file);
    sscanf(line, "name = %s", table.name);
    int &spots_count = table.spots_count;
    fgets(line, 256, file);
    sscanf(line, "spots_count = %i", &spots_count);

    int exceeding_spots = 0;
    if (spots_count > MAX_SUGGESTION_SPOTS) {
      fprintf(stderr, "Warning: too many spots (%i), truncating.", spots_count);
      exceeding_spots = spots_count - MAX_SUGGESTION_SPOTS;
      spots_count = MAX_SUGGESTION_SPOTS;
    }

    FOR_N(j, spots_count) {
      auto &spot = table.spots[j];
      fgets(line, 256, file);
      sscanf(line, "%f, %f\n", &spot.x, &spot.y);
    }

    FOR_N(j, exceeding_spots) { fgets(line, 256, file); }
  }
}
