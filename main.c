#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WORD_LENGTH 5
#define MAX_WORDS 10000

// function to load dictionary words from file
int load_dictionary(const char *filename, char words[][WORD_LENGTH + 1]) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Error: Could not open %s\n", filename);
        return 0;
    }

    int count = 0;
    while (fscanf(file, "%5s", words[count]) == 1) {
        count++;
        if (count >= MAX_WORDS) break;
    }

    fclose(file);
    return count;
}

int main() {
    char dictionary[MAX_WORDS][WORD_LENGTH + 1];
    int word_count = load_dictionary("words.txt", dictionary);

    if (word_count == 0) {
        printf("No words loaded!\n");
        return 1;
    }

    printf("Loaded %d words from dictionary.\n", word_count);
    printf("Example first word: %s\n", dictionary[0]);

    return 0;
}
