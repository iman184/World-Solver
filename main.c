#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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

// Picks a random word from the dictionary
const char* choose_random_word(char words[][WORD_LENGTH + 1], int count) {
    srand(time(NULL)); // Initialize random seed (based on current time)
    int index = rand() % count; // Pick a random number between 0 and count-1
    return words[index]; // Return that random word
}

int main() {
    char words[MAX_WORDS][WORD_LENGTH + 1]; // ✅ declare here first

    int count = load_dictionary("words.txt", words);
    if (count == 0) { // changed from -1 to 0 (since you return 0 on error)
        printf("No words loaded!\n");
        return 1;
    }

    printf("Loaded %d words from dictionary.\n", count);

    const char *target = choose_random_word(words, count);
    printf("Random word chosen (for testing): %s\n", target);

    return 0;
}
