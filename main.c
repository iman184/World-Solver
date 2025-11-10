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

// ==================================
// UTILITAIRES
// ==================================

// Cross-platform clear
void clear_screen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

// Sleep helper
void sleep_ms(int ms){
#ifdef _WIN32
    Sleep(ms);
#else
    usleep(ms*1000);
#endif
}

// Trim newline / espaces
void trim_newline(char *s) {
    size_t n = strlen(s);
    while (n > 0 && (s[n-1]=='\n' || s[n-1]=='\r' || isspace((unsigned char)s[n-1]))) {
        s[n-1]='\0';
        n--;
    }
}

// ==================================
// DICTIONNAIRE
// ==================================
int load_dictionary(const char *filename, char words[MAX_WORDS][WORD_LENGTH+1]) {
    FILE *file = fopen(filename,"r");
    if(!file){ printf("Erreur : impossible d'ouvrir '%s'\n",filename); return 0;}
    char buf[128]; int count=0;
    while(fgets(buf,sizeof(buf),file) && count<MAX_WORDS){
        trim_newline(buf);
        char tmp[WORD_LENGTH+2]; int len=0;
        for(size_t i=0;i<strlen(buf)&&len<WORD_LENGTH+1;i++)
            if(isalpha((unsigned char)buf[i])) tmp[len++]=tolower((unsigned char)buf[i]);
        tmp[len]='\0';
        if(len==WORD_LENGTH) strcpy(words[count++],tmp);
    }
    fclose(file);
    return count;
}

int is_valid_word(const char *guess, char dictionary[MAX_WORDS][WORD_LENGTH+1], int word_count){
    for(int i=0;i<word_count;i++) if(strcmp(guess,dictionary[i])==0) return 1;
    return 0;
}

// ==================================
// FEEDBACK / GRID / KEYBOARD
// ==================================
void generate_feedback(const char *guess, const char *target, char *feedback){
    int used_target[WORD_LENGTH]={0};
    int used_guess[WORD_LENGTH]={0};
    for(int i=0;i<WORD_LENGTH;i++){
        if(guess[i]==target[i]){ feedback[i]='G'; used_target[i]=1; used_guess[i]=1; }
        else feedback[i]='_';
    }
    for(int i=0;i<WORD_LENGTH;i++){
        if(used_guess[i]) continue;
        for(int j=0;j<WORD_LENGTH;j++){
            if(!used_target[j] && guess[i]==target[j]){ feedback[i]='Y'; used_target[j]=1; break; }
        }
    }
    feedback[WORD_LENGTH]='\0';
}

void print_grid(char guesses[MAX_ATTEMPTS][WORD_LENGTH+1],
                char feedbacks[MAX_ATTEMPTS][WORD_LENGTH+1],
                int attempts){
    printf("\n🎯🟩 WORDLE BOARD 🟨\n");
    for(int i=0;i<MAX_ATTEMPTS;i++){
        if(i>=attempts){ for(int j=0;j<WORD_LENGTH;j++) printf("[ _ ] "); printf("\n"); continue;}
        for(int j=0;j<WORD_LENGTH;j++){
            char ch = guesses[i][j]?guesses[i][j]:'_';
            if(feedbacks[i][j]=='G') printf("\x1b[42m[%c]\x1b[0m ",toupper(ch));
            else if(feedbacks[i][j]=='Y') printf("\x1b[43m[%c]\x1b[0m ",toupper(ch));
            else printf("\x1b[47m[%c]\x1b[0m ",toupper(ch));
        }
        printf("\n");
    }
    printf("\n");
}

void update_letter_status(int status[26], const char *guess, const char *feedback){
    for(int i=0;i<WORD_LENGTH;i++){
        int idx = guess[i]-'a';
        if(feedback[i]=='G') status[idx]=3;
        else if(feedback[i]=='Y' && status[idx]<2) status[idx]=2;
        else if(feedback[i]=='_' && status[idx]==0) status[idx]=1;
    }
}

void print_keyboard(int status[26]){
    const char *rows[]={"qwertyuiop","asdfghjkl","zxcvbnm"};
    printf("\nClavier: ");
    for(int r=0;r<3;r++){
        printf("\n");
        for(int i=0;i<(int)strlen(rows[r]);i++){
            char c=rows[r][i];
            int s=status[c-'a'];
            if(s==3) printf("\x1b[42m %c \x1b[0m ",toupper(c));
            else if(s==2) printf("\x1b[43m %c \x1b[0m ",toupper(c));
            else if(s==1) printf("\x1b[47m %c \x1b[0m ",toupper(c));
            else printf("[ %c ] ",toupper(c));
        }
    }
    printf("\n\n");
}

// ==================================
// SOLVER / CANDIDATES
// ==================================
int filter_candidates(char candidates[MAX_WORDS][WORD_LENGTH+1], int cand_count,
                      char guesses[MAX_ATTEMPTS][WORD_LENGTH+1],
                      char feedbacks[MAX_ATTEMPTS][WORD_LENGTH+1],
                      int attempts,
                      char out[MAX_WORDS][WORD_LENGTH+1]){
    int outc=0;
    for(int w=0;w<cand_count;w++){
        int valid=1;
        for(int a=0;a<attempts && valid;a++){
            char temp_feedback[WORD_LENGTH+1];
            generate_feedback(guesses[a],candidates[w],temp_feedback);
            if(strcmp(temp_feedback,feedbacks[a])!=0) valid=0;
        }
        if(valid) strcpy(out[outc++],candidates[w]);
    }
    return outc;
}

void compute_letter_freq(char candidates[MAX_WORDS][WORD_LENGTH+1], int cand_count, int freq[26]){
    for(int i=0;i<26;i++) freq[i]=0;
    for(int i=0;i<cand_count;i++){
        int seen[26]={0};
        for(int j=0;j<WORD_LENGTH;j++){
            int idx=candidates[i][j]-'a';
            if(!seen[idx]){ freq[idx]++; seen[idx]=1; }
        }
    }
}

char* best_word_by_freq(char candidates[MAX_WORDS][WORD_LENGTH+1], int cand_count, int freq[26]){
    int best_score=-1,best_idx=-1;
    for(int i=0;i<cand_count;i++){
        int score=0,seen[26]={0};
        for(int j=0;j<WORD_LENGTH;j++){
            int idx=candidates[i][j]-'a';
            if(!seen[idx]){ score+=freq[idx]; seen[idx]=1; }
        }
        if(score>best_score){ best_score=score; best_idx=i; }
    }
    if(best_idx>=0) return candidates[best_idx];
    return NULL;
}

char* solver_suggestion(char dictionary[MAX_WORDS][WORD_LENGTH+1], int word_count,
                        char feedbacks[MAX_ATTEMPTS][WORD_LENGTH+1],
                        char guesses[MAX_ATTEMPTS][WORD_LENGTH+1],
                        int attempts){
    static char candidates[MAX_WORDS][WORD_LENGTH+1];
    int cand_count=0;
    for(int i=0;i<word_count;i++) strcpy(candidates[cand_count++],dictionary[i]);
    if(attempts>0){
        static char filtered[MAX_WORDS][WORD_LENGTH+1];
        cand_count=filter_candidates(candidates,cand_count,guesses,feedbacks,attempts,filtered);
        for(int i=0;i<cand_count;i++) strcpy(candidates[i],filtered[i]);
    }
    if(cand_count==0) return NULL;
    int freq[26]; compute_letter_freq(candidates,cand_count,freq);
    return best_word_by_freq(candidates,cand_count,freq);
}

// ==================================
// SCORES
// ==================================
void save_score(const char *player, const char *target, int attempts, double time_taken, int win){
    FILE *f=fopen("scores.txt","a");
    if(!f) return;
    fprintf(f,"Player: %s | Word: %s | Attempts: %d | Time: %.1fs | Result: %s\n",
            player,target,attempts,time_taken,win?"WIN":"LOSE");
    fclose(f);
}

void show_scores(){
    FILE *f=fopen("scores.txt","r");
    if(!f){ printf("📂 Aucun score trouvé.\n"); printf("\n🔙 Appuyez sur Entrée pour revenir au menu..."); getchar(); return; }
    char line[256];
    printf("\n🏆 === HISTORIQUE DES SCORES === 🏆\n\n");
    while(fgets(line,sizeof(line),f)) printf("%s",line);
    fclose(f);
    printf("\n🔙 Appuyez sur Entrée pour revenir au menu..."); getchar();
}

void wait_enter_clear(){ printf("\n🔙 Appuyez sur Entrée pour continuer..."); getchar(); }

// ==================================
// GAME CLASSIQUE
// ==================================
void play_game(char dictionary[MAX_WORDS][WORD_LENGTH+1], int word_count,
               const char *player, int *total_games, int *total_wins){
    clear_screen();
    const char *target=dictionary[rand()%word_count];
    char guesses[MAX_ATTEMPTS][WORD_LENGTH+1]={{0}};
    char feedbacks[MAX_ATTEMPTS][WORD_LENGTH+1]={{0}};
    int letter_status[26]={0};
    int attempts=0;
    int mode;
    printf("\n🎯 Bienvenue dans WORDLE en C !\n");
    printf("Choisissez un mode :\n");
    printf(" (1) Jeu manuel\n (2) Mode solver (suggestions automatiques)\n (3) Mode Auto Solver (IA joue seule)\n");
    printf("Votre choix : ");
    if(scanf("%d",&mode)!=1) mode=1; getchar();
    time_t start_time=time(NULL);
    if(mode==2){ int freq[26]; compute_letter_freq(dictionary,word_count,freq); char *start=best_word_by_freq(dictionary,word_count,freq); if(start) printf("💡 Meilleur mot de départ : %s\n",start); }

    while(attempts<MAX_ATTEMPTS){
        print_grid(guesses,feedbacks,attempts);
        print_keyboard(letter_status);
        char guess[WORD_LENGTH+1];
        if(mode==3){
            char *sugg=solver_suggestion(dictionary,word_count,feedbacks,guesses,attempts);
            if(!sugg) break;
            strcpy(guess,sugg);
            printf("🤖 IA choisit : %s\n",guess);
            sleep_ms(500);
        } else {
            while(1){
                char input[64];
                printf("Essai %d/%d - Entrez un mot : ",attempts+1,MAX_ATTEMPTS);
                if(!fgets(input,sizeof(input),stdin)){ clearerr(stdin); continue; }
                trim_newline(input);
                int pos=0; for(size_t i=0;i<strlen(input)&&pos<WORD_LENGTH;i++) if(isalpha((unsigned char)input[i])) guess[pos++]=tolower((unsigned char)input[i]);
                guess[pos]='\0';
                if(strlen(guess)!=WORD_LENGTH){ printf("❌ Entrez %d lettres.\n",WORD_LENGTH); continue;}
                if(!is_valid_word(guess,dictionary,word_count)){ printf("🚫 Mot invalide.\n"); continue;}
                break;
            }
        }
        strcpy(guesses[attempts],guess);
        generate_feedback(guess,target,feedbacks[attempts]);
        update_letter_status(letter_status,guess,feedbacks[attempts]);
        if(strcmp(guess,target)==0){
            time_t end_time=time(NULL);
            double seconds=difftime(end_time,start_time);
            print_grid(guesses,feedbacks,attempts+1);
            printf("🎉 Bravo %s ! Mot trouvé : %s\n",player,target);
            printf("🕐 Temps : %.1f s | Tentatives : %d/%d\n",seconds,attempts+1,MAX_ATTEMPTS);
            save_score(player,target,attempts+1,seconds,1);
            (*total_wins)++;
            break;
        }
        attempts++;
        if(mode==2){ char *sugg=solver_suggestion(dictionary,word_count,feedbacks,guesses,attempts); if(sugg) printf("💡 Suggestion solver : %s\n",sugg); }
    }
    (*total_games)++;
    if(attempts==MAX_ATTEMPTS && strcmp(guesses[MAX_ATTEMPTS-1],target)!=0){
        time_t end_time=time(NULL); double seconds=difftime(end_time,start_time);
        print_grid(guesses,feedbacks,attempts);
        printf("❌ Perdu %s ! Le mot était : %s\n",player,target);
        printf("🕐 Temps total : %.1f s\n",seconds);
        save_score(player,target,attempts,seconds,0);
    }
    wait_enter_clear();
}

// ==================================
// MODE DUEL
// ==================================
void play_duel(char dictionary[MAX_WORDS][WORD_LENGTH+1], int word_count){
    clear_screen();
    printf("🎯 Mode Duel :\n1️⃣ Joueur vs Joueur\n2️⃣ Joueur vs IA\nVotre choix : ");
    int duel_mode; if(scanf("%d",&duel_mode)!=1) duel_mode=1; getchar();

    char player1[50], player2[50];
    printf("👤 Joueur 1 : "); scanf("%49s",player1); getchar();
    if(duel_mode==1){ printf("👤 Joueur 2 : "); scanf("%49s",player2); getchar(); } else strcpy(player2,"IA");

    const char *target=dictionary[rand()%word_count];
    char guesses1[MAX_ATTEMPTS][WORD_LENGTH+1]={{0}}, feedbacks1[MAX_ATTEMPTS][WORD_LENGTH+1]={{0}};
    char guesses2[MAX_ATTEMPTS][WORD_LENGTH+1]={{0}}, feedbacks2[MAX_ATTEMPTS][WORD_LENGTH+1]={{0}};
    int winner=0;

    for(int turn=0;turn<MAX_ATTEMPTS && winner==0;turn++){
        // Joueur 1
        printf("\n🔹 %s joue (tour %d/%d)\n",player1,turn+1,MAX_ATTEMPTS);
        char guess[WORD_LENGTH+1];
        while(1){
            printf("Entrez un mot : "); char input[64]; if(!fgets(input,sizeof(input),stdin)){clearerr(stdin);continue;}
            trim_newline(input); int pos=0; for(size_t i=0;i<strlen(input)&&pos<WORD_LENGTH;i++) if(isalpha((unsigned char)input[i])) guess[pos++]=tolower((unsigned char)input[i]); guess[pos]='\0';
            if(strlen(guess)!=WORD_LENGTH){ printf("❌ Entrez %d lettres.\n",WORD_LENGTH); continue;}
            if(!is_valid_word(guess,dictionary,word_count)){ printf("🚫 Mot invalide.\n"); continue;}
            break;
        }
        strcpy(guesses1[turn],guess);
        generate_feedback(guess,target,feedbacks1[turn]);
        print_grid(guesses1,feedbacks1,turn+1);
        if(strcmp(guess,target)==0){ winner=1; break; }

        // Joueur 2 / IA
        printf("\n🔹 %s joue (tour %d/%d)\n",player2,turn+1,MAX_ATTEMPTS);
        if(duel_mode==2){
            char *sugg=solver_suggestion(dictionary,word_count,feedbacks2,guesses2,turn);
            if(!sugg){ printf("IA ne peut pas trouver.\n"); continue; }
            strcpy(guess,sugg); printf("🤖 IA choisit : %s\n",guess); sleep_ms(500);
        } else {
            while(1){
                printf("Entrez un mot : "); char input[64]; if(!fgets(input,sizeof(input),stdin)){clearerr(stdin);continue;}
                trim_newline(input); int pos=0; for(size_t i=0;i<strlen(input)&&pos<WORD_LENGTH;i++) if(isalpha((unsigned char)input[i])) guess[pos++]=tolower((unsigned char)input[i]); guess[pos]='\0';
                if(strlen(guess)!=WORD_LENGTH){ printf("❌ Entrez %d lettres.\n",WORD_LENGTH); continue;}
                if(!is_valid_word(guess,dictionary,word_count)){ printf("🚫 Mot invalide.\n"); continue;}
                break;
            }
        }
        strcpy(guesses2[turn],guess);
        generate_feedback(guess,target,feedbacks2[turn]);
        print_grid(guesses2,feedbacks2,turn+1);
        if(strcmp(guess,target)==0){ winner=2; break; }
    }

    if(winner==0) printf("\n⚠️ Match nul ! Le mot était : %s\n",target);
    else if(winner==1) printf("\n🎉 %s a gagné ! 🎉\n",player1);
    else printf("\n🎉 %s a gagné ! 🎉\n",player2);

    wait_enter_clear();
}

// ==================================
// MAIN
// ==================================
int main(){
    srand((unsigned int)time(NULL));
    char dictionary[MAX_WORDS][WORD_LENGTH+1]; int word_count=0;

    // Choix langue
    int lang_choice;
    printf("🌐 Choisissez la langue :\n1 - Français (words_fr.txt)\n2 - English (words_en.txt)\nVotre choix : ");
    if(scanf("%d",&lang_choice)!=1) lang_choice=1; getchar();
    const char *dict_file = (lang_choice==1)?"words_fr.txt":"words_en.txt";
    word_count=load_dictionary(dict_file,dictionary);
    if(word_count==0){ printf("❌ Aucun mot chargé depuis '%s'.\n",dict_file); return 1;}

        char player[50];
    printf("👤 Entrez votre nom : "); scanf("%49s",player); getchar();

    int total_games=0, total_wins=0;
    int choice;

    do {
        clear_screen();
        printf("\n🎯 WORDLE C - Menu Principal\n");
        printf("1️⃣ Jouer (Mode Classique)\n");
        printf("2️⃣ Mode Duel (Joueur vs Joueur / Joueur vs IA)\n");
        printf("3️⃣ Voir les scores\n");
        printf("0️⃣ Quitter\n");
        printf("Votre choix : ");
        if(scanf("%d",&choice)!=1){ clearerr(stdin); choice=-1; getchar(); }
        getchar();

        switch(choice){
            case 1:
                play_game(dictionary, word_count, player, &total_games, &total_wins);
                break;
            case 2:
                play_duel(dictionary, word_count);
                break;
            case 3:
                show_scores();
                break;
            case 0:
                printf("\n👋 Merci d'avoir joué, %s ! Total parties : %d | Victoires : %d\n", player, total_games, total_wins);
                break;
            default:
                printf("❌ Choix invalide.\n"); sleep_ms(1000);
        }
    } while(choice!=0);

    return 0;
}

