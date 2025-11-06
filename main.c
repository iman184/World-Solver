#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_WORDS 10000
#define WORD_LENGTH 5
<<<<<<< HEAD
#define MAX_ATTEMPTS 6 
=======
#define MAX_ATTEMPTS 6
>>>>>>> 0dec0e08e510f07d9a9df0dfc420c9f19d6bb19c

// 🔹 Load dictionary from file
int load_dictionary(const char *filename, char words[MAX_WORDS][WORD_LENGTH + 1]) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Error: Cannot open dictionary file '%s'\n", filename);
        return 0;
    }

    int count = 0;
    while (fscanf(file, "%5s", words[count]) == 1) {
        for (int i = 0; i < WORD_LENGTH; i++) {
            if (words[count][i] >= 'A' && words[count][i] <= 'Z')
                words[count][i] += 32; // convert to lowercase
        }
        count++;
        if (count >= MAX_WORDS) break;
    }

    fclose(file);
    return count;
}

// 🔹 Generate feedback (G = green, Y = yellow, _ = gray)
void generate_feedback(const char *guess, const char *target, char *feedback) {
<<<<<<< HEAD
    int used_target[WORD_LENGTH] = {0};
    int used_guess[WORD_LENGTH] = {0};
=======
    int used[WORD_LENGTH] = {0};
>>>>>>> 0dec0e08e510f07d9a9df0dfc420c9f19d6bb19c

    // Step 1: Green
    for (int i = 0; i < WORD_LENGTH; i++) {
        if (guess[i] == target[i]) {
            feedback[i] = 'G';
<<<<<<< HEAD
            used_target[i] = 1;
            used_guess[i] = 1;
=======
            used[i] = 1;
>>>>>>> 0dec0e08e510f07d9a9df0dfc420c9f19d6bb19c
        } else {
            feedback[i] = '_';
        }
    }

    // Step 2: Yellow
    for (int i = 0; i < WORD_LENGTH; i++) {
<<<<<<< HEAD
        if (used_guess[i]) continue; // déjà vert
        for (int j = 0; j < WORD_LENGTH; j++) {
            if (!used_target[j] && guess[i] == target[j]) {
                feedback[i] = 'Y';
                used_target[j] = 1;
                break;
            }
        }
    }

    feedback[WORD_LENGTH] = '\0';
}

// 🔹 Print the Wordle grid
void print_grid(char guesses[MAX_ATTEMPTS][WORD_LENGTH + 1],
                char feedbacks[MAX_ATTEMPTS][WORD_LENGTH + 1],
                int attempts) {
    printf("\nWORDLE BOARD:\n");

    for (int i = 0; i < MAX_ATTEMPTS; i++) {
        if (i >= attempts) {
            for (int j = 0; j < WORD_LENGTH; j++)
                printf("[ _ ] ");
            printf("\n");
            continue;
        }

        for (int j = 0; j < WORD_LENGTH; j++) {
            if (feedbacks[i][j] == 'G')
                printf("\x1b[42m[%c]\x1b[0m ", guesses[i][j]); // green
            else if (feedbacks[i][j] == 'Y')
                printf("\x1b[43m[%c]\x1b[0m ", guesses[i][j]); // yellow
            else
                printf("\x1b[47m[%c]\x1b[0m ", guesses[i][j]); // white
        }
        printf("  %s\n", guesses[i]);
    }

    printf("\n");
}

// 🔹 Simple solver suggestion: pick first word matching known feedback
char *solver_suggestion(char dictionary[MAX_WORDS][WORD_LENGTH + 1],
                        int word_count,
                        char feedbacks[MAX_ATTEMPTS][WORD_LENGTH + 1],
                        char guesses[MAX_ATTEMPTS][WORD_LENGTH + 1],
                        int attempts) {
    for (int w = 0; w < word_count; w++) {
        int valid = 1;
        for (int a = 0; a < attempts; a++) {
            char temp_feedback[WORD_LENGTH + 1];
            generate_feedback(dictionary[w], guesses[a], temp_feedback);
            if (strcmp(temp_feedback, feedbacks[a]) != 0) {
                valid = 0;
                break;
            }
        }
        if (valid) return dictionary[w];
    }
    return NULL;
}

=======
        if (feedback[i] == 'G') continue;
        for (int j = 0; j < WORD_LENGTH; j++) {
            if (!used[j] && guess[i] == target[j]) {
                feedback[i] = 'Y';
                used[j] = 1;
                break;
            }
        }
    }
    feedback[WORD_LENGTH] = '\0';
}

// 🔹 Print the Wordle grid
void print_grid(char guesses[MAX_ATTEMPTS][WORD_LENGTH + 1],
                char feedbacks[MAX_ATTEMPTS][WORD_LENGTH + 1],
                int attempts) {
    printf("\nWORDLE BOARD:\n");

    for (int i = 0; i < MAX_ATTEMPTS; i++) {
        if (i >= attempts) {
            for (int j = 0; j < WORD_LENGTH; j++)
                printf("[ _ ] ");
            printf("\n");
            continue;
        }

        for (int j = 0; j < WORD_LENGTH; j++) {
            if (feedbacks[i][j] == 'G')
                printf("\x1b[42m[%c]\x1b[0m ", guesses[i][j]); // green
            else if (feedbacks[i][j] == 'Y')
                printf("\x1b[43m[%c]\x1b[0m ", guesses[i][j]); // yellow
            else
                printf("\x1b[47m[%c]\x1b[0m ", guesses[i][j]); // white
        }
        printf("  %s\n", guesses[i]);
    }

    printf("\n");
}

// 🔹 Simple solver suggestion: pick first word matching known feedback
char *solver_suggestion(char dictionary[MAX_WORDS][WORD_LENGTH + 1],
                        int word_count,
                        char feedbacks[MAX_ATTEMPTS][WORD_LENGTH + 1],
                        char guesses[MAX_ATTEMPTS][WORD_LENGTH + 1],
                        int attempts) {
    for (int w = 0; w < word_count; w++) {
        int valid = 1;
        for (int a = 0; a < attempts; a++) {
            char temp_feedback[WORD_LENGTH + 1];
            generate_feedback(dictionary[w], guesses[a], temp_feedback);
            if (strcmp(temp_feedback, feedbacks[a]) != 0) {
                valid = 0;
                break;
            }
        }
        if (valid) return dictionary[w];
    }
    return NULL;
}

>>>>>>> 0dec0e08e510f07d9a9df0dfc420c9f19d6bb19c
// 🔹 Main
int main() {
    srand(time(NULL));

    char dictionary[MAX_WORDS][WORD_LENGTH + 1];
    int word_count = load_dictionary("words.txt", dictionary);

    if (word_count == 0) {
        printf("No words loaded. Exiting...\n");
        return 1;
    }

    const char *target = dictionary[rand() % word_count];

    char guesses[MAX_ATTEMPTS][WORD_LENGTH + 1];
    char feedbacks[MAX_ATTEMPTS][WORD_LENGTH + 1];
    int attempts = 0;
    int mode;

    printf("Welcome to Wordle in C!\n");
    printf("Mode: (1) Manual guess   (2) Solver suggestions then choose\n");
    printf("Choose mode (1 or 2): ");
    scanf("%d", &mode);

    while (attempts < MAX_ATTEMPTS) {
        print_grid(guesses, feedbacks, attempts);

        char guess[WORD_LENGTH + 1];

        if (mode == 2) {
            char *suggestion = solver_suggestion(dictionary, word_count, feedbacks, guesses, attempts);
            if (suggestion)
                printf("Solver suggests: %s\n", suggestion);
            else
                printf("Solver: no valid suggestion.\n");
        }

        printf("Attempt %d/%d - Enter your %d-letter guess: ", attempts + 1, MAX_ATTEMPTS, WORD_LENGTH);
        scanf("%5s", guess);

        for (int i = 0; i < WORD_LENGTH; i++)
            if (guess[i] >= 'A' && guess[i] <= 'Z') guess[i] += 32;

        if (strlen(guess) != WORD_LENGTH) {
            printf("❌ Please enter exactly %d letters.\n", WORD_LENGTH);
            continue;
        }

        strcpy(guesses[attempts], guess);
        generate_feedback(guess, target, feedbacks[attempts]);

        if (strcmp(guess, target) == 0) {
            print_grid(guesses, feedbacks, attempts + 1);
            printf("🎉 Congratulations! You found the word: %s\n", target);
            return 0;
        }

        attempts++;
    }

    print_grid(guesses, feedbacks, attempts);
    printf("❌ Out of attempts! The word was: %s\n", target);

    return 0;
}
