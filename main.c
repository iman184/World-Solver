// wordle_complete_with_auto_solver.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#define MAX_WORDS 10000
#define WORD_LENGTH 5
#define MAX_ATTEMPTS 6

// Letter status for keyboard display
// 0 = unknown, 1 = gray, 2 = yellow, 3 = green
void update_letter_status(int status[26], const char *guess, const char *feedback) {
    for (int i = 0; i < WORD_LENGTH; ++i) {
        int idx = guess[i] - 'a';
        if (feedback[i] == 'G') status[idx] = 3;
        else if (feedback[i] == 'Y' && status[idx] < 2) status[idx] = 2;
        else if (feedback[i] == '_' && status[idx] == 0) status[idx] = 1;
    }
}

void print_keyboard(int status[26]) {
    const char *rows[] = {"qwertyuiop", "asdfghjkl", "zxcvbnm"};
    printf("\nClavier: ");
    for (int r = 0; r < 3; ++r) {
        printf("\n");
        for (int i = 0; i < (int)strlen(rows[r]); ++i) {
            char c = rows[r][i];
            int s = status[c - 'a'];
            if (s == 3) printf("\x1b[42m %c \x1b[0m ", toupper(c)); // green
            else if (s == 2) printf("\x1b[43m %c \x1b[0m ", toupper(c)); // yellow
            else if (s == 1) printf("\x1b[47m %c \x1b[0m ", toupper(c)); // gray
            else printf("[ %c ] ", toupper(c));
        }
    }
    printf("\n\n");
}

// Cross-platform clear
void clear_screen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

// trim newline and spaces
void trim_newline(char *s) {
    size_t n = strlen(s);
    while (n > 0 && (s[n-1] == '\n' || s[n-1] == '\r' || isspace((unsigned char)s[n-1]))) {
        s[n-1] = '\0';
        n--;
    }
}

// load dictionary from filename; accept only WORD_LENGTH alpha words
int load_dictionary(const char *filename, char words[MAX_WORDS][WORD_LENGTH + 1]) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Erreur : impossible d'ouvrir '%s'\n", filename);
        return 0;
    }

    char buf[128];
    int count = 0;
    while (fgets(buf, sizeof(buf), file) && count < MAX_WORDS) {
        trim_newline(buf);
        // normalize to lowercase and check alpha length
        int len = 0;
        char tmp[WORD_LENGTH + 2];
        for (size_t i = 0; i < strlen(buf) && len < WORD_LENGTH + 1; ++i) {
            if (isalpha((unsigned char)buf[i])) {
                tmp[len++] = tolower((unsigned char)buf[i]);
            }
        }
        tmp[len] = '\0';
        if (len == WORD_LENGTH) {
            strcpy(words[count++], tmp);
        }
    }
    fclose(file);
    return count;
}

int is_valid_word(const char *guess, char dictionary[MAX_WORDS][WORD_LENGTH + 1], int word_count) {
    for (int i = 0; i < word_count; ++i)
        if (strcmp(guess, dictionary[i]) == 0) return 1;
    return 0;
}

void generate_feedback(const char *guess, const char *target, char *feedback) {
    int used_target[WORD_LENGTH] = {0};
    int used_guess[WORD_LENGTH] = {0};

    // Green first
    for (int i = 0; i < WORD_LENGTH; ++i) {
        if (guess[i] == target[i]) {
            feedback[i] = 'G';
            used_target[i] = 1;
            used_guess[i] = 1;
        } else {
            feedback[i] = '_';
        }
    }

    // Yellow
    for (int i = 0; i < WORD_LENGTH; ++i) {
        if (used_guess[i]) continue;
        for (int j = 0; j < WORD_LENGTH; ++j) {
            if (!used_target[j] && guess[i] == target[j]) {
                feedback[i] = 'Y';
                used_target[j] = 1;
                break;
            }
        }
    }
    feedback[WORD_LENGTH] = '\0';
}

void print_grid(char guesses[MAX_ATTEMPTS][WORD_LENGTH + 1],
                char feedbacks[MAX_ATTEMPTS][WORD_LENGTH + 1],
                int attempts) {
    printf("\n🎯🟩 WORDLE BOARD 🟨\n");
    for (int i = 0; i < MAX_ATTEMPTS; ++i) {
        if (i >= attempts) {
            for (int j = 0; j < WORD_LENGTH; ++j) printf("[ _ ] ");
            printf("\n");
            continue;
        }
        for (int j = 0; j < WORD_LENGTH; ++j) {
            char ch = guesses[i][j] ? guesses[i][j] : '_';
            if (feedbacks[i][j] == 'G')
                printf("\x1b[42m[%c]\x1b[0m ", toupper(ch));
            else if (feedbacks[i][j] == 'Y')
                printf("\x1b[43m[%c]\x1b[0m ", toupper(ch));
            else
                printf("\x1b[47m[%c]\x1b[0m ", toupper(ch));
        }
        printf("\n");
    }
    printf("\n");
}

// Filter candidate dictionary based on past guesses & feedbacks
int filter_candidates(char candidates[MAX_WORDS][WORD_LENGTH + 1],
                      int cand_count,
                      char guesses[MAX_ATTEMPTS][WORD_LENGTH + 1],
                      char feedbacks[MAX_ATTEMPTS][WORD_LENGTH + 1],
                      int attempts,
                      char out[MAX_WORDS][WORD_LENGTH + 1]) {
    int outc = 0;
    for (int w = 0; w < cand_count; ++w) {
        int valid = 1;
        for (int a = 0; a < attempts && valid; ++a) {
            char temp_feedback[WORD_LENGTH + 1];
            generate_feedback(guesses[a], candidates[w], temp_feedback);
            if (strcmp(temp_feedback, feedbacks[a]) != 0) valid = 0;
        }
        if (valid) {
            strcpy(out[outc++], candidates[w]);
        }
    }
    return outc;
}

// compute letter frequencies across candidate list
void compute_letter_freq(char candidates[MAX_WORDS][WORD_LENGTH + 1], int cand_count, int freq[26]) {
    for (int i = 0; i < 26; ++i) freq[i] = 0;
    for (int i = 0; i < cand_count; ++i) {
        int seen[26] = {0};
        for (int j = 0; j < WORD_LENGTH; ++j) {
            int idx = candidates[i][j] - 'a';
            if (!seen[idx]) {
                freq[idx]++;
                seen[idx] = 1;
            }
        }
    }
}

// choose best word from candidates based on letter frequency
char *best_word_by_freq(char candidates[MAX_WORDS][WORD_LENGTH + 1], int cand_count, int freq[26]) {
    int best_score = -1;
    int best_idx = -1;
    for (int i = 0; i < cand_count; ++i) {
        int score = 0;
        int seen[26] = {0};
        for (int j = 0; j < WORD_LENGTH; ++j) {
            int idx = candidates[i][j] - 'a';
            if (!seen[idx]) {
                score += freq[idx];
                seen[idx] = 1;
            }
        }
        if (score > best_score) {
            best_score = score;
            best_idx = i;
        }
    }
    if (best_idx >= 0) return candidates[best_idx];
    return NULL;
}

// Solver suggestion
char *solver_suggestion(char dictionary[MAX_WORDS][WORD_LENGTH + 1],
                        int word_count,
                        char feedbacks[MAX_ATTEMPTS][WORD_LENGTH + 1],
                        char guesses[MAX_ATTEMPTS][WORD_LENGTH + 1],
                        int attempts) {
    static char candidates[MAX_WORDS][WORD_LENGTH + 1];
    int cand_count = 0;
    for (int i = 0; i < word_count; ++i) strcpy(candidates[cand_count++], dictionary[i]);

    if (attempts > 0) {
        static char filtered[MAX_WORDS][WORD_LENGTH + 1];
        cand_count = filter_candidates(candidates, cand_count, guesses, feedbacks, attempts, filtered);
        for (int i = 0; i < cand_count; ++i) strcpy(candidates[i], filtered[i]);
    }

    if (cand_count == 0) return NULL;

    int freq[26];
    compute_letter_freq(candidates, cand_count, freq);
    return best_word_by_freq(candidates, cand_count, freq);
}

// Save score
void save_score(const char *player, const char *target, int attempts, double time_taken, int win) {
    FILE *f = fopen("scores.txt", "a");
    if (!f) return;
    fprintf(f, "Player: %s | Word: %s | Attempts: %d | Time: %.1fs | Result: %s\n",
            player, target, attempts, time_taken, win ? "WIN" : "LOSE");
    fclose(f);
}

void show_scores() {
    FILE *f = fopen("scores.txt", "r");
    if (!f) {
        printf("📂 Aucun score trouvé.\n");
        printf("\n🔙 Appuyez sur Entrée pour revenir au menu...");
        getchar();
        return;
    }
    char line[256];
    printf("\n🏆 === HISTORIQUE DES SCORES === 🏆\n\n");
    while (fgets(line, sizeof(line), f)) {
        printf("%s", line);
    }
    fclose(f);
    printf("\n🔙 Appuyez sur Entrée pour revenir au menu...");
    getchar();
}

void wait_enter_clear() {
    printf("\n🔙 Appuyez sur Entrée pour continuer...");
    getchar();
}

// Sleep helper
void sleep_ms(int ms) {
#ifdef _WIN32
    Sleep(ms);
#else
    usleep(ms * 1000);
#endif
}

// Main game function
void play_game(char dictionary[MAX_WORDS][WORD_LENGTH + 1], int word_count,
               const char *player, int *total_games, int *total_wins) {
    clear_screen();
    const char *target = dictionary[rand() % word_count];

    static char guesses[MAX_ATTEMPTS][WORD_LENGTH + 1];
    static char feedbacks[MAX_ATTEMPTS][WORD_LENGTH + 1];
    for (int i = 0; i < MAX_ATTEMPTS; ++i) {
        guesses[i][0] = '\0';
        feedbacks[i][0] = '\0';
    }

    int letter_status[26] = {0};
    int attempts = 0;
    int mode;

    printf("\n🎯 Bienvenue dans WORDLE en C !\n");
    printf("Choisissez un mode :\n");
    printf(" (1) Jeu manuel\n (2) Mode solver (suggestions automatiques)\n (3) Mode Auto Solver (IA joue seule)\n");
    printf("Votre choix : ");
    if (scanf("%d", &mode) != 1) mode = 1;
    getchar();

    time_t start_time = time(NULL);

    if (mode == 2) {
        int freq[26];
        compute_letter_freq(dictionary, word_count, freq);
        char *start = best_word_by_freq(dictionary, word_count, freq);
        if (start) printf("💡 Meilleur mot de départ (heuristique): %s\n", start);
    }

    while (attempts < MAX_ATTEMPTS) {
        print_grid(guesses, feedbacks, attempts);
        print_keyboard(letter_status);

        char guess[WORD_LENGTH + 1];

        if (mode == 3) { // Auto solver
            char *sugg = solver_suggestion(dictionary, word_count, feedbacks, guesses, attempts);
            if (!sugg) break;
            strcpy(guess, sugg);
            printf("🤖 IA choisit : %s\n", guess);
            sleep_ms(500);
        } else { // Manual / suggestions
            char guess_in[64];
            printf("Essai %d/%d - Entrez un mot de %d lettres : ", attempts + 1, MAX_ATTEMPTS, WORD_LENGTH);
            if (!fgets(guess_in, sizeof(guess_in), stdin)) { clearerr(stdin); continue; }
            trim_newline(guess_in);
            int pos = 0;
            for (size_t i = 0; i < strlen(guess_in) && pos < WORD_LENGTH; ++i) {
                if (isalpha((unsigned char)guess_in[i])) guess[pos++] = tolower((unsigned char)guess_in[i]);
            }
            guess[pos] = '\0';
            if (strlen(guess) != WORD_LENGTH) { printf("❌ Entrez exactement %d lettres.\n", WORD_LENGTH); continue; }
            if (!is_valid_word(guess, dictionary, word_count)) { printf("🚫 '%s' n'existe pas dans le dictionnaire.\n", guess); continue; }
        }

        strcpy(guesses[attempts], guess);
        generate_feedback(guess, target, feedbacks[attempts]);
        update_letter_status(letter_status, guess, feedbacks[attempts]);

        if (strcmp(guess, target) == 0) {
            time_t end_time = time(NULL);
            double seconds = difftime(end_time, start_time);
            print_grid(guesses, feedbacks, attempts + 1);
            printf("🎉 Bravo %s ! Mot trouvé : %s\n", player, target);
            printf("🕐 Temps : %.1f secondes\n", seconds);
            printf("📊 Tentatives : %d / %d\n", attempts + 1, MAX_ATTEMPTS);
            save_score(player, target, attempts + 1, seconds, 1);
            (*total_wins)++;
            break;
        }

        attempts++;

        if (mode == 2) {
            char *sugg = solver_suggestion(dictionary, word_count, feedbacks, guesses, attempts);
            if (sugg) printf("💡 Suggestion solver : %s\n", sugg);
            else printf("🤖 Aucune suggestion valide.\n");
        }
    }

    (*total_games)++;

    if (attempts == MAX_ATTEMPTS && strcmp(guesses[MAX_ATTEMPTS-1], target) != 0) {
        time_t end_time = time(NULL);
        double seconds = difftime(end_time, start_time);
        print_grid(guesses, feedbacks, attempts);
        printf("❌ Perdu %s ! Le mot était : %s\n", player, target);
        printf("🕐 Temps total : %.1f secondes\n", seconds);
        save_score(player, target, attempts, seconds, 0);
    }

    wait_enter_clear();
}

int main() {
    srand((unsigned int)time(NULL));

    char dictionary[MAX_WORDS][WORD_LENGTH + 1];
    int word_count = 0;

    int lang_choice;
    printf("🌐 Choisissez la langue / Choose language :\n");
    printf("1 - Français (words_fr.txt)\n2 - English (words_en.txt)\n3 - Fichier par defaut (words.txt)\nVotre choix : ");
    if (scanf("%d", &lang_choice) != 1) lang_choice = 3;
    getchar();
    const char *dict_file = "words.txt";
    if (lang_choice == 1) dict_file = "words_fr.txt";
    else if (lang_choice == 2) dict_file = "words_en.txt";

    word_count = load_dictionary(dict_file, dictionary);
    if (word_count == 0) {
        printf("❌ Aucun mot chargé depuis '%s'. Assurez-vous que le fichier existe et contient des mots de %d lettres.\n", dict_file, WORD_LENGTH);
        return 1;
    }

    char player[50];
    printf("👤 Entrez votre nom : ");
    if (scanf("%49s", player) != 1) strcpy(player, "Player");
    getchar();

    int total_games = 0;
    int total_wins = 0;

    while (1) {
        clear_screen();
        printf("🎮 === MENU PRINCIPAL === 🎯\n");
        printf("1️⃣  Jouer à Wordle\n");
        printf("2️⃣  Voir les scores\n");
        printf("3️⃣  Quitter\n");
        printf("\nVos stats : %d parties | %d victoires 🏆\n", total_games, total_wins);
        printf("\n➡️  Choisissez une option : ");

        int choice;
        if (scanf("%d", &choice) != 1) { choice = 0; clearerr(stdin); }
        getchar();

        if (choice == 1) {
            play_game(dictionary, word_count, player, &total_games, &total_wins);
        } else if (choice == 2) {
            show_scores();
        } else if (choice == 3) {
            printf("👋 Merci d'avoir joué, %s ! À bientôt.\n", player);
            break;
        } else {
            printf("❌ Choix invalide.\n");
            wait_enter_clear();
        }
    }

    return 0;
}
