#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_WORDS 10000
#define WORD_LENGTH 5 
#define MAX_ATTEMPTS 6 

// 🔹 Charger le dictionnaire
int load_dictionary(const char *filename, char words[MAX_WORDS][WORD_LENGTH + 1]) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Erreur : impossible d'ouvrir le fichier '%s'\n", filename);
        return 0;
    }

    int count = 0;
    while (fscanf(file, "%5s", words[count]) == 1) {
        for (int i = 0; i < WORD_LENGTH; i++) {
            if (words[count][i] >= 'A' && words[count][i] <= 'Z')
                words[count][i] += 32; // Convertir en minuscules
        }
        count++;
        if (count >= MAX_WORDS) break;
    }

    fclose(file);
    return count;
}

// 🔹 Vérifier si le mot existe dans le dictionnaire
int is_valid_word(const char *guess, char dictionary[MAX_WORDS][WORD_LENGTH + 1], int word_count) {
    for (int i = 0; i < word_count; i++) {
        if (strcmp(guess, dictionary[i]) == 0)
            return 1;
    }
    return 0;
}

// 🔹 Générer le feedback (G, Y, _)
void generate_feedback(const char *guess, const char *target, char *feedback) {
    int used_target[WORD_LENGTH] = {0};
    int used_guess[WORD_LENGTH] = {0};

    // Vert
    for (int i = 0; i < WORD_LENGTH; i++) {
        if (guess[i] == target[i]) {
            feedback[i] = 'G';
            used_target[i] = 1;
            used_guess[i] = 1;
        } else {
            feedback[i] = '_';
        }
    }

    // Jaune
    for (int i = 0; i < WORD_LENGTH; i++) {
        if (used_guess[i]) continue;
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

// 🔹 Afficher la grille Wordle
void print_grid(char guesses[MAX_ATTEMPTS][WORD_LENGTH + 1],
                char feedbacks[MAX_ATTEMPTS][WORD_LENGTH + 1],
                int attempts) {
    printf("\n🟩 WORDLE BOARD 🟨\n");
    for (int i = 0; i < MAX_ATTEMPTS; i++) {
        if (i >= attempts) {
            for (int j = 0; j < WORD_LENGTH; j++)
                printf("[ _ ] ");
            printf("\n");
            continue;
        }

        for (int j = 0; j < WORD_LENGTH; j++) {
            if (feedbacks[i][j] == 'G')
                printf("\x1b[42m[%c]\x1b[0m ", guesses[i][j]); // vert
            else if (feedbacks[i][j] == 'Y')
                printf("\x1b[43m[%c]\x1b[0m ", guesses[i][j]); // jaune
            else
                printf("\x1b[47m[%c]\x1b[0m ", guesses[i][j]); // gris
        }
        printf("\n");
    }
    printf("\n");
}

// 🔹 Fonction solver (suggestion automatique)
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

// 🔹 Sauvegarder les scores dans un fichier
void save_score(const char *player, const char *target, int attempts, double time_taken, int win) {
    FILE *f = fopen("scores.txt", "a");
    if (!f) return;
    fprintf(f, "Player: %s | Word: %s | Attempts: %d | Time: %.1fs | Result: %s\n",
            player, target, attempts, time_taken, win ? "WIN" : "LOSE");
    fclose(f);
}

// 🔹 Fonction principale
int main() {
    srand(time(NULL));

    char dictionary[MAX_WORDS][WORD_LENGTH + 1];
    int word_count = load_dictionary("words.txt", dictionary);

    if (word_count == 0) {
        printf("❌ Aucun mot chargé.\n");
        return 1;
    }

    char player[50];
    printf("👤 Entrez votre nom : ");
    scanf("%49s", player);

    int play_again = 1; // 🔁 Variable qui contrôle la relance du jeu

    while (play_again) {
         system("cls"); // 🔹 Clear screen before starting a new game
        const char *target = dictionary[rand() % word_count];
        char guesses[MAX_ATTEMPTS][WORD_LENGTH + 1];
        char feedbacks[MAX_ATTEMPTS][WORD_LENGTH + 1];
        int attempts = 0;
        int mode;

        printf("\n🎯 Bienvenue dans WORDLE en C !\n");
        printf("Choisissez un mode :\n");
        printf(" (1) Jeu manuel\n (2) Mode solver (suggestions automatiques)\n");
        printf("Votre choix : ");
        scanf("%d", &mode);

        time_t start_time = time(NULL);

        while (attempts < MAX_ATTEMPTS) {
            print_grid(guesses, feedbacks, attempts);

            char guess[WORD_LENGTH + 1];

            if (mode == 2 && attempts > 0) {
                char *suggestion = solver_suggestion(dictionary, word_count, feedbacks, guesses, attempts);
                if (suggestion)
                    printf("💡 Suggestion solver : %s\n", suggestion);
                else
                    printf("🤖 Aucune suggestion valide.\n");
            }

            printf("Essai %d/%d - Entrez un mot de %d lettres : ", attempts + 1, MAX_ATTEMPTS, WORD_LENGTH);
            scanf("%5s", guess);

            for (int i = 0; i < WORD_LENGTH; i++)
                if (guess[i] >= 'A' && guess[i] <= 'Z') guess[i] += 32;

            if (strlen(guess) != WORD_LENGTH) {
                printf("❌ Entrez exactement %d lettres.\n", WORD_LENGTH);
                continue;
            }

            if (!is_valid_word(guess, dictionary, word_count)) {
                printf("🚫 '%s' n'existe pas dans le dictionnaire.\n", guess);
                continue;
            }

            strcpy(guesses[attempts], guess);
            generate_feedback(guess, target, feedbacks[attempts]);

            if (strcmp(guess, target) == 0) {
                time_t end_time = time(NULL);
                double seconds = difftime(end_time, start_time);
                print_grid(guesses, feedbacks, attempts + 1);
                printf("🎉 Bravo %s ! Mot trouvé : %s\n", player, target);
                printf("🕐 Temps : %.1f secondes\n", seconds);
                printf("📊 Tentatives : %d / %d\n", attempts + 1, MAX_ATTEMPTS);
                save_score(player, target, attempts + 1, seconds, 1);
                break;
            }

            attempts++;
        }

        // 🔹 Vérifier si le joueur a perdu
        if (attempts == MAX_ATTEMPTS) {
            time_t end_time = time(NULL);
            double seconds = difftime(end_time, start_time);
            print_grid(guesses, feedbacks, attempts);
            printf("❌ Perdu %s ! Le mot était : %s\n", player, target);
            printf("🕐 Temps total : %.1f secondes\n", seconds);
            save_score(player, target, attempts, seconds, 0);
        }

        // 🔁 Demander si le joueur veut rejouer
        char choice;
        printf("\n🔁 Voulez-vous rejouer ? (o/n) : ");
        scanf(" %c", &choice);

        if (choice == 'o' || choice == 'O')
            play_again = 1;
        else {
            printf("👋 Merci d'avoir joué, %s ! À bientôt.\n", player);
            play_again = 0;
        
        }
    }

    return 0;
}
