#include <stdio.h>   // for printf, scanf, fopen, fclose, fscanf
#include <stdlib.h>  // for malloc, free, rand, srand
#include <string.h>  // for strlen, strcmp
#include <time.h>    // for time (used in random seed)

#define WORD_LENGTH 5
#define MAX_WORDS 10000

// Function to load dictionary words from file
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

// Pick a random word from the dictionary
const char* choose_random_word(char words[][WORD_LENGTH + 1], int count) {
    srand(time(NULL)); // Initialize random seed
    int index = rand() % count;
    return words[index];
}

// Compare guess with target and show feedback
void give_feedback(const char *guess, const char *target) {
    for (int i = 0; i < WORD_LENGTH; i++) {
        if (guess[i] == target[i]) {
            printf("🟩 "); // Correct position
        } else {
            int found = 0;
            for (int j = 0; j < WORD_LENGTH; j++) {
                if (guess[i] == target[j]) {
                    found = 1;
                    break;
                }
            }
            if (found)
                printf("🟨 "); // Letter in word but wrong position
            else
                printf("⬜️ "); // Letter not in word
        }
    }
    printf("\n");
}

// Solver function (simple filter for possible words)
void solver(char words[][WORD_LENGTH + 1], int count) {
    char guess[WORD_LENGTH + 1];
    char feedback[WORD_LENGTH + 1];
    int attempts = 0;

    while (1) {
        printf("Enter your guess (5 letters): ");
        scanf("%5s", guess);
        for (int i = 0; i < WORD_LENGTH; i++)
            if (guess[i] >= 'A' && guess[i] <= 'Z')
                guess[i] += 32; // convert to lowercase

        printf("Enter feedback (G=green, Y=yellow, B=black): ");
        scanf("%5s", feedback);

        printf("Possible words:\n");
        int found = 0;
        for (int i = 0; i < count; i++) {
            int match = 1;
            for (int j = 0; j < WORD_LENGTH; j++) {
                if (feedback[j] == 'G' && words[i][j] != guess[j]) match = 0;
                if (feedback[j] == 'Y') {
                    int has = 0;
                    for (int k = 0; k < WORD_LENGTH; k++)
                        if (words[i][k] == guess[j] && words[i][k] != guess[k])
                            has = 1;
                    if (!has) match = 0;
                }
                if (feedback[j] == 'B' && strchr(words[i], guess[j]) != NULL) match = 0;
            }
            if (match) {
                printf("%s\n", words[i]);
                found = 1;
            }
        }
        if (!found) printf("No matching words found.\n");

        attempts++;
        if (strcmp(feedback, "GGGGG") == 0) {
            printf("Word guessed in %d attempts!\n", attempts);
            break;
        }
    }
}

int main() {
    char words[MAX_WORDS][WORD_LENGTH + 1];

    int count = load_dictionary("words.txt", words);
    if (count == 0) {
        printf("No words loaded!\n");
        return 1;
    }

    printf("✅ Loaded %d words from dictionary.\n", count);

    const char *target = choose_random_word(words, count);
    printf("🎯 (For testing) Random word chosen: %s\n", target);

    char guess[WORD_LENGTH + 1];
    int attempts = 0;
    int max_attempts = 6;

    while (attempts < max_attempts) {
        printf("\nAttempt %d/%d - Enter your 5-letter guess: ", attempts + 1, max_attempts);
        scanf("%5s", guess);

        // Convert guess to lowercase
        for (int i = 0; i < WORD_LENGTH; i++)
            if (guess[i] >= 'A' && guess[i] <= 'Z')
                guess[i] += 32;

        if (strlen(guess) != WORD_LENGTH) {
            printf("❌ Invalid length! Must be 5 letters.\n");
            continue;
        }


// Check if guess exists in dictionary
        int valid = 0;
        for (int i = 0; i < count; i++)
            if (strcmp(guess, words[i]) == 0) valid = 1;

        if (!valid) {
            printf("⚠️ Word not in dictionary!\n");
            continue;
        }

        // Give feedback
        give_feedback(guess, target);
        attempts++;

        if (strcmp(guess, target) == 0) {
            printf("🎉 Congratulations! You guessed the word in %d tries!\n", attempts);
            break;
        }

        if (attempts == max_attempts) {
            printf("❌ Game over! The word was: %s\n", target);
        }
    }

    // Optionally, you can call solver here if you want automated help
    // solver(words, count);

    return 0;
}