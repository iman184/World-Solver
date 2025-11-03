#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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
    srand(time(NULL)); // Initialize random seed (based on current time)
    int index = rand() % count; // Pick a random number between 0 and count-1
    return words[index]; // Return that random word
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
                printf("⬜ "); // Letter not in word
        }
    }
    printf("\n");
}

int main() {
    char words[MAX_WORDS][WORD_LENGTH + 1]; // Declare dictionary array

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
        scanf("%s", guess);

        // Convert guess to lowercase (optional, to avoid case mismatch)
        for (int i = 0; i < WORD_LENGTH; i++) {
            if (guess[i] >= 'A' && guess[i] <= 'Z') {
                guess[i] += 32; // Convert to lowercase
            }
        }

        // Check length
        if (strlen(guess) != WORD_LENGTH) {
            printf("❌ Invalid length! Must be 5 letters.\n");
            continue;
        }

        // Check if the guess exists in dictionary
        int valid = 0;
        for (int i = 0; i < count; i++) {
            if (strcmp(guess, words[i]) == 0) {
                valid = 1;
                break;
            }
        }

        if (!valid) {
            printf("⚠️ Word not in dictionary!\n");
            continue;
        }

        // Give feedback
        give_feedback(guess, target);
        attempts++;

        // Check win condition
        if (strcmp(guess, target) == 0) {
            printf("🎉 Congratulations! You guessed the word in %d tries!\n", attempts);
            break;
        }

        // If all attempts used
        if (attempts == max_attempts) {
            printf("❌ Game over! The word was: %s\n", target);
        }
    }

    // Pause before exit (Windows only)
    system("pause");

    return 0;
}
