#include <stdio.h>   // for printf, scanf, fopen, fclose, fscanf
#include <stdlib.h>  // for malloc, free, rand, srand
#include <string.h>  // for strlen, strcmp
#include <time.h>    // for time (used in random seed)

#define MAX_WORDS 10000
#define WORD_LENGTH 5
#define Dictionary_SIZE 5
#define MAX_ATTEMPTS 6 

// Prototype declarations
int load_dictionary(const char *filename, char words[][WORD_LENGTH + 1]);
const char* choose_random_word(char words[][WORD_LENGTH + 1], int count);
void compute_feedback(const char *target, const char *guess, char *feedback);
void print_grid(char guesses[MAX_ATTEMPTS][WORD_LENGTH + 1], char feedbacks[MAX_ATTEMPTS][WORD_LENGTH + 1], int attempts);
int matches_feedback(const char *candidate_word, const char *guess, const char *expected_feedback);
void solver(char words[][WORD_LENGTH + 1], int count, char guesses[MAX_ATTEMPTS][WORD_LENGTH + 1], char feedbacks[MAX_ATTEMPTS][WORD_LENGTH + 1], int attempts);


// Load dictionary from file (one 5-letter word per line)
int load_dictionary(const char *filename, char words[][WORD_LENGTH + 1]) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Error: Could not open %s\n", filename);
        return 0;
    }
    int count = 0;
    while (fscanf(file, "%5s", words[count]) == 1) {
        // ensure lowercase
        for (int i = 0; i < WORD_LENGTH; i++)
            if (words[count][i] >= 'A' && words[count][i] <= 'Z')
                words[count][i] += 32;
        words[count][WORD_LENGTH] = '\0';
        count++;
        if (count >= MAX_WORDS) break;
    }
    fclose(file);
    return count;
}

const char* choose_random_word(char words[][WORD_LENGTH + 1], int count) {
    srand((unsigned)time(NULL));
    int index = rand() % count;
    return words[index];
}

void compute_feedback(const char *target, const char *guess, char *feedback) {
    int target_counts[26] = {0};

    for (int i = 0; i < WORD_LENGTH; i++) {
        if (guess[i] == target[i]) {
            feedback[i] = 'G';
        } else {
            feedback[i] = '?';
            target_counts[target[i] - 'a']++;
        }
    }

    for (int i = 0; i < WORD_LENGTH; i++) {
        if (feedback[i] == 'G') continue;
        int idx = guess[i] - 'a';
        if (idx >= 0 && idx < 26 && target_counts[idx] > 0) {
            feedback[i] = 'Y';
            target_counts[idx]--;
        } else {
            feedback[i] = 'B';
        }
    }
    feedback[WORD_LENGTH] = '\0';
}

void print_grid(char guesses[MAX_ATTEMPTS][WORD_LENGTH + 1], char feedbacks[MAX_ATTEMPTS][WORD_LENGTH + 1], int attempts) {
    printf("\nWORDLE BOARD:\n");
    for (int i = 0; i < MAX_ATTEMPTS; i++) {
        if (i >= attempts) {
            for (int j = 0; j < WORD_LENGTH; j++) printf("⬜️ ");
            printf("  ");
            for (int j = 0; j < WORD_LENGTH; j++) printf("_");
            printf("\n");
            continue;
        }
        for (int j = 0; j < WORD_LENGTH; j++) {
            if (feedbacks[i][j] == 'G') printf("🟩 ");
            else if (feedbacks[i][j] == 'Y') printf("🟨 ");
            else printf("⬜️ ");
        }
        printf("  %s\n", guesses[i]);
    }
    printf("\n");
}

int matches_feedback(const char *candidate_word, const char *guess, const char *expected_feedback) {
    char computed[WORD_LENGTH + 1];
    compute_feedback(candidate_word, guess, computed);
    return (strcmp(computed, expected_feedback) == 0);
}

void solver(char words[][WORD_LENGTH + 1], int count, char guesses[MAX_ATTEMPTS][WORD_LENGTH + 1], char feedbacks[MAX_ATTEMPTS][WORD_LENGTH + 1], int attempts) {
    printf("\n🔎 Solver suggestions based on previous feedback:\n");
    int suggestions = 0;
    for (int i = 0; i < count; i++) {
        int ok = 1;
        for (int j = 0; j < attempts; j++) {
            if (!matches_feedback(words[i], guesses[j], feedbacks[j])) {


ok = 0;
                break;
            }
        }
        if (ok) {
            printf("%s  ", words[i]);
            suggestions++;
            if (suggestions % 10 == 0) printf("\n");
        }
    }
    if (suggestions == 0) printf("No possible words found.\n");
    else printf("\nTotal suggestions: %d\n", suggestions);
}

int main() {
    char words[MAX_WORDS][WORD_LENGTH + 1];
    int count = load_dictionary("words.txt", words);
    if (count == 0) {
        printf("Please create a words.txt file (one 5-letter word per line) in the program folder.\n");
        return 1;
    }

    printf("✅ Loaded %d words from dictionary.\n", count);
    const char *target = choose_random_word(words, count);

    char guesses[MAX_ATTEMPTS][WORD_LENGTH + 1] = {{0}};
    char feedbacks[MAX_ATTEMPTS][WORD_LENGTH + 1] = {{0}};
    int attempts = 0;

    while (attempts < MAX_ATTEMPTS) {
        print_grid(guesses, feedbacks, attempts);

        printf("Mode: (1) Manual guess   (2) Solver suggestions then choose\n");
        printf("Choose mode (1 or 2): ");
        int mode = 0;
        if (scanf("%d", &mode) != 1) {
            while (getchar() != '\n');
            printf("Invalid input. Try again.\n");
            continue;
        }
        char guess[WORD_LENGTH + 1];

        if (mode == 1) {
            printf("Enter your 5-letter guess: ");
            if (scanf("%5s", guess) != 1) {
                while (getchar() != '\n');
                printf("Invalid input. Try again.\n");
                continue;
            }
            for (int i = 0; i < WORD_LENGTH; i++)
                if (guess[i] >= 'A' && guess[i] <= 'Z') guess[i] += 32;

            if (strlen(guess) != WORD_LENGTH) {
                printf("❌ Invalid length! Must be 5 letters.\n");
                continue;
            }

            int valid = 0;
            for (int i = 0; i < count; i++)
                if (strcmp(guess, words[i]) == 0) { valid = 1; break; }
            if (!valid) {
                printf("⚠️ Word not in dictionary!\n");
                continue;
            }
        } else if (mode == 2) {
            solver(words, count, guesses, feedbacks, attempts);
            printf("Enter your chosen word from suggestions (or any 5-letter word): ");
            if (scanf("%5s", guess) != 1) {
                while (getchar() != '\n');
                printf("Invalid input. Try again.\n");
                continue;
            }
            for (int i = 0; i < WORD_LENGTH; i++)
                if (guess[i] >= 'A' && guess[i] <= 'Z') guess[i] += 32;
            if (strlen(guess) != WORD_LENGTH) {
                printf("❌ Invalid length! Must be 5 letters.\n");
                continue;
            }
        } else {
            printf("❌ Invalid mode selection.\n");
            continue;
        }

        strcpy(guesses[attempts], guess);
        compute_feedback(target, guess, feedbacks[attempts]);
        attempts++;

        print_grid(guesses, feedbacks, attempts);

        if (strcmp(guess, target) == 0) {
            printf("🎉 Congratulations! You guessed the word in %d attempts!\n", attempts);
            break;
        } else {
            printf("Attempt %d/%d complete.\n", attempts, MAX_ATTEMPTS);
        }

        if (attempts == MAX_ATTEMPTS) {
            printf("❌ Game over! The word was: %s\n", target);
            break;
        }
    }

    printf("\nWould you like to run the interactive solver (enter feedback manually)? (y/n): ");
    char choice = '\0';
    if (scanf(" %c", &choice) == 1 && (choice == 'y' || choice == 'Y')) {
        printf("Interactive solver mode: enter guess and feedback (G/Y/B) to filter possibilities.\n");
        char man_guess[WORD_LENGTH + 1], man_feedback[WORD_LENGTH + 1];
        int m_attempts = 0;


while (1) {
            printf("Enter guess (5 letters) or 'exit' to leave: ");
            if (scanf("%5s", man_guess) != 1) break;
            if (strcmp(man_guess, "exit") == 0) break;
            for (int i = 0; i < WORD_LENGTH; i++)
                if (man_guess[i] >= 'A' && man_guess[i] <= 'Z') man_guess[i] += 32;
            if (strlen(man_guess) != WORD_LENGTH) { printf("Invalid length.\n"); continue; }

            printf("Enter feedback for that guess (5 chars: G/Y/B): ");
            if (scanf("%5s", man_feedback) != 1) break;
            for (int i = 0; i < WORD_LENGTH; i++)
                if (man_feedback[i] >= 'a' && man_feedback[i] <= 'z') man_feedback[i] -= 32;
            if (strlen(man_feedback) != WORD_LENGTH) { printf("Invalid feedback length.\n"); continue; }

            if (m_attempts < MAX_ATTEMPTS) {
                strcpy(guesses[m_attempts], man_guess);
                strcpy(feedbacks[m_attempts], man_feedback);
                m_attempts++;
            } else {
                printf("Reached max manual attempts for solver.\n");
            }

            solver(words, count, guesses, feedbacks, m_attempts);
        }
    }

    printf("Thanks for playing!\n");
    return 0;
}