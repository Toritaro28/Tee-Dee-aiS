#include <iostream>
#include <string.h> // For strlen, strcmp, strcpy, strcat, strchr, strtok
#include <stdlib.h> // For atoi, atof, exit
#include <ctype.h>  // For tolower, isupper, islower, isdigit, ispunct
#include <time.h>   // For time, localtime, strftime
#include <windows.h> // For Sleep, system("cls"), _getch
#include <iomanip>  // For setprecision, fixed
#include <exception> // For std::exception base class
#include <conio.h>

// Define PackageType enum early, as it's used in Subscription struct
enum PackageType {
    FREE_TIER = 0,
    ONE_MONTH = 1,
    THREE_MONTH = 2,
    PERMANENT = 3
};

class FileException : public std::exception {
private:
    char message[256];
public:
    FileException(const char* msg) {
        strncpy(message, msg, sizeof(message) - 1); // Use std::strncpy
        message[sizeof(message) - 1] = '\0'; // Ensure null termination
    }
    // Changed throw() to noexcept for C++11 compatibility, but kept throw() as per original comment in code
    // If you compile with -std=c++11 or higher, consider changing throw() to noexcept
    virtual const char* what() const throw() {
        return message;
    }
};

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

// Forward declarations of structs
struct Song;
struct Album;
struct Playlist;
struct Subscription;
struct PaymentRecord;
struct AccountNode;
// enum PackageType; // No longer needed here, moved definition above.
struct PurchaseReportRecord; // For Admin report

// Forward declarations of classes
class Admin; // For friend functions
class User;   // For friend functions
class UserLogin; // For User methods
class MediaPlayer; // Base class for PlaylistPlayer
class PlaylistPlayer; // Derived class for playing music

// --- Custom String Functions (Already present and good) ---
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

        // Perform case-insensitive comparison
        while (*n_ptr && *h_ptr && (tolower(*h_ptr) == tolower(*n_ptr))) {
            h_ptr++;
            n_ptr++;
        }
        if (!*n_ptr) return h; // Needle found
        h++;
    }
    return NULL; // Needle not found
}

char* custom_strncpy(char* dest, const char* src, size_t n) {
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; ++i) {
        dest[i] = src[i];
    }
    for (; i < n; ++i) {
        dest[i] = '\0';
    }
    // Ensure null termination if n > 0. This is crucial if src is longer than n.
    // The original custom_strncpy could cause issues if n was 0, but size_t n is usually positive.
    // However, if n-1 wraps around for n=0, it's a problem. For typical use (MAX_LEN), it's fine.
    // Let's add an explicit check to prevent writing beyond bounds if n could be 0.
    if (n > 0) {
        dest[n - 1] = '\0';
    }
    return dest;
}

// --- Data Structures ---
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

// Subscription struct definition, now after PackageType enum
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

struct PurchaseReportRecord { // Separate struct for report to easily pass to sorting functions
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

// --- Hash Table Implementation (using new/delete) ---
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

    // Using new instead of malloc
    struct AccountNode* newNode = new (std::nothrow) AccountNode; // Using nothrow to handle allocation failure
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
    FILE* outFile = NULL;
    try {
        outFile = fopen(filename, "w");
        if (!outFile) {
            throw FileException("Error opening file for saving hash table!");
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
    } catch (const FileException& e) {
        cout << "\tCaught exception: " << e.what() << endl;
        if (outFile) fclose(outFile);
    } catch (...) {
        cout << "\tCaught unknown exception during hash table save!" << endl;
        if (outFile) fclose(outFile);
    }
}

void loadHashTableFromFile(struct AccountNode* hashTable[], int size, const char* filename) {
    FILE* inFile = NULL;
    try {
        inFile = fopen(filename, "r");
        if (!inFile) {
            // If file doesn't exist, create it and return. No error if it's the first run.
            FILE* newFile = fopen(filename, "w");
            if (newFile) {
                fclose(newFile);
                return;
            } else {
                throw FileException("Error creating new hash table file!");
            }
        }

        char username[MAX_NAME_LEN];
        char password[MAX_PASSWORD_LEN];
        char email[MAX_EMAIL_LEN];
        int type_int;
        long long expiry;
        int songsListened;

        // Make sure to clear existing hash table data if this function is called multiple times without freeing.
        // For a typical load-at-startup scenario, initHashTable would have been called already.
        // It's safer to clear it if this function could be called mid-program to reload.
        // For this code structure (called once in constructor), it's okay.

        while (fscanf(inFile, "%s %s %s %d %lld %d\n", username, password, email, &type_int, &expiry, &songsListened) == 6) {
            hashTableInsert(hashTable, size, username, password, email, (PackageType)type_int, expiry, songsListened);
        }
        fclose(inFile);
    } catch (const FileException& e) {
        cout << "\tCaught exception: " << e.what() << endl;
        if (inFile) fclose(inFile);
    } catch (...) {
        cout << "\tCaught unknown exception during hash table load!" << endl;
        if (inFile) fclose(inFile);
    }
}

void freeHashTable(struct AccountNode* hashTable[], int size) {
    for (int i = 0; i < size; i++) {
        struct AccountNode* current = hashTable[i];
        while (current != NULL) {
            struct AccountNode* temp = current;
            current = current->next;
            // Using delete instead of free
            delete temp;
        }
        hashTable[i] = NULL;
    }
}

// --- Base Class 1 ---
class UserBase {
protected:
    char username[MAX_NAME_LEN];
    char password[MAX_PASSWORD_LEN];
    char email[MAX_EMAIL_LEN];

public:
    virtual void login() = 0;
    virtual ~UserBase() {} // Virtual destructor

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
        char* dotComPos = custom_strstr(email, ".com"); // Using custom_strstr for consistency
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

// Derived Class 1 (from UserBase)
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
        // cout << "Admin accounts memory released." << endl; // Removed to avoid duplicate message at program end
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

    void login() /*override*/ { // Overridden function 1
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
                Sleep(1500); // Changed to 1.5 seconds

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

// --- Base Class 2 ---
class ContentStorage {
protected:
    char genres[MAX_GENRES][MAX_NAME_LEN];
    int genreCount;
    char languages[MAX_LANGUAGES][MAX_NAME_LEN];
    int languageCount;
    Song recommendedSongs[MAX_RECOMMENDED_SONGS];
    int recommendedSongCount;
    Album purchasableAlbums[MAX_ALBUMS];
    int purchasableAlbumCount;

public:
    ContentStorage() {
        genreCount = 0;
        languageCount = 0;
        recommendedSongCount = 0;
        purchasableAlbumCount = 0;
    }
    virtual ~ContentStorage() {} // Virtual destructor for proper cleanup

    // Pure virtual functions for loading/saving data
    virtual void loadData() = 0;
    virtual void saveData() = 0;

    // Getters for content info (for external access, e.g., friend functions)
    const char* getGenre(int index) const {
        if (index >= 0 && index < genreCount) return genres[index];
        return "";
    }
    int getGenreCount() const { return genreCount; }

    const char* getLanguage(int index) const {
        if (index >= 0 && index < languageCount) return languages[index];
        return "";
    }
    int getLanguageCount() const { return languageCount; }

    const Song* getRecommendedSongs() const { return recommendedSongs; }
    int getRecommendedSongCount() const { return recommendedSongCount; }

    const Album* getPurchasableAlbums() const { return purchasableAlbums; }
    int getPurchasableAlbumCount() const { return purchasableAlbumCount; }
};

// Derived Class 2 (from ContentStorage)
class Admin : public ContentStorage {
public:
    Admin() : ContentStorage() {
        loadData(); // Load all admin data on construction
        // Set default data if no data was loaded from files
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
        if (recommendedSongCount == 0) {
            addDefaultRecommendedSongs();
        }
        if (purchasableAlbumCount == 0) {
            addDefaultPurchasableAlbums();
        }
    }

    ~Admin() {
        saveData(); // Save all admin data on destruction
        // cout << "Admin content data saved." << endl; // Removed to avoid duplicate message at program end
    }

    void loadData() /*override*/ { // Overridden function (from ContentStorage)
        loadGenresLanguages();
        loadRecommendedSongs();
        loadPurchasableAlbums();
    }

    void saveData() /*override*/ { // Overridden function (from ContentStorage)
        saveRecommendedSongs();
        savePurchasableAlbums();
        saveGenresLanguages();
    }

    void saveGenresLanguages() {
        FILE* file = NULL;
        try {
            file = fopen("admin_data.txt", "w");
            if (!file) {
                throw FileException("Error opening file for saving admin data!");
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
        } catch (const FileException& e) {
            cout << "\tCaught exception during saveGenresLanguages: " << e.what() << endl;
            if (file) fclose(file);
        } catch (...) {
            cout << "\tCaught unknown exception during saving genres/languages!" << endl;
            if (file) fclose(file);
        }
    }

    void loadGenresLanguages() {
        FILE* file = NULL;
        try {
            file = fopen("admin_data.txt", "r");
            if (!file) {
                return; // File might not exist yet, not an error on first run
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
        } catch (const FileException& e) {
            cout << "\tCaught exception during loadGenresLanguages: " << e.what() << endl;
            if (file) fclose(file);
        } catch (...) {
            cout << "\tCaught unknown exception during loading genres/languages!" << endl;
            if (file) fclose(file);
        }
    }

    void saveRecommendedSongs() {
        FILE* file = NULL;
        try {
            file = fopen("recommended_songs.txt", "w");
            if (!file) {
                throw FileException("Error opening file for saving recommended songs!");
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
        } catch (const FileException& e) {
            cout << "\tCaught exception during saveRecommendedSongs: " << e.what() << endl;
            if (file) fclose(file);
        } catch (...) {
            cout << "\tCaught unknown exception during saving recommended songs!" << endl;
            if (file) fclose(file);
        }
    }

    void loadRecommendedSongs() {
        FILE* file = NULL;
        try {
            file = fopen("recommended_songs.txt", "r");
            if (!file) {
                return; // File might not exist yet
            }
            char line[256];
            recommendedSongCount = 0;
            while (fgets(line, sizeof(line), file) != NULL && recommendedSongCount < MAX_RECOMMENDED_SONGS) {
                line[strcspn(line, "\n")] = 0;

                char tempLine[256]; // strtok modifies string, so copy it
                strcpy(tempLine, line);

                char* token = strtok(tempLine, ",");
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
        } catch (const FileException& e) {
            cout << "\tCaught exception during loadRecommendedSongs: " << e.what() << endl;
            if (file) fclose(file);
        } catch (...) {
            cout << "\tCaught unknown exception during loading recommended songs!" << endl;
            if (file) fclose(file);
        }
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
        FILE* file = NULL;
        try {
            file = fopen("albums.txt", "w");
            if (!file) {
                throw FileException("Error opening file for saving purchasable albums!");
            }
            for (int i = 0; i < purchasableAlbumCount; ++i) {
                // First write album metadata
                fprintf(file, "%s,%s,%s,%s,%.2f,%d\n",
                        purchasableAlbums[i].albumName,
                        purchasableAlbums[i].artist,
                        purchasableAlbums[i].genre,
                        purchasableAlbums[i].language,
                        purchasableAlbums[i].price,
                        purchasableAlbums[i].soldCount);

                // Then write each song in the album, prefixed with "SONG:"
                for (int j = 0; j < purchasableAlbums[i].songCount; ++j) {
                	fprintf(file, "SONG:%s,%s,%s,%s\n",
                        purchasableAlbums[i].songs[j].songName,
                        purchasableAlbums[i].songs[j].artist,
                        purchasableAlbums[i].songs[j].genre,
                        purchasableAlbums[i].songs[j].language);
            	}
            }
        fclose(file);
    } catch (const FileException& e) {
        cout << "\tCaught exception during savePurchasableAlbums: " << e.what() << endl;
        if (file) fclose(file);
    } catch (...) {
        cout << "\tCaught unknown exception during saving purchasable albums!" << endl;
        if (file) fclose(file);
    }
    }

    void loadPurchasableAlbums() {
        FILE* file = NULL;
        try {
            file = fopen("albums.txt", "r");
            if (!file) {
                return; // File might not exist yet
            }
            char line[512];
            purchasableAlbumCount = 0;

            while (fgets(line, sizeof(line), file) != NULL) {
                line[strcspn(line, "\n")] = 0;

                if (strncmp(line, "SONG:", 5) == 0) {
                    if (purchasableAlbumCount > 0) { // Only add if there's an album to add to
                        char* song_data = line + 5;
                        Song newSong;
                        newSong.favoriteCount = 0;

                        char tempSongData[512];
                        strcpy(tempSongData, song_data);

                        char* token = strtok(tempSongData, ",");
                        if (token) custom_strncpy(newSong.songName, token, MAX_NAME_LEN); else continue;
                        token = strtok(NULL, ",");
                        if (token) custom_strncpy(newSong.artist, token, MAX_NAME_LEN); else continue;
                        token = strtok(NULL, ",");
                        if (token) custom_strncpy(newSong.genre, token, MAX_NAME_LEN); else continue;
                        token = strtok(NULL, ",");
                        if (token) custom_strncpy(newSong.language, token, MAX_NAME_LEN); else continue;

                        if (purchasableAlbums[purchasableAlbumCount - 1].songCount < MAX_SONGS) {
                            purchasableAlbums[purchasableAlbumCount - 1].songs[purchasableAlbums[purchasableAlbumCount - 1].songCount++] = newSong;
                        }
                    }
                } else {
                    // This line represents a new album header
                    if (purchasableAlbumCount >= MAX_ALBUMS) {
                        break; // Max albums reached
                    }

                    Album newAlbum; // Create a new Album object
                    char tempLine[512];
                    strcpy(tempLine, line);

                    char* token = strtok(tempLine, ",");
                    if (token) custom_strncpy(newAlbum.albumName, token, MAX_NAME_LEN); else continue;
                    token = strtok(NULL, ",");
                    if (token) custom_strncpy(newAlbum.artist, token, MAX_NAME_LEN); else continue;
                    token = strtok(NULL, ",");
                    if (token) custom_strncpy(newAlbum.genre, token, MAX_NAME_LEN); else continue;
                    token = strtok(NULL, ",");
                    if (token) custom_strncpy(newAlbum.language, token, MAX_NAME_LEN); else continue;
                    token = strtok(NULL, ",");
                    if (token) newAlbum.price = atof(token); else continue;
                    token = strtok(NULL, ",");
                    if (token) newAlbum.soldCount = atoi(token); else continue;

                    newAlbum.songCount = 0; // Reset song count for the new album
                    purchasableAlbums[purchasableAlbumCount++] = newAlbum;
                }
            }
            fclose(file);
        } catch (const FileException& e) {
            cout << "\tCaught exception during loadPurchasableAlbums: " << e.what() << endl;
            if (file) fclose(file);
        } catch (...) {
            cout << "\tCaught unknown exception during loading purchasable albums!" << endl;
            if (file) fclose(file);
        }
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

    // --- Merge Sort for Songs (by Name or Artist) ---
    void mergeSongs(Song arr[], int l, int m, int r, int sortChoice) {
        int i, j, k;
        int n1 = m - l + 1;
        int n2 = r - m;

        // Dynamically allocate temporary arrays
        Song* L = new (std::nothrow) Song[n1];
        Song* R = new (std::nothrow) Song[n2];

        if (!L || !R) {
            cout << "\tMemory allocation failed during merge operation!" << endl;
            if (L) delete[] L;
            if (R) delete[] R;
            return;
        }

        for (i = 0; i < n1; i++) L[i] = arr[l + i];
        for (j = 0; j < n2; j++) R[j] = arr[m + 1 + j];

        i = 0; j = 0; k = l; // Initial indexes of first, second, and merged subarrays
        while (i < n1 && j < n2) {
            int cmp;
            if (sortChoice == 1) { // Sort by Song Name
                cmp = strcmp(L[i].songName, R[j].songName);
            } else { // Sort by Artist Name
                cmp = strcmp(L[i].artist, R[j].artist);
            }

            if (cmp <= 0) {
                arr[k] = L[i];
                i++;
            } else {
                arr[k] = R[j];
                j++;
            }
            k++;
        }

        while (i < n1) { // Copy remaining elements of L[]
            arr[k] = L[i];
            i++; k++;
        }
        while (j < n2) { // Copy remaining elements of R[]
            arr[k] = R[j];
            j++; k++;
        }

        delete[] L; // Free dynamically allocated memory
        delete[] R;
    }

    void mergeSortSongs(Song arr[], int l, int r, int sortChoice) { // Recursive function
        if (l < r) {
            int m = l + (r - l) / 2; // Avoid overflow for large l and r
            mergeSortSongs(arr, l, m, sortChoice);
            mergeSortSongs(arr, m + 1, r, sortChoice);
            mergeSongs(arr, l, m, r, sortChoice);
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
            // Using fgets for safer string input
            if (fgets(keyword, MAX_NAME_LEN, stdin) == NULL) {
                cout << "\tInvalid input! Please try again." << endl;
                continue;
            }
            keyword[strcspn(keyword, "\n")] = 0; // Remove trailing newline

            cout << "\n\tSearch Results (Linear Search):\n"; // Indicate search type
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
                while(getchar() != '\n'); // Clear buffer
                strcpy(choice_input, "yes"); // Default to yes to re-prompt
            }
            while(getchar() != '\n'); // Clear buffer
            for (int i = 0; choice_input[i] != '\0'; i++) {
                choice_input[i] = tolower(choice_input[i]);
            }

        } while (strcmp(choice_input, "yes") == 0);

        cout << "\n\tReturning to the admin menu..." << endl;
        Sleep(700);
        system("cls");
    }

    // --- Binary Search for Recommended Songs ---
    int binarySearchRecommendedSong(const char* targetSongName) {
        // Ensure array is sorted by song name for binary search
        // This is crucial. If the array is modified and not resorted, binary search will fail.
        // It's called here for direct use, but if the main display also sorts, it might be redundant.
        // It's safer to sort explicitly before binary search.
        mergeSortSongs(recommendedSongs, 0, recommendedSongCount - 1, 1); // 1 for sort by song name

        int low = 0;
        int high = recommendedSongCount - 1;

        while (low <= high) {
            int mid = low + (high - low) / 2;
            int cmp = strcmp(recommendedSongs[mid].songName, targetSongName);

            if (cmp == 0) {
                return mid; // Found
            } else if (cmp < 0) {
                low = mid + 1;
            } else {
                high = mid - 1;
            }
        }
        return -1; // Not found
    }

    void searchRecommendedSongBinary() {
        char songName[MAX_NAME_LEN];
        system("cls");
        cout << "\n\t|-----------------------------------|\n";
        cout << "\t|   BINARY SEARCH RECOMMENDED SONG  |\n";
        cout << "\t|-----------------------------------|\n";

        cout << "\n\tEnter the exact song name to search: ";
        if (fgets(songName, MAX_NAME_LEN, stdin) == NULL) {
            cout << "\tInvalid input!" << endl;
            _getch(); system("cls"); return;
        }
        songName[strcspn(songName, "\n")] = 0;

        int index = binarySearchRecommendedSong(songName);

        if (index != -1) {
            cout << "\n\tSong found (Binary Search):\n";
            cout << "\t- Song: " << recommendedSongs[index].songName
                 << ", Artist: " << recommendedSongs[index].artist
                 << ", Genre: " << recommendedSongs[index].genre
                 << ", Language: " << recommendedSongs[index].language
                 << ", Favorites: " << recommendedSongs[index].favoriteCount << endl;
        } else {
            cout << "\n\tSong '" << songName << "' not found in recommendations." << endl;
        }
        cout << "\n\tPress ENTER to return to admin menu..." << endl;
        _getch(); system("cls");
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
        cout << "\n\tRecommended Songs (Sorted by Name using Merge Sort):\n";
        mergeSortSongs(recommendedSongs, 0, recommendedSongCount - 1, 1); // Sort by Song Name

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
            if (fgets(newSong.songName, MAX_NAME_LEN, stdin) == NULL) { cout << "\tInvalid input!" << endl; _getch(); return; }
            newSong.songName[strcspn(newSong.songName, "\n")] = 0;

            cout << "\tEnter artist: ";
            if (fgets(newSong.artist, MAX_NAME_LEN, stdin) == NULL) { cout << "\tInvalid input!" << endl; _getch(); return; }
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
            saveGenresLanguages(); // Save changes
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
            saveGenresLanguages(); // Save changes
            cout << "\n\tLanguage deleted successfully!" << endl;
        } else {
            cout << "\n\tInvalid choice!" << endl;
        }

        Sleep(900);
        system("cls");
    }

    // --- Admin Edit/Update Functions ---
    void editGenre() {
        system("cls");
        cout << "\n\t+-----------------------------------+\n";
        cout << "\t|           EDIT GENRE              |\n";
        cout << "\t+-----------------------------------+\n";

        if (genreCount == 0) {
            cout << "\n\tNo genres available to edit!" << endl;
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
        cout << "\n\tEnter the number of the genre to edit: ";
        if (scanf("%d", &choice) != 1) {
            cout << "\tInvalid input!" << endl;
            while(getchar() != '\n');
            Sleep(900); system("cls"); return;
        }
        while(getchar() != '\n');

        if (choice > 0 && choice <= genreCount) {
            char newName[MAX_NAME_LEN];
            cout << "\tEnter new name for genre '" << genres[choice - 1] << "': ";
            if (fgets(newName, MAX_NAME_LEN, stdin) == NULL) {
                cout << "\tInvalid input!" << endl;
                _getch(); system("cls"); return;
            }
            newName[strcspn(newName, "\n")] = 0;
            strcpy(genres[choice - 1], newName);
            saveGenresLanguages(); // Save changes
            cout << "\n\tGenre updated successfully!" << endl;
        } else {
            cout << "\n\tInvalid choice!" << endl;
        }
        Sleep(900); system("cls");
    }

    void editLanguage() {
        system("cls");
        cout << "\n\t+-----------------------------------+\n";
        cout << "\t|          EDIT LANGUAGE            |\n";
        cout << "\t+-----------------------------------+\n";

        if (languageCount == 0) {
            cout << "\n\tNo languages available to edit!" << endl;
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
        cout << "\n\tEnter the number of the language to edit: ";
        if (scanf("%d", &choice) != 1) {
            cout << "\tInvalid input!" << endl;
            while(getchar() != '\n');
            Sleep(900); system("cls"); return;
        }
        while(getchar() != '\n');

        if (choice > 0 && choice <= languageCount) {
            char newName[MAX_NAME_LEN];
            cout << "\tEnter new name for language '" << languages[choice - 1] << "': ";
            if (fgets(newName, MAX_NAME_LEN, stdin) == NULL) {
                cout << "\tInvalid input!" << endl;
                _getch(); system("cls"); return;
            }
            newName[strcspn(newName, "\n")] = 0;
            strcpy(languages[choice - 1], newName);
            saveGenresLanguages(); // Save changes
            cout << "\n\tLanguage updated successfully!" << endl;
        } else {
            cout << "\n\tInvalid choice!" << endl;
        }
        Sleep(900); system("cls");
    }

    void editAlbum() {
        system("cls");
        cout << "\n\t+-----------------------------------+\n";
        cout << "\t|            EDIT ALBUM             |\n";
        cout << "\t+-----------------------------------+\n";

        if (purchasableAlbumCount == 0) {
            cout << "\n\tNo albums available to edit!" << endl;
            cout << "\n\tPress ENTER to return to admin menu..." << endl;
            _getch(); system("cls"); return;
        }

        cout << "\n\tAvailable Albums:\n";
        for (int i = 0; i < purchasableAlbumCount; i++) {
            cout << "\t" << (i + 1) << ". Album: " << purchasableAlbums[i].albumName
                 << " by " << purchasableAlbums[i].artist << endl;
        }

        int choice;
        cout << "\n\tEnter the number of the album to edit: ";
        if (scanf("%d", &choice) != 1) {
            cout << "\tInvalid input!" << endl;
            while(getchar() != '\n');
            Sleep(900); system("cls"); return;
        }
        while(getchar() != '\n');

        if (choice > 0 && choice <= purchasableAlbumCount) {
            Album* albumToEdit = &purchasableAlbums[choice - 1];
            char newName[MAX_NAME_LEN];
            double newPrice;

            cout << "\n\tEditing album: " << albumToEdit->albumName << endl;
            cout << "\tEnter new album name (current: " << albumToEdit->albumName << "): ";
            if (fgets(newName, MAX_NAME_LEN, stdin) == NULL) { cout << "\tInvalid input!" << endl; _getch(); return; }
            newName[strcspn(newName, "\n")] = 0;
            if (strlen(newName) > 0) strcpy(albumToEdit->albumName, newName);

            cout << "\tEnter new artist (current: " << albumToEdit->artist << "): ";
            if (fgets(newName, MAX_NAME_LEN, stdin) == NULL) { cout << "\tInvalid input!" << endl; _getch(); return; }
            newName[strcspn(newName, "\n")] = 0;
            if (strlen(newName) > 0) strcpy(albumToEdit->artist, newName);

            // Allow changing genre/language if needed (similar to adding song)
            int genreChoice;
            cout << "\tSelect new primary genre (current: " << albumToEdit->genre << "). (";
            for (int i = 0; i < genreCount; i++) { cout << (i + 1) << ". " << genres[i] << " "; }
            cout << ") or 0 to keep current: ";
            if (scanf("%d", &genreChoice) == 1 && genreChoice > 0 && genreChoice <= genreCount) {
                strcpy(albumToEdit->genre, genres[genreChoice - 1]);
            } else if (genreChoice != 0) { cout << "\tInvalid genre choice, keeping current." << endl; }
            while(getchar() != '\n');

            int languageChoice;
            cout << "\tSelect new primary language (current: " << albumToEdit->language << "). (";
            for (int i = 0; i < languageCount; i++) { cout << (i + 1) << ". " << languages[i] << " "; }
            cout << ") or 0 to keep current: ";
            if (scanf("%d", &languageChoice) == 1 && languageChoice > 0 && languageChoice <= languageCount) {
                strcpy(albumToEdit->language, languages[languageChoice - 1]);
            } else if (languageChoice != 0) { cout << "\tInvalid language choice, keeping current." << endl; }
            while(getchar() != '\n');

            cout << "\tEnter new album price (current: $" << fixed << setprecision(2) << albumToEdit->price << "): $";
            if (scanf("%lf", &newPrice) == 1 && newPrice >= 0) {
                albumToEdit->price = newPrice;
            } else {
                cout << "\tInvalid price, keeping current." << endl;
            }
            while(getchar() != '\n');

            savePurchasableAlbums(); // Save changes
            cout << "\n\tAlbum updated successfully!" << endl;
        } else {
            cout << "\n\tInvalid choice!" << endl;
        }
        Sleep(900); system("cls");
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
        if (fgets(newAlbum.albumName, MAX_NAME_LEN, stdin) == NULL) { cout << "\tInvalid input!" << endl; _getch(); return; }
        newAlbum.albumName[strcspn(newAlbum.albumName, "\n")] = 0;

        cout << "\tEnter artist: ";
        if (fgets(newAlbum.artist, MAX_NAME_LEN, stdin) == NULL) { cout << "\tInvalid input!" << endl; _getch(); return; }
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

            // Use album's artist, genre, and language for simplicity for album songs
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

    // Friend functions for Admin module
    friend void generatePurchaseReport(Admin& admin_ref); // Friend Function 3
    friend void searchPurchaseRecords(Admin& admin_ref);  // Friend Function 4
};

// --- Base Class 3 (for media playback) ---
class MediaPlayer {
public:
    virtual void playSong(const Song* song) = 0;
    virtual void pause() = 0;
    virtual void resume() = 0;
    virtual ~MediaPlayer() {}
};

// Derived Class 3 (from MediaPlayer)
class PlaylistPlayer : public MediaPlayer {
private:
    long long currentUnixTime() {
        return (long long)time(NULL);
    }

public:
    void playSong(const Song* song) /*override*/ { // Overridden function (from MediaPlayer)
        cout << "\n\tNow Playing: " << song->songName << " by " << song->artist
             << " | Genre: " << song->genre << " | Language: " << song->language << endl;
    }
    void pause() /*override*/ { // Overridden function (from MediaPlayer)
        cout << "\n\tPlayback Paused." << endl;
    }
    void resume() /*override*/ { // Overridden function (from MediaPlayer)
        cout << "\n\tPlayback resumed." << endl;
    }
    ~PlaylistPlayer() {} // Destructor for PlaylistPlayer
};


class User {
public:
    Playlist playlists[MAX_PLAYLISTS];
    int playlistCount;
    char currentUsername[MAX_NAME_LEN];

    User(); // Constructor
    ~User(); // Destructor

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

    void juztodisplaydesign(); // Renamed to displayPlayerUI for clarity, but kept for main menu.

    void playPlaylist(UserLogin* userLogin);
    void sortSongsInPlaylist(); // New function for sorting songs in a playlist

    void generateUserReport(); // New function for user summary report

    void savePlaylists();
    void loadPlaylists();

    // Friend functions (already existed)
    friend void displayAllSongsInPlaylists(const User* user); // Friend Function 1
    friend void mergePlaylistsByGenreOrLanguage(User* user, Admin* admin_ptr); // Friend Function 2
};

User::User() {
    playlistCount = 0;
    strcpy(currentUsername, "");
}

User::~User() {
    // No dynamic memory allocated directly in User class, but it's good practice
    // to have a destructor for clarity and potential future use.
    // Playlists array will be destroyed automatically.
    // cout << "User object destroyed for " << currentUsername << "." << endl; // Removed to avoid duplicate message at program end
}


void User::savePlaylists() {
    char filename[MAX_NAME_LEN + 15];
    sprintf(filename, "%s_playlists.txt", currentUsername);

    FILE* file = NULL;
    try {
        file = fopen(filename, "w");
        if (!file) {
            throw FileException("Error: Could not save playlists for user!");
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
    } catch (const FileException& e) {
        cout << "\tCaught exception during savePlaylists: " << e.what() << endl;
        if (file) fclose(file);
    } catch (...) {
        cout << "\tCaught unknown exception during saving playlists!" << endl;
        if (file) fclose(file);
    }
}

void User::loadPlaylists() {
    char filename[MAX_NAME_LEN + 15];
    sprintf(filename, "%s_playlists.txt", currentUsername);

    FILE* file = NULL;
    try {
        file = fopen(filename, "r");
        if (!file) {
            return; // File might not exist yet, not an error on first run
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

                    char tempSongData[512];
                    strcpy(tempSongData, song_data);

                    char* token = strtok(tempSongData, ",");
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
    } catch (const FileException& e) {
        cout << "\tCaught exception during loadPlaylists: " << e.what() << endl;
        if (file) fclose(file);
    } catch (...) {
        cout << "\tCaught unknown exception during loading playlists!" << endl;
        if (file) fclose(file);
    }
}

// Derived Class 2 (from UserBase)
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
        // cout << "User accounts memory released." << endl; // Removed to avoid duplicate message at program end
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

    void login() /*override*/ { // Overridden function 2
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
                    Sleep(1500); // Changed to 1.5 seconds

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

    if (admin_ptr->getPurchasableAlbumCount() == 0) {
        cout << "\n\tNo digital albums available for purchase at the moment." << endl;
        cout << "\n\tPress ENTER to return to user menu..." << endl;
        _getch(); system("cls"); return;
    }

    cout << "\n\tAvailable Albums:\n";
    const Album* albums = admin_ptr->getPurchasableAlbums();
    for (int i = 0; i < admin_ptr->getPurchasableAlbumCount(); i++) {
        cout << "\t" << (i + 1) << ". Album: " << albums[i].albumName
             << " by " << albums[i].artist
             << " - Price: $" << fixed << setprecision(2) << albums[i].price << endl;
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
    if (choice < 1 || choice > admin_ptr->getPurchasableAlbumCount()) {
        cout << "\tInvalid album choice!" << endl;
        _getch(); system("cls"); return;
    }

    Album selectedAlbum = albums[choice - 1];

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
    FILE* file = NULL;
    try {
        file = fopen("payment_records.txt", "a");
        if (!file) {
            throw FileException("Error opening payment_records.txt for saving!");
        }
        fprintf(file, "%s,%s,%s,%s,%.2f,%s,%s,%lld,%s,%lld\n",
                record->transactionId, record->username, record->itemName, record->itemType, record->amount,
                record->paymentMethod, record->paymentDetails, record->timestamp, record->status, record->purchasedItemExpiry);
        fclose(file);
    } catch (const FileException& e) {
        cout << "\tCaught exception during savePaymentRecord: " << e.what() << endl;
        if (file) fclose(file);
    } catch (...) {
        cout << "\tCaught unknown exception during saving payment record!" << endl;
        if (file) fclose(file);
    }
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
            for (int i = 0; i < admin_ptr->getGenreCount(); i++) {
                cout << (i + 1) << ". " << admin_ptr->getGenre(i) << " ";
            }
            cout << "): ";
            if (scanf("%d", &genreChoice) != 1) {
                cout << "\tInvalid choice. Please select a valid genre number!" << endl << endl;
                while(getchar() != '\n');
                validChoice = 0;
            } else if (genreChoice < 1 || genreChoice > admin_ptr->getGenreCount()) {
                cout << "\tInvalid choice. Please select a valid genre number!" << endl << endl;
                while(getchar() != '\n');
                validChoice = 0;
            } else {
                validChoice = 1;
            }
        } while (!validChoice);
        while(getchar() != '\n');
        strcpy(newSong.genre, admin_ptr->getGenre(genreChoice - 1));

        int languageChoice;
        do {
            cout << "\tSelect language (";
            for (int i = 0; i < admin_ptr->getLanguageCount(); i++) {
                cout << (i + 1) << ". " << admin_ptr->getLanguage(i) << " ";
            }
            cout << "): ";
            if (scanf("%d", &languageChoice) != 1) {
                cout << "\tInvalid choice. Please select a valid language number!" << endl << endl;
                while(getchar() != '\n');
                validChoice = 0;
            } else if (languageChoice < 1 || languageChoice > admin_ptr->getLanguageCount()) {
                cout << "\tInvalid choice. Please select a valid language number!" << endl << endl;
                while(getchar() != '\n');
                validChoice = 0;
            } else {
                validChoice = 1;
            }
        } while (!validChoice);
        while(getchar() != '\n');
        strcpy(newSong.language, admin_ptr->getLanguage(languageChoice - 1));

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

            const Song* recommended = admin_ptr->getRecommendedSongs();
            for (int i = 0; i < admin_ptr->getRecommendedSongCount(); i++) {
                if (strcmp(recommended[i].genre, newSong.genre) == 0 ||
                    strcmp(recommended[i].language, newSong.language) == 0) {
                    cout << "\t" << recommendationCounter << ". " << recommended[i].songName
                         << " by " << recommended[i].artist
                         << " [" << recommended[i].genre
                         << ", " << recommended[i].language
                         << "] - Favorites: " << recommended[i].favoriteCount << endl;
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
                            Song selectedSong = recommended[displayedIndexes[songChoice - 1]];

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
            for (int j = 0; j < admin_ptr->getGenreCount(); j++) {
                cout << (j + 1) << ". " << admin_ptr->getGenre(j) << " ";
            }
            cout << "): ";
            if (scanf("%d", &genreChoice) != 1) {
                cout << "\tInvalid choice! Please select a valid genre number." << endl << endl;
                while(getchar() != '\n');
            } else if (genreChoice < 1 || genreChoice > admin_ptr->getGenreCount()) {
                cout << "\tInvalid choice! Please select a valid genre number." << endl << endl;
                while(getchar() != '\n');
            } else {
                break;
            }
        }
        while(getchar() != '\n');

        strcpy(newSong.genre, admin_ptr->getGenre(genreChoice - 1));

        int languageChoice;
        while (1) {
            cout << "\tSelect language (";
            for (int j = 0; j < admin_ptr->getLanguageCount(); j++) {
                cout << (j + 1) << ". " << admin_ptr->getLanguage(j) << " ";
            }
            cout << "): ";
            if (scanf("%d", &languageChoice) != 1) {
                cout << "\tInvalid choice! Please select a valid language number." << endl << endl;
                while(getchar() != '\n');
            } else if (languageChoice < 1 || languageChoice > admin_ptr->getLanguageCount()) {
                cout << "\tInvalid choice! Please select a valid language number." << endl << endl;
                while(getchar() != '\n');
            } else {
                break;
            }
        }
        while(getchar() != '\n');

        strcpy(newSong.language, admin_ptr->getLanguage(languageChoice - 1));

        if (selectedPlaylist->songCount >= MAX_SONGS) {
            cout << "\tError: Maximum songs reached for this playlist! Cannot add more." << endl;
            break;
        }
        selectedPlaylist->songs[selectedPlaylist->songCount++] = newSong;

        cout << "\n\tSong added to playlist!" << endl;
        system("cls");
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

        FILE* file = NULL;
        try {
            file = fopen(filename, "r");
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

                char tempSongLine[4 * MAX_NAME_LEN];
                strcpy(tempSongLine, songLine); // Copy for strtok

                char* token;
                token = strtok(tempSongLine, ",");
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
        } catch (const FileException& e) {
            cout << "\tCaught exception during importPlaylist: " << e.what() << endl;
            if (file) fclose(file);
            cout << "\n\tPress ENTER to return to user menu..." << endl;
            _getch(); system("cls"); return;
        } catch (...) {
            cout << "\tCaught unknown exception during importing playlist!" << endl;
            if (file) fclose(file);
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

        FILE* checkFile = NULL;
        try {
            checkFile = fopen(fullFilename, "r");
            if (checkFile) {
                fclose(checkFile);
                cout << "\n\tError: A file with the same playlist name already exists!" << endl;
                cout << "\n\tPlease select another playlist." << endl;
                _getch(); continue;
            }
        } catch (const FileException& e) {
            cout << "\tCaught exception during file check for download: " << e.what() << endl;
        } catch (...) {
            cout << "\tCaught unknown exception during file check for download!" << endl;
        }

        FILE* file = NULL;
        try {
            file = fopen(fullFilename, "w");
            if (!file) {
                throw FileException("Error: Could not create or write to the file!");
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
        } catch (const FileException& e) {
            cout << "\tCaught exception during downloadPlaylist: " << e.what() << endl;
            if (file) fclose(file);
            cout << "\n\tPress ENTER to continue..." << endl;
            _getch();
        } catch (...) {
            cout << "\tCaught unknown exception during downloading playlist!" << endl;
            if (file) fclose(file);
            cout << "\n\tPress ENTER to continue..." << endl;
            _getch();
        }
    }
}

void User::juztodisplaydesign() { // Renamed for clarity but kept original function name as per request
    cout << "\n\t|-------------------------------|\n";
    cout << "\t|       PLAYING PLAYLIST...     |\n";
    cout << "\t|-------------------------------|\n";
}

void User::playPlaylist(UserLogin* userLogin) {
    system("cls");
    juztodisplaydesign(); // Display player UI

    PlaylistPlayer player; // Create a PlaylistPlayer object (derived from MediaPlayer)

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
            player.playSong(&selectedPlaylist->songs[currentSongIndex]); // Use the derived class's method

            if (userNode->subscription.type == FREE_TIER) {
                userNode->subscription.songsListened++;
                saveHashTableToFile((const struct AccountNode* const*)userLogin->userHashTable, HASH_TABLE_SIZE, userLogin->FILE_NAME);
                cout << "\t(Free Tier: " << userNode->subscription.songsListened << "/10 songs listened)" << endl;
                if (userNode->subscription.songsListened >= 10) {
                    cout << "\n\tFree tier limit reached after this song! You will not be able to play more songs." << endl;
                }
            }
        } else if (isPaused) {
            player.pause(); // Use the derived class's method
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
                player.resume(); // Use the derived class's method
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

void User::sortSongsInPlaylist() {
    system("cls");
    cout << "\n\t|-------------------------------|\n";
    cout << "\t|    SORT SONGS IN PLAYLIST     |\n";
    cout << "\t|-------------------------------|\n";

    if (playlistCount == 0) {
        cout << "\n\tNo playlists available to sort songs!" << endl;
        cout << "\n\tPress ENTER to return to user menu..." << endl;
        _getch(); system("cls"); return;
    }

    cout << "\n\tAvailable playlists:\n";
    for (int i = 0; i < playlistCount; i++) {
        cout << "\t" << (i + 1) << ". " << playlists[i].name << endl;
    }
    cout << "\n\t0. Go back to the user menu\n";

    int playlistChoice;
    while (1) {
        cout << "\n\tSelect the playlist number to sort songs (1-" << playlistCount << "): ";
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
        Sleep(900); system("cls"); return;
    }

    Playlist* selectedPlaylist = &playlists[playlistChoice - 1];

    if (selectedPlaylist->songCount <= 1) {
        cout << "\tPlaylist '" << selectedPlaylist->name << "' has too few songs to sort." << endl;
        cout << "\n\tPress ENTER to continue..." << endl;
        _getch(); system("cls"); return;
    }

    int sortChoice;
    while (1) {
        cout << "\n\tSort songs in '" << selectedPlaylist->name << "' by:\n";
        cout << "\t1. Song Name\n";
        cout << "\t2. Artist Name\n";
        cout << "\n\tEnter your choice: ";
        if (scanf("%d", &sortChoice) != 1) {
            cout << "\tInvalid choice! Please enter 1 or 2." << endl;
            while(getchar() != '\n');
        } else if (sortChoice != 1 && sortChoice != 2) {
            cout << "\tInvalid choice! Please enter 1 or 2." << endl;
            while(getchar() != '\n');
        } else {
            break;
        }
    }
    while(getchar() != '\n');

    // START - Copied Merge Sort Implementation for User
    class UserSongSorter { // Helper class to encapsulate merge sort logic
    public:
        void merge(Song arr[], int l, int m, int r, int sortChoice) {
            int i, j, k;
            int n1 = m - l + 1;
            int n2 = r - m;

            // Dynamically allocate temporary arrays for larger sizes or to be safe
            Song* L = new (std::nothrow) Song[n1];
            Song* R = new (std::nothrow) Song[n2];

            if (!L || !R) {
                cout << "\tMemory allocation failed during merge operation!" << endl;
                if (L) delete[] L;
                if (R) delete[] R;
                return;
            }

            for (i = 0; i < n1; i++) L[i] = arr[l + i];
            for (j = 0; j < n2; j++) R[j] = arr[m + 1 + j];

            i = 0; j = 0; k = l;
            while (i < n1 && j < n2) {
                int cmp;
                if (sortChoice == 1) { // Sort by Song Name
                    cmp = strcmp(L[i].songName, R[j].songName);
                } else { // Sort by Artist Name
                    cmp = strcmp(L[i].artist, R[j].artist);
                }

                if (cmp <= 0) {
                    arr[k] = L[i];
                    i++;
                } else {
                    arr[k] = R[j];
                    j++;
                }
                k++;
            }

            while (i < n1) {
                arr[k] = L[i];
                i++; k++;
            }
            while (j < n2) {
                arr[k] = R[j];
                j++; k++;
            }

            delete[] L; // Free dynamically allocated memory
            delete[] R;
        }

        void sort(Song arr[], int l, int r, int sortChoice) { // Renamed to sort to avoid conflict
            if (l < r) {
                int m = l + (r - l) / 2;
                sort(arr, l, m, sortChoice);
                sort(arr, m + 1, r, sortChoice);
                merge(arr, l, m, r, sortChoice);
            }
        }
    };
    // END - Copied Merge Sort Implementation for User

    UserSongSorter sorter;
    sorter.sort(selectedPlaylist->songs, 0, selectedPlaylist->songCount - 1, sortChoice);

    cout << "\n\tSongs in playlist '" << selectedPlaylist->name << "' sorted successfully!\n";
    cout << "\n\tSorted Songs:\n";
    for (int i = 0; i < selectedPlaylist->songCount; i++) {
        cout << "\t" << (i + 1) << ". " << selectedPlaylist->songs[i].songName
             << " by " << selectedPlaylist->songs[i].artist << endl;
    }

    cout << "\n\tPress ENTER to return to user menu..." << endl;
    _getch(); system("cls");
}

void User::generateUserReport() {
    system("cls");
    cout << "\n\t+-----------------------------------+\n";
    cout << "\t|        USER PLAYLIST REPORT       |\n";
    cout << "\t+-----------------------------------+\n";

    char reportFilename[MAX_NAME_LEN + 20];
    sprintf(reportFilename, "%s_user_report.txt", currentUsername);

    FILE* file = NULL;
    try {
        file = fopen(reportFilename, "w");
        if (!file) {
            throw FileException("Error opening file for user report!");
        }

        fprintf(file, "User Report for: %s\n", currentUsername);

        // FIX: Correct usage of localtime and avoid memory leak
        time_t currentTime = getCurrentUnixTime();
        struct tm* dt = localtime(&currentTime);
        char date_str[50];
        strftime(date_str, sizeof(date_str), "%Y-%m-%d %H:%M:%S", dt);
        fprintf(file, "Date Generated: %s\n", date_str);

        if (playlistCount == 0) {
            fprintf(file, "\nNo playlists found.\n");
            cout << "\n\tNo playlists found to generate report." << endl;
        } else {
            cout << "\n\tGenerating report for " << currentUsername << "...\n";
            fprintf(file, "\nTotal Playlists: %d\n", playlistCount);
            int totalSongs = 0;

            for (int i = 0; i < playlistCount; i++) {
                fprintf(file, "\n--- Playlist: %s (Songs: %d) ---\n", playlists[i].name, playlists[i].songCount);
                for (int j = 0; j < playlists[i].songCount; j++) {
                    const Song* song = &playlists[i].songs[j];
                    fprintf(file, "  - Song: %s, Artist: %s, Genre: %s, Language: %s\n",
                            song->songName, song->artist, song->genre, song->language);
                    totalSongs++;
                }
            }
            fprintf(file, "\nTotal Songs Across All Playlists: %d\n", totalSongs);
            cout << "\n\tReport generated successfully to '" << reportFilename << "'." << endl;
        }
        fclose(file);
    } catch (const FileException& e) {
        cout << "\tCaught exception during generateUserReport: " << e.what() << endl;
        if (file) fclose(file);
    } catch (...) {
        cout << "\tCaught unknown exception during generating user report!" << endl;
        if (file) fclose(file);
    }

    cout << "\n\tPress ENTER to return to user menu..." << endl;
    _getch(); system("cls");
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

void mergePlaylistsByGenreOrLanguage(User* user, Admin* admin_ptr) { // Friend Function 2
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
        for (int i = 0; i < admin_ptr->getGenreCount(); i++) {
            cout << "\t" << (i + 1) << ". " << admin_ptr->getGenre(i) << endl;
        }
    } else {
        cout << "\n\tAvailable Languages:\n";
        for (int i = 0; i < admin_ptr->getLanguageCount(); i++) {
            cout << "\t" << (i + 1) << ". " << admin_ptr->getLanguage(i) << endl;
        }
    }

    int selectedOptionIndex;
    while (1) {
        cout << "\n\tEnter the number of your choice: ";
        if (scanf("%d", &selectedOptionIndex) != 1) {
            cout << "\tInvalid input! Please select a valid number." << endl;
            while(getchar() != '\n');
        } else if ((choice == 1 && (selectedOptionIndex < 1 || selectedOptionIndex > admin_ptr->getGenreCount())) ||
                   (choice == 2 && (selectedOptionIndex < 1 || selectedOptionIndex > admin_ptr->getLanguageCount()))) {
            cout << "\tInvalid input! Please select a valid number." << endl;
            while(getchar() != '\n');
        } else {
            break;
        }
    }
    while(getchar() != '\n');

    char selectedOptionName[MAX_NAME_LEN];
    if (choice == 1) { strcpy(selectedOptionName, admin_ptr->getGenre(selectedOptionIndex - 1)); }
    else { strcpy(selectedOptionName, admin_ptr->getLanguage(selectedOptionIndex - 1)); }

    Playlist mergedPlaylist;
    mergedPlaylist.songCount = 0;

    for (int i = 0; i < user->playlistCount; i++) {
        for (int j = 0; j < user->playlists[i].songCount; j++) {
            if ((choice == 1 && strcmp(user->playlists[i].songs[j].genre, selectedOptionName) == 0) ||
                (choice == 2 && strcmp(user->playlists[i].songs[j].language, selectedOptionName) == 0)) {

                int isDuplicate = 0;
                for(int k=0; k < mergedPlaylist.songCount; ++k) {
                    if(strcmp(mergedPlaylist.songs[k].songName, user->playlists[i].songs[j].songName) == 0 &&
                       strcmp(mergedPlaylist.songs[k].artist, user->playlists[i].songs[j].artist) == 0) {
                        isDuplicate = 1;
                        break;
                    }
                }

                if(!isDuplicate) {
                    if (mergedPlaylist.songCount < MAX_SONGS) {
                        mergedPlaylist.songs[mergedPlaylist.songCount++] = user->playlists[i].songs[j];
                    } else {
                        cout << "\n\tWarning: Merged playlist reached maximum song limit. Not all songs were added." << endl;
                        break;
                    }
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

// --- Merge Sort for Purchase Records ---
void mergePurchaseRecords(PurchaseReportRecord arr[], int l, int m, int r, int sortChoice) {
    int i, j, k;
    int n1 = m - l + 1;
    int n2 = r - m;

    // Dynamically allocate temporary arrays
    PurchaseReportRecord* L = new (std::nothrow) PurchaseReportRecord[n1];
    PurchaseReportRecord* R = new (std::nothrow) PurchaseReportRecord[n2];

    if (!L || !R) {
        cout << "\tMemory allocation failed during merge operation for purchase records!" << endl;
        if (L) delete[] L;
        if (R) delete[] R;
        return;
    }

    for (i = 0; i < n1; i++) L[i] = arr[l + i];
    for (j = 0; j < n2; j++) R[j] = arr[m + 1 + j];

    i = 0; j = 0; k = l;
    while (i < n1 && j < n2) {
        int cmp = 0;
        if (sortChoice == 1) { // Sort by Username
            cmp = strcmp(L[i].username, R[j].username);
        } else if (sortChoice == 2) { // Sort by Amount (ascending)
            if (L[i].amount < R[j].amount) cmp = -1;
            else if (L[i].amount > R[j].amount) cmp = 1;
            else cmp = 0;
        }

        if (cmp <= 0) {
            arr[k] = L[i];
            i++;
        } else {
            arr[k] = R[j];
            j++;
        }
        k++;
    }

    while (i < n1) {
        arr[k] = L[i];
        i++; k++;
    }
    while (j < n2) {
        arr[k] = R[j];
        j++; k++;
    }

    delete[] L; // Free dynamically allocated memory
    delete[] R;
}

void mergeSortPurchaseRecords(PurchaseReportRecord arr[], int l, int r, int sortChoice) { // Recursive function
    if (l < r) {
        int m = l + (r - l) / 2;
        mergeSortPurchaseRecords(arr, l, m, sortChoice);
        mergeSortPurchaseRecords(arr, m + 1, r, sortChoice);
        mergePurchaseRecords(arr, l, m, r, sortChoice);
    }
}

// --- Binary Search for Purchase Records (by Username) ---
int binarySearchPurchaseRecords(const PurchaseReportRecord records[], int n, const char* targetUsername) {
    int low = 0;
    int high = n - 1;

    while (low <= high) {
        int mid = low + (high - low) / 2;
        int cmp = strcmp(records[mid].username, targetUsername);

        if (cmp == 0) {
            return mid; // Found
        } else if (cmp < 0) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }
    return -1; // Not found
}

void generatePurchaseReport(Admin& admin_ref) { // Friend Function 3
    system("cls");
    cout << "\n\t+-----------------------------------+\n";
    cout << "\t|       PURCHASE REPORT             |\n";
    cout << "\t+-----------------------------------+\n";

    FILE* file = NULL;
    PurchaseReportRecord records[100]; // Assuming max 100 records for simplicity. Consider dynamic array for real apps.
    int recordCount = 0;
    char line[512];

    try {
        file = fopen("payment_records.txt", "r");
        if (!file) {
            cout << "\n\tNo purchase records found!" << endl;
            cout << "\n\tPress ENTER to return to admin menu..." << endl;
            _getch(); system("cls"); return;
        }

        while (fgets(line, sizeof(line), file) != NULL && recordCount < 100) {
            line[strcspn(line, "\n")] = 0;

            char temp_line[512];
            strcpy(temp_line, line);

            char* token = strtok(temp_line, ",");
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
    } catch (const FileException& e) {
        cout << "\tCaught exception during generatePurchaseReport (load): " << e.what() << endl;
        if (file) fclose(file);
        cout << "\n\tPress ENTER to return to admin menu..." << endl;
        _getch(); system("cls"); return;
    } catch (...) {
        cout << "\tCaught unknown exception during loading purchase records for report!" << endl;
        if (file) fclose(file);
        cout << "\n\tPress ENTER to return to admin menu..." << endl;
        _getch(); system("cls"); return;
    }


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
    cout << "\t1. Sort by Username\n";
    cout << "\t2. Sort by Amount\n";
    cout << "\t3. Filter by Status (SUCCESS/FAILED)\n";
    cout << "\tEnter your choice: ";
    if (scanf("%d", &reportChoice) != 1) {
        cout << "\tInvalid input, displaying default sort (by Username)." << endl;
        while(getchar() != '\n');
        reportChoice = 1;
    }
    while(getchar() != '\n');

    if (reportChoice == 1) {
        mergeSortPurchaseRecords(records, 0, recordCount - 1, 1); // Sort by Username
    } else if (reportChoice == 2) {
        mergeSortPurchaseRecords(records, 0, recordCount - 1, 2); // Sort by Amount
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
        cout << "\tInvalid choice, displaying default sort (by Username)." << endl;
        mergeSortPurchaseRecords(records, 0, recordCount - 1, 1); // Default to Username
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

void searchPurchaseRecords(Admin& admin_ref) { // Friend Function 4
    char keyword[MAX_NAME_LEN];
    char choice_input[10];

    do {
        system("cls");
        cout << "\n\t+-----------------------------------+\n";
        cout << "\t|       SEARCH PURCHASE RECORDS     |\n";
        cout << "\t+-----------------------------------+\n";

        cout << "\n\t1. Search using Linear Search (keyword in any field)\n";
        cout << "\t2. Search using Binary Search (exact username match, requires sorting)\n";
        cout << "\t0. Back to Admin Menu\n"; // Added option to go back
        cout << "\tEnter your search method choice: ";
        int searchMethodChoice;
        if (scanf("%d", &searchMethodChoice) != 1) {
            cout << "\tInvalid choice. Defaulting to Linear Search." << endl;
            while(getchar() != '\n');
            searchMethodChoice = 1;
        } else if (searchMethodChoice == 0) { // Handle back option
            cout << "\n\tReturning to Admin menu..." << endl;
            Sleep(700);
            system("cls");
            return;
        } else if (searchMethodChoice != 1 && searchMethodChoice != 2) {
            cout << "\tInvalid choice. Defaulting to Linear Search." << endl;
            while(getchar() != '\n');
            searchMethodChoice = 1;
        }
        while(getchar() != '\n');

        FILE* file = NULL;
        PurchaseReportRecord records[100]; // Assuming max 100 records
        int recordCount = 0;
        char line[512];

        try {
            file = fopen("payment_records.txt", "r");
            if (!file) {
                cout << "\n\tNo purchase records found to search!" << endl;
                cout << "\n\tPress ENTER to return to admin menu..." << endl;
                _getch(); system("cls"); return;
            }

            while (fgets(line, sizeof(line), file) != NULL && recordCount < 100) {
                line[strcspn(line, "\n")] = 0;

                char temp_line[512];
                strcpy(temp_line, line);

                char* token = strtok(temp_line, ",");
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
        } catch (const FileException& e) {
            cout << "\tCaught exception during searchPurchaseRecords (load): " << e.what() << endl;
            if (file) fclose(file);
            cout << "\n\tPress ENTER to return to admin menu..." << endl;
            _getch(); system("cls"); return;
        } catch (...) {
            cout << "\tCaught unknown exception during loading purchase records for search!" << endl;
            if (file) fclose(file);
            cout << "\n\tPress ENTER to return to admin menu..." << endl;
            _getch(); system("cls"); return;
        }

        if (recordCount == 0) {
            cout << "\n\tNo purchase records to search!" << endl;
            cout << "\n\tPress ENTER to return to admin menu..." << endl;
            _getch(); system("cls"); return;
        }

        if (searchMethodChoice == 1) { // Linear Search
            cout << "\n\tEnter keyword to search (ID, username, item, method, status): ";
            if (fgets(keyword, MAX_NAME_LEN, stdin) == NULL) { cout << "\tInvalid input!" << endl; continue; }
            keyword[strcspn(keyword, "\n")] = 0;

            cout << "\n\tSearch Results (Linear Search):\n";
            int found_count = 0;
            for (int i = 0; i < recordCount; ++i) {
                if (custom_strstr(records[i].transactionId, keyword) != NULL ||
                    custom_strstr(records[i].username, keyword) != NULL ||
                    custom_strstr(records[i].itemName, keyword) != NULL ||
                    custom_strstr(records[i].itemType, keyword) != NULL ||
                    custom_strstr(records[i].paymentMethod, keyword) != NULL ||
                    custom_strstr(records[i].status, keyword) != NULL) {

                    time_t rawtime = records[i].timestamp;
                    struct tm* dt = localtime(&rawtime);
                    char date_str[50];
                    strftime(date_str, sizeof(date_str), "%Y-%m-%d %H:%M:%S", dt);

                    char expiry_str[50];
                    long long exp_val = records[i].purchasedItemExpiry;
                    if (exp_val == -1) { strcpy(expiry_str, "Permanent"); }
                    else if (exp_val == 0) { strcpy(expiry_str, "N/A (Free/Expired)"); }
                    else { time_t expiry_rawtime = exp_val; struct tm* expiry_dt = localtime(&expiry_rawtime); strftime(expiry_str, sizeof(expiry_str), "%Y-%m-%d %H:%M:%S", expiry_dt); }

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
                    found_count++;
                }
            }
            if (found_count == 0) { cout << "\n\tNo matching purchase records found!" << endl; }

        } else { // Binary Search
            mergeSortPurchaseRecords(records, 0, recordCount - 1, 1); // Sort by Username for binary search
            cout << "\n\tEnter exact username to search: ";
            if (fgets(keyword, MAX_NAME_LEN, stdin) == NULL) { cout << "\tInvalid input!" << endl; continue; }
            keyword[strcspn(keyword, "\n")] = 0;

            int index = binarySearchPurchaseRecords(records, recordCount, keyword);
            if (index != -1) {
                cout << "\n\tRecord found (Binary Search):\n";
                time_t rawtime = records[index].timestamp;
                struct tm* dt = localtime(&rawtime);
                char date_str[50];
                strftime(date_str, sizeof(date_str), "%Y-%m-%d %H:%M:%S", dt);

                char expiry_str[50];
                long long exp_val = records[index].purchasedItemExpiry;
                if (exp_val == -1) { strcpy(expiry_str, "Permanent"); }
                else if (exp_val == 0) { strcpy(expiry_str, "N/A (Free/Expired)"); }
                else { time_t expiry_rawtime = exp_val; struct tm* expiry_dt = localtime(&expiry_rawtime); strftime(expiry_str, sizeof(expiry_str), "%Y-%m-%d %H:%M:%S", expiry_dt); }

                cout << "\t--------------------------------------------------\n";
                cout << "\tTransaction ID: " << records[index].transactionId << endl;
                cout << "\tUsername: " << records[index].username << endl;
                cout << "\tItem: " << records[index].itemName << " (" << records[index].itemType << ")" << endl;
                cout << "\tAmount: $" << fixed << setprecision(2) << records[index].amount << endl;
                cout << "\tMethod: " << records[index].paymentMethod << endl;
                cout << "\tDetails: " << records[index].paymentDetails << endl;
                cout << "\tDate: " << date_str << endl;
                cout << "\tStatus: " << records[index].status << endl;
                cout << "\tExpiry: " << expiry_str << endl;
            } else {
                cout << "\n\tNo record found for username '" << keyword << "'." << endl;
            }
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

// --- Utility/Display Functions (global) ---
void SetColor(int color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

void PrintRainbowText(const char* text) {
    int colors[] = {12, 14, 10, 9, 13, 11}; // Red, Yellow, Green, Blue, Magenta, Cyan
    int len = strlen(text);
    for (int i = 0; i < len; ++i) {
        SetColor(colors[i % 6]);
        cout << text[i];
    }
    SetColor(7); // Reset to default white
}

void PrintRainbowEffectText(const char* text, int timer) {
    int colors[] = {12, 14, 10, 9, 13, 11}; // Red, Yellow, Green, Blue, Magenta, Cyan
    int len = strlen(text);
    for (int i = 0; i < len; ++i) {
        SetColor(colors[i % 6]);
        cout << text[i];
        Sleep(timer);
    }
    SetColor(7); // Reset to default white
}

void PrintItalicText(const char* text, int timer) {
    // This is a placeholder as true italic rendering is complex in console.
    // We can simulate with a slight delay or different color.
    // For now, just print normally with a delay.
    for (int i = 0; text[i] != '\0'; ++i) {
        cout << text[i];
        Sleep(timer);
    }
}

void PrintColoredText(const char* text, int color) {
    SetColor(color);
    cout << text;
    SetColor(7); // Reset to default white
}

void PrintwEffect(const char* text, int color, int timer) {
    SetColor(color);
    for (int i = 0; text[i] != '\0'; ++i) {
        cout << text[i];
        Sleep(timer);
    }
    SetColor(7); // Reset to default white
}

void PrintFlashingText(const char* text, int timer, int flashDuration) {
    for (int j = 0; j < flashDuration; ++j) {
        SetColor(12); // Red
        cout << text << "\r";
        Sleep(timer);
        SetColor(7); // White
        cout << text << "\r";
        Sleep(timer);
    }
    SetColor(7); // Ensure reset
    cout << text << endl; // Print finally and move to next line
}

void Effect(const char* text, int timer) {
    for (int i = 0; text[i] != '\0'; ++i) {
        cout << text[i];
        Sleep(timer);
    }
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
    Admin admin; // Admin object, now derived from ContentStorage
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
                        user.loadPlaylists();
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
                            cout << "\t|    12. Sort Songs in Playlist |\n"; // New option
                            cout << "\t|    13. Purchase Package       |\n";
                            cout << "\t|    14. Purchase Digital Album |\n";
                            cout << "\t|    15. Generate User Report   |\n"; // New option
                            cout << "\t|    16. Logout                 |\n"; // Option number changed
                            cout << "\t|                               |\n";
                            cout << "\t+++++++++++++++++++++++++++++++++\n";
                            cout << "\n\tEnter your choice: ";
                            if (scanf("%d", &choice) != 1) {
                                cout << "\tInvalid choice! Please select a valid option (1-16)." << endl;
                                while (getchar() != '\n');
                                Sleep(700); system("cls"); continue;
                            }
                            while (getchar() != '\n');

                            if (choice == 16) { // Logout option
                                user.savePlaylists();
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
                                        case 12: user.sortSongsInPlaylist(); break; // Call new sort function
                                        case 13: user.purchasePackage(&userLogin, &admin); break;
                                        case 14: user.purchaseDigitalAlbum(&userLogin, &admin); break;
                                        case 15: user.generateUserReport(); break; // Call new user report
                                        default: cout << "\n\tInvalid choice! Please select a valid option (1-16)." << endl; break;
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
                            cout << "\t!    7. Edit Genre                          !\n"; // New option
                            cout << "\t!    8. Edit Language                       !\n"; // New option
                            cout << "\t!    9. Edit Album                          !\n"; // New option
                            cout << "\t!    10. Display All Genres                 !\n";
                            cout << "\t!    11. Display All Languages              !\n";
                            cout << "\t!    12. Display All Recommended Songs      !\n";
                            cout << "\t!    13. Search Recommended Songs (Linear)  !\n"; // Renamed
                            cout << "\t!    14. Search Recommended Songs (Binary)  !\n"; // New option
                            cout << "\t!    15. Add Album for Sale                 !\n";
                            cout << "\t!    16. Display All Albums for Sale        !\n";
                            cout << "\t!    17. Generate Purchase Report           !\n";
                            cout << "\t!    18. Search Purchase Records            !\n";
                            cout << "\t!    19. Logout                             !\n";
                            cout << "\t!                                           !\n";
                            cout << "\t#############################################\n";
                            cout << "\n\tEnter your choice: ";
                            if (scanf("%d", &choice) != 1) {
                                cout << "\tInvalid choice! Please select a valid option (1-19)." << endl;
                                 while (getchar() != '\n');
                                 Sleep(900); system("cls"); continue;
                            }
                            while (getchar() != '\n');

                            if (choice == 19) { // Logout option
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
                                        case 7: admin.editGenre(); break; // Call new edit function
                                        case 8: admin.editLanguage(); break; // Call new edit function
                                        case 9: admin.editAlbum(); break; // Call new edit function
                                        case 10: admin.displayAllGenres(); break;
                                        case 11: admin.displayAllLanguages(); break;
                                        case 12: admin.displayAllRecommendedSongs(); break;
                                        case 13: admin.searchRecommendedSongsAdmin(); break; // Linear Search
                                        case 14: admin.searchRecommendedSongBinary(); break; // Binary Search
                                        case 15: admin.addAlbumToSale(); break;
                                        case 16: admin.displayAllPurchasableAlbums(); break;
                                        case 17: generatePurchaseReport(admin); break; // Pass admin object to friend
                                        case 18: searchPurchaseRecords(admin); break; // Pass admin object to friend
                                        default: cout << "\n\tInvalid choice! Please select a valid option (1-19)." << endl; break;
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

