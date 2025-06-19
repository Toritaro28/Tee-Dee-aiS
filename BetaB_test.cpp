#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <windows.h>
#include <math.h>
#include <conio.h>
#include <algorithm>
#include <iomanip>

#define MAX_PLAYLISTS 10
#define MAX_NAME_LEN 50
#define MAX_SONGS 100
#define MAX_GENRES 10
#define MAX_LANGUAGES 8
#define MAX_RECOMMENDED_SONGS 20
#define MAX_EMAIL_LEN 100
#define MAX_PASSWORD_LEN 50
#define MAX_ALBUMS 10
#define HASH_TABLE_SIZE 101

using namespace std;

void juztodisplaydesign();
void laodingPage(int timer);
void landingPage();

int custom_strcmpi(const char* s1, const char* s2) {
    while (*s1 && *s2) {
        if (tolower(*s1) != tolower(*s2)) {
            return tolower(*s1) - tolower(*s2);
        }
        s1++;
        s2++;
    }
    return tolower(*s1) - tolower(*s2);
}

char* custom_strstr(const char* haystack, const char* needle) {
    if (!*needle) return (char*)haystack;

    char* h = (char*)haystack;
    while (*h) {
        char* h_ptr = h;
        char* n_ptr = (char*)needle;

        while (*n_ptr && *h_ptr && (tolower(*h_ptr) == tolower(*n_ptr))) {
            h_ptr++;
            n_ptr++;
        }
        if (!*n_ptr) return h;
        h++;
    }
    return NULL;
}

char* custom_strncpy(char* dest, const char* src, size_t n) {
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; ++i) {
        dest[i] = src[i];
    }
    for (; i < n; ++i) {
        dest[i] = '\0';
    }
    if (n > 0) {
        dest[n - 1] = '\0';
    }
    return dest;
}

struct Song {
    char songName[MAX_NAME_LEN];
    char artist[MAX_NAME_LEN];
    char genre[MAX_NAME_LEN];
    char language[MAX_NAME_LEN];
    int favoriteCount;
};

struct Album {
    char albumName[MAX_NAME_LEN];
    char artist[MAX_NAME_LEN];
    char genre[MAX_NAME_LEN];
    char language[MAX_NAME_LEN];
    Song songs[MAX_SONGS];
    int songCount;
    double price;
    int soldCount;

    Album() {
        songCount = 0;
        price = 0.0;
        soldCount = 0;
        strcpy(albumName, "");
        strcpy(artist, "");
        strcpy(genre, "");
        strcpy(language, "");
    }
};

struct Playlist {
    char name[MAX_NAME_LEN];
    Song songs[MAX_SONGS];
    int songCount;

    Playlist() {
        songCount = 0;
        strcpy(name, "");
    }
};

enum PackageType {
    FREE_TIER = 0,
    ONE_MONTH = 1,
    THREE_MONTH = 2,
    PERMANENT = 3
};

struct Subscription {
    PackageType type;
    long long expiryUnixTime;
    int songsListened;

    Subscription() : type(FREE_TIER), expiryUnixTime(0), songsListened(0) {}
};

struct PaymentRecord {
    char transactionId[MAX_NAME_LEN];
    char username[MAX_NAME_LEN];
    char itemName[MAX_NAME_LEN];
    char itemType[20];
    double amount;
    char paymentMethod[MAX_NAME_LEN];
    char paymentDetails[MAX_NAME_LEN];
    long long timestamp;
    char status[20];
    long long purchasedItemExpiry;
};

struct AccountNode {
    char username[MAX_NAME_LEN];
    char password[MAX_PASSWORD_LEN];
    char email[MAX_EMAIL_LEN];
    Subscription subscription;
    struct AccountNode* next;
};

unsigned int customHash(const char* key, int tableSize) {
    unsigned int hash = 0;
    for (int i = 0; key[i] != '\0'; i++) {
        hash = hash * 31 + key[i];
    }
    return hash % tableSize;
}

void initHashTable(struct AccountNode* hashTable[], int size) {
    for (int i = 0; i < size; i++) {
        hashTable[i] = NULL;
    }
}

void hashTableInsert(struct AccountNode* hashTable[], int size, const char* username, const char* password, const char* email, PackageType type, long long expiry, int songsListened) {
    unsigned int index = customHash(username, size);
    
    struct AccountNode* newNode = (struct AccountNode*) malloc(sizeof(struct AccountNode));
    if (newNode == NULL) {
        cout << "\tMemory allocation failed during hash table insertion!" << endl;
        return;
    }

    custom_strncpy(newNode->username, username, MAX_NAME_LEN);
    custom_strncpy(newNode->password, password, MAX_PASSWORD_LEN);
    custom_strncpy(newNode->email, email, MAX_EMAIL_LEN);
    newNode->subscription.type = type;
    newNode->subscription.expiryUnixTime = expiry;
    newNode->subscription.songsListened = songsListened;
    
    newNode->next = hashTable[index];
    hashTable[index] = newNode;
}

struct AccountNode* hashTableSearch(const struct AccountNode* const* hashTable, int size, const char* username) {
    unsigned int index = customHash(username, size);
    const struct AccountNode* current = hashTable[index];

    while (current != NULL) {
        if (strcmp(current->username, username) == 0) {
            return (struct AccountNode*)current;
        }
        current = current->next;
    }
    return NULL;
}

void hashTableUpdatePassword(struct AccountNode* hashTable[], int size, const char* username, const char* newPassword) {
    struct AccountNode* node = hashTableSearch((const struct AccountNode* const*)hashTable, size, username);
    if (node != NULL) {
        custom_strncpy(node->password, newPassword, MAX_PASSWORD_LEN);
    }
}

void hashTableUpdateSubscription(struct AccountNode* hashTable[], int size, const char* username, PackageType type, long long expiry, int songsListened) {
    struct AccountNode* node = hashTableSearch((const struct AccountNode* const*)hashTable, size, username);
    if (node != NULL) {
        node->subscription.type = type;
        node->subscription.expiryUnixTime = expiry;
        node->subscription.songsListened = songsListened;
    }
}

void saveHashTableToFile(const struct AccountNode* const* hashTable, int size, const char* filename) {
    FILE* outFile = fopen(filename, "w");
    if (!outFile) {
        cout << "\tError opening file for saving hash table!" << endl;
        return;
    }
    for (int i = 0; i < size; i++) {
        const struct AccountNode* current = hashTable[i];
        while (current != NULL) {
            fprintf(outFile, "%s %s %s %d %lld %d\n",
                    current->username, current->password, current->email,
                    current->subscription.type, current->subscription.expiryUnixTime, current->subscription.songsListened);
            current = current->next;
        }
    }
    fclose(outFile);
}

void loadHashTableFromFile(struct AccountNode* hashTable[], int size, const char* filename) {
    FILE* inFile = fopen(filename, "r");
    if (!inFile) {
        FILE* newFile = fopen(filename, "w");
        if (newFile) fclose(newFile);
        return;
    }

    char username[MAX_NAME_LEN];
    char password[MAX_PASSWORD_LEN];
    char email[MAX_EMAIL_LEN];
    int type_int;
    long long expiry;
    int songsListened;

    while (fscanf(inFile, "%s %s %s %d %lld %d\n", username, password, email, &type_int, &expiry, &songsListened) == 6) {
        hashTableInsert(hashTable, size, username, password, email, (PackageType)type_int, expiry, songsListened);
    }
    fclose(inFile);
}

void freeHashTable(struct AccountNode* hashTable[], int size) {
    for (int i = 0; i < size; i++) {
        struct AccountNode* current = hashTable[i];
        while (current != NULL) {
            struct AccountNode* temp = current;
            current = current->next;
            free(temp);
        }
        hashTable[i] = NULL;
    }
}

class UserBase {
protected:
    char username[MAX_NAME_LEN];
    char password[MAX_PASSWORD_LEN];
    char email[MAX_EMAIL_LEN];

public:
    virtual void login() = 0;
    virtual ~UserBase() {}

    const char* getUsername() const { return username; }
    void setUsername(const char* user) { custom_strncpy(username, user, MAX_NAME_LEN); }

    const char* getPassword() const { return password; }
    void setPassword(const char* pass) { custom_strncpy(password, pass, MAX_PASSWORD_LEN); }

    const char* getEmail() const { return email; }
    void setEmail(const char* mail) { custom_strncpy(email, mail, MAX_EMAIL_LEN); }
    
protected:
    int isLowercase(const char* str) const {
        for (int i = 0; str[i] != '\0'; i++) {
            if (isupper(str[i])) return 0;
        }
        return 1;
    }

    int isValidEmail(const char* email) const {
        char* atPos = strchr(email, '@');
        char* dotComPos = custom_strstr(email, ".com");
        return (atPos != NULL && dotComPos != NULL && dotComPos > atPos);
    }

    int isValidPassword(const char* pass) const {
        if (strlen(pass) < 6) return 0;

        int hasUpper = 0, hasLower = 0, hasDigit = 0, hasSpecial = 0;
        for (int i = 0; pass[i] != '\0'; i++) {
            if (isupper(pass[i])) hasUpper = 1;
            else if (islower(pass[i])) hasLower = 1;
            else if (isdigit(pass[i])) hasDigit = 1;
            else if (ispunct(pass[i])) hasSpecial = 1;
        }
        return hasUpper && hasLower && hasDigit && hasSpecial;
    }
};

class AdminLogin : public UserBase {
private:
    struct AccountNode* adminHashTable[HASH_TABLE_SIZE];
    const char* ADMIN_CODE;
    const char* FILE_NAME;
    int isLoggedIn;

public:
    AdminLogin() : ADMIN_CODE("MMU"), FILE_NAME("admin.txt"), isLoggedIn(0) {
        initHashTable(adminHashTable, HASH_TABLE_SIZE);
        loadHashTableFromFile(adminHashTable, HASH_TABLE_SIZE, FILE_NAME);
    }

    ~AdminLogin() {
        saveHashTableToFile((const struct AccountNode* const*)adminHashTable, HASH_TABLE_SIZE, FILE_NAME);
        freeHashTable(adminHashTable, HASH_TABLE_SIZE);
        cout << "Admin accounts memory released." << endl;
    }

    void signup() {
        char username_input[MAX_NAME_LEN];
        char password_input[MAX_PASSWORD_LEN];
        char confirmPassword_input[MAX_PASSWORD_LEN];
        char email_input[MAX_EMAIL_LEN];
        char adminCode_input[MAX_NAME_LEN];

        system("cls");
        cout << "\n\t*************************************\n";
        cout << "\t*           Admin Signup            *\n";
        cout << "\t*************************************\n";

        while (1) {
            cout << "\n\n\tEnter Admin Code: ";
            if (scanf("%s", adminCode_input) != 1) {
                cout << "\tInvalid input! Please try again." << endl;
                while(getchar() != '\n');
                continue;
            }
            while(getchar() != '\n');
            if (strcmp(adminCode_input, ADMIN_CODE) != 0) {
                cout << "\tInvalid Admin Code! Please try again." << endl;
            } else {
                break;
            }
        }

        while (1) {
            cout << "\n\tEnter username: ";
            if (scanf("%s", username_input) != 1) {
                cout << "\tInvalid input! Please try again." << endl;
                while(getchar() != '\n');
                continue;
            }
            while(getchar() != '\n');
            if (isUsernameExists(username_input)) {
                cout << "\tUsername already exists! Please try again." << endl << endl;
            } else {
                break;
            }
        }

        while (1) {
            cout << "\tEnter email (lowercase only): ";
            if (scanf("%s", email_input) != 1) {
                cout << "\tInvalid input! Please try again." << endl;
                while(getchar() != '\n');
                continue;
            }
            while(getchar() != '\n');
            if (!isLowercase(email_input)) {
                cout << "\tInvalid email! Please enter the email in lowercase only." << endl << endl;
            } else if (!isValidEmail(email_input)) {
                cout << "\tInvalid email format! Please enter a correct email format." << endl << endl;
            } else if (isEmailRegistered(email_input)) {
                cout << "\tEmail already in use! Please try again with a different email." << endl << endl;
            } else {
                break;
            }
        }

        while (1) {
            cout << "\tEnter new password: ";
            if (scanf("%s", password_input) != 1) {
                cout << "\tInvalid input! Please try again." << endl;
                while(getchar() != '\n');
                continue;
            }
            while(getchar() != '\n');
            if (isValidPassword(password_input)) {
                cout << "\tConfirm password: ";
                if (scanf("%s", confirmPassword_input) != 1) {
                    cout << "\tInvalid input! Please try again." << endl;
                    while(getchar() != '\n');
                    continue;
                }
                while(getchar() != '\n');
                if (strcmp(password_input, confirmPassword_input) == 0) {
                    break;
                } else {
                    cout << "\tPasswords do not match! Please try again." << endl << endl;
                }
            } else {
                cout << "\tPassword must be at least 6 characters long, include numbers, punctuation, and be case-sensitive." << endl << endl;
            }
        }

        hashTableInsert(adminHashTable, HASH_TABLE_SIZE, username_input, password_input, email_input, FREE_TIER, 0, 0);
        saveHashTableToFile((const struct AccountNode* const*)adminHashTable, HASH_TABLE_SIZE, FILE_NAME);

        cout << "\n\tAdmin account created successfully!" << endl;
        Sleep(900);
        system("cls");
    }

    void login() {
        char username_input[MAX_NAME_LEN];
        char password_input[MAX_PASSWORD_LEN];
        int attempts = 0;

        system("cls");
        cout << "\n\t*************************************\n";
        cout << "\t*           ADMIN LOGIN             *\n";
        cout << "\t*************************************\n";

        while (attempts < 3) {
            cout << "\n\tEnter username: ";
            if (scanf("%s", username_input) != 1) {
                cout << "\tInvalid input! Please try again." << endl;
                while(getchar() != '\n');
                continue;
            }
            while(getchar() != '\n');
            cout << "\tEnter password: ";
            if (scanf("%s", password_input) != 1) {
                cout << "\tInvalid input! Please try again." << endl;
                while(getchar() != '\n');
                continue;
            }
            while(getchar() != '\n');

            struct AccountNode* adminAccount = hashTableSearch((const struct AccountNode* const*)adminHashTable, HASH_TABLE_SIZE, username_input);
            if (adminAccount != NULL && strcmp(adminAccount->password, password_input) == 0) {
                cout << "\n\tLogin successful!" << endl;
                isLoggedIn = 1;
                Sleep(900);
                system("cls");
                return;
            }

            attempts++;
            if (attempts < 3) {
                cout << "\tInvalid username or password. Please try again." << endl;
            } else {
                cout << "\tToo many failed attempts!" << endl << endl;
                cout << "\tDirect to reset password page....";
                Sleep(60 * 5);

                while (1) {
                    system("cls");
                    cout << "\n\t-------------------------------------\n";
                    cout << "\t|          RESET PASSWORD           |\n";
                    cout << "\t-------------------------------------\n";

                    cout << "\n\t1. Re-enter your registered email\n"
                            "\t2. Return to main menu\n"
                            "\n\tEnter your choice: ";
                    int choice;
                    if (scanf("%d", &choice) != 1) {
                        cout << "\tInvalid choice! Please enter 1 or 2." << endl << endl;
                        while(getchar() != '\n');
                        Sleep(900);
                        continue;
                    }
                    while(getchar() != '\n');

                    if (choice == 1) {
                        char enteredEmail[MAX_EMAIL_LEN];
                        while (1) {
                            cout << "\n\tEnter your registered email: ";
                            if (scanf("%s", enteredEmail) != 1) {
                                cout << "\tInvalid input! Please try again." << endl;
                                while(getchar() != '\n');
                                continue;
                            }
                            while(getchar() != '\n');
                            if (!isLowercase(enteredEmail)) {
                                cout << "\tInvalid email! Please enter the email in lowercase only." << endl;
                                continue;
                            }
                            if (!isValidEmail(enteredEmail)) {
                                cout << "\tInvalid email format!" << endl;
                                continue;
                            }
                            break;
                        }
                        if (recoverAccount(enteredEmail)) {
                            return;
                        } else {
                            cout << "\n\tPlease choose your selection." << endl;
                            Sleep(900);
                            continue;
                        }
                    } else if (choice == 2) {
                        cout << "\n\tReturning to main menu";
                        for (int i = 0; i < 4; i++) {
                            Sleep(500);
                            cout << ".";
                        }
                        system("cls");
                        return;
                    } else {
                        cout << "\tInvalid choice! Please enter 1 or 2." << endl << endl;
                        Sleep(900);
                    }
                }
            }
        }
    }

    int getLoginStatus() const {
        return isLoggedIn;
    }

private:
    int isUsernameExists(const char* username) const {
        return (hashTableSearch((const struct AccountNode* const*)adminHashTable, HASH_TABLE_SIZE, username) != NULL);
    }

    int isEmailRegistered(const char* email) const {
        for (int i = 0; i < HASH_TABLE_SIZE; ++i) {
            const struct AccountNode* current = adminHashTable[i];
            while (current != NULL) {
                if (strcmp(current->email, email) == 0) {
                    return 1;
                }
                current = current->next;
            }
        }
        return 0;
    }

    int recoverAccount(const char* email) {
        for (int i = 0; i < HASH_TABLE_SIZE; ++i) {
            struct AccountNode* current = (struct AccountNode*)adminHashTable[i];
            while (current != NULL) {
                if (strcmp(current->email, email) == 0) {
                    cout << "\tAccount found!";
                    cout << " Username: " << current->username;
                    cout << " Password: " << current->password << endl;

                    char resetChoice_input[MAX_NAME_LEN];
                    while (1) {
                        cout << "\n\tDo you want to reset your password? (yes/no): ";
                        if (scanf("%s", resetChoice_input) != 1) {
                             cout << "\tInvalid input! Please try again." << endl;
                             while(getchar() != '\n');
                             continue;
                        }
                        while(getchar() != '\n');
                        
                        for (int j = 0; resetChoice_input[j] != '\0'; ++j) {
                            resetChoice_input[j] = tolower(resetChoice_input[j]);
                        }

                        if (strcmp(resetChoice_input, "yes") == 0) {
                            system("cls");
                            cout << "\n\t-------------------------------------\n";
                            cout << "\t|          RESET PASSWORD           |\n";
                            cout << "\t-------------------------------------\n";

                            char newPass[MAX_PASSWORD_LEN], confirmPass[MAX_PASSWORD_LEN];
                            while (1) {
                                cout << "\n\tEnter new password: ";
                                if (scanf("%s", newPass) != 1) {
                                    cout << "\tInvalid input!" << endl;
                                    while(getchar() != '\n');
                                    continue;
                                }
                                while(getchar() != '\n');

                                if (isValidPassword(newPass)) {
                                    cout << "\tConfirm password: ";
                                    if (scanf("%s", confirmPass) != 1) {
                                        cout << "\tInvalid input!" << endl;
                                        while(getchar() != '\n');
                                        continue;
                                    }
                                    while(getchar() != '\n');

                                    if (strcmp(newPass, confirmPass) == 0) {
                                        hashTableUpdatePassword(adminHashTable, HASH_TABLE_SIZE, current->username, newPass);
                                        saveHashTableToFile((const struct AccountNode* const*)adminHashTable, HASH_TABLE_SIZE, FILE_NAME);
                                        cout << "\n\tPassword reset successful!" << endl;
                                        Sleep(900);
                                        system("cls");
                                        return 1;
                                    } else {
                                        cout << "\tPasswords do not match! Please try again." << endl;
                                    }
                                } else {
                                    cout << "\tPassword must be at least 6 characters long, include numbers, punctuation, and be case-sensitive." << endl;
                                }
                            }
                        } else if (strcmp(resetChoice_input, "no") == 0) {
                            system("cls");
                            return 1;
                        } else {
                            cout << "\tInvalid choice! Please enter 'yes' or 'no'." << endl;
                        }
                    }
                }
                current = current->next;
            }
        }
        cout << "\tNo account found with the provided email." << endl;
        return 0;
    }
};

class Admin {
public:
    char genres[MAX_GENRES][MAX_NAME_LEN];
    int genreCount;

    char languages[MAX_LANGUAGES][MAX_NAME_LEN];
    int languageCount;

    Song recommendedSongs[MAX_RECOMMENDED_SONGS];
    int recommendedSongCount;

    Album purchasableAlbums[MAX_ALBUMS];
    int purchasableAlbumCount;

    Admin() {
        genreCount = 0;
        languageCount = 0;
        recommendedSongCount = 0;
        purchasableAlbumCount = 0;

        loadGenresLanguages();
        if (genreCount == 0) {
            genreCount = 3;
            strcpy(genres[0], "Rock");
            strcpy(genres[1], "Pop");
            strcpy(genres[2], "Blues");
        }
        if (languageCount == 0) {
            languageCount = 2;
            strcpy(languages[0], "English");
            strcpy(languages[1], "Chinese");
        }

        loadRecommendedSongs();
        if (recommendedSongCount == 0) {
            addDefaultRecommendedSongs();
        }

        loadPurchasableAlbums();
        if (purchasableAlbumCount == 0) {
            addDefaultPurchasableAlbums();
        }
    }

    ~Admin() {
        saveRecommendedSongs();
        savePurchasableAlbums();
        saveGenresLanguages();
        cout << "Thanks for using our PLAYLIST SYSTEM. SEE YOU SOON~" << endl;
    }

    void saveGenresLanguages() {
        FILE* file = fopen("admin_data.txt", "w");
        if (!file) {
            cout << "Error saving admin data!" << endl;
            return;
        }

        fprintf(file, "GENRES:");
        for (int i = 0; i < genreCount; ++i) {
            fprintf(file, "%s%s", genres[i], (i == genreCount - 1 ? "" : ","));
        }
        fprintf(file, "\n");

        fprintf(file, "LANGUAGES:");
        for (int i = 0; i < languageCount; ++i) {
            fprintf(file, "%s%s", languages[i], (i == languageCount - 1 ? "" : ","));
        }
        fprintf(file, "\n");

        fclose(file);
    }

    void loadGenresLanguages() {
        FILE* file = fopen("admin_data.txt", "r");
        if (!file) {
            return;
        }

        char line[512];
        genreCount = 0;
        languageCount = 0;

        while (fgets(line, sizeof(line), file) != NULL) {
            line[strcspn(line, "\n")] = 0;

            if (strncmp(line, "GENRES:", 7) == 0) {
                char* data = line + 7;
                char* token = strtok(data, ",");
                while (token != NULL && genreCount < MAX_GENRES) {
                    custom_strncpy(genres[genreCount++], token, MAX_NAME_LEN);
                    token = strtok(NULL, ",");
                }
            } else if (strncmp(line, "LANGUAGES:", 10) == 0) {
                char* data = line + 10;
                char* token = strtok(data, ",");
                while (token != NULL && languageCount < MAX_LANGUAGES) {
                    custom_strncpy(languages[languageCount++], token, MAX_NAME_LEN);
                    token = strtok(NULL, ",");
                }
            }
        }
        fclose(file);
    }

    void saveRecommendedSongs() {
        FILE* file = fopen("recommended_songs.txt", "w");
        if (!file) {
            cout << "\tError saving recommended songs!" << endl;
            return;
        }
        for (int i = 0; i < recommendedSongCount; ++i) {
            fprintf(file, "%s,%s,%s,%s,%d\n",
                    recommendedSongs[i].songName,
                    recommendedSongs[i].artist,
                    recommendedSongs[i].genre,
                    recommendedSongs[i].language,
                    recommendedSongs[i].favoriteCount);
        }
        fclose(file);
    }

    void loadRecommendedSongs() {
        FILE* file = fopen("recommended_songs.txt", "r");
        if (!file) {
            return;
        }
        char line[256];
        recommendedSongCount = 0;
        while (fgets(line, sizeof(line), file) != NULL && recommendedSongCount < MAX_RECOMMENDED_SONGS) {
            line[strcspn(line, "\n")] = 0;

            char* token = strtok(line, ",");
            if (token) custom_strncpy(recommendedSongs[recommendedSongCount].songName, token, MAX_NAME_LEN); else continue;
            token = strtok(NULL, ",");
            if (token) custom_strncpy(recommendedSongs[recommendedSongCount].artist, token, MAX_NAME_LEN); else continue;
            token = strtok(NULL, ",");
            if (token) custom_strncpy(recommendedSongs[recommendedSongCount].genre, token, MAX_NAME_LEN); else continue;
            token = strtok(NULL, ",");
            if (token) custom_strncpy(recommendedSongs[recommendedSongCount].language, token, MAX_NAME_LEN); else continue;
            token = strtok(NULL, ",");
            if (token) recommendedSongs[recommendedSongCount].favoriteCount = atoi(token); else continue;

            recommendedSongCount++;
        }
        fclose(file);
    }

    void addDefaultRecommendedSongs() {
        Song englishSong;
        strcpy(englishSong.songName, "Imagine");
        strcpy(englishSong.artist, "John Lennon");
        strcpy(englishSong.genre, "Rock");
        strcpy(englishSong.language, "English");
        englishSong.favoriteCount = 0;
        recommendedSongs[recommendedSongCount++] = englishSong;

        Song chineseSong;
        strcpy(chineseSong.songName, "Moon Represent My Heart");
        strcpy(chineseSong.artist, "LiJun Deng");
        strcpy(chineseSong.genre, "Blues");
        strcpy(chineseSong.language, "Chinese");
        chineseSong.favoriteCount = 0;
        recommendedSongs[recommendedSongCount++] = chineseSong;
        saveRecommendedSongs();
    }

    void savePurchasableAlbums() {
        FILE* file = fopen("albums.txt", "w");
        if (!file) {
            cout << "\tError saving purchasable albums!" << endl;
            return;
        }
        for (int i = 0; i < purchasableAlbumCount; ++i) {
            fprintf(file, "%s,%s,%s,%s,%.2f,%d\n",
                    purchasableAlbums[i].albumName,
                    purchasableAlbums[i].artist,
                    purchasableAlbums[i].genre,
                    purchasableAlbums[i].language,
                    purchasableAlbums[i].price,
                    purchasableAlbums[i].soldCount);
            for (int j = 0; j < purchasableAlbums[i].songCount; ++j) {
                fprintf(file, "SONG:%s,%s,%s,%s\n",
                        purchasableAlbums[i].songs[j].songName,
                        purchasableAlbums[i].songs[j].artist,
                        purchasableAlbums[i].songs[j].genre,
                        purchasableAlbums[i].songs[j].language);
            }
        }
        fclose(file);
    }

    void loadPurchasableAlbums() {
        FILE* file = fopen("albums.txt", "r");
        if (!file) {
            return;
        }
        char line[512];
        purchasableAlbumCount = 0;
        Album currentAlbum;

        while (fgets(line, sizeof(line), file) != NULL) {
            line[strcspn(line, "\n")] = 0;

            if (strncmp(line, "SONG:", 5) == 0) {
                char* song_data = line + 5;
                Song newSong;
                newSong.favoriteCount = 0;

                char* token = strtok(song_data, ",");
                if (token) custom_strncpy(newSong.songName, token, MAX_NAME_LEN); else continue;
                token = strtok(NULL, ",");
                if (token) custom_strncpy(newSong.artist, token, MAX_NAME_LEN); else continue;
                token = strtok(NULL, ",");
                if (token) custom_strncpy(newSong.genre, token, MAX_NAME_LEN); else continue;
                token = strtok(NULL, ",");
                if (token) custom_strncpy(newSong.language, token, MAX_NAME_LEN); else continue;

                if (currentAlbum.songCount < MAX_SONGS) {
                    currentAlbum.songs[currentAlbum.songCount++] = newSong;
                }
            } else {
                if (purchasableAlbumCount > 0) {
                    purchasableAlbums[purchasableAlbumCount-1] = currentAlbum;
                }
                
                char* token = strtok(line, ",");
                if (token) custom_strncpy(currentAlbum.albumName, token, MAX_NAME_LEN); else continue;
                token = strtok(NULL, ",");
                if (token) custom_strncpy(currentAlbum.artist, token, MAX_NAME_LEN); else continue;
                token = strtok(NULL, ",");
                if (token) custom_strncpy(currentAlbum.genre, token, MAX_NAME_LEN); else continue;
                token = strtok(NULL, ",");
                if (token) custom_strncpy(currentAlbum.language, token, MAX_NAME_LEN); else continue;
                token = strtok(NULL, ",");
                if (token) currentAlbum.price = atof(token); else continue;
                token = strtok(NULL, ",");
                if (token) currentAlbum.soldCount = atoi(token); else continue;

                currentAlbum.songCount = 0;
                purchasableAlbumCount++;
                if (purchasableAlbumCount > MAX_ALBUMS) {
                    purchasableAlbumCount--; 
                    break;
                }
            }
        }
        if (purchasableAlbumCount > 0) {
            purchasableAlbums[purchasableAlbumCount-1] = currentAlbum;
        }
        fclose(file);
    }

    void addDefaultPurchasableAlbums() {
        Album album1;
        strcpy(album1.albumName, "The Dark Side of the Moon");
        strcpy(album1.artist, "Pink Floyd");
        strcpy(album1.genre, "Rock");
        strcpy(album1.language, "English");
        album1.price = 15.99;
        album1.soldCount = 0;
        
        Song s1_1; strcpy(s1_1.songName, "Money"); strcpy(s1_1.artist, "Pink Floyd"); strcpy(s1_1.genre, "Rock"); strcpy(s1_1.language, "English"); s1_1.favoriteCount=0;
        Song s1_2; strcpy(s1_2.songName, "Time"); strcpy(s1_2.artist, "Pink Floyd"); strcpy(s1_2.genre, "Rock"); strcpy(s1_2.language, "English"); s1_2.favoriteCount=0;
        album1.songs[album1.songCount++] = s1_1;
        album1.songs[album1.songCount++] = s1_2;
        purchasableAlbums[purchasableAlbumCount++] = album1;

        Album album2;
        strcpy(album2.albumName, "Thriller");
        strcpy(album2.artist, "Michael Jackson");
        strcpy(album2.genre, "Pop");
        strcpy(album2.language, "English");
        album2.price = 12.50;
        album2.soldCount = 0;

        Song s2_1; strcpy(s2_1.songName, "Billie Jean"); strcpy(s2_1.artist, "Michael Jackson"); strcpy(s2_1.genre, "Pop"); strcpy(s2_1.language, "English"); s2_1.favoriteCount=0;
        Song s2_2; strcpy(s2_2.songName, "Beat It"); strcpy(s2_2.artist, "Michael Jackson"); strcpy(s2_2.genre, "Pop"); strcpy(s2_2.language, "English"); s2_2.favoriteCount=0;
        album2.songs[album2.songCount++] = s2_1;
        album2.songs[album2.songCount++] = s2_2;
        purchasableAlbums[purchasableAlbumCount++] = album2;

        savePurchasableAlbums();
    }

    void binggoSortSongs(Song songs[], int n) {
        for (int i = 0; i < n - 1; i++) {
            int min_idx = i;
            for (int j = i + 1; j < n; j++) {
                if (strcmp(songs[j].songName, songs[min_idx].songName) < 0) {
                    min_idx = j;
                }
            }
            Song temp = songs[min_idx];
            songs[min_idx] = songs[i];
            songs[i] = temp;
        }
    }

    void searchRecommendedSongsAdmin() {
        char keyword[MAX_NAME_LEN];
        char choice_input[10];

        do {
            system("cls");
            cout << "\n\t|-----------------------------------|\n";
            cout << "\t|     SEARCH RECOMMENDED SONGS      |\n";
            cout << "\t|-----------------------------------|\n";

            cout << "\n\tEnter keyword to search (song name, artist, genre, or language): ";
            if (fgets(keyword, MAX_NAME_LEN, stdin) == NULL) {
                cout << "\tInvalid input!" << endl;
                continue;
            }
            keyword[strcspn(keyword, "\n")] = 0;

            cout << "\n\tSearch Results:\n";
            int found_count = 0;
            for (int i = 0; i < recommendedSongCount; ++i) {
                if (custom_strstr(recommendedSongs[i].songName, keyword) != NULL ||
                    custom_strstr(recommendedSongs[i].artist, keyword) != NULL ||
                    custom_strstr(recommendedSongs[i].genre, keyword) != NULL ||
                    custom_strstr(recommendedSongs[i].language, keyword) != NULL) {
                    cout << "\t- Song: " << recommendedSongs[i].songName
                         << ", Artist: " << recommendedSongs[i].artist
                         << ", Genre: " << recommendedSongs[i].genre
                         << ", Language: " << recommendedSongs[i].language
                         << ", Favorites: " << recommendedSongs[i].favoriteCount << endl;
                    found_count++;
                }
            }

            if (found_count == 0) {
                cout << "\n\tNo matching songs found!" << endl;
            }

            cout << "\n\tDo you want to search again? (yes/no): ";
            if (scanf("%s", choice_input) != 1) {
                cout << "\tInvalid input!" << endl;
                while(getchar() != '\n');
                strcpy(choice_input, "yes");
            }
            while(getchar() != '\n');
            for (int i = 0; choice_input[i] != '\0'; i++) {
                choice_input[i] = tolower(choice_input[i]);
            }

        } while (strcmp(choice_input, "yes") == 0);

        cout << "\n\tReturning to the admin menu..." << endl;
        Sleep(700);
        system("cls");
    }

    void displayAllGenres() const {
        system("cls");
        cout << "\n\t+-----------------------------------+\n";
        cout << "\t|           GENRE LIST              |\n";
        cout << "\t+-----------------------------------+\n";

        if (genreCount == 0) {
            cout << "\n\tNo genres available!" << endl;
            Sleep(900);
            system("cls");
            return;
        }
        cout << "\n\tAvailable Genres:\n";
        for (int i = 0; i < genreCount; i++) {
            cout << "\t" << (i + 1) << ". " << genres[i] << endl;
        }

        cout << "\n\tPress ENTER to return to admin menu..." << endl;
        _getch();
        system("cls");
    }

    void displayAllLanguages() const {
        system("cls");
        cout << "\n\t+-----------------------------------+\n";
        cout << "\t|          LANGUAGE LIST            |\n";
        cout << "\t+-----------------------------------+\n";

        if (languageCount == 0) {
            cout << "\n\tNo languages available!" << endl;
            Sleep(900);
            system("cls");
            return;
        }
        cout << "\n\tAvailable Languages:\n";
        for (int i = 0; i < languageCount; i++) {
            cout << "\t" << (i + 1) << ". " << languages[i] << endl;
        }

        cout << "\n\tPress ENTER to return to admin menu..." << endl;
        _getch();
        system("cls");
    }

    void displayAllRecommendedSongs() {
        system("cls");
        cout << "\n\t+-----------------------------------+\n";
        cout << "\t|     SONG RECOMMENDATION LIST      |\n";
        cout << "\t+-----------------------------------+\n";

        if (recommendedSongCount == 0) {
            cout << "\n\tNo recommended songs available!" << endl;
            Sleep(900);
            system("cls");
            return;
        }
        cout << "\n\tRecommended Songs (Sorted by Name):\n";
        binggoSortSongs(recommendedSongs, recommendedSongCount);

        for (int i = 0; i < recommendedSongCount; i++) {
            cout << "\t" << (i + 1) << ". " << recommendedSongs[i].songName
                 << " by " << recommendedSongs[i].artist
                 << " [" << recommendedSongs[i].genre << ", "
                 << recommendedSongs[i].language << "] - Favorites: "
                 << recommendedSongs[i].favoriteCount << endl;
        }

        cout << "\n\tPress ENTER to return to admin menu..." << endl;
        _getch();
        system("cls");
    }

    void addGenre() {
        system("cls");
        cout << "\n\t+-----------------------------------+\n";
        cout << "\t|           ADD GENRE               |\n";
        cout << "\t+-----------------------------------+\n";

        if (genreCount < MAX_GENRES) {
            char newGenre[MAX_NAME_LEN];
            cout << "\n\tEnter new genre: ";
            if (fgets(newGenre, MAX_NAME_LEN, stdin) == NULL) {
                cout << "\tInvalid input!" << endl;
                Sleep(900);
                system("cls");
                return;
            }
            newGenre[strcspn(newGenre, "\n")] = 0;
            strcpy(genres[genreCount++], newGenre);
            cout << "\n\tGenre added!" << endl;
        } else {
            cout << "\n\tCannot add more genres! Maximum limit reached." << endl;
        }

        Sleep(900);
        system("cls");
    }

    void addLanguage() {
        system("cls");
        cout << "\n\t+-----------------------------------+\n";
        cout << "\t|           ADD LANGUAGE            |\n";
        cout << "\t+-----------------------------------+\n";

        if (languageCount < MAX_LANGUAGES) {
            char newLanguage[MAX_NAME_LEN];
            cout << "\n\tEnter new language: ";
            if (fgets(newLanguage, MAX_NAME_LEN, stdin) == NULL) {
                cout << "\tInvalid input!" << endl;
                Sleep(900);
                system("cls");
                return;
            }
            newLanguage[strcspn(newLanguage, "\n")] = 0;
            strcpy(languages[languageCount++], newLanguage);
            cout << "\n\tLanguage added!" << endl;
        } else {
            cout << "\n\tCannot add more languages! Maximum limit reached." << endl;
        }

        Sleep(900);
        system("cls");
    }

    void addSongToRecommendation() {
        system("cls");
        cout << "\n\t+-----------------------------------+\n";
        cout << "\t|      ADD RECOMMENDATION SONG      |\n";
        cout << "\t+-----------------------------------+\n";

        if (recommendedSongCount < MAX_RECOMMENDED_SONGS) {
            Song newSong;
            newSong.favoriteCount = 0;

            cout << "\n\tEnter song name: ";
            if (fgets(newSong.songName, MAX_NAME_LEN, stdin) == NULL) { cout << "\tInvalid input!" << endl; return; }
            newSong.songName[strcspn(newSong.songName, "\n")] = 0;

            cout << "\tEnter artist: ";
            if (fgets(newSong.artist, MAX_NAME_LEN, stdin) == NULL) { cout << "\tInvalid input!" << endl; return; }
            newSong.artist[strcspn(newSong.artist, "\n")] = 0;

            int genreChoice;
            int validChoice;
            do {
                cout << "\tSelect genre (";
                for (int i = 0; i < genreCount; i++) {
                    cout << (i + 1) << ". " << genres[i] << " ";
                }
                cout << "): ";
                if (scanf("%d", &genreChoice) != 1) {
                    cout << "\tInvalid choice. Please select a valid genre number!" << endl << endl;
                    while(getchar() != '\n');
                    validChoice = 0;
                } else if (genreChoice < 1 || genreChoice > genreCount) {
                    cout << "\tInvalid choice. Please select a valid genre number!" << endl << endl;
                    while(getchar() != '\n');
                    validChoice = 0;
                } else {
                    validChoice = 1;
                }
            } while (!validChoice);
            while(getchar() != '\n');
            strcpy(newSong.genre, genres[genreChoice - 1]);

            int languageChoice;
            do {
                cout << "\tSelect language (";
                for (int i = 0; i < languageCount; i++) {
                    cout << (i + 1) << ". " << languages[i] << " ";
                }
                cout << "): ";
                if (scanf("%d", &languageChoice) != 1) {
                    cout << "\tInvalid choice. Please select a valid language number!" << endl << endl;
                    while(getchar() != '\n');
                    validChoice = 0;
                } else if (languageChoice < 1 || languageChoice > languageCount) {
                    cout << "\tInvalid choice. Please select a valid language number!" << endl << endl;
                    while(getchar() != '\n');
                    validChoice = 0;
                } else {
                    validChoice = 1;
                }
            } while (!validChoice);
            while(getchar() != '\n');
            strcpy(newSong.language, languages[languageChoice - 1]);

            recommendedSongs[recommendedSongCount++] = newSong;
            saveRecommendedSongs();
            cout << "\n\tSong added to recommendations!" << endl;
        } else {
            cout << "\n\tCannot add more recommended songs! Maximum limit reached." << endl;
        }

        Sleep(900);
        system("cls");
    }

    void deleteSongFromRecommendation() {
        system("cls");
        cout << "\n\t+-----------------------------------+\n";
        cout << "\t|     DELETE RECOMMENDATION SONG    |\n";
        cout << "\t+-----------------------------------+\n";

        if (recommendedSongCount == 0) {
            cout << "\n\tNo songs available to delete!" << endl;
            cout << "\n\tPress ENTER to return to admin menu..." << endl;
            _getch();
            system("cls");
            return;
        }

        cout << "\n\tRecommended Songs: \n";
        for (int i = 0; i < recommendedSongCount; i++) {
            cout << "\t" << (i + 1) << ". " << recommendedSongs[i].songName
                 << " by " << recommendedSongs[i].artist << endl;
        }

        int choice;
        cout << "\n\tEnter the number of the song to delete: ";
        if (scanf("%d", &choice) != 1) {
            cout << "\tInvalid input!" << endl;
            while(getchar() != '\n');
            Sleep(900);
            system("cls");
            return;
        }
        while(getchar() != '\n');

        if (choice > 0 && choice <= recommendedSongCount) {
            for (int i = choice - 1; i < recommendedSongCount - 1; i++) {
                recommendedSongs[i] = recommendedSongs[i + 1];
            }
            recommendedSongCount--;
            saveRecommendedSongs();
            cout << "\n\tSong deleted successfully!" << endl;
        } else {
            cout << "\n\tInvalid choice!" << endl;
        }

        Sleep(900);
        system("cls");
    }

    void deleteGenre() {
        system("cls");
        cout << "\n\t+-----------------------------------+\n";
        cout << "\t|           DELETE GENRE            |\n";
        cout << "\t+-----------------------------------+\n";

        if (genreCount == 0) {
            cout << "\n\tNo genres available to delete!" << endl;
            cout << "\n\tPress ENTER to return to admin menu..." << endl;
            _getch();
            system("cls");
            return;
        }

        cout << "\n\tGenres: \n";
        for (int i = 0; i < genreCount; i++) {
            cout << "\t" << (i + 1) << ". " << genres[i] << endl;
        }

        int choice;
        cout << "\n\tEnter the number of the genre to delete: ";
        if (scanf("%d", &choice) != 1) {
             cout << "\tInvalid input!" << endl;
             while(getchar() != '\n');
             Sleep(900);
             system("cls");
             return;
        }
        while(getchar() != '\n');

        if (choice > 0 && choice <= genreCount) {
            for (int i = choice - 1; i < genreCount - 1; i++) {
                strcpy(genres[i], genres[i + 1]);
            }
            genreCount--;
            cout << "\n\tGenre deleted successfully!" << endl;
        } else {
            cout << "\n\tInvalid choice!" << endl;
        }

        Sleep(900);
        system("cls");
    }

    void deleteLanguage() {
        system("cls");
        cout << "\n\t+-----------------------------------+\n";
        cout << "\t|          DELETE LANGUAGE          |\n";
        cout << "\t+-----------------------------------+\n";

        if (languageCount == 0) {
            cout << "\n\tNo languages available to delete!" << endl;
            cout << "\n\tPress ENTER to return to admin menu..." << endl;
            _getch();
            system("cls");
            return;
        }

        cout << "\n\tLanguages: \n";
        for (int i = 0; i < languageCount; i++) {
            cout << "\t" << (i + 1) << ". " << languages[i] << endl;
        }

        int choice;
        cout << "\n\tEnter the number of the language to delete: ";
        if (scanf("%d", &choice) != 1) {
            cout << "\tInvalid input!" << endl;
            while(getchar() != '\n');
            Sleep(900);
            system("cls");
            return;
        }
        while(getchar() != '\n');

        if (choice > 0 && choice <= languageCount) {
            for (int i = choice - 1; i < languageCount - 1; i++) {
                strcpy(languages[i], languages[i + 1]);
            }
            languageCount--;
            cout << "\n\tLanguage deleted successfully!" << endl;
        } else {
            cout << "\n\tInvalid choice!" << endl;
        }

        Sleep(900);
        system("cls");
    }

    void incrementFavoriteCount(const char* songName, const char* artist) {
        for (int i = 0; i < recommendedSongCount; ++i) {
            if (strcmp(recommendedSongs[i].songName, songName) == 0 &&
                strcmp(recommendedSongs[i].artist, artist) == 0) {
                recommendedSongs[i].favoriteCount++;
                saveRecommendedSongs();
                return;
            }
        }
    }

    void addAlbumToSale() {
        system("cls");
        cout << "\n\t+-----------------------------------+\n";
        cout << "\t|        ADD ALBUM FOR SALE         |\n";
        cout << "\t+-----------------------------------+\n";

        if (purchasableAlbumCount >= MAX_ALBUMS) {
            cout << "\n\tCannot add more albums! Maximum limit reached." << endl;
            Sleep(900);
            system("cls");
            return;
        }

        Album newAlbum;
        newAlbum.soldCount = 0;

        cout << "\n\tEnter album name: ";
        if (fgets(newAlbum.albumName, MAX_NAME_LEN, stdin) == NULL) { cout << "\tInvalid input!" << endl; return; }
        newAlbum.albumName[strcspn(newAlbum.albumName, "\n")] = 0;

        cout << "\tEnter artist: ";
        if (fgets(newAlbum.artist, MAX_NAME_LEN, stdin) == NULL) { cout << "\tInvalid input!" << endl; return; }
        newAlbum.artist[strcspn(newAlbum.artist, "\n")] = 0;

        int genreChoice;
        int validChoice;
        do {
            cout << "\tSelect primary genre (";
            for (int i = 0; i < genreCount; i++) {
                cout << (i + 1) << ". " << genres[i] << " ";
            }
            cout << "): ";
            if (scanf("%d", &genreChoice) != 1) {
                cout << "\tInvalid choice. Please select a valid genre number!" << endl << endl;
                while(getchar() != '\n');
                validChoice = 0;
            } else if (genreChoice < 1 || genreChoice > genreCount) {
                cout << "\tInvalid choice. Please select a valid genre number!" << endl << endl;
                while(getchar() != '\n');
                validChoice = 0;
            } else {
                validChoice = 1;
            }
        } while (!validChoice);
        while(getchar() != '\n');
        strcpy(newAlbum.genre, genres[genreChoice - 1]);

        int languageChoice;
        do {
            cout << "\tSelect primary language (";
            for (int i = 0; i < languageCount; i++) {
                cout << (i + 1) << ". " << languages[i] << " ";
            }
            cout << "): ";
            if (scanf("%d", &languageChoice) != 1) {
                cout << "\tInvalid choice. Please select a valid language number!" << endl << endl;
                while(getchar() != '\n');
                validChoice = 0;
            } else if (languageChoice < 1 || languageChoice > languageCount) {
                cout << "\tInvalid choice. Please select a valid language number!" << endl << endl;
                while(getchar() != '\n');
                validChoice = 0;
            } else {
                validChoice = 1;
            }
        } while (!validChoice);
        while(getchar() != '\n');
        strcpy(newAlbum.language, languages[languageChoice - 1]);

        cout << "\tEnter album price: $";
        if (scanf("%lf", &newAlbum.price) != 1) {
            cout << "\tInvalid input! Price must be a number." << endl;
            while(getchar() != '\n');
            Sleep(900); system("cls"); return;
        }
        while(getchar() != '\n');

        int numSongsInAlbum;
        cout << "\tEnter number of songs in this album (max " << MAX_SONGS << "): ";
        if (scanf("%d", &numSongsInAlbum) != 1 || numSongsInAlbum <= 0 || numSongsInAlbum > MAX_SONGS) {
            cout << "\tInvalid input! Please enter a positive number of songs (max " << MAX_SONGS << ")." << endl;
            while(getchar() != '\n');
            Sleep(900); system("cls"); return;
        }
        while(getchar() != '\n');

        for (int i = 0; i < numSongsInAlbum; ++i) {
            cout << "\n\t--- Song " << (i + 1) << " of " << numSongsInAlbum << " ---" << endl;
            Song albumSong;
            albumSong.favoriteCount = 0;

            cout << "\tEnter song name: ";
            if (fgets(albumSong.songName, MAX_NAME_LEN, stdin) == NULL) { cout << "\tInvalid input!" << endl; i--; continue; }
            albumSong.songName[strcspn(albumSong.songName, "\n")] = 0;

            strcpy(albumSong.artist, newAlbum.artist);
            strcpy(albumSong.genre, newAlbum.genre);
            strcpy(albumSong.language, newAlbum.language);

            newAlbum.songs[newAlbum.songCount++] = albumSong;
        }

        purchasableAlbums[purchasableAlbumCount++] = newAlbum;
        savePurchasableAlbums();
        cout << "\n\tAlbum added for sale successfully!" << endl;
        Sleep(900);
        system("cls");
    }

    void displayAllPurchasableAlbums() {
        system("cls");
        cout << "\n\t+-----------------------------------+\n";
        cout << "\t|      DIGITAL ALBUMS FOR SALE      |\n";
        cout << "\t+-----------------------------------+\n";

        if (purchasableAlbumCount == 0) {
            cout << "\n\tNo albums available for sale!" << endl;
            Sleep(900);
            system("cls");
            return;
        }
        cout << "\n\tAvailable Albums:\n";
        for (int i = 0; i < purchasableAlbumCount; i++) {
            cout << "\t" << (i + 1) << ". Album: " << purchasableAlbums[i].albumName
                 << " by " << purchasableAlbums[i].artist
                 << " [" << purchasableAlbums[i].genre << ", "
                 << purchasableAlbums[i].language << "] - Price: $"
                 << fixed << setprecision(2) << purchasableAlbums[i].price
                 << " - Songs: " << purchasableAlbums[i].songCount
                 << " - Sold: " << purchasableAlbums[i].soldCount << endl;
            if (purchasableAlbums[i].songCount > 0) {
                cout << "\t   Songs in album:\n";
                for(int j=0; j<purchasableAlbums[i].songCount; j++) {
                    cout << "\t   - " << purchasableAlbums[i].songs[j].songName << endl;
                }
            }
        }

        cout << "\n\tPress ENTER to return to admin menu..." << endl;
        _getch();
        system("cls");
    }

    void incrementAlbumSoldCount(const char* albumName, const char* artist) {
        for (int i = 0; i < purchasableAlbumCount; ++i) {
            if (strcmp(purchasableAlbums[i].albumName, albumName) == 0 &&
                strcmp(purchasableAlbums[i].artist, artist) == 0) {
                purchasableAlbums[i].soldCount++;
                savePurchasableAlbums();
                return;
            }
        }
    }
};

class UserLogin;
class Admin;

class User {
public:
    Playlist playlists[MAX_PLAYLISTS];
    int playlistCount;
    char currentUsername[MAX_NAME_LEN];

    User();

    void setCurrentUsername(const char* username);

    long long getCurrentUnixTime();

    void purchasePackage(UserLogin* userLogin, Admin* admin_ptr);
    void purchaseDigitalAlbum(UserLogin* userLogin, Admin* admin_ptr);

    int processPayment(const char* username_payer, const char* itemName, const char* itemType, double amount, long long itemExpiry);

    void savePaymentRecord(const PaymentRecord* record);
    
    void createPlaylist(Admin* admin_ptr);

    void deletePlaylist(); 

    void searchPlaylist();

    void updatePlaylist();
    
    void addSong(Admin* admin_ptr);

    void deleteSong();

    void importPlaylist();

    void downloadPlaylist();

    void juztodisplaydesign();

    void playPlaylist(UserLogin* userLogin);

    void savePlaylists();
    void loadPlaylists();
    
    friend void displayAllSongsInPlaylists(const User* user);
    friend void mergePlaylistsByGenreOrLanguage(User* user, Admin* admin_ptr);
};

void User::savePlaylists() {
    char filename[MAX_NAME_LEN + 15];
    sprintf(filename, "%s_playlists.txt", currentUsername);

    FILE* file = fopen(filename, "w");
    if (!file) {
        cout << "Error: Could not save playlists for " << currentUsername << "!" << endl;
        return;
    }

    for (int i = 0; i < playlistCount; ++i) {
        fprintf(file, "PLAYLIST_NAME:%s\n", playlists[i].name);
        for (int j = 0; j < playlists[i].songCount; ++j) {
            fprintf(file, "SONG:%s,%s,%s,%s\n",
                    playlists[i].songs[j].songName,
                    playlists[i].songs[j].artist,
                    playlists[i].songs[j].genre,
                    playlists[i].songs[j].language);
        }
    }
    fclose(file);
}

void User::loadPlaylists() {
    char filename[MAX_NAME_LEN + 15];
    sprintf(filename, "%s_playlists.txt", currentUsername);

    FILE* file = fopen(filename, "r");
    if (!file) {
        return;
    }

    playlistCount = 0;
    char line[512];
    
    while (fgets(line, sizeof(line), file) != NULL && playlistCount < MAX_PLAYLISTS) {
        line[strcspn(line, "\n")] = 0;

        if (strncmp(line, "PLAYLIST_NAME:", 14) == 0) {
            char* playlist_name_data = line + 14;
            Playlist newPlaylist;
            custom_strncpy(newPlaylist.name, playlist_name_data, MAX_NAME_LEN);
            newPlaylist.songCount = 0;
            playlists[playlistCount++] = newPlaylist;
        } else if (strncmp(line, "SONG:", 5) == 0) {
            if (playlistCount > 0) {
                char* song_data = line + 5;
                Song newSong;
                newSong.favoriteCount = 0;

                char* token = strtok(song_data, ",");
                if (token) custom_strncpy(newSong.songName, token, MAX_NAME_LEN); else continue;
                token = strtok(NULL, ",");
                if (token) custom_strncpy(newSong.artist, token, MAX_NAME_LEN); else continue;
                token = strtok(NULL, ",");
                if (token) custom_strncpy(newSong.genre, token, MAX_NAME_LEN); else continue;
                token = strtok(NULL, ",");
                if (token) custom_strncpy(newSong.language, token, MAX_NAME_LEN); else continue;
                
                if (playlists[playlistCount - 1].songCount < MAX_SONGS) {
                    playlists[playlistCount - 1].songs[playlists[playlistCount - 1].songCount++] = newSong;
                } else {
                    cout << "Warning: Playlist '" << playlists[playlistCount - 1].name << "' reached max song limit during load." << endl;
                }
            }
        }
    }
    fclose(file);
}

class UserLogin : public UserBase {
public:
    struct AccountNode* userHashTable[HASH_TABLE_SIZE];
    const char* FILE_NAME;

    UserLogin() : FILE_NAME("user.txt") {
        initHashTable(userHashTable, HASH_TABLE_SIZE);
        loadHashTableFromFile(userHashTable, HASH_TABLE_SIZE, FILE_NAME);
    }

    ~UserLogin() {
        saveHashTableToFile((const struct AccountNode* const*)userHashTable, HASH_TABLE_SIZE, FILE_NAME);
        freeHashTable(userHashTable, HASH_TABLE_SIZE);
        cout << "User accounts memory released." << endl;
    }

    void registerUser() {
        char newUser_input[MAX_NAME_LEN];
        char newPass_input[MAX_PASSWORD_LEN];
        char confirmPass_input[MAX_PASSWORD_LEN];
        char newEmail_input[MAX_EMAIL_LEN];

        system("cls");
        cout << "\n\t*************************************\n";
        cout << "\t|        REGISTER NEW USER          |\n";
        cout << "\t*************************************\n";
        
        while (1) {
            cout << "\n\tEnter new username: ";
            if (scanf("%s", newUser_input) != 1) {
                cout << "\tInvalid input! Please try again." << endl;
                while(getchar() != '\n');
                continue;
            }
            while(getchar() != '\n');
            if (isUsernameExists(newUser_input)) {
                cout << "\tUsername already exists! Please try again." << endl;
            } else {
                break;
            }
        }

        while (1) {
            cout << "\tEnter email (lowercase only): ";
            if (scanf("%s", newEmail_input) != 1) {
                cout << "\tInvalid input! Please try again." << endl;
                while(getchar() != '\n');
                continue;
            }
            while(getchar() != '\n');
            if (!isLowercase(newEmail_input)) {
                cout << "\tInvalid email! Please enter the email in lowercase only." << endl << endl;
            } else if (!isValidEmail(newEmail_input)) {
                cout << "\tInvalid email format! Please enter the correct email format." << endl << endl;
            } else if (isEmailRegistered(newEmail_input)) {
                cout << "\tEmail already in use! Please try again with a different email." << endl << endl;
            } else {
                break;
            }
        }

        while (1) {
            cout << "\tEnter new password: ";
            if (scanf("%s", newPass_input) != 1) {
                cout << "\tInvalid input! Please try again." << endl;
                while(getchar() != '\n');
                continue;
            }
            while(getchar() != '\n');
            if (isValidPassword(newPass_input)) {
                cout << "\tConfirm password: ";
                if (scanf("%s", confirmPass_input) != 1) {
                    cout << "\tInvalid input! Please try again." << endl;
                    while(getchar() != '\n');
                    continue;
                }
                while(getchar() != '\n');
                if (strcmp(newPass_input, confirmPass_input) == 0) {
                    break;
                } else {
                    cout << "\tPasswords do not match! Please try again." << endl << endl;
                }
            } else {
                cout << "\tPassword must be at least 6 characters long, include numbers, punctuation, and be case-sensitive." << endl << endl;
            }
        }

        hashTableInsert(userHashTable, HASH_TABLE_SIZE, newUser_input, newPass_input, newEmail_input, FREE_TIER, 0, 0);
        saveHashTableToFile((const struct AccountNode* const*)userHashTable, HASH_TABLE_SIZE, FILE_NAME);

        cout << "\n\tUser registered successfully!" << endl;
        Sleep(900);
        system("cls");
    }

    void login() {
        int attempts = 0;
        
        cout << "\n\t*************************************\n";
        cout << "\t|            USER LOGIN             |\n";
        cout << "\t*************************************\n";

        while (1) {
            cout << "\n\tEnter username: ";
            if (scanf("%s", username) != 1) {
                cout << "\tInvalid input! Please try again." << endl;
                while(getchar() != '\n');
                continue;
            }
            while(getchar() != '\n');
            cout << "\tEnter password: ";
            if (scanf("%s", password) != 1) {
                cout << "\tInvalid input! Please try again." << endl;
                while(getchar() != '\n');
                continue;
            }
            while(getchar() != '\n');

            struct AccountNode* userAccount = hashTableSearch((const struct AccountNode* const*)userHashTable, HASH_TABLE_SIZE, username);
            if (userAccount != NULL && strcmp(userAccount->password, password) == 0) {
                cout << "\n\tLogin successful!" << endl;
                Sleep(900); system("cls"); return;
            } else {
                attempts++;
                if (attempts >= 3) {
                    cout << "\tToo many failed attempts!" << endl << endl;
                    cout << "\tDirect to reset password page....";
                    Sleep(50 * 3);
                    
                    while (1) {
                        system("cls");
                        cout << "\n\t-------------------------------------\n";
                        cout << "\t|          RESET PASSWORD           |\n";
                        cout << "\t-------------------------------------\n";

                        cout << "\n\t1. Re-enter your registered email\n"
                                "\t2. Return to main menu\n"
                                "\n\tEnter your choice: ";
                        int choice;
                        if (scanf("%d", &choice) != 1) {
                            cout << "\tInvalid choice! Please enter 1 or 2." << endl << endl;
                            while(getchar() != '\n');
                             Sleep(900); continue;
                        }
                        while(getchar() != '\n');

                        if (choice == 1) {
                            char enteredEmail[MAX_EMAIL_LEN];
                            while (1) {
                                cout << "\n\tEnter your registered email: ";
                                if (scanf("%s", enteredEmail) != 1) {
                                    cout << "\tInvalid input!" << endl;
                                    while(getchar() != '\n');
                                    continue;
                                }
                                while(getchar() != '\n');
                                if (!isLowercase(enteredEmail)) {
                                    cout << "\tInvalid email! Please enter the email in lowercase only." << endl;
                                    continue;
                                }
                                if (!isValidEmail(enteredEmail)) {
                                    cout << "\tInvalid email format!" << endl;
                                    continue;
                                }
                                break;
                            }
                            if (recoverAccount(enteredEmail)) { return; }
                            else {
                                cout << "\n\tPlease choose your selection." << endl;
                                Sleep(900); continue;
                            }
                        } else if (choice == 2) {
                            cout << "\n\tReturning to main menu";
                            for (int i = 0; i < 3; i++) { Sleep(500); cout << "."; }
                            system("cls"); return;
                        } else {
                            cout << "\tInvalid choice! Please enter 1 or 2." << endl << endl;
                            Sleep(900);
                        }
                    }
                } else {
                    cout << "\tInvalid username or password. Please try again." << endl;
                }
            }
        }
    }

    int validateLogin(const char* inputUsername, const char* inputPassword) const {
        struct AccountNode* userAccount = hashTableSearch((const struct AccountNode* const*)userHashTable, HASH_TABLE_SIZE, inputUsername);
        return (userAccount != NULL && strcmp(userAccount->password, inputPassword) == 0);
    }

    Subscription getUserSubscription(const char* username) const {
        const struct AccountNode* userAccount = hashTableSearch((const struct AccountNode* const*)userHashTable, HASH_TABLE_SIZE, username);
        if (userAccount) {
            return userAccount->subscription;
        }
        Subscription defaultSub;
        return defaultSub;
    }

private:
    int isUsernameExists(const char* username) const {
        return (hashTableSearch((const struct AccountNode* const*)userHashTable, HASH_TABLE_SIZE, username) != NULL);
    }

    int isEmailRegistered(const char* email) const {
        for (int i = 0; i < HASH_TABLE_SIZE; ++i) {
            const struct AccountNode* current = userHashTable[i];
            while (current != NULL) {
                if (strcmp(current->email, email) == 0) {
                    return 1;
                }
                current = current->next;
            }
        }
        return 0;
    }

    int recoverAccount(const char* email_input) {
        for (int i = 0; i < HASH_TABLE_SIZE; ++i) {
            struct AccountNode* current = (struct AccountNode*)userHashTable[i];
            while (current != NULL) {
                if (strcmp(current->email, email_input) == 0) {
                    cout << "\tAccount found!";
                    cout << " Username: " << current->username;
                    cout << " Password: " << current->password << endl;

                    char resetChoice_input[10];
                    while (1) {
                        cout << "\n\tDo you want to reset your password? (yes/no): ";
                        if (scanf("%s", resetChoice_input) != 1) {
                            cout << "\tInvalid input!" << endl;
                            while(getchar() != '\n');
                            continue;
                        }
                        while(getchar() != '\n');
                        for (int j = 0; resetChoice_input[j] != '\0'; ++j) {
                            resetChoice_input[j] = tolower(resetChoice_input[j]);
                        }

                        if (strcmp(resetChoice_input, "yes") == 0) {
                            system("cls");
                            cout << "\n\t-------------------------------------\n";
                            cout << "\t|          RESET PASSWORD           |\n";
                            cout << "\t-------------------------------------\n";
                            
                            char newPass[MAX_PASSWORD_LEN], confirmPass[MAX_PASSWORD_LEN];
                            while (1) {
                                cout << "\n\tEnter new password: ";
                                if (scanf("%s", newPass) != 1) {
                                    cout << "\tInvalid input!" << endl;
                                    while(getchar() != '\n');
                                    continue;
                                }
                                while(getchar() != '\n');
                                if (isValidPassword(newPass)) {
                                    cout << "\tConfirm password: ";
                                    if (scanf("%s", confirmPass) != 1) {
                                        cout << "\tInvalid input!" << endl;
                                        while(getchar() != '\n');
                                        continue;
                                    }
                                    while(getchar() != '\n');
                                    if (strcmp(newPass, confirmPass) == 0) {
                                        hashTableUpdatePassword(userHashTable, HASH_TABLE_SIZE, current->username, newPass);
                                        saveHashTableToFile((const struct AccountNode* const*)userHashTable, HASH_TABLE_SIZE, FILE_NAME);
                                        cout << "\n\tPassword reset successful!" << endl;
                                        Sleep(900); system("cls"); return 1;
                                    } else {
                                        cout << "\tPasswords do not match! Please try again." << endl;
                                    }
                                } else {
                                    cout << "\tPassword must be at least 6 characters long, include numbers, punctuation, and be case-sensitive." << endl;
                                }
                            }
                        } else if (strcmp(resetChoice_input, "no") == 0) {
                            Sleep(500); system("cls"); return 1;
                        } else {
                            cout << "\tInvalid choice! Please enter 'yes' or 'no'." << endl;
                        }
                    }
                }
                current = current->next;
            }
        }
        cout << "\tNo account found with the provided email." << endl;
        return 0;
    }
};

User::User() {
    playlistCount = 0;
    strcpy(currentUsername, "");
}

void User::setCurrentUsername(const char* username) {
    custom_strncpy(currentUsername, username, MAX_NAME_LEN);
}

long long User::getCurrentUnixTime() {
    return (long long)time(NULL);
}

void User::purchasePackage(UserLogin* userLogin, Admin* admin_ptr) {
    system("cls");
    cout << "\n\t|-------------------------------|\n";
    cout << "\t|        PURCHASE PACKAGE       |\n";
    cout << "\t|-------------------------------|\n";

    struct AccountNode* userNode = hashTableSearch((const struct AccountNode* const*)userLogin->userHashTable, HASH_TABLE_SIZE, currentUsername);
    if (!userNode) {
        cout << "\tError: User account not found. Please relogin." << endl;
        _getch(); system("cls"); return;
    }

    cout << "\n\tCurrent Package: ";
    if (userNode->subscription.type == FREE_TIER) { cout << "Free Tier" << endl; }
    else if (userNode->subscription.type == ONE_MONTH) { cout << "1-Month Subscription" << endl; }
    else if (userNode->subscription.type == THREE_MONTH) { cout << "3-Month Subscription" << endl; }
    else if (userNode->subscription.type == PERMANENT) { cout << "Permanent Subscription" << endl; }
    
    if (userNode->subscription.type != PERMANENT && userNode->subscription.expiryUnixTime > 0) {
        long long remaining_seconds = userNode->subscription.expiryUnixTime - getCurrentUnixTime();
        if (remaining_seconds <= 0) {
            cout << "\t(Expired!)" << endl;
        } else {
            long long days = remaining_seconds / (60 * 60 * 24);
            long long hours = (remaining_seconds % (60 * 60 * 24)) / (60 * 60);
            long long minutes = (remaining_seconds % (60 * 60)) / 60;
            cout << "\t(Expires in " << days << " days, " << hours << " hours, " << minutes << " minutes)" << endl;
        }
    }
    
    cout << "\n\tAvailable Packages:\n";
    cout << "\t1. 1-Month Package: $10.00\n";
    cout << "\t2. 3-Month Package: $25.00 (Save $5!)\n";
    cout << "\t3. Permanent Package: $100.00\n";
    cout << "\t0. Back to User Menu\n";

    int choice;
    double package_price = 0.0;
    PackageType selectedType = FREE_TIER;
    char packageName_str[MAX_NAME_LEN];
    long long newExpiry = 0;

    cout << "\n\tEnter your choice: ";
    if (scanf("%d", &choice) != 1) {
        cout << "\tInvalid input!" << endl;
        while(getchar() != '\n');
         _getch(); system("cls"); return;
    }
    while(getchar() != '\n');

    if (choice == 0) { system("cls"); return; }
    else if (choice == 1) { selectedType = ONE_MONTH; package_price = 10.00; strcpy(packageName_str, "1-Month Package"); }
    else if (choice == 2) { selectedType = THREE_MONTH; package_price = 25.00; strcpy(packageName_str, "3-Month Package"); }
    else if (choice == 3) { selectedType = PERMANENT; package_price = 100.00; strcpy(packageName_str, "Permanent Package"); }
    else { cout << "\tInvalid package choice!" << endl; _getch(); system("cls"); return; }

    if (userNode->subscription.type == PERMANENT) {
        if (selectedType == PERMANENT) {
            cout << "\tYou already have a permanent subscription. No need to purchase again." << endl;
        } else {
            cout << "\tYou have a permanent subscription. Cannot downgrade or extend timed packages." << endl;
        }
        _getch(); system("cls"); return;
    }
    
    newExpiry = getCurrentUnixTime();
    if (userNode->subscription.type != FREE_TIER && userNode->subscription.expiryUnixTime > newExpiry) {
        newExpiry = userNode->subscription.expiryUnixTime;
    }

    if (selectedType == ONE_MONTH) { newExpiry += (long long)30 * 24 * 60 * 60; }
    else if (selectedType == THREE_MONTH) { newExpiry += (long long)90 * 24 * 60 * 60; }
    else if (selectedType == PERMANENT) { newExpiry = -1; }

    if (processPayment(currentUsername, packageName_str, "Subscription", package_price, newExpiry)) {
        hashTableUpdateSubscription(userLogin->userHashTable, HASH_TABLE_SIZE, currentUsername, selectedType, newExpiry, 0);
        saveHashTableToFile((const struct AccountNode* const*)userLogin->userHashTable, HASH_TABLE_SIZE, userLogin->FILE_NAME);

        cout << "\n\tPackage purchased successfully!" << endl;
    } else {
        cout << "\n\tPayment failed or canceled." << endl;
    }

    Sleep(1500);
    system("cls");
}

void User::purchaseDigitalAlbum(UserLogin* userLogin, Admin* admin_ptr) {
    system("cls");
    cout << "\n\t|-------------------------------|\n";
    cout << "\t|     PURCHASE DIGITAL ALBUM    |\n";
    cout << "\t|-------------------------------|\n";

    if (admin_ptr->purchasableAlbumCount == 0) {
        cout << "\n\tNo digital albums available for purchase at the moment." << endl;
        cout << "\n\tPress ENTER to return to user menu..." << endl;
        _getch(); system("cls"); return;
    }

    cout << "\n\tAvailable Albums:\n";
    for (int i = 0; i < admin_ptr->purchasableAlbumCount; i++) {
        cout << "\t" << (i + 1) << ". Album: " << admin_ptr->purchasableAlbums[i].albumName
             << " by " << admin_ptr->purchasableAlbums[i].artist
             << " - Price: $" << fixed << setprecision(2) << admin_ptr->purchasableAlbums[i].price << endl;
    }
    cout << "\t0. Back to User Menu\n";

    int choice;
    cout << "\n\tEnter your choice: ";
    if (scanf("%d", &choice) != 1) {
        cout << "\tInvalid input!" << endl;
        while(getchar() != '\n');
        _getch(); system("cls"); return;
    }
    while(getchar() != '\n');

    if (choice == 0) { system("cls"); return; }
    if (choice < 1 || choice > admin_ptr->purchasableAlbumCount) {
        cout << "\tInvalid album choice!" << endl;
        _getch(); system("cls"); return;
    }

    Album selectedAlbum = admin_ptr->purchasableAlbums[choice - 1];

    char purchasedPlaylistName[MAX_NAME_LEN + 15];
    sprintf(purchasedPlaylistName, "Purchased - %s", selectedAlbum.albumName);

    int alreadyOwned = 0;
    for (int i = 0; i < playlistCount; ++i) {
        if (strcmp(playlists[i].name, purchasedPlaylistName) == 0) {
            alreadyOwned = 1;
            break;
        }
    }

    if (alreadyOwned) {
        cout << "\tYou already own this album. No need to purchase again." << endl;
        _getch(); system("cls"); return;
    }

    if (processPayment(currentUsername, selectedAlbum.albumName, "Album", selectedAlbum.price, -1)) {
        if (playlistCount >= MAX_PLAYLISTS) {
            cout << "\tCannot create playlist for purchased album: Maximum number of playlists reached!" << endl;
            _getch(); system("cls"); return;
        }

        Playlist newAlbumPlaylist;
        custom_strncpy(newAlbumPlaylist.name, purchasedPlaylistName, MAX_NAME_LEN);
        newAlbumPlaylist.songCount = 0;
        for (int i = 0; i < selectedAlbum.songCount; ++i) {
            if (newAlbumPlaylist.songCount < MAX_SONGS) {
                newAlbumPlaylist.songs[newAlbumPlaylist.songCount++] = selectedAlbum.songs[i];
            } else {
                cout << "\tWarning: Playlist for album \"" << selectedAlbum.albumName
                     << "\" reached maximum song limit. Not all songs were added." << endl;
                break;
            }
        }
        playlists[playlistCount++] = newAlbumPlaylist;
        
        admin_ptr->incrementAlbumSoldCount(selectedAlbum.albumName, selectedAlbum.artist);

        cout << "\n\tAlbum '" << selectedAlbum.albumName
             << "' purchased successfully! Songs added to your playlist: '" << newAlbumPlaylist.name << "'." << endl;
    } else {
        cout << "\n\tPayment failed or canceled." << endl;
    }

    Sleep(1500);
    system("cls");
}


int User::processPayment(const char* username_payer, const char* itemName, const char* itemType, double amount_to_pay, long long itemExpiry) {
    system("cls");
    cout << "\n\t|-------------------------------|\n";
    cout << "\t|         PAYMENT SYSTEM        |\n";
    cout << "\t|-------------------------------|\n";
    cout << "\n\tItem: " << itemName << " (" << itemType << ") - Amount: $" << fixed << setprecision(2) << amount_to_pay << endl;

    cout << "\n\tChoose Payment Method:\n";
    cout << "\t1. TNG eWallet\n";
    cout << "\t2. VISA Card\n";
    cout << "\t0. Cancel Payment\n";

    int paymentChoice;
    cout << "\n\tEnter your choice: ";
    if (scanf("%d", &paymentChoice) != 1) {
        cout << "\tInvalid input!" << endl;
        while(getchar() != '\n');
         return 0;
    }
    while(getchar() != '\n');

    if (paymentChoice == 0) {
        cout << "\tPayment canceled!" << endl;
        return 0;
    }

    PaymentRecord record;
    long long current_time = getCurrentUnixTime();
    sprintf(record.transactionId, "%lld%s", current_time, username_payer);
    custom_strncpy(record.username, username_payer, MAX_NAME_LEN);
    custom_strncpy(record.itemName, itemName, MAX_NAME_LEN);
    custom_strncpy(record.itemType, itemType, 20);
    record.amount = amount_to_pay;
    record.timestamp = current_time;
    record.purchasedItemExpiry = itemExpiry;
    custom_strncpy(record.status, "SUCCESS", 20);

    if (paymentChoice == 1) {
        custom_strncpy(record.paymentMethod, "TNG", MAX_NAME_LEN);
        char phoneNum[MAX_NAME_LEN];
        cout << "\n\tEnter TNG phone number: ";
        if (scanf("%s", phoneNum) != 1) {
            cout << "\tInvalid input!" << endl;
            while(getchar() != '\n');
            custom_strncpy(record.status, "FAILED", 20);
            savePaymentRecord(&record);
            return 0;
        }
        while(getchar() != '\n');
        custom_strncpy(record.paymentDetails, phoneNum, sizeof(record.paymentDetails));
        cout << "\n\tProcessing TNG payment..." << endl;
    } else if (paymentChoice == 2) {
        custom_strncpy(record.paymentMethod, "VISA", MAX_NAME_LEN);
        char cardNumber[MAX_NAME_LEN];
        char cvv[5];
        cout << "\n\tEnter VISA card number (16 digits): ";
        if (scanf("%s", cardNumber) != 1) {
            cout << "\tInvalid input!" << endl;
            while(getchar() != '\n');
            custom_strncpy(record.status, "FAILED", 20);
            savePaymentRecord(&record);
            return 0;
        }
        while(getchar() != '\n');
        cout << "\tEnter 3-digit CVV on back of card: ";
        if (scanf("%s", cvv) != 1) {
            cout << "\tInvalid input!" << endl;
            while(getchar() != '\n');
            custom_strncpy(record.status, "FAILED", 20);
            savePaymentRecord(&record);
            return 0;
        }
        while(getchar() != '\n');
        
        int len = strlen(cardNumber);
        char maskedCardNum[MAX_NAME_LEN];
        if (len > 4) {
            for(int i = 0; i < len - 4; ++i) maskedCardNum[i] = 'X';
            maskedCardNum[len-4] = '\0';
            strcat(maskedCardNum, cardNumber + len - 4);
        } else {
            custom_strncpy(maskedCardNum, cardNumber, MAX_NAME_LEN);
        }
        custom_strncpy(record.paymentDetails, maskedCardNum, sizeof(record.paymentDetails));
        cout << "\n\tProcessing VISA payment..." << endl;
    } else {
        cout << "\tInvalid payment method choice!" << endl;
        custom_strncpy(record.status, "FAILED", 20);
        savePaymentRecord(&record);
        return 0;
    }

    Sleep(1500);
    cout << "\tPayment successful!" << endl;
    savePaymentRecord(&record);
    return 1;
}

void User::savePaymentRecord(const PaymentRecord* record) {
    FILE* file = fopen("payment_records.txt", "a");
    if (!file) {
        cout << "\tError opening payment_records.txt for saving!" << endl;
        return;
    }
    fprintf(file, "%s,%s,%s,%s,%.2f,%s,%s,%lld,%s,%lld\n",
            record->transactionId, record->username, record->itemName, record->itemType, record->amount,
            record->paymentMethod, record->paymentDetails, record->timestamp, record->status, record->purchasedItemExpiry);
    fclose(file);
}

void User::createPlaylist(Admin* admin_ptr) {
    char playlistName[MAX_NAME_LEN];
    
    system("cls");
    cout << "\n\t|-------------------------------|\n";
    cout << "\t|        CREATE PLAYLIST        |\n";
    cout << "\t|-------------------------------|\n";
    
    while (1) {
        cout << "\n\tEnter playlist name: ";
        if (fgets(playlistName, MAX_NAME_LEN, stdin) == NULL) {
            cout << "\tInvalid input!" << endl;
            continue;
        }
        playlistName[strcspn(playlistName, "\n")] = 0;

        int isDuplicate = 0;
        for (int i = 0; i < playlistCount; i++) {
            if (strcmp(playlists[i].name, playlistName) == 0) {
                isDuplicate = 1;
                break;
            }
        }

        if (isDuplicate) {
            cout << "\tError: A playlist with the name '" << playlistName << "' already exists! Please choose a different name." << endl;
        } else {
            break;
        }
    }

    Playlist newPlaylist;
    strcpy(newPlaylist.name, playlistName);
    newPlaylist.songCount = 0;

    char addSongs_input[10];
    while (1) {
        cout << "\tDo you want to add songs to the playlist? (yes/no): ";
        if (scanf("%s", addSongs_input) != 1) {
             cout << "\tInvalid input!" << endl;
             while(getchar() != '\n');
             continue;
        }
        while(getchar() != '\n');
        for (int i = 0; addSongs_input[i] != '\0'; i++) {
            addSongs_input[i] = tolower(addSongs_input[i]);
        }

        if (strcmp(addSongs_input, "yes") == 0 || strcmp(addSongs_input, "no") == 0) {
            break;
        } else {
            cout << "\tInvalid input! Please answer with 'yes' or 'no'." << endl << endl;
        }
    }

    if (strcmp(addSongs_input, "no") == 0) {
        if (playlistCount < MAX_PLAYLISTS) {
            playlists[playlistCount++] = newPlaylist;
            cout << "\n\tPlaylist created without songs!" << endl;
        } else {
            cout << "\n\tCannot create playlist: Maximum number of playlists reached!" << endl;
        }
        Sleep(700); system("cls"); return;
    }

    char addMoreSongs_input[10];
    strcpy(addMoreSongs_input, "yes");
    while (strcmp(addMoreSongs_input, "yes") == 0) {
        Song newSong;
        newSong.favoriteCount = 0;

        cout << "\n\t++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
        cout << "\tEnter song name: ";
        if (fgets(newSong.songName, MAX_NAME_LEN, stdin) == NULL) { cout << "\tInvalid input!" << endl; continue; }
        newSong.songName[strcspn(newSong.songName, "\n")] = 0;

        cout << "\tEnter artist: ";
        if (fgets(newSong.artist, MAX_NAME_LEN, stdin) == NULL) { cout << "\tInvalid input!" << endl; continue; }
        newSong.artist[strcspn(newSong.artist, "\n")] = 0;

        int genreChoice;
        int validChoice;
        do {
            cout << "\tSelect genre (";
            for (int i = 0; i < admin_ptr->genreCount; i++) {
                cout << (i + 1) << ". " << admin_ptr->genres[i] << " ";
            }
            cout << "): ";
            if (scanf("%d", &genreChoice) != 1) {
                cout << "\tInvalid choice. Please select a valid genre number!" << endl << endl;
                while(getchar() != '\n');
                validChoice = 0;
            } else if (genreChoice < 1 || genreChoice > admin_ptr->genreCount) {
                cout << "\tInvalid choice. Please select a valid genre number!" << endl << endl;
                while(getchar() != '\n');
                validChoice = 0;
            } else {
                validChoice = 1;
            }
        } while (!validChoice);
        while(getchar() != '\n');
        strcpy(newSong.genre, admin_ptr->genres[genreChoice - 1]);

        int languageChoice;
        do {
            cout << "\tSelect language (";
            for (int i = 0; i < admin_ptr->languageCount; i++) {
                cout << (i + 1) << ". " << admin_ptr->languages[i] << " ";
            }
            cout << "): ";
            if (scanf("%d", &languageChoice) != 1) {
                cout << "\tInvalid choice. Please select a valid language number!" << endl << endl;
                while(getchar() != '\n');
                validChoice = 0;
            } else if (languageChoice < 1 || languageChoice > admin_ptr->languageCount) {
                cout << "\tInvalid choice. Please select a valid language number!" << endl << endl;
                while(getchar() != '\n');
                validChoice = 0;
            } else {
                validChoice = 1;
            }
        } while (!validChoice);
        while(getchar() != '\n');
        strcpy(newSong.language, admin_ptr->languages[languageChoice - 1]);
        
        if (newPlaylist.songCount >= MAX_SONGS) {
            cout << "\tError: Maximum songs reached for this playlist! Cannot add more." << endl;
            strcpy(addMoreSongs_input, "no");
            break;
        }
        newPlaylist.songs[newPlaylist.songCount++] = newSong;

        cout << "\n\tDo you want to add more songs? (yes/no): ";
        if (scanf("%s", addMoreSongs_input) != 1) {
            cout << "\tInvalid input!" << endl;
            while(getchar() != '\n');
            strcpy(addMoreSongs_input, "yes");
        }
        while(getchar() != '\n');
        for (int i = 0; addMoreSongs_input[i] != '\0'; i++) {
            addMoreSongs_input[i] = tolower(addMoreSongs_input[i]);
        }

        if (strcmp(addMoreSongs_input, "no") == 0) {
            cout << "\n\t--------------------------------------------------------------------\n";
            cout << "\n\tHere are some recommended songs based on your playlist's songs: \n";
            int displayedIndexes[MAX_RECOMMENDED_SONGS];
            int displayedCount = 0;
            int recommendationCounter = 1;

            for (int i = 0; i < admin_ptr->recommendedSongCount; i++) {
                if (strcmp(admin_ptr->recommendedSongs[i].genre, newSong.genre) == 0 ||
                    strcmp(admin_ptr->recommendedSongs[i].language, newSong.language) == 0) {
                    cout << "\t" << recommendationCounter << ". " << admin_ptr->recommendedSongs[i].songName
                         << " by " << admin_ptr->recommendedSongs[i].artist
                         << " [" << admin_ptr->recommendedSongs[i].genre
                         << ", " << admin_ptr->recommendedSongs[i].language
                         << "] - Favorites: " << admin_ptr->recommendedSongs[i].favoriteCount << endl;
                    displayedIndexes[displayedCount++] = i;
                    recommendationCounter++;
                }
            }

            cout << "\t--------------------------------------------------------------------\n";

            if (displayedCount == 0) {
                cout << "\tNo recommended songs match your preferences." << endl;
                Sleep(700);
            } else {
                char addRecommendedSongs_input[10];
                while (1) {
                    cout << "\n\tDo you want to add any of the recommended songs to your playlist? (yes/no): ";
                    if (scanf("%s", addRecommendedSongs_input) != 1) {
                        cout << "\tInvalid input!" << endl;
                        while(getchar() != '\n');
                        continue;
                    }
                    while(getchar() != '\n');
                    for (int i = 0; addRecommendedSongs_input[i] != '\0'; i++) {
                        addRecommendedSongs_input[i] = tolower(addRecommendedSongs_input[i]);
                    }
                    if (strcmp(addRecommendedSongs_input, "yes") == 0 || strcmp(addRecommendedSongs_input, "no") == 0) {
                        break;
                    } else {
                        cout << "\tInvalid input! Please answer with 'yes' or 'no'." << endl;
                    }
                }

                if (strcmp(addRecommendedSongs_input, "yes") == 0) {
                    int songChoice;
                    while (1) {
                        cout << "\tEnter the number of the song you want to add (or 0 to stop): ";
                        if (scanf("%d", &songChoice) != 1) {
                            cout << "\tInvalid input! Please enter a valid number." << endl << endl;
                            while(getchar() != '\n');
                            continue;
                        }
                        while(getchar() != '\n');

                        if (songChoice == 0) { break; }

                        if (songChoice >= 1 && songChoice <= displayedCount) {
                            Song selectedSong = admin_ptr->recommendedSongs[displayedIndexes[songChoice - 1]];

                            int isDuplicate = 0;
                            for (int i = 0; i < newPlaylist.songCount; i++) {
                                if (strcmp(newPlaylist.songs[i].songName, selectedSong.songName) == 0 &&
                                    strcmp(newPlaylist.songs[i].artist, selectedSong.artist) == 0) {
                                    isDuplicate = 1;
                                    break;
                                }
                            }

                            if (isDuplicate) {
                                cout << "\tThis song is already in the playlist! Please select another song." << endl << endl;
                            } else {
                                if (newPlaylist.songCount >= MAX_SONGS) {
                                    cout << "\tError: Maximum songs reached for this playlist! Cannot add more." << endl;
                                    break;
                                }
                                newPlaylist.songs[newPlaylist.songCount++] = selectedSong;
                                admin_ptr->incrementFavoriteCount(selectedSong.songName, selectedSong.artist);
                                cout << "\tSong added successfully!" << endl << endl;
                            }
                        } else {
                            cout << "\tInvalid choice! Please select a valid song number or 0 to stop." << endl << endl;
                        }
                    }
                }
            }
        }
    }

    if (playlistCount < MAX_PLAYLISTS) {
        playlists[playlistCount++] = newPlaylist;
        cout << "\n\tPlaylist created successfully!" << endl;
    } else {
        cout << "\n\tCannot create playlist: Maximum number of playlists reached!" << endl;
    }
    Sleep(700);
    system("cls");
}

void User::deletePlaylist() {
    system("cls");
    cout << "\n\t|-------------------------------|\n";
    cout << "\t|        DELETE PLAYLIST        |\n";
    cout << "\t|-------------------------------|\n";

    if (playlistCount == 0) {
        cout << "\n\tNo playlists to delete!" << endl;
        cout << "\n\tPress ENTER to return to user menu..." << endl;
        _getch();
        system("cls");
        return;
    }

    cout << "\n\tSelect a playlist to delete:\n";
    for (int i = 0; i < playlistCount; i++) {
        cout << "\t" << (i + 1) << ". " << playlists[i].name << endl;
    }
    cout << "\n\t0. Go back to the user menu\n";

    int playlistChoice;
    while (1) {
        cout << "\n\tEnter your choice: ";
        if (scanf("%d", &playlistChoice) != 1) {
            cout << "\tInvalid choice! Please select a valid option." << endl;
            while(getchar() != '\n');
        } else if (playlistChoice < 0 || playlistChoice > playlistCount) {
            cout << "\tInvalid choice! Please select a valid option." << endl;
            while(getchar() != '\n');
        } else {
            break;
        }
    }
    while(getchar() != '\n');

    if (playlistChoice == 0) {
        cout << "\n\tReturning to the user menu..." << endl;
        Sleep(700);
        system("cls");
        return;
    }

    for (int i = playlistChoice - 1; i < playlistCount - 1; i++) {
        playlists[i] = playlists[i + 1];
    }

    playlistCount--;
    cout << "\n\tPlaylist deleted!" << endl;
    Sleep(700);
    system("cls");
} 

void User::searchPlaylist() {
    char searchName[MAX_NAME_LEN];
    char choice_input[10];

    do {
        system("cls");
        cout << "\n\t|-------------------------------|\n";
        cout << "\t|        SEARCH PLAYLIST        |\n";
        cout << "\t|-------------------------------|\n";

        cout << "\n\tEnter playlist name to search: ";
        if (fgets(searchName, MAX_NAME_LEN, stdin) == NULL) {
            cout << "\tInvalid input!" << endl;
            continue;
        }
        searchName[strcspn(searchName, "\n")] = 0;

        int found = 0;
        for (int i = 0; i < playlistCount; i++) {
            if (custom_strcmpi(playlists[i].name, searchName) == 0) {
                cout << "\n\tPlaylist found: " << playlists[i].name << endl;
                found = 1;
                for (int j = 0; j < playlists[i].songCount; j++) {
                    cout << "\tSong: " << playlists[i].songs[j].songName
                         << ", Artist: " << playlists[i].songs[j].artist
                         << ", Genre: " << playlists[i].songs[j].genre
                         << ", Language: " << playlists[i].songs[j].language << endl;
                }
                break;
            }
        }

        if (!found) {
            cout << "\n\tPlaylist not found!" << endl;
        }

        cout << "\n\tDo you want to search again? (yes/no): ";
        if (scanf("%s", choice_input) != 1) {
            cout << "\tInvalid input!" << endl;
            while(getchar() != '\n');
            strcpy(choice_input, "yes");
        }
        while(getchar() != '\n');
        for (int i = 0; choice_input[i] != '\0'; i++) {
            choice_input[i] = tolower(choice_input[i]);
        }

    } while (strcmp(choice_input, "yes") == 0);

    cout << "\n\tReturning to the user menu..." << endl;
    Sleep(700);
    system("cls");
}

void User::updatePlaylist() {
    char playlistName[MAX_NAME_LEN];
    
    system("cls");
    cout << "\n\t|-------------------------------|\n";
    cout << "\t|        UPDATE PLAYLIST        |\n";
    cout << "\t|-------------------------------|\n";
    
    cout << "\n\tEnter playlist name to update: ";

    if (fgets(playlistName, MAX_NAME_LEN, stdin) == NULL) {
         cout << "\tInvalid input!" << endl; Sleep(700); system("cls"); return;
    }
    playlistName[strcspn(playlistName, "\n")] = 0;

    int found = 0;
    for (int i = 0; i < playlistCount; i++) {
        if (strcmp(playlists[i].name, playlistName) == 0) {
            found = 1;
            cout << "\tEnter new name for the playlist: ";
            if (fgets(playlists[i].name, MAX_NAME_LEN, stdin) == NULL) {
                 cout << "\tInvalid input!" << endl; Sleep(700); system("cls"); return;
            }
            playlists[i].name[strcspn(playlists[i].name, "\n")] = 0;
            cout << "\n\tPlaylist updated successfully!" << endl;
            Sleep(700);
            system("cls");
            break;
        }
    }

    if (!found) {
        cout << "\n\tPlaylist not found!" << endl;
        Sleep(700);
        system("cls");
    }
}

void User::addSong(Admin* admin_ptr) {
    system("cls");
    cout << "\n\t|-------------------------------|\n";
    cout << "\t|           ADD SONG            |\n";
    cout << "\t|-------------------------------|\n";
    
    if (playlistCount == 0) {
        cout << "\n\tNo playlists available to add songs! Please create one first." << endl;
        cout << "\n\tPress ENTER to return to user menu..." << endl;
        _getch();
        system("cls");
        return;
    }

    cout << "\n\tAvailable playlists:\n";
    for (int i = 0; i < playlistCount; i++) {
        cout << "\t" << (i + 1) << ". " << playlists[i].name << endl;
    }
    cout << "\n\t0. Go back to the user menu\n";

    int playlistChoice;
    while (1) {
        cout << "\n\tSelect the playlist number to add songs (1-" << playlistCount << "): ";
        if (scanf("%d", &playlistChoice) != 1) {
            cout << "\tInvalid choice! Please select a valid playlist number." << endl;
            while(getchar() != '\n');
        } else if (playlistChoice < 0 || playlistChoice > playlistCount) {
            cout << "\tInvalid choice! Please select a valid playlist number." << endl;
            while(getchar() != '\n');
        } else {
            break;
        }
    }
    while(getchar() != '\n');
    if (playlistChoice == 0) {
        cout << "\n\tReturning to the user menu..." << endl;
        Sleep(900);
        system("cls");
        return;
    }

    Playlist* selectedPlaylist = &playlists[playlistChoice - 1];

    int numSongs;
    system("cls");
    cout << "\n\t|-------------------------------|\n";
    cout << "\t|          ADDING SONG...       |\n";
    cout << "\t|-------------------------------|\n";
    
    while (1) {
        cout << "\n\tEnter the number of songs you want to add: ";
        if (scanf("%d", &numSongs) != 1) {
            cout << "\tInvalid input! Please enter a positive integer." << endl;
            while(getchar() != '\n');
        } else if (numSongs < 1) {
            cout << "\tInvalid input! Please enter a positive integer." << endl;
            while(getchar() != '\n');
        } else {
            break;
        }
    }
    while(getchar() != '\n');

    for (int s = 0; s < numSongs; s++) {
        Song newSong;
        newSong.favoriteCount = 0;

        cout << "\n\t******************************************************************\n"; 
        cout << "\tEnter song name: ";
        if (fgets(newSong.songName, MAX_NAME_LEN, stdin) == NULL) { cout << "\tInvalid input!" << endl; continue; }
        newSong.songName[strcspn(newSong.songName, "\n")] = 0;

        cout << "\tEnter artist: ";
        if (fgets(newSong.artist, MAX_NAME_LEN, stdin) == NULL) { cout << "\tInvalid input!" << endl; continue; }
        newSong.artist[strcspn(newSong.artist, "\n")] = 0;

        int genreChoice;
        while (1) {
            cout << "\tSelect genre ("; 
            for (int j = 0; j < admin_ptr->genreCount; j++) {
                cout << (j + 1) << ". " << admin_ptr->genres[j] << " ";
            }
            cout << "): ";
            if (scanf("%d", &genreChoice) != 1) {
                cout << "\tInvalid choice! Please select a valid genre number." << endl << endl;
                while(getchar() != '\n');
            } else if (genreChoice < 1 || genreChoice > admin_ptr->genreCount) {
                cout << "\tInvalid choice! Please select a valid genre number." << endl << endl; 
                while(getchar() != '\n');
            } else {
                break;
            }
        }
        while(getchar() != '\n');
  
        strcpy(newSong.genre, admin_ptr->genres[genreChoice - 1]);

        int languageChoice;
        while (1) {
            cout << "\tSelect language ("; 
            for (int j = 0; j < admin_ptr->languageCount; j++) {
                cout << (j + 1) << ". " << admin_ptr->languages[j] << " ";
            }
            cout << "): ";
            if (scanf("%d", &languageChoice) != 1) {
                cout << "\tInvalid choice! Please select a valid language number." << endl << endl; 
                while(getchar() != '\n');
            } else if (languageChoice < 1 || languageChoice > admin_ptr->languageCount) {
                cout << "\tInvalid choice! Please select a valid language number." << endl << endl; 
                while(getchar() != '\n');
            } else {
                break;
            }
        }
        while(getchar() != '\n');
      
        strcpy(newSong.language, admin_ptr->languages[languageChoice - 1]);

        if (selectedPlaylist->songCount >= MAX_SONGS) {
            cout << "\tError: Maximum songs reached for this playlist! Cannot add more." << endl; 
            break;
        }
        selectedPlaylist->songs[selectedPlaylist->songCount++] = newSong;

        cout << "\n\tSong added to playlist!" << endl;
        system("cls");
        // Assuming laodingPage is a typo for loadingPage; left as comment since it's undefined
        // loadingPage(40);
    }
    Sleep(700);
    system("cls");
}

void User::deleteSong() {
    system("cls");
    cout << "\n\t|-------------------------------|\n";
    cout << "\t|           DELETE SONG         |\n";
    cout << "\t|-------------------------------|\n";
    
    if (playlistCount == 0) {
        cout << "\n\tNo playlists available to delete songs!" << endl;
        cout << "\n\tPress ENTER to return to user menu..." << endl;
        _getch();
        system("cls");
        return;
    }

    cout << "\n\tAvailable playlists:\n";
    for (int i = 0; i < playlistCount; i++) {
        cout << "\t" << (i + 1) << ". " << playlists[i].name << endl;
    }
    cout << "\n\t0. Go back to the user menu\n";

    int playlistChoice;
    while (1) {
        cout << "\n\tEnter the playlist number to delete a song (1-" << playlistCount << "): ";
        if (scanf("%d", &playlistChoice) != 1) {
            cout << "\tInvalid input! Please select a valid playlist number." << endl;
            while(getchar() != '\n');
        } else if (playlistChoice < 0 || playlistChoice > playlistCount) {
            cout << "\tInvalid input! Please select a valid playlist number." << endl;
            while(getchar() != '\n');
        } else {
            break;
        }
    }
    while(getchar() != '\n');
    if (playlistChoice == 0) {
        cout << "\n\tReturning to the user menu..." << endl;
        Sleep(700);
        system("cls");
        return;
    }

    int playlistIndex = playlistChoice - 1;

    if (playlists[playlistIndex].songCount == 0) {
        cout << "\tThe playlist \"" << playlists[playlistIndex].name << "\" has no songs to delete!" << endl;
        cout << "\n\tPress ENTER to continue..." << endl;
        _getch();
        system("cls");
        return;
    }

    cout << "\n\tSongs in playlist \"" << playlists[playlistIndex].name << "\":\n";
    for (int j = 0; j < playlists[playlistIndex].songCount; j++) {
        cout << "\t" << (j + 1) << ". " << playlists[playlistIndex].songs[j].songName << endl;
    }

    int songChoice;
    while (1) {
        cout << "\n\tEnter the song number to delete (1-" << playlists[playlistIndex].songCount << "): ";
        if (scanf("%d", &songChoice) != 1) {
            cout << "\tInvalid input! Please select a valid song number." << endl;
            while(getchar() != '\n');
        } else if (songChoice < 1 || songChoice > playlists[playlistIndex].songCount) {
            cout << "\tInvalid input! Please select a valid song number." << endl;
            while(getchar() != '\n');
        } else {
            break;
        }
    }
    while(getchar() != '\n');

    for (int j = songChoice - 1; j < playlists[playlistIndex].songCount - 1; j++) {
        playlists[playlistIndex].songs[j] = playlists[playlistIndex].songs[j + 1];
    }
    playlists[playlistIndex].songCount--;

    cout << "\n\tSong deleted successfully!" << endl;
    Sleep(700);
    system("cls");
}

void User::importPlaylist() {
    while (1) {
        system("cls");
        cout << "\n\t|-------------------------------|\n";
        cout << "\t|        IMPORT PLAYLIST        |\n";
        cout << "\t|-------------------------------|\n";

        char filename[MAX_NAME_LEN];
        cout << "\n\tEnter the filename to import playlist (e.g., myplaylist.txt): ";
        if (fgets(filename, MAX_NAME_LEN, stdin) == NULL) {
            cout << "\tInvalid input!" << endl;
            continue;
        }
        filename[strcspn(filename, "\n")] = 0;
        
        size_t len = strlen(filename);
        const char* txt_ext = ".txt";
        size_t txt_ext_len = strlen(txt_ext);

        FILE* file = fopen(filename, "r");
        if (!file) {
            cout << "\n\tError: Could not open file!" << endl;
            cout << "\n\t1. Try again\n";
            cout << "\t0. Back to user menu\n";
            int choice;
            if (scanf("%d", &choice) != 1) {
                cout << "\tInvalid input!" << endl;
                while(getchar() != '\n');
                continue;
            }
            while(getchar() != '\n');
            if (choice == 0) { system("cls"); return; }
            else { continue; }
        }

        Playlist newPlaylist;
        
        if (fgets(newPlaylist.name, MAX_NAME_LEN, file) == NULL) {
            cout << "\n\tError: Failed to read playlist name or file is empty!" << endl;
            fclose(file);
            cout << "\n\t1. Try again\n";
            cout << "\t0. Back to user menu\n";
            int choice;
            if (scanf("%d", &choice) != 1) {
                cout << "\tInvalid input!" << endl;
                while(getchar() != '\n');
                continue;
            }
            while(getchar() != '\n');
            if (choice == 0) { system("cls"); return; }
            else { continue; }
        }
        newPlaylist.name[strcspn(newPlaylist.name, "\n")] = 0;

        int playlistExists = 0;
        for (int i = 0; i < playlistCount; i++) {
            if (strcmp(playlists[i].name, newPlaylist.name) == 0) {
                cout << "\n\tError: A playlist with the name '" << newPlaylist.name << "' already exists!" << endl;
                playlistExists = 1;
                break;
            }
        }

        if (playlistExists) {
            fclose(file);
            cout << "\n\t1. Try again\n";
            cout << "\t0. Back to user menu\n";
            int choice;
            if (scanf("%d", &choice) != 1) {
                cout << "\tInvalid input!" << endl;
                while(getchar() != '\n');
                continue;
            }
            while(getchar() != '\n');
        
            if (choice == 0) { system("cls"); return; }
            else { continue; }
        }

        char songLine[4 * MAX_NAME_LEN];
        newPlaylist.songCount = 0;
        while (fgets(songLine, sizeof(songLine), file) != NULL && newPlaylist.songCount < MAX_SONGS) {
            Song currentSong;
            currentSong.favoriteCount = 0;

            songLine[strcspn(songLine, "\n")] = 0;

            char* token;
            token = strtok(songLine, ",");
            if (token) custom_strncpy(currentSong.songName, token, MAX_NAME_LEN); else continue;
            token = strtok(NULL, ",");
            if (token) custom_strncpy(currentSong.artist, token, MAX_NAME_LEN); else continue;
            token = strtok(NULL, ",");
            if (token) custom_strncpy(currentSong.genre, token, MAX_NAME_LEN); else continue;
            token = strtok(NULL, ",");
            if (token) custom_strncpy(currentSong.language, token, MAX_NAME_LEN); else continue;
            
            newPlaylist.songs[newPlaylist.songCount++] = currentSong;
        }
        fclose(file);

        if (playlistCount < MAX_PLAYLISTS) {
            playlists[playlistCount++] = newPlaylist;
            cout << "\n\tPlaylist imported successfully from " << filename << "!" << endl;

            cout << "\n\t1. Import another playlist\n";
            cout << "\t0. Back to user menu\n";
            int choice;
            if (scanf("%d", &choice) != 1) {
                cout << "\tInvalid input!" << endl;
                while(getchar() != '\n');
                continue;
            }
            while(getchar() != '\n');
            
            if (choice == 0) { system("cls"); return; }
            else { continue; }
        } else {
            cout << "\n\tError: Maximum number of playlists reached! Cannot import." << endl;
            cout << "\n\tPress ENTER to return to user menu..." << endl;
            _getch(); system("cls"); return;
        }
    }
}

void User::downloadPlaylist() {
    system("cls");
    cout << "\n\t|-------------------------------|\n";
    cout << "\t|        DOWNLOAD PLAYLIST      |\n";
    cout << "\t|-------------------------------|\n";
    
    if (playlistCount == 0) {
        cout << "\n\tNo playlists available to download!" << endl;
        cout << "\n\tPress ENTER to return to user menu..." << endl;
        _getch();
        system("cls");
        return;
    }

    while (1) {
        system("cls");
        cout << "\n\t|-------------------------------|\n";
        cout << "\t|        DOWNLOAD PLAYLIST      |\n";
        cout << "\t|-------------------------------|\n";

        cout << "\n\tAvailable playlists:\n";
        for (int i = 0; i < playlistCount; i++) {
            cout << "\t" << (i + 1) << ". " << playlists[i].name << endl;
        }
        cout << "\n\t0. Back to user menu\n";
        
        int playlistChoice;
        cout << "\n\tEnter the playlist number to download (0 to go back): ";
        if (scanf("%d", &playlistChoice) != 1) {
            cout << "\tInvalid input!" << endl;
            while(getchar() != '\n');
             Sleep(700); continue;
        }
        while(getchar() != '\n');
        

        if (playlistChoice == 0) { system("cls"); return; }

        if (playlistChoice < 1 || playlistChoice > playlistCount) {
            cout << "\tInvalid input! Please select a valid playlist number." << endl;
            Sleep(700); continue;
        }

        Playlist* selectedPlaylist = &playlists[playlistChoice - 1];

        char fullFilename[MAX_NAME_LEN + 5];
        custom_strncpy(fullFilename, selectedPlaylist->name, MAX_NAME_LEN);
        for (int i = 0; fullFilename[i] != '\0'; i++) {
            if (fullFilename[i] == ' ' || fullFilename[i] == '/' || fullFilename[i] == '\\' || 
                fullFilename[i] == ':' || fullFilename[i] == '*' || fullFilename[i] == '?' || 
                fullFilename[i] == '"' || fullFilename[i] == '<' || fullFilename[i] == '>' || 
                fullFilename[i] == '|') {
                fullFilename[i] = '_';
            }
        }
        strcat(fullFilename, ".txt");

        FILE* checkFile = fopen(fullFilename, "r");
        if (checkFile) {
            fclose(checkFile);
            cout << "\n\tError: A file with the same playlist name already exists!" << endl;
            cout << "\n\tPlease select another playlist." << endl;
            _getch(); continue;
        }

        FILE* file = fopen(fullFilename, "w");
        if (!file) {
            cout << "\n\tError: Could not create or write to the file!" << endl;
            continue;
        }

        fprintf(file, "%s\n", selectedPlaylist->name);

        for (int i = 0; i < selectedPlaylist->songCount; i++) {
            Song* song = &selectedPlaylist->songs[i];
            fprintf(file, "%s,%s,%s,%s\n", song->songName, song->artist, song->genre, song->language);
        }

        fclose(file);

        cout << "\n\tPlaylist downloaded successfully to " << fullFilename << "!" << endl;

        char choice_input[10];
        while (1) {
            cout << "\n\tDo you want to download another playlist? (yes/no): ";
            if (scanf("%s", choice_input) != 1) {
                cout << "\tInvalid input!" << endl;
                while(getchar() != '\n');
                continue;
            }
            while(getchar() != '\n');
            
            for (int i = 0; choice_input[i] != '\0'; i++) {
                choice_input[i] = tolower(choice_input[i]);
            }

            if (strcmp(choice_input, "yes") == 0) { break; }
            else if (strcmp(choice_input, "no") == 0) { system("cls"); return; }
            else { cout << "\tInvalid input! Please enter 'yes' or 'no'." << endl; }
        }
    }
}

void User::juztodisplaydesign() {
    cout << "\n\t|-------------------------------|\n";
    cout << "\t|       PLAYING PLAYLIST...     |\n";
    cout << "\t|-------------------------------|\n";
}

void User::playPlaylist(UserLogin* userLogin) {
    system("cls");
    juztodisplaydesign();
    
    struct AccountNode* userNode = hashTableSearch((const struct AccountNode* const*)userLogin->userHashTable, HASH_TABLE_SIZE, currentUsername);
    if (!userNode) {
        cout << "\tError: User account not found. Please relogin." << endl;
        _getch(); system("cls"); return;
    }

    if (playlistCount == 0) {
        cout << "\n\tNo playlists available to play!" << endl;
        cout << "\n\tPress ENTER to return to user menu..." << endl;
        _getch();
        system("cls");
        return;
    }

    cout << "\n\tAvailable playlists:\n";
    for (int i = 0; i < playlistCount; i++) {
        cout << "\t" << (i + 1) << ". " << playlists[i].name << endl;
    }

    int playlistChoice;
    while (1) {
        cout << "\n\tEnter the playlist number to play (1-" << playlistCount << ", 0 to go back): ";
        if (scanf("%d", &playlistChoice) != 1) {
            cout << "\tInvalid input! Please select a valid playlist number." << endl; 
            while(getchar() != '\n');
        } else if (playlistChoice < 0 || playlistChoice > playlistCount) {
            cout << "\tInvalid input! Please select a valid playlist number." << endl; 
            while(getchar() != '\n');
        } else if (playlistChoice == 0) {
            system("cls"); return;
        } else {
            system("cls");
            juztodisplaydesign();
            break;
        }
    }
    while(getchar() != '\n');

    Playlist* selectedPlaylist = &playlists[playlistChoice - 1];

    if (selectedPlaylist->songCount == 0) {
        cout << "\n\tThe selected playlist is empty!" << endl;
        Sleep(900); system("cls"); return;
    }

    int currentSongIndex = 0;
    int isPlaying = 1;
    int isPaused = 0;
    
    while (1) {
        if (userNode->subscription.type == FREE_TIER) {
            if (userNode->subscription.songsListened >= 10) {
                cout << "\n\tFree tier limit reached! You have listened to 10 songs. Please purchase a package for unlimited access." << endl;
                cout << "\n\tPress ENTER to exit player..." << endl;
                _getch(); system("cls"); return;
            }
        } else if (userNode->subscription.type != PERMANENT) {
            if (getCurrentUnixTime() >= userNode->subscription.expiryUnixTime) {
                cout << "\n\tYour subscription has expired! Please renew your package for unlimited access." << endl;
                userNode->subscription.type = FREE_TIER;
                userNode->subscription.expiryUnixTime = 0;
                userNode->subscription.songsListened = 0;
                saveHashTableToFile((const struct AccountNode* const*)userLogin->userHashTable, HASH_TABLE_SIZE, userLogin->FILE_NAME);
                cout << "\n\tPress ENTER to exit player..." << endl;
                _getch(); system("cls"); return;
            }
        }

        if (isPlaying && !isPaused) {
            Song* song = &selectedPlaylist->songs[currentSongIndex];
            cout << "\n\tNow Playing: " << song->songName << " by " << song->artist
                 << " | Genre: " << song->genre << " | Language: " << song->language << endl;
            
            if (userNode->subscription.type == FREE_TIER) {
                userNode->subscription.songsListened++;
                saveHashTableToFile((const struct AccountNode* const*)userLogin->userHashTable, HASH_TABLE_SIZE, userLogin->FILE_NAME);
                cout << "\t(Free Tier: " << userNode->subscription.songsListened << "/10 songs listened)" << endl;
                if (userNode->subscription.songsListened >= 10) {
                    cout << "\n\tFree tier limit reached after this song! You will not be able to play more songs." << endl;
                }
            }
        } else if (isPaused) {
            Song* song = &selectedPlaylist->songs[currentSongIndex];
            cout << "\n\tPlayback Paused: " << song->songName << " by " << song->artist
                 << " | Genre: " << song->genre << " | Language: " << song->language << endl;
        }

        cout << "\n\tPlayback Options:\n";
        cout << "\t1. Pause\n";
        cout << "\t2. Continue\n";
        cout << "\t3. Next Song\n";
        cout << "\t4. Previous Song\n";
        cout << "\t5. Exit Player\n";
        cout << "\tEnter your choice: ";

        int choice;
        if (scanf("%d", &choice) != 1) {
            cout << "\n\tInvalid choice! Please try again." << endl;
            while(getchar() != '\n');
            Sleep(700); system("cls"); juztodisplaydesign(); continue;
        }
        while(getchar() != '\n');
       
        if (choice == 1) {
            if (isPlaying) {
                isPaused = 1;
                system("cls"); juztodisplaydesign();
                cout << "\n\tPress ENTER to select playback options..." << endl;
                _getch(); system("cls"); juztodisplaydesign();
            } else {
                cout << "\n\tCannot pause. Playback is already stopped." << endl;
                cout << "\n\tPress ENTER to select playback options..." << endl;
                _getch(); system("cls"); juztodisplaydesign();
            }
        } else if (choice == 2) {
            if (isPlaying && isPaused) {
                isPaused = 0;
                cout << "\n\tPlayback resumed." << endl;
                cout << "\n\tPress ENTER to select playback options..." << endl;        			
                _getch(); system("cls"); juztodisplaydesign();
            } else {
                cout << "\n\tCannot continue. Playback is not paused." << endl;
                cout << "\n\tPress ENTER to select playback options..." << endl;
                _getch(); system("cls"); juztodisplaydesign();
            }
        } else if (choice == 3) {
            if (currentSongIndex + 1 < selectedPlaylist->songCount) {
                currentSongIndex++;
                isPaused = 0;
                cout << "\n\tNow playing next song" << endl;
                Sleep(40);
                cout << "\n\tPress ENTER to select playback options..." << endl;
                _getch(); system("cls"); juztodisplaydesign();
            } else {
                cout << "\n\tYou are already at the last song in the playlist!" << endl;
                cout << "\n\tPress ENTER to select playback options..." << endl;
                _getch(); system("cls"); juztodisplaydesign();
            }
        } else if (choice == 4) {
            if (currentSongIndex - 1 >= 0) {
                currentSongIndex--;
                isPaused = 0;
                cout << "\n\tNow playing previous song" << endl;
                Sleep(40);
                cout << "\n\tPress ENTER to select playback options..." << endl;
                _getch(); system("cls"); juztodisplaydesign();
            } else {
                cout << "\n\tYou are already at the first song in the playlist!" << endl;
                cout << "\n\tPress ENTER to select playback options..." << endl;
                _getch(); system("cls"); juztodisplaydesign();
            }
        } else if (choice == 5) {
            cout << "\n\tExiting playlist player." << endl;
            Sleep(40);
            Sleep(700); system("cls"); break;
        } else {
            cout << "\n\tInvalid choice! Please try again." << endl;
            Sleep(700); system("cls"); juztodisplaydesign();
        }
    }
}

void displayAllSongsInPlaylists(const User* user) {
    system("cls");
    cout << "\n\t|-------------------------------|\n";
    cout << "\t|       DISPLAY ALL SONGS       |\n";
    cout << "\t|-------------------------------|\n";
    
    if (user->playlistCount == 0) {
        cout << "\n\tNo playlists available!" << endl; 
        cout << "\n\tPress ENTER to return to user menu..." << endl;
        _getch(); system("cls"); return;
    }

    for (int i = 0; i < user->playlistCount; i++) {
        cout << "\n\tPlaylist: " << user->playlists[i].name << endl;
        if (user->playlists[i].songCount == 0) {
            cout << "\t  (No songs in this playlist)" << endl;
            continue;
        }
        for (int j = 0; j < user->playlists[i].songCount; j++) {
            const Song* song = &user->playlists[i].songs[j];
            cout << "\t  Song: " << song->songName << ", Artist: " << song->artist
                 << ", Genre: " << song->genre << ", Language: " << song->language << endl;
        }
    }
    
    cout << "\n\tPress ENTER to return to user menu..." << endl;
    _getch(); system("cls");
}

void mergePlaylistsByGenreOrLanguage(User* user, Admin* admin_ptr) {
    int choice;
    
    system("cls");
    cout << "\n\t|-------------------------------|\n";
    cout << "\t|         MERGE PLAYLIST        |\n";
    cout << "\t|-------------------------------|\n";
        
    while (1) {
        cout << "\n\tChoose an option to merge by:\n";
        cout << "\t1. Genre\n";
        cout << "\t2. Language\n";
        cout << "\n\tEnter your choice: ";
        if (scanf("%d", &choice) != 1) {
            cout << "\tInvalid input! Please enter 1 for Genre or 2 for Language." << endl; 
            while(getchar() != '\n');
        } else if (choice != 1 && choice != 2) {
            cout << "\tInvalid input! Please enter 1 for Genre or 2 for Language." << endl; 
            while(getchar() != '\n');
        } else {
            break;
        }
    }
    while(getchar() != '\n');

    if (choice == 1) {
        cout << "\n\tAvailable Genres:\n";
        for (int i = 0; i < admin_ptr->genreCount; i++) {
            cout << "\t" << (i + 1) << ". " << admin_ptr->genres[i] << endl;
        }
    } else {
        cout << "\n\tAvailable Languages:\n";
        for (int i = 0; i < admin_ptr->languageCount; i++) {
            cout << "\t" << (i + 1) << ". " << admin_ptr->languages[i] << endl;
        }
    }

    int selectedOptionIndex;
    while (1) {
        cout << "\n\tEnter the number of your choice: ";
        if (scanf("%d", &selectedOptionIndex) != 1) {
            cout << "\tInvalid input! Please select a valid number." << endl;
            while(getchar() != '\n');
        } else if ((choice == 1 && (selectedOptionIndex < 1 || selectedOptionIndex > admin_ptr->genreCount)) ||
                   (choice == 2 && (selectedOptionIndex < 1 || selectedOptionIndex > admin_ptr->languageCount))) {
            cout << "\tInvalid input! Please select a valid number." << endl;
            while(getchar() != '\n');
        } else {
            break;
        }
    }
    while(getchar() != '\n');

    char selectedOptionName[MAX_NAME_LEN];
    if (choice == 1) { strcpy(selectedOptionName, admin_ptr->genres[selectedOptionIndex - 1]); }
    else { strcpy(selectedOptionName, admin_ptr->languages[selectedOptionIndex - 1]); }

    Playlist mergedPlaylist;
    mergedPlaylist.songCount = 0;

    for (int i = 0; i < user->playlistCount; i++) {
        for (int j = 0; j < user->playlists[i].songCount; j++) {
            if ((choice == 1 && strcmp(user->playlists[i].songs[j].genre, selectedOptionName) == 0) ||
                (choice == 2 && strcmp(user->playlists[i].songs[j].language, selectedOptionName) == 0)) {
                if (mergedPlaylist.songCount < MAX_SONGS) {
                    mergedPlaylist.songs[mergedPlaylist.songCount++] = user->playlists[i].songs[j];
                } else {
                    cout << "\n\tWarning: Merged playlist reached maximum song limit. Not all songs were added." << endl;
                    break;
                }
            }
        }
    }

    if (mergedPlaylist.songCount > 0) {
        if (user->playlistCount >= MAX_PLAYLISTS) {
            cout << "\n\tError: Cannot create merged playlist. Maximum number of playlists reached!" << endl;
            cout << "\n\tPress ENTER to return to user menu..." << endl;
            _getch(); system("cls"); return;
        }

        char newPlaylistName[MAX_NAME_LEN];
        cout << "\tEnter name for the merged playlist: ";
        if (fgets(newPlaylistName, MAX_NAME_LEN, stdin) == NULL) {
            cout << "\tInvalid input!" << endl;
            cout << "\n\tPress ENTER to return to user menu..." << endl;
            _getch(); system("cls"); return;
        }
        newPlaylistName[strcspn(newPlaylistName, "\n")] = 0;

        strcpy(mergedPlaylist.name, newPlaylistName);
        user->playlists[user->playlistCount++] = mergedPlaylist;
        cout << "\n\tMerged Playlist created successfully!" << endl;

        cout << "\n\tMerged Playlist (" << selectedOptionName << "):\n";
        for (int i = 0; i < mergedPlaylist.songCount; i++) {
            cout << "\tSong: " << mergedPlaylist.songs[i].songName
                 << ", Artist: " << mergedPlaylist.songs[i].artist
                 << ", Genre: " << mergedPlaylist.songs[i].genre
                 << ", Language: " << mergedPlaylist.songs[i].language << endl;
        }
    } else {
        cout << "\n\tNo songs found for the selected " << (choice == 1 ? "genre" : "language") << "!" << endl;
    }
    
    cout << "\n\tPress ENTER to return to user menu..." << endl;
    _getch(); system("cls");
}

struct PurchaseReportRecord {
    char transactionId[MAX_NAME_LEN];
    char username[MAX_NAME_LEN];
    char itemName[MAX_NAME_LEN];
    char itemType[20];
    double amount;
    char paymentMethod[MAX_NAME_LEN];
    char paymentDetails[MAX_NAME_LEN];
    long long timestamp;
    char status[20];
    long long purchasedItemExpiry;
};

void binggoSortPurchaseRecords(struct PurchaseReportRecord records[], int n) { // Binggo Sort (Selection Sort) for Purchase Records by Username
    for (int i = 0; i < n - 1; i++) {
        int min_idx = i;
        for (int j = i + 1; j < n; j++) {
            if (strcmp(records[j].username, records[min_idx].username) < 0) {
                min_idx = j;
            }
        }
        struct PurchaseReportRecord temp = records[min_idx];
        records[min_idx] = records[i];
        records[i] = temp;
    }
}

void binggoSortPurchaseRecordsByAmount(struct PurchaseReportRecord records[], int n) { // Binggo Sort (Selection Sort) for Purchase Records by Amount
    for (int i = 0; i < n - 1; i++) {
        int min_idx = i;
        for (int j = i + 1; j < n; j++) {
            if (records[j].amount < records[min_idx].amount) {
                min_idx = j;
            }
        }
        struct PurchaseReportRecord temp = records[min_idx];
        records[min_idx] = records[i];
        records[i] = temp;
    }
}

void generatePurchaseReport() { // Admin Report Enhancements: Detailed purchase report with sorting/filtering
    system("cls");
    cout << "\n\t+-----------------------------------+\n";
    cout << "\t|       PURCHASE REPORT             |\n";
    cout << "\t+-----------------------------------+\n";

    FILE* file = fopen("payment_records.txt", "r");
    if (!file) {
        cout << "\n\tNo purchase records found!" << endl;
        cout << "\n\tPress ENTER to return to admin menu..." << endl;
        _getch(); system("cls"); return;
    }

    struct PurchaseReportRecord records[100];
    int recordCount = 0;
    char line[512];

    while (fgets(line, sizeof(line), file) != NULL && recordCount < 100) {
        line[strcspn(line, "\n")] = 0;

        char temp_line[512];
        strcpy(temp_line, line);

        char* token;
        token = strtok(temp_line, ",");
        if (token) custom_strncpy(records[recordCount].transactionId, token, MAX_NAME_LEN); else continue;
        token = strtok(NULL, ",");
        if (token) custom_strncpy(records[recordCount].username, token, MAX_NAME_LEN); else continue;
        token = strtok(NULL, ",");
        if (token) custom_strncpy(records[recordCount].itemName, token, MAX_NAME_LEN); else continue;
        token = strtok(NULL, ",");
        if (token) custom_strncpy(records[recordCount].itemType, token, 20); else continue;
        token = strtok(NULL, ",");
        if (token) records[recordCount].amount = atof(token); else continue;
        token = strtok(NULL, ",");
        if (token) custom_strncpy(records[recordCount].paymentMethod, token, MAX_NAME_LEN); else continue;
        token = strtok(NULL, ",");
        if (token) custom_strncpy(records[recordCount].paymentDetails, token, MAX_NAME_LEN); else continue;
        token = strtok(NULL, ",");
        if (token) records[recordCount].timestamp = atoll(token); else continue;
        token = strtok(NULL, ",");
        if (token) custom_strncpy(records[recordCount].status, token, 20); else continue;
        token = strtok(NULL, ",");
        if (token) records[recordCount].purchasedItemExpiry = atoll(token); else continue;

        recordCount++;
    }
    fclose(file);

    if (recordCount == 0) {
        cout << "\n\tNo purchase records found!" << endl;
        cout << "\n\tPress ENTER to return to admin menu..." << endl;
        _getch(); system("cls"); return;
    }

    int reportChoice;
    char filterStatus[20] = "";
    double totalRevenue = 0.0;
    int totalSubscriptions = 0;
    int totalAlbums = 0;

    cout << "\n\tChoose report options:\n";
    cout << "\t1. Sort by Username (Default)\n";
    cout << "\t2. Sort by Amount\n";
    cout << "\t3. Filter by Status (SUCCESS/FAILED)\n";
    cout << "\tEnter your choice: ";
    if (scanf("%d", &reportChoice) != 1) {
        cout << "\tInvalid input, displaying default sort." << endl;
        while(getchar() != '\n');
        reportChoice = 1;
    }
    while(getchar() != '\n');

    if (reportChoice == 1) {
        binggoSortPurchaseRecords(records, recordCount); // Binggo Sort (Selection Sort) for Purchase Records by Username
    } else if (reportChoice == 2) {
        binggoSortPurchaseRecordsByAmount(records, recordCount); // Binggo Sort (Selection Sort) for Purchase Records by Amount
    } else if (reportChoice == 3) {
        cout << "\tEnter status to filter by (SUCCESS/FAILED): ";
        if (scanf("%s", filterStatus) != 1) {
            cout << "\tInvalid input, no filter applied." << endl;
            while(getchar() != '\n');
            strcpy(filterStatus, "");
        }
        while(getchar() != '\n');
        for (int i = 0; filterStatus[i] != '\0'; i++) {
            filterStatus[i] = toupper(filterStatus[i]);
        }
    } else {
        cout << "\tInvalid choice, displaying default sort." << endl;
        binggoSortPurchaseRecords(records, recordCount); // Binggo Sort (Selection Sort) for Purchase Records by Username
    }

    cout << "\n\t--- Purchase Report ---\n";
    for (int i = 0; i < recordCount; i++) {
        if (strlen(filterStatus) > 0 && strcmp(records[i].status, filterStatus) != 0) {
            continue;
        }

        time_t rawtime = records[i].timestamp;
        struct tm* dt = localtime(&rawtime);
        char date_str[50];
        strftime(date_str, sizeof(date_str), "%Y-%m-%d %H:%M:%S", dt);

        char expiry_str[50];
        long long exp_val = records[i].purchasedItemExpiry;
        if (exp_val == -1) {
            strcpy(expiry_str, "Permanent");
        } else if (exp_val == 0) {
            strcpy(expiry_str, "N/A (Free/Expired)");
        } else {
            time_t expiry_rawtime = exp_val;
            struct tm* expiry_dt = localtime(&expiry_rawtime);
            strftime(expiry_str, sizeof(expiry_str), "%Y-%m-%d %H:%M:%S", expiry_dt);
        }

        cout << "\t--------------------------------------------------\n";
        cout << "\tTransaction ID: " << records[i].transactionId << endl;
        cout << "\tUsername: " << records[i].username << endl;
        cout << "\tItem: " << records[i].itemName << " (" << records[i].itemType << ")" << endl;
        cout << "\tAmount: $" << fixed << setprecision(2) << records[i].amount << endl;
        cout << "\tMethod: " << records[i].paymentMethod << endl;
        cout << "\tDetails: " << records[i].paymentDetails << endl;
        cout << "\tDate: " << date_str << endl;
        cout << "\tStatus: " << records[i].status << endl;
        cout << "\tExpiry: " << expiry_str << endl;

        if (strcmp(records[i].status, "SUCCESS") == 0) {
            totalRevenue += records[i].amount;
            if (strcmp(records[i].itemType, "Subscription") == 0) {
                totalSubscriptions++;
            } else if (strcmp(records[i].itemType, "Album") == 0) {
                totalAlbums++;
            }
        }
    }
    cout << "\t--------------------------------------------------\n";
    cout << "\n\tSummary:\n";
    cout << "\tTotal Records Displayed: " << recordCount << endl;
    cout << "\tTotal Revenue: $" << fixed << setprecision(2) << totalRevenue << endl;
    cout << "\tTotal Subscriptions Purchased: " << totalSubscriptions << endl;
    cout << "\tTotal Albums Purchased: " << totalAlbums << endl;

    cout << "\n\tPress ENTER to return to admin menu..." << endl;
    _getch(); system("cls");
}

void searchPurchaseRecords() { // Admin Report Enhancements: Search purchase records
    char keyword[MAX_NAME_LEN];
    char choice_input[10];

    do {
        system("cls");
        cout << "\n\t+-----------------------------------+\n";
        cout << "\t|       SEARCH PURCHASE RECORDS     |\n";
        cout << "\t+-----------------------------------+\n";

        cout << "\n\tEnter keyword to search (ID, username, item, method, status): ";
      
        if (fgets(keyword, MAX_NAME_LEN, stdin) == NULL) {
            cout << "\tInvalid input!" << endl;
            continue;
        }
        keyword[strcspn(keyword, "\n")] = 0;

        FILE* file = fopen("payment_records.txt", "r");
        if (!file) {
            cout << "\n\tNo purchase records found to search!" << endl;
            cout << "\n\tPress ENTER to return to admin menu..." << endl;
            _getch(); system("cls"); return;
        }

        cout << "\n\tSearch Results:\n";
        int found_count = 0;
        char line[512];

        while (fgets(line, sizeof(line), file) != NULL) {
            line[strcspn(line, "\n")] = 0;

            char temp_line[512];
            strcpy(temp_line, line);

            char* token_id = strtok(temp_line, ",");
            char* token_username = strtok(NULL, ",");
            char* token_itemname = strtok(NULL, ",");
            char* token_itemtype = strtok(NULL, ",");
            char* token_amount = strtok(NULL, ",");
            char* token_method = strtok(NULL, ",");
            char* token_details = strtok(NULL, ",");
            char* token_timestamp = strtok(NULL, ",");
            char* token_status = strtok(NULL, ",");
            char* token_expiry = strtok(NULL, ",");

            if ((token_id && custom_strstr(token_id, keyword) != NULL) || // String Search: custom_strstr usage
                (token_username && custom_strstr(token_username, keyword) != NULL) || // String Search: custom_strstr usage
                (token_itemname && custom_strstr(token_itemname, keyword) != NULL) || // String Search: custom_strstr usage
                (token_itemtype && custom_strstr(token_itemtype, keyword) != NULL) || // String Search: custom_strstr usage
                (token_method && custom_strstr(token_method, keyword) != NULL) || // String Search: custom_strstr usage
                (token_status && custom_strstr(token_status, keyword) != NULL)) { // String Search: custom_strstr usage
                
                time_t rawtime = atoll(token_timestamp);
                struct tm* dt = localtime(&rawtime);
                char date_str[50];
                strftime(date_str, sizeof(date_str), "%Y-%m-%d %H:%M:%S", dt);

                char expiry_str[50];
                long long exp_val = atoll(token_expiry);
                if (exp_val == -1) {
                    strcpy(expiry_str, "Permanent");
                } else if (exp_val == 0) {
                    strcpy(expiry_str, "N/A (Free/Expired)");
                } else {
                    time_t expiry_rawtime = exp_val;
                    struct tm* expiry_dt = localtime(&expiry_rawtime);
                    strftime(expiry_str, sizeof(expiry_str), "%Y-%m-%d %H:%M:%S", expiry_dt);
                }

                cout << "\t--------------------------------------------------\n";
                cout << "\tTransaction ID: " << token_id << endl;
                cout << "\tUsername: " << token_username << endl;
                cout << "\tItem: " << token_itemname << " (" << token_itemtype << ")" << endl;
                cout << "\tAmount: $" << fixed << setprecision(2) << atof(token_amount) << endl;
                cout << "\tMethod: " << token_method << endl;
                cout << "\tDetails: " << token_details << endl;
                cout << "\tDate: " << date_str << endl;
                cout << "\tStatus: " << token_status << endl;
                cout << "\tExpiry: " << expiry_str << endl;
                found_count++;
            }
        }
        fclose(file);

        if (found_count == 0) {
            cout << "\n\tNo matching purchase records found!" << endl;
        }

        cout << "\n\tDo you want to search again? (yes/no): ";
        if (scanf("%s", choice_input) != 1) {
            cout << "\tInvalid input!" << endl;
            while(getchar() != '\n');
            strcpy(choice_input, "yes");
        }
        while(getchar() != '\n');
        for (int i = 0; choice_input[i] != '\0'; i++) {
            choice_input[i] = tolower(choice_input[i]);
        }

    } while (strcmp(choice_input, "yes") == 0);

    cout << "\n\tReturning to the admin menu..." << endl;
    Sleep(700);
    system("cls");
}

void SetColor(int color) {
    // Placeholder function, no changes needed
}

void PrintRainbowText(const char* text) {
    cout << text;
}

void PrintRainbowEffectText(const char* text, int timer) {
    cout << text;
}

void PrintItalicText(const char* text, int timer) {
    cout << text;
}

void PrintColoredText(const char* text, int color) {
    cout << text;
}

void PrintwEffect(const char* text, int color, int timer) {
    cout << text;
}

void PrintFlashingText(const char* text, int timer, int flashDuration) {
    cout << text;
}

void Effect(const char* text, int timer) {
    cout << text;
}

void loadingPage(int timer) {
    int code = 177, code2 = 219;
    
    cout << "\n\n\n\n\n\n\n\n\t\t\tLoading...\n\n\t\t\t";
    
    for(int i = 0; i < 25; i++) {
        cout << (char)code;
    }
    
    cout << "\r\t\t\t";
             
    for(int i = 0; i < 25; i++) {
        cout << (char)code2;
        Sleep(timer);
    }
    
    cout << "\n";
}

void juztodisplaydesign() {
    cout << "\n\t|-------------------------------|\n";
    cout << "\t|       PLAYING PLAYLIST...     |\n";
    cout << "\t|-------------------------------|\n";
}

void landingPage() {
    cout << "\n\n\n\n\n\n\n";
    cout << "\t\t+-----------------------------------------+\n";
    cout << "\t\t|                                         |\n";
    cout << "\t\t|   Welcome to Riffmix Playlist system    |\n";
    cout << "\t\t|                                         |\n";
    cout << "\t\t+-----------------------------------------+\n";
    cout << "\t\t  \t\t----Enjoy High-Quality Music Experience----\n";

    Sleep(500);
    system("cls");

    loadingPage(50);
    
    system("cls");
}

int main() {
    landingPage();
    
    User user;
    Admin admin;
    AdminLogin adminLogin;
    UserLogin userLogin;
    
    int choice;
    char decision_input[10];

    while (1) {
        cout << "\t+++++++++++++++++++++++++++++++++\n";
        cout << "\t|                               |\n";
        cout << "\t|    Main Menu:                 |\n";
        cout << "\t|    1. User Login              |\n";
        cout << "\t|    2. User Sign Up            |\n";
        cout << "\t|    3. Admin Login             |\n";
        cout << "\t|    4. Admin Sign Up           |\n";
        cout << "\t|    5. Exit                    |\n";
        cout << "\t|                               |\n";
        cout << "\t+++++++++++++++++++++++++++++++++\n";
        cout << "\n\tEnter choice: ";
        if (scanf("%d", &choice) != 1) {
            cout << "\tInvalid choice! Please select a valid option (1-5)." << endl;
            while (getchar() != '\n');
            Sleep(700); system("cls"); continue;
        }
        while (getchar() != '\n');

        if (choice == 1) {
            while (1) {
                cout << "\n\tDo you want to login as user? (yes/no): ";
                if (scanf("%s", decision_input) != 1) {
                    cout << "\tInvalid input!" << endl;
                    while (getchar() != '\n');
                    continue;
                }
                while (getchar() != '\n');
             
                for (int i = 0; decision_input[i] != '\0'; i++) {
                    decision_input[i] = tolower(decision_input[i]);
                }

                if (strcmp(decision_input, "yes") == 0) {
                    system("cls");
                    userLogin.login();
                    if (userLogin.validateLogin(userLogin.getUsername(), userLogin.getPassword())) {
                        user.setCurrentUsername(userLogin.getUsername());
                        user.loadPlaylists(); // User Data Persistence: Load user's playlists after login
                        while (1) {
                            cout << "\t+++++++++++++++++++++++++++++++++\n";
                            cout << "\t|                               |\n";
                            cout << "\t|    User Menu:                 |\n";
                            cout << "\t|    1. Create Playlist         |\n";
                            cout << "\t|    2. Search Playlist         |\n";
                            cout << "\t|    3. Update Playlist         |\n";
                            cout << "\t|    4. Add Songs               |\n";
                            cout << "\t|    5. Delete Song             |\n";
                            cout << "\t|    6. Delete Playlist         |\n";
                            cout << "\t|    7. Display All Songs       |\n";
                            cout << "\t|    8. Merge Playlists         |\n";
                            cout << "\t|    9. Import Playlist         |\n";
                            cout << "\t|    10. Download Playlist      |\n";
                            cout << "\t|    11. Play Playlist          |\n";
                            cout << "\t|    12. Purchase Package       |\n";
                            cout << "\t|    13. Purchase Digital Album |\n";
                            cout << "\t|    14. Logout                 |\n";
                            cout << "\t|                               |\n";
                            cout << "\t+++++++++++++++++++++++++++++++++\n";
                            cout << "\n\tEnter your choice: ";
                            if (scanf("%d", &choice) != 1) {
                                cout << "\tInvalid choice! Please select a valid option (1-14)." << endl;
                                while (getchar() != '\n');
                                Sleep(700); system("cls"); continue;
                            }
                            while (getchar() != '\n');

                            if (choice == 14) {
                                user.savePlaylists(); // User Data Persistence: Save playlists before logout
                                cout << "\n\tLogging out";
                                for (int i = 0; i < 4; i++) { Sleep(500); cout << "."; }
                                system("cls"); break;
                            }

                            char proceedDecision_input[10];
                            while (1) {
                                cout << "\tDo you want to proceed with this action? (yes/no): ";
                                if (scanf("%s", proceedDecision_input) != 1) {
                                    cout << "\tInvalid input!" << endl;
                                    while (getchar() != '\n');
                                    continue;
                                }
                                while (getchar() != '\n');
                    
                                for (int i = 0; proceedDecision_input[i] != '\0'; i++) {
                                    proceedDecision_input[i] = tolower(proceedDecision_input[i]);
                                }

                                if (strcmp(proceedDecision_input, "yes") == 0) {
                                    switch (choice) {
                                        case 1: user.createPlaylist(&admin); loadingPage(40); system("cls"); break;
                                        case 2: user.searchPlaylist(); loadingPage(40); system("cls"); break;
                                        case 3: user.updatePlaylist(); loadingPage(40); system("cls"); break;
                                        case 4: user.addSong(&admin); system("cls"); break;
                                        case 5: user.deleteSong(); loadingPage(40); system("cls"); break;
                                        case 6: user.deletePlaylist(); loadingPage(40); system("cls"); break;
                                        case 7: displayAllSongsInPlaylists(&user); break;
                                        case 8: mergePlaylistsByGenreOrLanguage(&user, &admin); break;
                                        case 9: user.importPlaylist(); loadingPage(40); system("cls"); break;
                                        case 10: user.downloadPlaylist(); loadingPage(40); system("cls"); break;
                                        case 11: user.playPlaylist(&userLogin); break;
                                        case 12: user.purchasePackage(&userLogin, &admin); break;
                                        case 13: user.purchaseDigitalAlbum(&userLogin, &admin); break; // New Purchasable Items: Buy digital album
                                        default: cout << "\n\tInvalid choice! Please select a valid option (1-14)." << endl; break;
                                    }
                                    break;
                                } else if (strcmp(proceedDecision_input, "no") == 0) {
                                    cout << "\n\tAction canceled. Returning to the User Menu..." << endl;
                                    Sleep(50);
                                    system("cls"); break;
                                } else {
                                    cout << "\tInvalid input! Please enter 'yes' or 'no'." << endl << endl;
                                }
                            }
                        }
                    }
                    break;
                } else if (strcmp(decision_input, "no") == 0) {
                    cout << "\n\tReturning to main menu";
                    for (int i = 0; i < 3; i++) { Sleep(500); cout << "."; }
                    system("cls"); break;
                } else {
                    cout << "\tInvalid value! Please enter 'yes' or 'no'." << endl;
                }
            }
        } else if (choice == 2) {
            while (1) {
                cout << "\n\tDo you want to register as user? (yes/no): ";
                if (scanf("%s", decision_input) != 1) {
                    cout << "\tInvalid input!" << endl;
                    while (getchar() != '\n');
                    continue;
                }
                while (getchar() != '\n');

                for (int i = 0; decision_input[i] != '\0'; i++) {
                    decision_input[i] = tolower(decision_input[i]);
                }

                if (strcmp(decision_input, "yes") == 0) {
                    userLogin.registerUser(); break;
                } else if (strcmp(decision_input, "no") == 0) {
                    cout << "\n\tReturning to main menu";
                    for (int i = 0; i < 3; i++) { Sleep(500); cout << "."; }
                    system("cls"); break;
                } else {
                    cout << "\tInvalid value! Please enter 'yes' or 'no'." << endl;
                }
            }
        } else if (choice == 3) {
            while (1) {
                cout << "\n\tDo you want to login as admin? (yes/no): ";
                if (scanf("%s", decision_input) != 1) {
                    cout << "\tInvalid input!" << endl;
                    while (getchar() != '\n');
                    continue;
                }
                while (getchar() != '\n');

                for (int i = 0; decision_input[i] != '\0'; i++) {
                    decision_input[i] = tolower(decision_input[i]);
                }

                if (strcmp(decision_input, "yes") == 0) {
                    adminLogin.login();
                    if (adminLogin.getLoginStatus()) {
                        while (1) {
                            cout << "\t#############################################\n";
                            cout << "\t!                                           !\n";
                            cout << "\t!    Admin Menu:                            !\n";
                            cout << "\t!    1. Add Genre                           !\n";
                            cout << "\t!    2. Add Language                        !\n";
                            cout << "\t!    3. Add Song to Recommendation          !\n";
                            cout << "\t!    4. Delete Song from Recommendation     !\n";
                            cout << "\t!    5. Delete Genre                        !\n";
                            cout << "\t!    6. Delete Language                     !\n";
                            cout << "\t!    7. Display All Genres                  !\n";
                            cout << "\t!    8. Display All Languages               !\n";
                            cout << "\t!    9. Display All Recommended Songs       !\n";
                            cout << "\t!    10. Add Album for Sale                 !\n";
                            cout << "\t!    11. Display All Albums for Sale        !\n";
                            cout << "\t!    12. Generate Purchase Report           !\n";
                            cout << "\t!    13. Search Purchase Records            !\n";
                            cout << "\t!    14. Search Recommended Songs           !\n";
                            cout << "\t!    15. Logout                             !\n";
                            cout << "\t!                                           !\n";
                            cout << "\t#############################################\n";
                            cout << "\n\tEnter your choice: ";
                            if (scanf("%d", &choice) != 1) {
                                cout << "\tInvalid choice! Please select a valid option (1-15)." << endl;
                                 while (getchar() != '\n');
                                 Sleep(900); system("cls"); continue;
                            }
                            while (getchar() != '\n');

                            if (choice == 15) {
                                cout << "\n\tLogging out";
                                for (int i = 0; i < 4; i++) { Sleep(700); cout << "."; }
                                system("cls"); break;
                            }

                            char proceedDecision_input[10];
                            while(1){
                                cout << "\tDo you want to proceed with this action? (yes/no): ";
                                if (scanf("%s", proceedDecision_input) != 1) {
                                    cout << "\tInvalid input!" << endl;
                                    while (getchar() != '\n');
                                    continue;
                                }
                                while (getchar() != '\n');

                                for (int i = 0; proceedDecision_input[i] != '\0'; i++) {
                                    proceedDecision_input[i] = tolower(proceedDecision_input[i]);
                                }

                                if (strcmp(proceedDecision_input, "yes") == 0) {
                                    switch (choice) {
                                        case 1: admin.addGenre(); break;
                                        case 2: admin.addLanguage(); break;
                                        case 3: admin.addSongToRecommendation(); break;
                                        case 4: admin.deleteSongFromRecommendation(); break;
                                        case 5: admin.deleteGenre(); break;
                                        case 6: admin.deleteLanguage(); break;
                                        case 7: admin.displayAllGenres(); break;
                                        case 8: admin.displayAllLanguages(); break;
                                        case 9: admin.displayAllRecommendedSongs(); break;
                                        case 10: admin.addAlbumToSale(); break; // Admin add album for sale
                                        case 11: admin.displayAllPurchasableAlbums(); break; // Admin view purchasable albums
                                        case 12: generatePurchaseReport(); break; // Admin generate purchase report
                                        case 13: searchPurchaseRecords(); break; // Admin search purchase records
                                        case 14: admin.searchRecommendedSongsAdmin(); break;
                                        default: cout << "\n\tInvalid choice! Please select a valid option (1-15)." << endl; break;
                                    }
                                    break;
                                } else if (strcmp(proceedDecision_input, "no") == 0) {
                                    cout << "\n\tAction canceled. Returning to the Admin Menu..." << endl;
                                    Sleep(50);
                                    system("cls"); break;
                                } else {
                                    cout << "\tInvalid input! Please enter 'yes' or 'no'." << endl << endl;
                                }
                            }
                        }
                    }
                    break;
                } else if (strcmp(decision_input, "no") == 0) {
                    cout << "\n\tReturning to main menu";
                    for (int i = 0; i < 3; i++) { Sleep(500); cout << "."; }
                    system("cls"); break;
                } else {
                    cout << "\tInvalid value! Please enter 'yes' or 'no'." << endl;
                }
            }
        } else if (choice == 4) {
            while (1) {
                cout << "\n\tDo you want to sign up as an admin? (yes/no): ";
                if (scanf("%s", decision_input) != 1) {
                    cout << "\tInvalid input!" << endl;
                    while (getchar() != '\n');
                    continue;
                }
                while (getchar() != '\n');

                for (int i = 0; decision_input[i] != '\0'; i++) {
                    decision_input[i] = tolower(decision_input[i]);
                }

                if (strcmp(decision_input, "yes") == 0) {
                    system("cls"); adminLogin.signup(); break;
                } else if (strcmp(decision_input, "no") == 0) {
                    cout << "\n\tReturning to main menu";
                    for (int i = 0; i < 3; i++) { Sleep(500); cout << "."; }
                    system("cls"); break;
                } else {
                    cout << "\tInvalid value! Please enter 'yes' or 'no'." << endl;
                }
            }
        } else if (choice == 5) {
            while (1) {
                cout << "\n\tDo you want to Exit? (yes/no): ";
                if (scanf("%s", decision_input) != 1) {
                    cout << "\tInvalid input!" << endl;
                    while (getchar() != '\n');
                    continue;
                }
                while (getchar() != '\n');
              
                for (int i = 0; decision_input[i] != '\0'; i++) {
                    decision_input[i] = tolower(decision_input[i]);
                }

                if (strcmp(decision_input, "yes") == 0) {
                    cout << "\n\tExiting program. Goodbye!" << endl;
                    Sleep(60);
                    exit(0); 
                } else if (strcmp(decision_input, "no") == 0) {
                    cout << "\n\tReturning to main menu";
                    for (int i = 0; i < 3; i++) { Sleep(500); cout << "."; }
                    system("cls"); break;
                } else {
                    cout << "\tInvalid value! Please enter 'yes' or 'no'." << endl;
                }
            }
        } else {
            cout << "\tInvalid choice! Please select a valid option (1-5)." << endl;
            Sleep(700); system("cls");
        }
    }

    return 0;
}

