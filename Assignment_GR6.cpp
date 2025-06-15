#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <limits>
#include <iomanip>
#include <stdio.h>
#include <windows.h>
#include <conio.h>
#include <sstream>

using namespace std;

// Define maximum capacities for various data types, crucial for fixed-size array implementations.
#define MAX_PLAYLISTS 10
#define MAX_NAME_LEN 50
#define MAX_SONGS 100
#define MAX_GENRES 10
#define MAX_LANGUAGES 8
#define MAX_RECOMMENDED_SONGS 30
#define MAX_EMAIL_LEN 100
#define MAX_PASSWORD_LEN 50
#define MAX_ALBUMS 30

// Define the size for the custom hash table, used for account management.
#define HASH_TABLE_SIZE 101

// Forward declarations for utility and display functions.
void laodingPage(int timer);
void juztodisplaydesign(); // Placeholder function for display design.

// Custom string search function (manual implementation for 'strstr').
// Performs a case-insensitive search for a substring within a larger string.
char* custom_strstr(const char* haystack, const char* needle) {
    if (!*needle) return (char*)haystack; // Empty needle means match at beginning.

    char* h = (char*)haystack;
    while (*h) { // Iterate through haystack.
        char* h_ptr = h;
        char* n_ptr = (char*)needle;

        // Compare characters case-insensitively until mismatch or end of needle.
        while (*n_ptr && *h_ptr && (tolower((unsigned char)*h_ptr) == tolower((unsigned char)*n_ptr))) {
            h_ptr++;
            n_ptr++;
        }
        if (!*n_ptr) return h; // Full needle found.
        h++; // Move to next character in haystack.
    }
    return NULL; // Needle not found.
}

// Custom string comparison function (manual implementation for 'strcmpi' or 'stricmp').
// Performs a case-insensitive comparison of two strings.
int custom_strcmpi(const char* s1, const char* s2) {
    while (*s1 && *s2) { // Iterate while both strings have characters.
        if (tolower((unsigned char)*s1) != tolower((unsigned char)*s2)) {
            // Return difference if characters don't match (case-insensitively).
            return tolower((unsigned char)*s1) - tolower((unsigned char)*s2);
        }
        s1++; // Move to next character.
        s2++;
    }
    // Handle cases where one string is a prefix of another or both are empty.
    return tolower((unsigned char)*s1) - tolower((unsigned char)*s2);
}

// Custom string copy function (manual implementation for 'strncpy').
// Copies up to 'n' characters from source to destination, ensuring null-termination.
char* custom_strncpy(char* dest, const char* src, size_t n) {
    size_t i;
    for (i = 0; i < n - 1 && src[i] != '\0'; ++i) { // Copy characters up to n-1 or null terminator.
        dest[i] = src[i];
    }
    dest[i] = '\0'; // Ensure null-termination.
    return dest;
}

// Structure to hold song information.
class Song {
public:
    char songName[MAX_NAME_LEN];
    char artist[MAX_NAME_LEN];
    char genre[MAX_NAME_LEN];
    char language[MAX_NAME_LEN];
    int favoriteCount;

    // Constructor to initialize members.
    Song() : favoriteCount(0) {
        songName[0] = '\0'; artist[0] = '\0'; genre[0] = '\0'; language[0] = '\0';
    }
};

// Structure to hold album information, including an array of songs.
class Album {
public:
    char albumName[MAX_NAME_LEN];
    char artist[MAX_NAME_LEN];
    char genre[MAX_NAME_LEN];
    char language[MAX_NAME_LEN];
    Song songs[MAX_SONGS]; // Raw array for songs within the album.
    int songCount;
    double price;
    int soldCount;

    // Constructor to initialize members.
    Album() : songCount(0), price(0.0), soldCount(0) {
        albumName[0] = '\0'; artist[0] = '\0'; genre[0] = '\0'; language[0] = '\0';
    }
};

// Structure to hold playlist information, including an array of songs.
class Playlist {
public:
    char name[MAX_NAME_LEN];
    Song songs[MAX_SONGS]; // Raw array for songs within the playlist.
    int songCount;

    // Constructor to initialize members.
    Playlist() : songCount(0) {
        name[0] = '\0';
    }
};

// Enum to define different subscription package types.
enum PackageType {
    FREE_TIER = 0,
    ONE_MONTH = 1,
    THREE_MONTH = 2,
    PERMANENT = 3
};

// Structure to hold subscription details for a user.
class Subscription {
public:
    PackageType type;
    long long expiryUnixTime;
    int songsListened;
    long long lastResetUnixTime;

    // Constructor to initialize members.
    Subscription() : type(FREE_TIER), expiryUnixTime(0), songsListened(0), lastResetUnixTime(0) {}
};

// Account node for the custom hash table. This serves as a node in a linked list,
// specifically used for separate chaining collision handling in the hash table.
class AccountNode {
public:
    char username[MAX_NAME_LEN];
    char password[MAX_PASSWORD_LEN];
    char email[MAX_EMAIL_LEN];
    Subscription subscription;
    AccountNode* next; // Pointer to the next node in case of collision (linked list element).

    // Constructor to initialize members.
    AccountNode() : next(NULL) {
        username[0] = '\0'; password[0] = '\0'; email[0] = '\0';
    }
};

// Custom hash function (multiplication method variant).
// Computes a hash index for a given string key, ensuring it fits within the table size.
unsigned int customHash(const char* key, int tableSize) {
    unsigned int hash = 0;
    // Simple polynomial rolling hash with a multiplier (31 is a common prime).
    for (int i = 0; key[i] != '\0'; i++) {
        hash = hash * 31 + key[i];
    }
    return hash % tableSize; // Modulo operation to fit hash into table size.
}

// Initializes the hash table by setting all pointers to NULL.
void initHashTable(AccountNode* hashTable[], int size) {
    for (int i = 0; i < size; i++) {
        hashTable[i] = NULL;
    }
}

// Inserts a new account into the hash table.
// Uses separate chaining for collision resolution: if a collision occurs, the new node is added to the beginning of the linked list at that hash index.
void hashTableInsert(AccountNode* hashTable[], int size, const char* username, const char* password, const char* email, PackageType type, long long expiry, int songsListened, long long lastReset) {
    unsigned int index = customHash(username, size); // Calculate hash index.
    AccountNode* newNode = new (nothrow) AccountNode(); // Allocate new node.
    if (newNode == NULL) { // Handle memory allocation failure.
        fprintf(stderr, "\tMemory allocation failed during hash table insertion!\n");
        return;
    }
    // Populate new node data.
    custom_strncpy(newNode->username, username, MAX_NAME_LEN);
    custom_strncpy(newNode->password, password, MAX_PASSWORD_LEN);
    custom_strncpy(newNode->email, email, MAX_EMAIL_LEN);
    newNode->subscription.type = type;
    newNode->subscription.expiryUnixTime = expiry;
    newNode->subscription.songsListened = songsListened;
    newNode->subscription.lastResetUnixTime = lastReset;
    // Insert at the head of the linked list at the calculated index.
    newNode->next = hashTable[index];
    hashTable[index] = newNode;
}

// Searches for an account in the hash table by username.
// Traverses the linked list at the calculated hash index if collisions occurred.
AccountNode* hashTableSearch(AccountNode* const* hashTable, int size, const char* username) {
    unsigned int index = customHash(username, size); // Calculate hash index.
    AccountNode* current = hashTable[index]; // Start traversal from the head of the list at this index.
    while (current != NULL) { // Iterate through the linked list.
        if (strcmp(current->username, username) == 0) { // Found matching username.
            return current;
        }
        current = current->next; // Move to the next node in the list.
    }
    return NULL; // Username not found.
}

// Updates subscription details for an existing user in the hash table.
void hashTableUpdateSubscription(AccountNode* hashTable[], int size, const char* username, PackageType type, long long expiry, int songsListened, long long lastReset) {
    AccountNode* node = hashTableSearch(hashTable, size, username); // Find the user node.
    if (node != NULL) { // If user found, update their subscription details.
        node->subscription.type = type;
        node->subscription.expiryUnixTime = expiry;
        node->subscription.songsListened = songsListened;
        node->subscription.lastResetUnixTime = lastReset;
    }
}

// Saves the entire hash table (all accounts) to a file.
void saveHashTableToFile(AccountNode* const* hashTable, int size, const char* filename) {
    FILE* outFile = fopen(filename, "w"); // Open file in write mode.
    if (!outFile) { // Handle file open error.
        fprintf(stderr, "\tError opening file for saving hash table: %s\n", filename);
        return;
    }
    for (int i = 0; i < size; i++) { // Iterate through each bucket of the hash table.
        AccountNode* current = hashTable[i]; // Start traversal of linked list in current bucket.
        while (current != NULL) { // Iterate through linked list nodes.
            fprintf(outFile, "%s %s %s %d %lld %d %lld\n", // Write account data to file.
                    current->username, current->password, current->email,
                    current->subscription.type, current->subscription.expiryUnixTime,
                    current->subscription.songsListened, current->subscription.lastResetUnixTime);
            current = current->next; // Move to next node.
        }
    }
    fclose(outFile); // Close file.
}

// Loads account data from a file into the hash table.
void loadHashTableFromFile(AccountNode* hashTable[], int size, const char* filename) {
    FILE* inFile = fopen(filename, "r"); // Open file in read mode.
    if (!inFile) { // If file doesn't exist, try to create it.
        FILE* newFile = fopen(filename, "w");
        if (newFile) {
            fclose(newFile);
        } else {
            fprintf(stderr, "\tError creating file: %s\n", filename);
        }
        return;
    }
    // Buffer and variables for parsing each line.
    char username[MAX_NAME_LEN];
    char password[MAX_PASSWORD_LEN];
    char email[MAX_EMAIL_LEN];
    int type_int;
    long long expiry;
    int songsListened;
    long long lastReset = 0; // Default for older formats.
    char line_buffer[512];
    bool newFormatDetectedViaSuccessfulRead = false; // Flag to handle old/new file formats.
    while (fgets(line_buffer, sizeof(line_buffer), inFile) != NULL) { // Read line by line.
        // Attempt to parse line with 7 items (new format).
        int items_scanned = sscanf(line_buffer, "%s %s %s %d %lld %d %lld",
                                   username, password, email, &type_int, &expiry, &songsListened, &lastReset);
        if (items_scanned == 7) { // Successfully read 7 items (new format).
            newFormatDetectedViaSuccessfulRead = true;
            if (hashTableSearch(hashTable, size, username) == NULL) { // Prevent duplicate insertions.
                hashTableInsert(hashTable, size, username, password, email, static_cast<PackageType>(type_int), expiry, songsListened, lastReset);
            }
        } else if (items_scanned == 6 && !newFormatDetectedViaSuccessfulRead) { // Old format (6 items).
            lastReset = 0; // Set default for missing 'lastResetUnixTime'.
            if (hashTableSearch(hashTable, size, username) == NULL) {
                hashTableInsert(hashTable, size, username, password, email, static_cast<PackageType>(type_int), expiry, songsListened, lastReset);
            }
        } else if (items_scanned > 0) { // Partial read or malformed line.
            fprintf(stderr, "\tWarning: Malformed line or unexpected data in %s: %s", filename, line_buffer);
        }
    }
    fclose(inFile); // Close file.
}

// Frees all dynamically allocated memory for AccountNodes in the hash table.
void freeHashTable(AccountNode* hashTable[], int size) {
    for (int i = 0; i < size; i++) { // Iterate through each bucket.
        AccountNode* current = hashTable[i];
        while (current != NULL) { // Traverse and delete nodes in the linked list.
            AccountNode* temp = current;
            current = current->next;
            delete temp;
        }
        hashTable[i] = NULL; // Set bucket pointer to NULL after freeing.
    }
}

// Manages user subscriptions using a custom hash table for efficient lookup (Team A - Hashing).
class SubscriptionManager {
public:
    AccountNode* userHashTable[HASH_TABLE_SIZE]; // Raw array for the hash table buckets.
    const char* FILE_NAME;

    // Constructor: Initializes the hash table and loads data from file.
    SubscriptionManager() : FILE_NAME("user_subscriptions.txt") {
        initHashTable(userHashTable, HASH_TABLE_SIZE);
        loadHashTableFromFile(userHashTable, HASH_TABLE_SIZE, FILE_NAME);
    }

    // Destructor: Saves current hash table state to file and frees memory.
    ~SubscriptionManager() {
        saveHashTableToFile(userHashTable, HASH_TABLE_SIZE, FILE_NAME);
        freeHashTable(userHashTable, HASH_TABLE_SIZE);
    }

    // Retrieves subscription details for a given username.
    Subscription getUserSubscription(const char* username) const {
        AccountNode* userAccount = hashTableSearch(userHashTable, HASH_TABLE_SIZE, username);
        if (userAccount) {
            return userAccount->subscription;
        }
        return Subscription(); // Return default if not found.
    }

    // Updates subscription details for a user.
    void updateUserSubscription(const char* username, PackageType type, long long expiry, int songsListened, long long lastReset) {
        hashTableUpdateSubscription(userHashTable, HASH_TABLE_SIZE, username, type, expiry, songsListened, lastReset);
    }

    // Ensures a user account exists; creates a new one if not found.
    bool ensureUserExists(const char* username, bool showMessages = true) {
        if (hashTableSearch(userHashTable, HASH_TABLE_SIZE, username) == NULL) { // Check if user exists.
            char defaultPassword[MAX_PASSWORD_LEN] = "password123";
            char defaultEmail[MAX_EMAIL_LEN];
            sprintf(defaultEmail, "%s@riffmix.user", username); // Generate default email.
            if (strlen(defaultEmail) >= MAX_EMAIL_LEN) {
                 custom_strncpy(defaultEmail, "newuser@riffmix.user", MAX_EMAIL_LEN); // Fallback if username too long.
            }
            // Insert new user with free tier subscription.
            hashTableInsert(userHashTable, HASH_TABLE_SIZE, username, defaultPassword, defaultEmail, FREE_TIER, 0, 0, 0);
            saveHashTableToFile(userHashTable, HASH_TABLE_SIZE, FILE_NAME); // Save changes.
            if (showMessages) {
                cout << "\n\tWelcome, new user '" << username << "'! Your account has been created with a Free Tier subscription." << endl;
                Sleep(1500);
            }
            return true; // User was created.
        }
        return false; // User already existed.
    }
};

// Node structure for the custom Linked Queue (Team B - Queue).
class QueueNode {
public:
    Song song; // Data stored in the queue node.
    QueueNode* next; // Pointer to the next node in the queue.

    // Constructor: Initializes with a song and sets next to NULL.
    QueueNode(const Song& s) : song(s), next(NULL) {}
};

// Custom implementation of a Queue using a Linked List (Team B - Queue).
// This fulfills the requirement to use a Queue data structure without std::queue.
class LinkedQueue {
private:
    QueueNode* front; // Pointer to the front of the queue.
    QueueNode* rear;  // Pointer to the rear of the queue.
    int currentSize;  // Tracks the number of elements in the queue.

public:
    // Constructor: Initializes an empty queue.
    LinkedQueue() : front(NULL), rear(NULL), currentSize(0) {}

    // Destructor: Clears all nodes to prevent memory leaks.
    ~LinkedQueue() {
        clear();
    }

    // Adds a song to the rear of the queue (enqueue operation).
    bool enqueue(const Song& song) {
        QueueNode* newNode = new (nothrow) QueueNode(song); // Allocate new node.
        if (newNode == NULL) { // Handle memory allocation failure.
            fprintf(stderr, "\tMemory allocation failed for queue node!\n");
            return false;
        }
        if (isEmpty()) { // If queue is empty, new node is both front and rear.
            front = newNode;
            rear = newNode;
        } else { // Otherwise, add to the rear and update rear pointer.
            rear->next = newNode;
            rear = newNode;
        }
        currentSize++; // Increment size.
        return true;
    }

    // Removes and returns the song from the front of the queue (dequeue operation).
    Song dequeue() {
        if (isEmpty()) { // Check for empty queue.
            fprintf(stderr, "\tError: Dequeuing from an empty queue. Returning an empty song.\n");
            return Song(); // Return default song if empty.
        }
        QueueNode* temp = front; // Store front node for deletion.
        Song dequeuedSong = temp->song; // Get the song data.
        front = front->next; // Move front pointer to next node.
        if (front == NULL) { // If queue becomes empty, update rear.
            rear = NULL;
        }
        currentSize--; // Decrement size.
        delete temp; // Free memory of the dequeued node.
        return dequeuedSong;
    }

    // Returns a pointer to the song at the front of the queue without removing it (peek operation).
    Song* peek() const {
        if (isEmpty()) { // Check for empty queue.
            return NULL;
        }
        return &(front->song); // Return address of the song data.
    }

    // Checks if the queue is empty.
    bool isEmpty() const {
        return front == NULL;
    }

    // Returns the current number of songs in the queue.
    int size() const {
        return currentSize;
    }

    // Clears all elements from the queue.
    void clear() {
        while (!isEmpty()) {
            dequeue(); // Dequeue all elements until empty.
        }
    }

    // Displays the songs currently in the queue.
    void display() const {
        if (isEmpty()) {
            cout << "\tQueue is empty." << endl;
            return;
        }
        cout << "\tCurrent Play Next Queue (" << currentSize << " songs):" << endl;
        QueueNode* current = front;
        int count = 1;
        while (current != NULL) {
            cout << "\t" << count << ". " << current->song.songName << " by " << current->song.artist << endl;
            current = current->next;
            count++;
        }
    }
};

// Manages application-wide data like genres, languages, recommended songs, and purchasable albums.
class AppData {
public:
    char genres[MAX_GENRES][MAX_NAME_LEN]; // Raw array for genres.
    int genreCount;
    char languages[MAX_LANGUAGES][MAX_NAME_LEN]; // Raw array for languages.
    int languageCount;
    Song recommendedSongs[MAX_RECOMMENDED_SONGS]; // Raw array for recommended songs.
    int recommendedSongCount;
    Album purchasableAlbums[MAX_ALBUMS]; // Raw array for purchasable albums.
    int purchasableAlbumCount;

    // Constructor: Loads data from files or initializes default data if files are not found.
    AppData() : genreCount(0), languageCount(0), recommendedSongCount(0), purchasableAlbumCount(0) {
        loadData(); // Attempt to load initial data.
        // Initialize default genres if not loaded from file.
        if (genreCount == 0) {
            genreCount = 0;
            if (genreCount < MAX_GENRES) custom_strncpy(genres[genreCount++], "Rock", MAX_NAME_LEN);
            if (genreCount < MAX_GENRES) custom_strncpy(genres[genreCount++], "Pop", MAX_NAME_LEN);
            if (genreCount < MAX_GENRES) custom_strncpy(genres[genreCount++], "Blues", MAX_NAME_LEN);
            if (genreCount < MAX_GENRES) custom_strncpy(genres[genreCount++], "Hip-Hop", MAX_NAME_LEN);
            if (genreCount < MAX_GENRES) custom_strncpy(genres[genreCount++], "Techno", MAX_NAME_LEN);
            if (genreCount < MAX_GENRES) custom_strncpy(genres[genreCount++], "Flamenco", MAX_NAME_LEN);
            if (genreCount < MAX_GENRES) custom_strncpy(genres[genreCount++], "Dance Music", MAX_NAME_LEN);
        }
        // Initialize default languages if not loaded from file.
        if (languageCount == 0) {
            languageCount = 0;
            if (languageCount < MAX_LANGUAGES) custom_strncpy(languages[languageCount++], "English", MAX_NAME_LEN);
            if (languageCount < MAX_LANGUAGES) custom_strncpy(languages[languageCount++], "Chinese", MAX_NAME_LEN);
            if (languageCount < MAX_LANGUAGES) custom_strncpy(languages[languageCount++], "Japanese", MAX_NAME_LEN);
            if (languageCount < MAX_LANGUAGES) custom_strncpy(languages[languageCount++], "Korean", MAX_NAME_LEN);
            if (languageCount < MAX_LANGUAGES) custom_strncpy(languages[languageCount++], "Russian", MAX_NAME_LEN);
            if (languageCount < MAX_LANGUAGES) custom_strncpy(languages[languageCount++], "others", MAX_NAME_LEN);
        }
        // Add default recommended songs if not loaded.
        if (recommendedSongCount == 0) {
            addDefaultRecommendedSongs();
        }
        // Add default purchasable albums if not loaded.
        if (purchasableAlbumCount == 0) {
            addDefaultPurchasableAlbums();
        }
    }

    // Destructor: Saves current application data to files.
    ~AppData() {
        saveData();
    }

private:
    // Loads genres and languages from 'app_data.txt'.
    void loadData() {
        FILE* file = fopen("app_data.txt", "r");
        if (file) {
            char line[512];
            while (fgets(line, sizeof(line), file) != NULL) {
                line[strcspn(line, "\n")] = 0; // Remove newline character.
                if (strncmp(line, "GENRES:", 7) == 0) {
                    char* data = line + 7;
                    char* data_copy = new char[strlen(data) + 1]; // Create mutable copy for strtok.
                    if(!data_copy) { fprintf(stderr, "Memory allocation failed for genre parsing\n"); continue;}
                    strcpy(data_copy, data);
                    char* token = strtok(data_copy, ","); // Parse comma-separated genres.
                    genreCount = 0;
                    while (token != NULL && genreCount < MAX_GENRES) {
                        custom_strncpy(genres[genreCount++], token, MAX_NAME_LEN);
                        token = strtok(NULL, ",");
                    }
                    delete[] data_copy;
                } else if (strncmp(line, "LANGUAGES:", 10) == 0) {
                    char* data = line + 10;
                    char* data_copy = new char[strlen(data) + 1]; // Create mutable copy for strtok.
                    if(!data_copy) { fprintf(stderr, "Memory allocation failed for language parsing\n"); continue;}
                    strcpy(data_copy, data);
                    char* token = strtok(data_copy, ","); // Parse comma-separated languages.
                    languageCount = 0;
                    while (token != NULL && languageCount < MAX_LANGUAGES) {
                        custom_strncpy(languages[languageCount++], token, MAX_NAME_LEN);
                        token = strtok(NULL, ",");
                    }
                    delete[] data_copy;
                }
            }
            fclose(file);
        } else {
             FILE* newFile = fopen("app_data.txt", "w"); // Create file if it doesn't exist.
             if(newFile) fclose(newFile);
        }
        loadRecommendedSongs(); // Also load recommended songs and albums.
        loadPurchasableAlbums();
    }

    // Saves current genres and languages to 'app_data.txt'.
    void saveData() {
        FILE* file = fopen("app_data.txt", "w");
        if (file) {
            fprintf(file, "GENRES:");
            for (int i = 0; i < genreCount; ++i) {
                fprintf(file, "%s%s", genres[i], (i == genreCount - 1 ? "" : ",")); // Write genres, comma-separated.
            }
            fprintf(file, "\n");
            fprintf(file, "LANGUAGES:");
            for (int i = 0; i < languageCount; ++i) {
                fprintf(file, "%s%s", languages[i], (i == languageCount - 1 ? "" : ",")); // Write languages, comma-separated.
            }
            fprintf(file, "\n");
            fclose(file);
        } else {
            fprintf(stderr, "\tError saving app_data.txt!\n");
        }
        saveRecommendedSongs(); // Also save recommended songs and albums.
        savePurchasableAlbums();
    }

    // Saves recommended songs to 'recommended_songs.txt'.
    void saveRecommendedSongs() {
        FILE* file = fopen("recommended_songs.txt", "w");
        if (!file) {
            fprintf(stderr, "\tError saving recommended_songs.txt!\n");
            return;
        }
        for (int i = 0; i < recommendedSongCount; ++i) {
            fprintf(file, "%s,%s,%s,%s,%d\n", // Write song details.
                    recommendedSongs[i].songName,
                    recommendedSongs[i].artist,
                    recommendedSongs[i].genre,
                    recommendedSongs[i].language,
                    recommendedSongs[i].favoriteCount);
        }
        fclose(file);
    }

    // Loads recommended songs from 'recommended_songs.txt'.
    void loadRecommendedSongs() {
        FILE* file = fopen("recommended_songs.txt", "r");
        if (!file) {
            FILE* newFile = fopen("recommended_songs.txt", "w"); // Create file if it doesn't exist.
            if(newFile) fclose(newFile);
            return;
        }
        char line[256];
        recommendedSongCount = 0;
        while (fgets(line, sizeof(line), file) != NULL && recommendedSongCount < MAX_RECOMMENDED_SONGS) {
            line[strcspn(line, "\n")] = 0; // Remove newline.
            char* line_copy = new char[strlen(line) + 1]; // Create mutable copy for strtok.
            if(!line_copy) { fprintf(stderr, "Memory allocation failed for recommended song parsing\n"); continue;}
            strcpy(line_copy, line);
            char* token;
            // Parse song details from CSV line.
            token = strtok(line_copy, ","); if (token) custom_strncpy(recommendedSongs[recommendedSongCount].songName, token, MAX_NAME_LEN); else { delete[] line_copy; continue; }
            token = strtok(NULL, ","); if (token) custom_strncpy(recommendedSongs[recommendedSongCount].artist, token, MAX_NAME_LEN); else { delete[] line_copy; continue; }
            token = strtok(NULL, ","); if (token) custom_strncpy(recommendedSongs[recommendedSongCount].genre, token, MAX_NAME_LEN); else { delete[] line_copy; continue; }
            token = strtok(NULL, ","); if (token) custom_strncpy(recommendedSongs[recommendedSongCount].language, token, MAX_NAME_LEN); else { delete[] line_copy; continue; }
            token = strtok(NULL, ","); if (token) recommendedSongs[recommendedSongCount].favoriteCount = atoi(token); else { delete[] line_copy; continue; }
            recommendedSongCount++;
            delete[] line_copy;
        }
        fclose(file);
    }

	// Adds a predefined list of default recommended songs.
	void addDefaultRecommendedSongs() {
    struct {
        const char* name;
        const char* artist; 
        const char* genre;
        const char* lang;
    } defaults[] = {
        {"Heart of Stone", "The Rebels", "Rock", "English"},
        {"Dream On", "Liu Xin", "Pop", "Chinese"},
        {"Delta Bound", "Mississippi Joe", "Blues", "English"},
        {"Mic Check", "Young Flow", "Hip-Hop", "English"},
        {"Pulse Reactor", "Neon Samurai", "Techno", "Japanese"},
        
        {"Duende", "Carmen Ruiz", "Flamenco", "others"},
        {"Flamenco del Alba", "Rosa Garcia", "Flamenco", "others"},
        {"Rhythm & Wine", "Luis Delgado", "Flamenco", "others"},
        
        {"Dance Revolution", "Beat Makers", "Dance Music", "English"},
        {"The Brightest Star", "Chen Li", "Pop", "Chinese"},
        {"Electric Storm", "DJ Thunder", "Techno", "English"},
        {"Rustic Blues", "Old Town Trio", "Blues", "English"},
        
        {"City Cypher", "MC Seoul", "Hip-Hop", "Korean"},
        {"Underground Kings", "K-Pop Crew", "Pop", "Korean"},
        {"Rivers of Seoul", "Park Min-ji", "Pop", "Korean"},
        
        {"Neon Skyline", "Future Beat", "Techno", "English"},
        {"Rock the Night", "Midnight Riders", "Rock", "English"},
        {"Wanderer", "Zhang Lei", "Blues", "Chinese"},
        {"Stardust Memories", "Nova Quartet", "Pop", "English"},
        
        {"Kremlin Echoes", "Boris Ivanov", "Rock", "Russian"},
        {"Blues for Russia", "Olga Kuznetsova", "Blues", "Russian"},
        
        {"Cyber Pulse", "Tech Masters", "Techno", "English"},
        {"Flamenco Shadows", "Diego Rivera", "Flamenco", "others"},
        {"Midnight Dance", "Velvet Beats", "Dance Music", "English"},
        {"Soul on Fire", "The Blue Flames", "Blues", "English"},
        {"Electric Caravan", "Zeta Project", "Techno", "English"},
        {"Flamenco Corazon", "Isabella Cruz", "Flamenco", "others"},
        {"Rock the Castle", "Dragon's Breath", "Rock", "English"},
        {"Far Away", "Wu Yue", "Pop", "Chinese"},
        {"Hip-Hop Horizon", "MC Blaze", "Hip-Hop", "English"}
    };
    
    int count = sizeof(defaults) / sizeof(defaults[0]);
    
    for (int i = 0; i < count && recommendedSongCount < MAX_RECOMMENDED_SONGS; i++) {
        Song song;
        
        // Initialize the struct to zero before copying to avoid junk data.
        memset(&song, 0, sizeof(Song));
        
        custom_strncpy(song.songName, defaults[i].name, MAX_NAME_LEN);
        custom_strncpy(song.artist, defaults[i].artist, MAX_NAME_LEN);
        custom_strncpy(song.genre, defaults[i].genre, MAX_NAME_LEN);
        custom_strncpy(song.language, defaults[i].lang, MAX_NAME_LEN);
        song.favoriteCount = 0; // Default favorite count.
        
        recommendedSongs[recommendedSongCount++] = song; // Add song to array.
    }
    
    saveRecommendedSongs(); // Save the newly added default songs.
}

    // Saves purchasable albums to 'albums.txt', including their songs.
    void savePurchasableAlbums() {
        FILE* file = fopen("albums.txt", "w");
        if (!file) {
            fprintf(stderr, "\tError saving albums.txt!\n");
            return;
        }
        for (int i = 0; i < purchasableAlbumCount; ++i) {
            // Write album details.
            fprintf(file, "%s,%s,%s,%s,%.2f,%d\n",
                    purchasableAlbums[i].albumName,
                    purchasableAlbums[i].artist,
                    purchasableAlbums[i].genre,
                    purchasableAlbums[i].language,
                    purchasableAlbums[i].price,
                    purchasableAlbums[i].soldCount);
            // Write each song within the album, prefixed with "SONG:".
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

    // Loads purchasable albums from 'albums.txt'.
    void loadPurchasableAlbums() {
        FILE* file = fopen("albums.txt", "r");
        if (!file) {
            FILE* newFile = fopen("albums.txt", "w"); // Create file if it doesn't exist.
            if(newFile) fclose(newFile);
            return;
        }
        char line[512];
        purchasableAlbumCount = 0;
        Album currentAlbum; // Temporary album to build data.
        while (fgets(line, sizeof(line), file) != NULL && purchasableAlbumCount < MAX_ALBUMS) {
            line[strcspn(line, "\n")] = 0; // Remove newline.
            if (strncmp(line, "SONG:", 5) == 0) { // If line represents a song.
                // Add song to current album or the last stored album if currentAlbum is empty.
                if (currentAlbum.albumName[0] == '\0' && purchasableAlbumCount > 0) {
                    Album* lastStoredAlbum = &purchasableAlbums[purchasableAlbumCount - 1];
                    if (lastStoredAlbum->songCount < MAX_SONGS) {
                         char* song_data = line + 5;
                         Song newSong;
                         char* song_data_copy = new char[strlen(song_data) + 1];
                         if(!song_data_copy) { fprintf(stderr, "Memory alloc failed for orphan song\n"); continue;}
                         strcpy(song_data_copy, song_data);
                         char* token = strtok(song_data_copy, ","); if (token) custom_strncpy(newSong.songName, token, MAX_NAME_LEN); else { delete[] song_data_copy; continue; }
                         token = strtok(NULL, ","); if (token) custom_strncpy(newSong.artist, token, MAX_NAME_LEN); else { delete[] song_data_copy; continue; }
                         token = strtok(NULL, ","); if (token) custom_strncpy(newSong.genre, token, MAX_NAME_LEN); else { delete[] song_data_copy; continue; }
                         token = strtok(NULL, ","); if (token) custom_strncpy(newSong.language, token, MAX_NAME_LEN); else { delete[] song_data_copy; continue; }
                         lastStoredAlbum->songs[lastStoredAlbum->songCount++] = newSong;
                         delete[] song_data_copy;
                    } else {
                        fprintf(stderr, "Warning: Orphan SONG line, and last album '%s' is full.\n", lastStoredAlbum->albumName);
                    }
                } else if (currentAlbum.albumName[0] != '\0') { // If building a new album.
                    if (currentAlbum.songCount < MAX_SONGS) {
                        char* song_data = line + 5;
                        Song newSong;
                        char* song_data_copy = new char[strlen(song_data) + 1];
                        if(!song_data_copy) { fprintf(stderr, "Memory allocation failed for album song parsing\n"); continue;}
                        strcpy(song_data_copy, song_data);
                        char* token = strtok(song_data_copy, ",");
                        if (token) custom_strncpy(newSong.songName, token, MAX_NAME_LEN); else { delete[] song_data_copy; continue; }
                        token = strtok(NULL, ",");
                        if (token) custom_strncpy(newSong.artist, token, MAX_NAME_LEN); else { delete[] song_data_copy; continue; }
                        token = strtok(NULL, ",");
                        if (token) custom_strncpy(newSong.genre, token, MAX_NAME_LEN); else { delete[] song_data_copy; continue; }
                        token = strtok(NULL, ",");
                        if (token) custom_strncpy(newSong.language, token, MAX_NAME_LEN); else { delete[] song_data_copy; continue; }
                        currentAlbum.songs[currentAlbum.songCount++] = newSong;
                        delete[] song_data_copy;
                    } else {
                        fprintf(stderr, "Warning: Album '%s' reached max song limit during load.\n", currentAlbum.albumName);
                    }
                } else {
                     fprintf(stderr, "Warning: SONG line encountered before album details in albums.txt. Skipping song.\n");
                }
            } else { // If line represents album details.
                if (currentAlbum.albumName[0] != '\0') { // If a previous album was being built, store it.
                    if (purchasableAlbumCount < MAX_ALBUMS) {
                        purchasableAlbums[purchasableAlbumCount++] = currentAlbum;
                    } else {
                         fprintf(stderr, "Warning: Maximum albums reached. Cannot load album '%s'.\n", currentAlbum.albumName);
                         currentAlbum = Album(); // Reset currentAlbum.
                    }
                }
                if (purchasableAlbumCount < MAX_ALBUMS) {
                    currentAlbum = Album(); // Start building a new album.
                    char* line_copy = new char[strlen(line) + 1]; // Create mutable copy for strtok.
                    if(!line_copy) { fprintf(stderr, "Memory allocation failed for album header parsing\n"); continue;}
                    strcpy(line_copy, line);
                    char* token;
                    // Parse album details from CSV line.
                    token = strtok(line_copy, ",");
                    if (token) custom_strncpy(currentAlbum.albumName, token, MAX_NAME_LEN); else { delete[] line_copy; continue; }
                    token = strtok(NULL, ",");
                    if (token) custom_strncpy(currentAlbum.artist, token, MAX_NAME_LEN); else { delete[] line_copy; continue; }
                    token = strtok(NULL, ",");
                    if (token) custom_strncpy(currentAlbum.genre, token, MAX_NAME_LEN); else { delete[] line_copy; continue; }
                    token = strtok(NULL, ",");
                    if (token) custom_strncpy(currentAlbum.language, token, MAX_NAME_LEN); else { delete[] line_copy; continue; }
                    token = strtok(NULL, ",");
                    if (token) currentAlbum.price = atof(token); else { delete[] line_copy; continue; }
                    token = strtok(NULL, ",");
                    if (token) currentAlbum.soldCount = atoi(token); else { delete[] line_copy; continue; }
                    delete[] line_copy;
                }
            }
        }
        // Store the last album if it was being built.
        if (currentAlbum.albumName[0] != '\0' && purchasableAlbumCount < MAX_ALBUMS) {
            purchasableAlbums[purchasableAlbumCount++] = currentAlbum;
        }
        fclose(file);
    }

    // Adds a predefined list of default purchasable albums with their songs.
    void addDefaultPurchasableAlbums() {
    Album album;
    Song albumSong;

    // Rock albums first - my favorite genre
    custom_strncpy(album.albumName, "Thunderroad", MAX_NAME_LEN);
    custom_strncpy(album.artist, "Jack Mercer", MAX_NAME_LEN);
    custom_strncpy(album.genre, "Rock", MAX_NAME_LEN);
    custom_strncpy(album.language, "English", MAX_NAME_LEN);
    album.price = 29.90;
    album.soldCount = 0;
    album.songCount = 0;

    custom_strncpy(albumSong.songName, "Main Track - Thunderroad", MAX_NAME_LEN);
    custom_strncpy(albumSong.artist, album.artist, MAX_NAME_LEN);
    custom_strncpy(albumSong.genre, album.genre, MAX_NAME_LEN);
    custom_strncpy(albumSong.language, album.language, MAX_NAME_LEN);
    if (album.songCount < MAX_SONGS) {
        album.songs[album.songCount++] = albumSong;
    }
    purchasableAlbums[purchasableAlbumCount++] = album;

    // Some Chinese pop
    if (purchasableAlbumCount < MAX_ALBUMS) {
        custom_strncpy(album.albumName, "Midnight Confession", MAX_NAME_LEN);
        custom_strncpy(album.artist, "Li Wei", MAX_NAME_LEN);
        custom_strncpy(album.genre, "Pop", MAX_NAME_LEN);
        custom_strncpy(album.language, "Chinese", MAX_NAME_LEN);
        album.price = 19.50;
        album.soldCount = 0;
        album.songCount = 0;

        custom_strncpy(albumSong.songName, "Title Track - Midnight Confession", MAX_NAME_LEN);
        custom_strncpy(albumSong.artist, album.artist, MAX_NAME_LEN);
        custom_strncpy(albumSong.genre, album.genre, MAX_NAME_LEN);
        custom_strncpy(albumSong.language, album.language, MAX_NAME_LEN);
        if (album.songCount < MAX_SONGS) {
            album.songs[album.songCount++] = albumSong;
        }
        purchasableAlbums[purchasableAlbumCount++] = album;
    }

    // Blues section
    if (purchasableAlbumCount >= MAX_ALBUMS) goto cleanup;
    
    custom_strncpy(album.albumName, "Blue Horizon", MAX_NAME_LEN);
    custom_strncpy(album.artist, "Anna Sullivan", MAX_NAME_LEN);
    custom_strncpy(album.genre, "Blues", MAX_NAME_LEN);
    custom_strncpy(album.language, "English", MAX_NAME_LEN);
    album.price = 24.80;
    album.soldCount = 0;
    album.songCount = 0;
    custom_strncpy(albumSong.songName, "Blue Horizon Theme", MAX_NAME_LEN);
    custom_strncpy(albumSong.artist, album.artist, MAX_NAME_LEN);
    custom_strncpy(albumSong.genre, album.genre, MAX_NAME_LEN);
    custom_strncpy(albumSong.language, album.language, MAX_NAME_LEN);
    album.songs[album.songCount++] = albumSong;
    purchasableAlbums[purchasableAlbumCount++] = album;

    // Korean Hip-Hop is hot right now
    custom_strncpy(album.albumName, "Urban Groove", MAX_NAME_LEN);
    custom_strncpy(album.artist, "Kim Soo-jin", MAX_NAME_LEN);
    custom_strncpy(album.genre, "Hip-Hop", MAX_NAME_LEN);
    custom_strncpy(album.language, "Korean", MAX_NAME_LEN);
    album.price = 22.00;
    album.soldCount = 0;
    album.songCount = 0;
    custom_strncpy(albumSong.songName, "Beat of the City", MAX_NAME_LEN);
    custom_strncpy(albumSong.artist, album.artist, MAX_NAME_LEN); 
    custom_strncpy(albumSong.genre, album.genre, MAX_NAME_LEN);
    custom_strncpy(albumSong.language, album.language, MAX_NAME_LEN);
    album.songs[album.songCount++] = albumSong;
    purchasableAlbums[purchasableAlbumCount++] = album;

    if (purchasableAlbumCount >= MAX_ALBUMS) goto cleanup;

    // Japanese techno stuff
    custom_strncpy(album.albumName, "Neon Lights", MAX_NAME_LEN);
    custom_strncpy(album.artist, "Yamada Haruto", MAX_NAME_LEN);
    custom_strncpy(album.genre, "Techno", MAX_NAME_LEN);
    custom_strncpy(album.language, "Japanese", MAX_NAME_LEN);
    album.price = 27.30;
    album.soldCount = 0; album.songCount = 0;
    custom_strncpy(albumSong.songName, "Electric Pulse", MAX_NAME_LEN);
    custom_strncpy(albumSong.artist, album.artist, MAX_NAME_LEN);
    custom_strncpy(albumSong.genre, album.genre, MAX_NAME_LEN);
    custom_strncpy(albumSong.language, album.language, MAX_NAME_LEN);
    if (album.songCount < MAX_SONGS)
        album.songs[album.songCount++] = albumSong;
    purchasableAlbums[purchasableAlbumCount++] = album;

    // Flamenco - always expensive but worth it
    custom_strncpy(album.albumName, "Flamenco Sunset", MAX_NAME_LEN);
    custom_strncpy(album.artist, "Carlos Ramirez", MAX_NAME_LEN);
    custom_strncpy(album.genre, "Flamenco", MAX_NAME_LEN);
    custom_strncpy(album.language, "others", MAX_NAME_LEN);
    album.price = 31.00;
    album.soldCount = 0;
    album.songCount = 0;
    custom_strncpy(albumSong.songName, "Spanish Fire", MAX_NAME_LEN);
    custom_strncpy(albumSong.artist, "Carlos Ramirez", MAX_NAME_LEN);
    custom_strncpy(albumSong.genre, "Flamenco", MAX_NAME_LEN);
    custom_strncpy(albumSong.language, "others", MAX_NAME_LEN);
    album.songs[album.songCount++] = albumSong;
    purchasableAlbums[purchasableAlbumCount++] = album;

    // Dance music for the clubs
    if(purchasableAlbumCount < MAX_ALBUMS) {
        custom_strncpy(album.albumName, "Dance All Night", MAX_NAME_LEN);
        custom_strncpy(album.artist, "Fiona Tan", MAX_NAME_LEN);
        custom_strncpy(album.genre, "Dance Music", MAX_NAME_LEN);
        custom_strncpy(album.language, "English", MAX_NAME_LEN);
        album.price = 21.50;
        album.soldCount = 0;
        album.songCount = 0;
        custom_strncpy(albumSong.songName, "Club Anthem", MAX_NAME_LEN);
        custom_strncpy(albumSong.artist, album.artist, MAX_NAME_LEN);
        custom_strncpy(albumSong.genre, album.genre, MAX_NAME_LEN);
        custom_strncpy(albumSong.language, album.language, MAX_NAME_LEN);
        album.songs[album.songCount++] = albumSong;
        purchasableAlbums[purchasableAlbumCount++] = album;
    }

    // More rock - can't have enough
    custom_strncpy(album.albumName, "Wildfire", MAX_NAME_LEN);
    custom_strncpy(album.artist, "Zhang Mei", MAX_NAME_LEN);
    custom_strncpy(album.genre, "Rock", MAX_NAME_LEN);
    custom_strncpy(album.language, "Chinese", MAX_NAME_LEN);
    album.price = 28.75;
    album.soldCount = 0;
    album.songCount = 0;
    custom_strncpy(albumSong.songName, "Burning Strings", MAX_NAME_LEN);
    custom_strncpy(albumSong.artist, album.artist, MAX_NAME_LEN);
    custom_strncpy(albumSong.genre, album.genre, MAX_NAME_LEN);
    custom_strncpy(albumSong.language, album.language, MAX_NAME_LEN);
    album.songs[album.songCount++] = albumSong;
    if(purchasableAlbumCount < MAX_ALBUMS)
        purchasableAlbums[purchasableAlbumCount++] = album;

    // Japanese pop is popular
    custom_strncpy(album.albumName, "Sakura Rain", MAX_NAME_LEN);
    custom_strncpy(album.artist, "Ito Akiko", MAX_NAME_LEN);
    custom_strncpy(album.genre, "Pop", MAX_NAME_LEN);
    custom_strncpy(album.language, "Japanese", MAX_NAME_LEN);
    album.price = 20.00;
    album.soldCount = 0;
    album.songCount = 0;
    custom_strncpy(albumSong.songName, "Cherry Blossom Melody", MAX_NAME_LEN);
    custom_strncpy(albumSong.artist, album.artist, MAX_NAME_LEN);
    custom_strncpy(albumSong.genre, album.genre, MAX_NAME_LEN);
    custom_strncpy(albumSong.language, album.language, MAX_NAME_LEN);
    album.songs[album.songCount++] = albumSong;
    purchasableAlbums[purchasableAlbumCount++] = album;

    // Adding more blues classics
    if (purchasableAlbumCount < MAX_ALBUMS) {
        custom_strncpy(album.albumName, "Delta Blues", MAX_NAME_LEN);
        custom_strncpy(album.artist, "Robert Johnson", MAX_NAME_LEN);
        custom_strncpy(album.genre, "Blues", MAX_NAME_LEN);
        custom_strncpy(album.language, "English", MAX_NAME_LEN);
        album.price = 26.40;
        album.soldCount = 0;
        album.songCount = 0;
        custom_strncpy(albumSong.songName, "Crossroads Blues", MAX_NAME_LEN);
        custom_strncpy(albumSong.artist, album.artist, MAX_NAME_LEN);
        custom_strncpy(albumSong.genre, album.genre, MAX_NAME_LEN);
        custom_strncpy(albumSong.language, album.language, MAX_NAME_LEN);
        album.songs[album.songCount++] = albumSong;
        purchasableAlbums[purchasableAlbumCount++] = album;
    }

    // Russian Hip-Hop scene
    if (purchasableAlbumCount < MAX_ALBUMS) {
        custom_strncpy(album.albumName, "Street Poet", MAX_NAME_LEN);
        custom_strncpy(album.artist, "Vera Petrovna", MAX_NAME_LEN);
        custom_strncpy(album.genre, "Hip-Hop", MAX_NAME_LEN);
        custom_strncpy(album.language, "Russian", MAX_NAME_LEN);
        album.price = 23.90;
        album.soldCount = 0;
        album.songCount = 0;
        custom_strncpy(albumSong.songName, "Moscow Streets", MAX_NAME_LEN);
        custom_strncpy(albumSong.artist, album.artist, MAX_NAME_LEN);
        custom_strncpy(albumSong.genre, album.genre, MAX_NAME_LEN);
        custom_strncpy(albumSong.language, album.language, MAX_NAME_LEN);
        album.songs[album.songCount++] = albumSong;
        purchasableAlbums[purchasableAlbumCount++] = album;
    }

    // More electronic beats
    if (purchasableAlbumCount < MAX_ALBUMS) {
        custom_strncpy(album.albumName, "Quantum Beat", MAX_NAME_LEN);
        custom_strncpy(album.artist, "DJ Electro", MAX_NAME_LEN);
        custom_strncpy(album.genre, "Techno", MAX_NAME_LEN);
        custom_strncpy(album.language, "English", MAX_NAME_LEN);
        album.price = 25.00;
        album.soldCount = 0;
        album.songCount = 0;
        custom_strncpy(albumSong.songName, "Digital Waves", MAX_NAME_LEN);
        custom_strncpy(albumSong.artist, album.artist, MAX_NAME_LEN);
        custom_strncpy(albumSong.genre, album.genre, MAX_NAME_LEN);
        custom_strncpy(albumSong.language, album.language, MAX_NAME_LEN);
        album.songs[album.songCount++] = albumSong;
        purchasableAlbums[purchasableAlbumCount++] = album;
    }

    // Another flamenco masterpiece
    if (purchasableAlbumCount < MAX_ALBUMS) {
        custom_strncpy(album.albumName, "Luna Flamenca", MAX_NAME_LEN);
        custom_strncpy(album.artist, "Sofia Martinez", MAX_NAME_LEN);
        custom_strncpy(album.genre, "Flamenco", MAX_NAME_LEN);
        custom_strncpy(album.language, "others", MAX_NAME_LEN);
        album.price = 30.20;
        album.soldCount = 0;
        album.songCount = 0;
        custom_strncpy(albumSong.songName, "Moonlight Guitar", MAX_NAME_LEN);
        custom_strncpy(albumSong.artist, album.artist, MAX_NAME_LEN);
        custom_strncpy(albumSong.genre, album.genre, MAX_NAME_LEN);
        custom_strncpy(albumSong.language, album.language, MAX_NAME_LEN);
        album.songs[album.songCount++] = albumSong;
        purchasableAlbums[purchasableAlbumCount++] = album;
    }

    // Chinese dance vibes
    if (purchasableAlbumCount < MAX_ALBUMS) {
        custom_strncpy(album.albumName, "Rhythm of Life", MAX_NAME_LEN);
        custom_strncpy(album.artist, "Mei Ling", MAX_NAME_LEN);
        custom_strncpy(album.genre, "Dance Music", MAX_NAME_LEN);
        custom_strncpy(album.language, "Chinese", MAX_NAME_LEN);
        album.price = 18.60;
        album.soldCount = 0;
        album.songCount = 0;
        custom_strncpy(albumSong.songName, "Life Beat", MAX_NAME_LEN);
        custom_strncpy(albumSong.artist, album.artist, MAX_NAME_LEN);
        custom_strncpy(albumSong.genre, album.genre, MAX_NAME_LEN);
        custom_strncpy(albumSong.language, album.language, MAX_NAME_LEN);
        album.songs[album.songCount++] = albumSong;
        purchasableAlbums[purchasableAlbumCount++] = album;
    }

    // Irish rock power
    if (purchasableAlbumCount < MAX_ALBUMS) {
        custom_strncpy(album.albumName, "Steel Strings", MAX_NAME_LEN);
        custom_strncpy(album.artist, "Tom O'Connor", MAX_NAME_LEN);
        custom_strncpy(album.genre, "Rock", MAX_NAME_LEN);
        custom_strncpy(album.language, "English", MAX_NAME_LEN);
        album.price = 27.10;
        album.soldCount = 0;
        album.songCount = 0;
        custom_strncpy(albumSong.songName, "Irish Thunder", MAX_NAME_LEN);
        custom_strncpy(albumSong.artist, album.artist, MAX_NAME_LEN);
        custom_strncpy(albumSong.genre, album.genre, MAX_NAME_LEN);
        custom_strncpy(albumSong.language, album.language, MAX_NAME_LEN);
        album.songs[album.songCount++] = albumSong;
        purchasableAlbums[purchasableAlbumCount++] = album;
    }

    // K-Pop sensation
    if (purchasableAlbumCount < MAX_ALBUMS) {
        custom_strncpy(album.albumName, "City Lights", MAX_NAME_LEN);
        custom_strncpy(album.artist, "Park Ji-hoon", MAX_NAME_LEN);
        custom_strncpy(album.genre, "Pop", MAX_NAME_LEN);
        custom_strncpy(album.language, "Korean", MAX_NAME_LEN);
        album.price = 19.80;
        album.soldCount = 0;
        album.songCount = 0;
        custom_strncpy(albumSong.songName, "Seoul Nights", MAX_NAME_LEN);
        custom_strncpy(albumSong.artist, album.artist, MAX_NAME_LEN);
        custom_strncpy(albumSong.genre, album.genre, MAX_NAME_LEN);
        custom_strncpy(albumSong.language, album.language, MAX_NAME_LEN);
        album.songs[album.songCount++] = albumSong;
        purchasableAlbums[purchasableAlbumCount++] = album;
    }

    // Chinese blues underground
    if (purchasableAlbumCount < MAX_ALBUMS) {
        custom_strncpy(album.albumName, "Harbin Blues", MAX_NAME_LEN);
        custom_strncpy(album.artist, "Li Na", MAX_NAME_LEN);
        custom_strncpy(album.genre, "Blues", MAX_NAME_LEN);
        custom_strncpy(album.language, "Chinese", MAX_NAME_LEN);
        album.price = 24.00;
        album.soldCount = 0;
        album.songCount = 0;
        custom_strncpy(albumSong.songName, "Winter Blues", MAX_NAME_LEN);
        custom_strncpy(albumSong.artist, album.artist, MAX_NAME_LEN);
        custom_strncpy(albumSong.genre, album.genre, MAX_NAME_LEN);
        custom_strncpy(albumSong.language, album.language, MAX_NAME_LEN);
        album.songs[album.songCount++] = albumSong;
        purchasableAlbums[purchasableAlbumCount++] = album;
    }

    // Hip-Hop revolution
    if (purchasableAlbumCount < MAX_ALBUMS) {
        custom_strncpy(album.albumName, "Rap Revolution", MAX_NAME_LEN);
        custom_strncpy(album.artist, "Big V", MAX_NAME_LEN);
        custom_strncpy(album.genre, "Hip-Hop", MAX_NAME_LEN);
        custom_strncpy(album.language, "English", MAX_NAME_LEN);
        album.price = 22.50;
        album.soldCount = 0;
        album.songCount = 0;
        custom_strncpy(albumSong.songName, "Revolution Anthem", MAX_NAME_LEN);
        custom_strncpy(albumSong.artist, album.artist, MAX_NAME_LEN);
        custom_strncpy(albumSong.genre, album.genre, MAX_NAME_LEN);
        custom_strncpy(albumSong.language, album.language, MAX_NAME_LEN);
        album.songs[album.songCount++] = albumSong;
        purchasableAlbums[purchasableAlbumCount++] = album;
    }

    // Japanese techno dreams
    if (purchasableAlbumCount < MAX_ALBUMS) {
        custom_strncpy(album.albumName, "Digital Dreams", MAX_NAME_LEN);
        custom_strncpy(album.artist, "Nakamura Ken", MAX_NAME_LEN);
        custom_strncpy(album.genre, "Techno", MAX_NAME_LEN);
        custom_strncpy(album.language, "Japanese", MAX_NAME_LEN);
        album.price = 26.75;
        album.soldCount = 0;
        album.songCount = 0;
        custom_strncpy(albumSong.songName, "Tokyo Cyber", MAX_NAME_LEN);
        custom_strncpy(albumSong.artist, album.artist, MAX_NAME_LEN);
        custom_strncpy(albumSong.genre, album.genre, MAX_NAME_LEN);
        custom_strncpy(albumSong.language, album.language, MAX_NAME_LEN);
        album.songs[album.songCount++] = albumSong;
        purchasableAlbums[purchasableAlbumCount++] = album;
    }

    // Premium flamenco artistry
    if (purchasableAlbumCount < MAX_ALBUMS) {
        custom_strncpy(album.albumName, "Flamenco Fantasia", MAX_NAME_LEN);
        custom_strncpy(album.artist, "Elena López", MAX_NAME_LEN);
        custom_strncpy(album.genre, "Flamenco", MAX_NAME_LEN);
        custom_strncpy(album.language, "others", MAX_NAME_LEN);
        album.price = 32.00;
        album.soldCount = 0;
        album.songCount = 0;
        custom_strncpy(albumSong.songName, "Fantasia Española", MAX_NAME_LEN);
        custom_strncpy(albumSong.artist, album.artist, MAX_NAME_LEN);
        custom_strncpy(albumSong.genre, album.genre, MAX_NAME_LEN);
        custom_strncpy(albumSong.language, album.language, MAX_NAME_LEN);
        album.songs[album.songCount++] = albumSong;
        purchasableAlbums[purchasableAlbumCount++] = album;
    }

    // Electronic dance floor energy
    if (purchasableAlbumCount < MAX_ALBUMS) {
        custom_strncpy(album.albumName, "Electro Dancefloor", MAX_NAME_LEN);
        custom_strncpy(album.artist, "Sonic Pulse", MAX_NAME_LEN);
        custom_strncpy(album.genre, "Dance Music", MAX_NAME_LEN);
        custom_strncpy(album.language, "English", MAX_NAME_LEN);
        album.price = 23.30;
        album.soldCount = 0;
        album.songCount = 0;
        custom_strncpy(albumSong.songName, "Bass Drop", MAX_NAME_LEN);
        custom_strncpy(albumSong.artist, album.artist, MAX_NAME_LEN);
        custom_strncpy(albumSong.genre, album.genre, MAX_NAME_LEN);
        custom_strncpy(albumSong.language, album.language, MAX_NAME_LEN);
        album.songs[album.songCount++] = albumSong;
        purchasableAlbums[purchasableAlbumCount++] = album;
    }

    // Chinese rock continues
    if (purchasableAlbumCount < MAX_ALBUMS) {
        custom_strncpy(album.albumName, "Night Rider", MAX_NAME_LEN);
        custom_strncpy(album.artist, "Chen Tao", MAX_NAME_LEN);
        custom_strncpy(album.genre, "Rock", MAX_NAME_LEN);
        custom_strncpy(album.language, "Chinese", MAX_NAME_LEN);
        album.price = 29.00;
        album.soldCount = 0;
        album.songCount = 0;
        custom_strncpy(albumSong.songName, "Midnight Highway", MAX_NAME_LEN);
        custom_strncpy(albumSong.artist, album.artist, MAX_NAME_LEN);
        custom_strncpy(albumSong.genre, album.genre, MAX_NAME_LEN);
        custom_strncpy(albumSong.language, album.language, MAX_NAME_LEN);
        album.songs[album.songCount++] = albumSong;
        purchasableAlbums[purchasableAlbumCount++] = album;
    }

    // More J-Pop magic
    if (purchasableAlbumCount < MAX_ALBUMS) {
        custom_strncpy(album.albumName, "Cherry Blossom Dream", MAX_NAME_LEN);
        custom_strncpy(album.artist, "Sato Yuki", MAX_NAME_LEN);
        custom_strncpy(album.genre, "Pop", MAX_NAME_LEN);
        custom_strncpy(album.language, "Japanese", MAX_NAME_LEN);
        album.price = 20.50;
        album.soldCount = 0;
        album.songCount = 0;
        custom_strncpy(albumSong.songName, "Spring Dreams", MAX_NAME_LEN);
        custom_strncpy(albumSong.artist, album.artist, MAX_NAME_LEN);
        custom_strncpy(albumSong.genre, album.genre, MAX_NAME_LEN);
        custom_strncpy(albumSong.language, album.language, MAX_NAME_LEN);
        album.songs[album.songCount++] = albumSong;
        purchasableAlbums[purchasableAlbumCount++] = album;
    }

    // Southern blues tradition
    if (purchasableAlbumCount < MAX_ALBUMS) {
        custom_strncpy(album.albumName, "Delta River Blues", MAX_NAME_LEN);
        custom_strncpy(album.artist, "Marcus King", MAX_NAME_LEN);
        custom_strncpy(album.genre, "Blues", MAX_NAME_LEN);
        custom_strncpy(album.language, "English", MAX_NAME_LEN);
        album.price = 25.90;
        album.soldCount = 0;
        album.songCount = 0;
        custom_strncpy(albumSong.songName, "River Song", MAX_NAME_LEN);
        custom_strncpy(albumSong.artist, album.artist, MAX_NAME_LEN);
        custom_strncpy(albumSong.genre, album.genre, MAX_NAME_LEN);
        custom_strncpy(albumSong.language, album.language, MAX_NAME_LEN);
        album.songs[album.songCount++] = albumSong;
        purchasableAlbums[purchasableAlbumCount++] = album;
    }

    // Korean underground hip-hop
    if (purchasableAlbumCount < MAX_ALBUMS) {
        custom_strncpy(album.albumName, "Underground Cipher", MAX_NAME_LEN);
        custom_strncpy(album.artist, "Lee Hyun", MAX_NAME_LEN);
        custom_strncpy(album.genre, "Hip-Hop", MAX_NAME_LEN);
        custom_strncpy(album.language, "Korean", MAX_NAME_LEN);
        album.price = 21.75;
        album.soldCount = 0;
        album.songCount = 0;
        custom_strncpy(albumSong.songName, "Cipher Code", MAX_NAME_LEN);
        custom_strncpy(albumSong.artist, album.artist, MAX_NAME_LEN);
        custom_strncpy(albumSong.genre, album.genre, MAX_NAME_LEN);
        custom_strncpy(albumSong.language, album.language, MAX_NAME_LEN);
        album.songs[album.songCount++] = albumSong;
        purchasableAlbums[purchasableAlbumCount++] = album;
    }

    // Futuristic techno
    if (purchasableAlbumCount < MAX_ALBUMS) {
        custom_strncpy(album.albumName, "Hyperdrive", MAX_NAME_LEN);
        custom_strncpy(album.artist, "Tech Nomad", MAX_NAME_LEN);
        custom_strncpy(album.genre, "Techno", MAX_NAME_LEN);
        custom_strncpy(album.language, "English", MAX_NAME_LEN);
        album.price = 28.40;
        album.soldCount = 0;
        album.songCount = 0;
        custom_strncpy(albumSong.songName, "Space Travel", MAX_NAME_LEN);
        custom_strncpy(albumSong.artist, album.artist, MAX_NAME_LEN);
        custom_strncpy(albumSong.genre, album.genre, MAX_NAME_LEN);
        custom_strncpy(albumSong.language, album.language, MAX_NAME_LEN);
        album.songs[album.songCount++] = albumSong;
        purchasableAlbums[purchasableAlbumCount++] = album;
    }

    // Sevillana nights
    if (purchasableAlbumCount < MAX_ALBUMS) {
        custom_strncpy(album.albumName, "Sevillana Nights", MAX_NAME_LEN);
        custom_strncpy(album.artist, "Miguel Herrera", MAX_NAME_LEN);
        custom_strncpy(album.genre, "Flamenco", MAX_NAME_LEN);
        custom_strncpy(album.language, "others", MAX_NAME_LEN);
        album.price = 31.50;
        album.soldCount = 0;
        album.songCount = 0;
        custom_strncpy(albumSong.songName, "Andalusian Night", MAX_NAME_LEN);
        custom_strncpy(albumSong.artist, album.artist, MAX_NAME_LEN);
        custom_strncpy(albumSong.genre, album.genre, MAX_NAME_LEN);
        custom_strncpy(albumSong.language, album.language, MAX_NAME_LEN);
        album.songs[album.songCount++] = albumSong;
        purchasableAlbums[purchasableAlbumCount++] = album;
    }

    // Future dance beats
    if (purchasableAlbumCount < MAX_ALBUMS) {
        custom_strncpy(album.albumName, "Pulse of Tomorrow", MAX_NAME_LEN);
        custom_strncpy(album.artist, "Aurora Beats", MAX_NAME_LEN);
        custom_strncpy(album.genre, "Dance Music", MAX_NAME_LEN);
        custom_strncpy(album.language, "English", MAX_NAME_LEN);
        album.price = 22.20;
        album.soldCount = 0;
        album.songCount = 0;
        custom_strncpy(albumSong.songName, "Future Pulse", MAX_NAME_LEN);
        custom_strncpy(albumSong.artist, album.artist, MAX_NAME_LEN);
        custom_strncpy(albumSong.genre, album.genre, MAX_NAME_LEN);
        custom_strncpy(albumSong.language, album.language, MAX_NAME_LEN);
        album.songs[album.songCount++] = albumSong;
        purchasableAlbums[purchasableAlbumCount++] = album;
    }

    // Chinese progressive rock
    if (purchasableAlbumCount < MAX_ALBUMS) {
        custom_strncpy(album.albumName, "Descendant", MAX_NAME_LEN);
        custom_strncpy(album.artist, "Wang Shu", MAX_NAME_LEN);
        custom_strncpy(album.genre, "Rock", MAX_NAME_LEN);
        custom_strncpy(album.language, "Chinese", MAX_NAME_LEN);
        album.price = 28.10;
        album.soldCount = 0;
        album.songCount = 0;
        custom_strncpy(albumSong.songName, "Legacy Path", MAX_NAME_LEN);
        custom_strncpy(albumSong.artist, album.artist, MAX_NAME_LEN);
        custom_strncpy(albumSong.genre, album.genre, MAX_NAME_LEN);
        custom_strncpy(albumSong.language, album.language, MAX_NAME_LEN);
        album.songs[album.songCount++] = albumSong;
        purchasableAlbums[purchasableAlbumCount++] = album;
    }

    // Russian blues - unique sound from the east
    if (purchasableAlbumCount < MAX_ALBUMS) {
        custom_strncpy(album.albumName, "Soviet Blues", MAX_NAME_LEN);
        custom_strncpy(album.artist, "Ivan Sokolov", MAX_NAME_LEN);
        custom_strncpy(album.genre, "Blues", MAX_NAME_LEN);
        custom_strncpy(album.language, "Russian", MAX_NAME_LEN);
        album.price = 24.70;
        album.soldCount = 0;
        album.songCount = 0;
        custom_strncpy(albumSong.songName, "Red Square Blues", MAX_NAME_LEN);
        custom_strncpy(albumSong.artist, album.artist, MAX_NAME_LEN);
        custom_strncpy(albumSong.genre, album.genre, MAX_NAME_LEN);
        custom_strncpy(albumSong.language, album.language, MAX_NAME_LEN);
        album.songs[album.songCount++] = albumSong;
        purchasableAlbums[purchasableAlbumCount++] = album;
    }

cleanup:
    savePurchasableAlbums();
}

public:
    /* Custom sorting algorithm: Bingo Sort (a variant of Selection Sort).
     Sorts an array of Song objects based on a chosen criterion (name, genre, or language).
     This fulfills the Team B - Sorting Algorithm requirement.
    */
    void binggoSortSongs(Song songs[], int n, int sortBy) {
        // Iterate through the array.
        for (int i = 0; i < n - 1; i++) {
            int min_idx = i; // Assume current element is the minimum.
            // Find the minimum element in the unsorted part of the array.
            for (int j = i + 1; j < n; j++) {
                int comparison = 0;
                // Perform comparison based on the chosen sort criterion.
                if (sortBy == 0) { // Sort by Song Name.
                    comparison = strcmp(songs[j].songName, songs[min_idx].songName);
                } else if (sortBy == 1) { // Sort by Genre.
                    comparison = strcmp(songs[j].genre, songs[min_idx].genre);
                     if (comparison == 0) { // If genres are same, sort by song name as tie-breaker.
                        comparison = strcmp(songs[j].songName, songs[min_idx].songName);
                    }
                } else if (sortBy == 2) { // Sort by Language.
                    comparison = strcmp(songs[j].language, songs[min_idx].language);
                    if (comparison == 0) { // If languages are same, sort by song name as tie-breaker.
                        comparison = strcmp(songs[j].songName, songs[min_idx].songName);
                    }
                }
                if (comparison < 0) { // Update min_idx if a smaller element is found.
                    min_idx = j;
                }
            }
            // Swap the found minimum element with the current element if they are different.
            if (min_idx != i) {
                Song temp = songs[min_idx];
                songs[min_idx] = songs[i];
                songs[i] = temp;
            }
        }
    }

    // Increments the favorite count for a specific recommended song.
    void incrementFavoriteCount(const char* songName, const char* artist) {
        for (int i = 0; i < recommendedSongCount; ++i) { // Linear search through recommended songs.
            if (strcmp(recommendedSongs[i].songName, songName) == 0 &&
                strcmp(recommendedSongs[i].artist, artist) == 0) {
                recommendedSongs[i].favoriteCount++; // Increment count.
                saveRecommendedSongs(); // Save updated data.
                return;
            }
        }
    }

    // Increments the sold count for a specific purchasable album.
    void incrementAlbumSoldCount(const char* albumName, const char* artist) {
        for (int i = 0; i < purchasableAlbumCount; ++i) { // Linear search through albums.
            if (strcmp(purchasableAlbums[i].albumName, albumName) == 0 &&
                strcmp(purchasableAlbums[i].artist, artist) == 0) {
                purchasableAlbums[i].soldCount++; // Increment count.
                savePurchasableAlbums(); // Save updated data.
                return;
            }
        }
    }
};

// Structure for a payment transaction record.
class PaymentRecord {
public:
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

    // Constructor to initialize members.
    PaymentRecord() : amount(0.0), timestamp(0), purchasedItemExpiry(0) {
        transactionId[0] = '\0'; username[0] = '\0'; itemName[0] = '\0'; itemType[0] = '\0';
        paymentMethod[0] = '\0'; paymentDetails[0] = '\0'; status[0] = '\0';
    }
};

// Manages user-specific data and actions, including playlists and interactions.
class User {
public:
    Playlist playlists[MAX_PLAYLISTS]; // Raw array for user's playlists.
    int playlistCount;
    char currentUsername[MAX_NAME_LEN];
    LinkedQueue playNextQueue; // Custom queue for managing song playback order.

    // Constructor: Initializes playlist count and username.
    User() : playlistCount(0) {
        currentUsername[0] = '\0';
    }

    // Sets the current active username.
    void setCurrentUsername(const char* username) {
        custom_strncpy(currentUsername, username, MAX_NAME_LEN);
    }

    // Gets the current Unix timestamp.
    long long getCurrentUnixTime() {
        return static_cast<long long>(time(NULL));
    }

    // Member functions for various user operations.
    void createPlaylist(AppData* appData);
    void deletePlaylist();
    void searchPlaylist(); // Implements a searching capability for playlists.
    void updatePlaylist();
    void addSong(AppData* appData);
    void deleteSong();
    void importPlaylist();
    void downloadPlaylist();
    void playPlaylist(SubscriptionManager* subManager, AppData* appData);
    void purchasePackage(SubscriptionManager* subManager, AppData* appData);
    void purchaseDigitalAlbum(SubscriptionManager* subManager, AppData* appData);
    void viewPurchaseHistory();
    int processPayment(const char* username_payer, const char* itemName, const char* itemType, double amount, long long itemExpiry);
    void savePaymentRecord(const PaymentRecord* record);
    void savePlaylists();
    void loadPlaylists();

    // Friend declarations for functions that need access to User's private members.
    friend void displayAllSongsInPlaylists(const User* user, AppData* appData);
    friend void mergePlaylistsByGenreOrLanguage(User* user, AppData* appData);
};

// Saves all playlists for the current user to a dedicated file.
void User::savePlaylists() {
    if (currentUsername[0] == '\0') {
        fprintf(stderr, "Error: Cannot save playlists, current username not set.\n");
        return;
    }
    char filename[MAX_NAME_LEN + 20];
    sprintf(filename, "%s_playlists.txt", currentUsername); // Dynamic filename based on username.
    FILE* file = fopen(filename, "w"); // Open file in write mode.
    if (!file) {
        fprintf(stderr, "Error: Could not save playlists for %s to file %s!\n", currentUsername, filename);
        return;
    }
    for (int i = 0; i < playlistCount; ++i) { // Iterate through each playlist.
        fprintf(file, "PLAYLIST_NAME:%s\n", playlists[i].name); // Write playlist name.
        for (int j = 0; j < playlists[i].songCount; ++j) { // Write each song in the playlist.
            fprintf(file, "SONG:%s,%s,%s,%s\n",
                    playlists[i].songs[j].songName,
                    playlists[i].songs[j].artist,
                    playlists[i].songs[j].genre,
                    playlists[i].songs[j].language);
        }
    }
    fclose(file);
}

// Loads playlists for the current user from their dedicated file.
void User::loadPlaylists() {
    if (currentUsername[0] == '\0') {
        return; // No user set, nothing to load.
    }
    char filename[MAX_NAME_LEN + 20];
    sprintf(filename, "%s_playlists.txt", currentUsername); // Dynamic filename.
    FILE* file = fopen(filename, "r");
    if (!file) { // If file doesn't exist, reset playlists and create new file.
        playlistCount = 0;
        FILE* newFile = fopen(filename, "w");
        if (newFile) fclose(newFile);
        return;
    }
    playlistCount = 0; // Reset existing playlists before loading.
    char line[512];
    int currentPlaylistIndex = -1; // Keep track of the playlist being parsed.
    while (fgets(line, sizeof(line), file) != NULL ) { // Read line by line.
        line[strcspn(line, "\n")] = 0; // Remove newline.
        if (strncmp(line, "PLAYLIST_NAME:", 14) == 0) { // If line defines a playlist name.
            if (playlistCount >= MAX_PLAYLISTS) { // Check max playlist capacity.
                fprintf(stderr, "Warning: Maximum playlists reached during load for %s. Further playlists in file ignored.\n", currentUsername);
                break;
            }
            char* playlist_name_data = line + 14;
            custom_strncpy(playlists[playlistCount].name, playlist_name_data, MAX_NAME_LEN);
            playlists[playlistCount].songCount = 0;
            currentPlaylistIndex = playlistCount;
            playlistCount++; // Increment count of loaded playlists.
        } else if (strncmp(line, "SONG:", 5) == 0) { // If line defines a song.
            // Add song to the current playlist.
            if (currentPlaylistIndex != -1 && playlists[currentPlaylistIndex].songCount < MAX_SONGS) {
                char* song_data = line + 5;
                Song newSong;
                char* song_data_copy = new char[strlen(song_data) + 1]; // Create mutable copy.
                if(!song_data_copy) { fprintf(stderr, "Memory allocation failed for playlist song parsing\n"); continue;}
                strcpy(song_data_copy, song_data);
                char* token = strtok(song_data_copy, ",");
                if (token) custom_strncpy(newSong.songName, token, MAX_NAME_LEN); else { delete[] song_data_copy; continue; }
                token = strtok(NULL, ",");
                if (token) custom_strncpy(newSong.artist, token, MAX_NAME_LEN); else { delete[] song_data_copy; continue; }
                token = strtok(NULL, ",");
                if (token) custom_strncpy(newSong.genre, token, MAX_NAME_LEN); else { delete[] song_data_copy; continue; }
                token = strtok(NULL, ",");
                if (token) custom_strncpy(newSong.language, token, MAX_NAME_LEN); else { delete[] song_data_copy; continue; }
                playlists[currentPlaylistIndex].songs[playlists[currentPlaylistIndex].songCount++] = newSong;
                delete[] song_data_copy;
            } else if (currentPlaylistIndex == -1) { // Song line without preceding playlist name.
                 fprintf(stderr, "Warning: SONG line encountered before PLAYLIST_NAME in %s. Skipping song.\n", filename);
            } else { // Current playlist is full.
                 fprintf(stderr, "Warning: Playlist '%s' reached max song limit (%d) during load for %s. Skipping further songs for this playlist.\n", playlists[currentPlaylistIndex].name, MAX_SONGS, currentUsername);
            }
        }
    }
    fclose(file);
}

// Saves a single payment record to 'payment_records.txt' in append mode.
void User::savePaymentRecord(const PaymentRecord* record) {
    FILE* file = fopen("payment_records.txt", "a"); // Open file in append mode.
    if (!file) {
        fprintf(stderr, "\tError opening payment_records.txt for saving!\n");
        return;
    }
    // Write payment record details in CSV format.
    fprintf(file, "%s,%s,%s,%s,%.2f,%s,%s,%lld,%s,%lld\n",
            record->transactionId, record->username, record->itemName, record->itemType, record->amount,
            record->paymentMethod, record->paymentDetails, record->timestamp, record->status, record->purchasedItemExpiry);
    fclose(file);
}

// Displays the current user's purchase history by reading from 'payment_records.txt'.
void User::viewPurchaseHistory() {
    system("cls");
    cout << "\n\t|-------------------------------|" << endl;
    cout << "\t|       PURCHASE HISTORY        |" << endl;
    cout << "\t|-------------------------------|" << endl;
    FILE* file = fopen("payment_records.txt", "r");
    if (!file) { // If file does not exist, no history.
        cout << "\n\tNo purchase history found." << endl;
        cout << "\n\tPress ENTER to return to user menu..." << endl;
        _getch();
        system("cls");
        return;
    }
    char line_buffer[512];
    int recordCount = 0;
    cout << "\n\tYour Purchase History for user: " << currentUsername << endl;
    // Define column widths and headers for formatted display.
    const int colWidths[] = { 16, 18, 15, 7, 11, 19, 10, 19 };
    const char* headers[] = { "ID", "Item Name", "Type", "Amount", "Method", "Date/Time", "Status", "Expiry Date" };
    const int numCols = sizeof(colWidths) / sizeof(colWidths[0]);
    // Print table header.
    cout << "\t";
    for (int i = 0; i < numCols; ++i) {
        cout << left << setw(colWidths[i]) << headers[i];
        if (i < numCols - 1) cout << " | ";
    }
    cout << endl;
    cout << "\t";
    for (int i = 0; i < numCols; ++i) {
        for(int k=0; k < colWidths[i]; ++k) cout << "-";
        if (i < numCols - 1) cout << "---";
    }
    cout << endl;
    while (fgets(line_buffer, sizeof(line_buffer), file) != NULL) { // Read each line.
        line_buffer[strcspn(line_buffer, "\n")] = 0; // Remove newline.
        char* line_copy = new char[strlen(line_buffer) + 1]; // Create mutable copy.
        if(!line_copy) { fprintf(stderr, "Memory allocation failed for purchase history parsing\n"); continue;}
        strcpy(line_copy, line_buffer);
        PaymentRecord record;
        char* token;
        // Parse payment record fields from CSV line.
        token = strtok(line_copy, ","); if (token) custom_strncpy(record.transactionId, token, MAX_NAME_LEN); else { delete[] line_copy; continue; }
        token = strtok(NULL, ","); if (token) custom_strncpy(record.username, token, MAX_NAME_LEN); else { delete[] line_copy; continue; }
        token = strtok(NULL, ","); if (token) custom_strncpy(record.itemName, token, MAX_NAME_LEN); else { delete[] line_copy; continue; }
        token = strtok(NULL, ","); if (token) custom_strncpy(record.itemType, token, 20); else { delete[] line_copy; continue; }
        token = strtok(NULL, ","); if (token) record.amount = atof(token); else { delete[] line_copy; continue; }
        token = strtok(NULL, ","); if (token) custom_strncpy(record.paymentMethod, token, MAX_NAME_LEN); else { delete[] line_copy; continue; }
        token = strtok(NULL, ","); if (token) custom_strncpy(record.paymentDetails, token, MAX_NAME_LEN); else { delete[] line_copy; continue; }
        token = strtok(NULL, ","); if (token) record.timestamp = atoll(token); else { delete[] line_copy; continue; }
        token = strtok(NULL, ","); if (token) custom_strncpy(record.status, token, 20); else { delete[] line_copy; continue; }
        token = strtok(NULL, ","); if (token) record.purchasedItemExpiry = atoll(token); else { delete[] line_copy; continue; }
        // Display record only if it belongs to the current user.
        if (strcmp(record.username, currentUsername) == 0) {
            char amount_str_c[20]; sprintf(amount_str_c, "%.2f", record.amount);
            char timestamp_str_buf[80];
            time_t rawtime_val = static_cast<time_t>(record.timestamp);
            struct tm* dt_val = localtime(&rawtime_val); // Convert timestamp to readable date/time.
            if (dt_val) { strftime(timestamp_str_buf, sizeof(timestamp_str_buf), "%Y-%m-%d %H:%M", dt_val); }
            else { strcpy(timestamp_str_buf, "Invalid Date"); }
            char expiry_str_buf[80];
            if (record.purchasedItemExpiry == -1) { strcpy(expiry_str_buf, "Permanent"); } // Special case for permanent.
            else if (record.purchasedItemExpiry > 0) {
                time_t expiry_rawtime = static_cast<time_t>(record.purchasedItemExpiry);
                struct tm* expiry_dt = localtime(&expiry_rawtime);
                if (expiry_dt) { strftime(expiry_str_buf, sizeof(expiry_str_buf), "%Y-%m-%d %H:%M", expiry_dt); }
                else { strcpy(expiry_str_buf, "Invalid Date"); }
            } else { strcpy(expiry_str_buf, "N/A"); } // Not applicable.
            // Print formatted row.
            cout << "\t";
            cout << left << setw(colWidths[0]) << record.transactionId << " | ";
            cout << left << setw(colWidths[1]) << record.itemName << " | ";
            cout << left << setw(colWidths[2]) << record.itemType << " | ";
            cout << right << setw(colWidths[3]) << amount_str_c << " | ";
            cout << left << setw(colWidths[4]) << record.paymentMethod << " | ";
            cout << left << setw(colWidths[5]) << timestamp_str_buf << " | ";
            cout << left << setw(colWidths[6]) << record.status << " | ";
            cout << left << setw(colWidths[7]) << expiry_str_buf << endl;
            recordCount++;
        }
        delete[] line_copy;
    }
    fclose(file);
    // Print table footer.
    cout << "\t";
    for (int i = 0; i < numCols; ++i) {
        for(int k=0; k < colWidths[i]; ++k) cout << "-";
        if (i < numCols - 1) cout << "---";
    }
    cout << endl;
    if (recordCount == 0) {
        cout << "\n\tNo purchase records found for user '" << currentUsername << "'." << endl;
    }
    cout << "\n\tPress ENTER to return to user menu..." << endl;
    _getch();
    system("cls");
}

// Allows the user to rename an existing playlist.
void User::updatePlaylist() {
    system("cls");
    cout << "\n\t|-------------------------------|" << endl;
    cout << "\t|        UPDATE PLAYLIST        |" << endl;
    cout << "\t|-------------------------------|" << endl;
    if (playlistCount == 0) { // Check if there are any playlists.
        cout << "\n\tNo playlists available to update." << endl;
        _getch(); system("cls"); return;
    }
    cout << "\n\tAvailable playlists to update:" << endl;
    for (int i = 0; i < playlistCount; i++) { // Display available playlists.
        cout << "\t" << (i + 1) << ". " << playlists[i].name << endl;
    }
    cout << "\n\t0. Go back" << endl;
    int choice;
    cout << "\n\tEnter the number of the playlist to rename: ";
    cin >> choice; // Get user choice.
    if (cin.fail()) { // Handle invalid input.
        cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "\tInvalid input!" << endl; _getch(); system("cls"); return;
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    if (choice == 0) { system("cls"); return; }
    if (choice < 1 || choice > playlistCount) { // Validate choice.
        cout << "\tInvalid playlist choice." << endl; _getch(); system("cls"); return;
    }
    int playlistIndex = choice - 1; // Get array index.
    char newName[MAX_NAME_LEN];
    cout << "\n\tCurrent name: " << playlists[playlistIndex].name << endl;
    cout << "\tEnter new name for the playlist: ";
    cin.getline(newName, MAX_NAME_LEN); // Get new playlist name.
    if (strlen(newName) == 0) { // Check if new name is empty.
        cout << "\tNew name cannot be empty. Update cancelled." << endl;
        _getch(); system("cls"); return;
    }
    for (int i = 0; i < playlistCount; i++) { // Check for duplicate playlist names.
        if (i != playlistIndex && strcmp(playlists[i].name, newName) == 0) {
            cout << "\tError: A playlist with the name '" << newName << "' already exists. Update cancelled." << endl;
            _getch(); system("cls"); return;
        }
    }
    custom_strncpy(playlists[playlistIndex].name, newName, MAX_NAME_LEN); // Update playlist name.
    cout << "\n\tPlaylist renamed to '" << newName << "' successfully!" << endl;
    savePlaylists(); // Save changes.
    _getch(); system("cls");
}

// Allows the user to add a new song to an existing playlist.
void User::addSong(AppData* appData) {
    system("cls");
    cout << "\n\t|-------------------------------|" << endl;
    cout << "\t|            ADD SONG           |" << endl;
    cout << "\t|-------------------------------|" << endl;
    if (playlistCount == 0) { // Check if there are any playlists.
        cout << "\n\tNo playlists available. Please create a playlist first." << endl;
        _getch(); system("cls"); return;
    }
    cout << "\n\tAvailable playlists:" << endl;
    for (int i = 0; i < playlistCount; i++) { // Display available playlists.
        cout << "\t" << (i + 1) << ". " << playlists[i].name << endl;
    }
    cout << "\n\t0. Go back" << endl;
    int playlistChoice;
    while (true) { // Loop for valid playlist choice.
        cout << "\n\tEnter the playlist number to add a song to (1-" << playlistCount << ", or 0 to go back): ";
        cin >> playlistChoice;
        if (cin.fail()) { cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); cout << "\tInvalid input! Please enter a number." << endl; continue; }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        if (playlistChoice == 0) { system("cls"); return; }
        if (playlistChoice < 1 || playlistChoice > playlistCount) { cout << "\tInvalid choice! Please select a valid option." << endl; } else break;
    }
    Playlist* selectedPlaylist = &playlists[playlistChoice - 1]; // Get selected playlist.
    if (selectedPlaylist->songCount >= MAX_SONGS) { // Check if playlist is full.
        cout << "\tError: Playlist \"" << selectedPlaylist->name << "\" is full! Cannot add more songs." << endl;
        _getch(); system("cls"); return;
    }
    Song newSong; // Create a new Song object.
    cout << "\n\t--- Adding Song to Playlist: " << selectedPlaylist->name << " ---" << endl;
    cout << "\tEnter song name: "; cin.getline(newSong.songName, MAX_NAME_LEN); // Get song details from user.
    if (strlen(newSong.songName) == 0) { cout << "\tSong name cannot be empty. Aborted." << endl; _getch(); system("cls"); return;}
    cout << "\tEnter artist: "; cin.getline(newSong.artist, MAX_NAME_LEN);
    if (strlen(newSong.artist) == 0) { cout << "\tArtist name cannot be empty. Aborted." << endl; _getch(); system("cls"); return;}
    int genreChoice; bool validChoice;
    do { // Loop for valid genre choice.
        cout << "\tSelect genre ("; for (int i = 0; i < appData->genreCount; i++) cout << (i + 1) << ". " << appData->genres[i] << (i == appData->genreCount -1 ? "" : ", "); cout << "): ";
        cin >> genreChoice;
        if (cin.fail()) { cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); cout << "\tInvalid input!" << endl; validChoice = false; }
        else { cin.ignore(numeric_limits<streamsize>::max(), '\n'); if (genreChoice < 1 || genreChoice > appData->genreCount) { cout << "\tInvalid choice!" << endl; validChoice = false; } else validChoice = true; }
    } while (!validChoice);
    custom_strncpy(newSong.genre, appData->genres[genreChoice - 1], MAX_NAME_LEN); // Set genre.
    int languageChoice;
    do { // Loop for valid language choice.
        cout << "\tSelect language ("; for (int i = 0; i < appData->languageCount; i++) cout << (i + 1) << ". " << appData->languages[i] << (i == appData->languageCount -1 ? "" : ", "); cout << "): ";
        cin >> languageChoice;
        if (cin.fail()) { cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); cout << "\tInvalid input!" << endl; validChoice = false; }
        else { cin.ignore(numeric_limits<streamsize>::max(), '\n'); if (languageChoice < 1 || languageChoice > appData->languageCount) { cout << "\tInvalid choice!" << endl; validChoice = false; } else validChoice = true; }
    } while (!validChoice);
    custom_strncpy(newSong.language, appData->languages[languageChoice - 1], MAX_NAME_LEN); // Set language.
    newSong.favoriteCount = 0; // Initialize favorite count.
    bool isDuplicate = false;
    for (int i = 0; i < selectedPlaylist->songCount; i++) { // Check for duplicate song in playlist.
        if (strcmp(selectedPlaylist->songs[i].songName, newSong.songName) == 0 &&
            strcmp(selectedPlaylist->songs[i].artist, newSong.artist) == 0) {
            isDuplicate = true; break;
        }
    }
    if (isDuplicate) {
        cout << "\tError: This song ('" << newSong.songName << "' by '" << newSong.artist << "') already exists in this playlist!" << endl;
    } else {
        selectedPlaylist->songs[selectedPlaylist->songCount++] = newSong; // Add song to playlist.
        cout << "\n\tSong '" << newSong.songName << "' added successfully to playlist \"" << selectedPlaylist->name << "\"!" << endl;
        savePlaylists(); // Save changes.
    }
    _getch(); system("cls");
}

// Searches for playlists by name (partial and case-insensitive matching).
// This demonstrates a custom searching functionality.
void User::searchPlaylist() {
    system("cls");
    cout << "\n\t|-------------------------------|" << endl;
    cout << "\t|        SEARCH PLAYLIST        |" << endl;
    cout << "\t|-------------------------------|" << endl;
    if (playlistCount == 0) { // Check if any playlists exist.
        cout << "\n\tNo playlists to search." << endl;
        _getch(); system("cls"); return;
    }
    char searchName[MAX_NAME_LEN];
    cout << "\n\tEnter playlist name (or part of it) to search for (case-insensitive): ";
    cin.getline(searchName, MAX_NAME_LEN); // Get search term.
    if (strlen(searchName) == 0) {
        cout << "\tSearch term cannot be empty." << endl;
        _getch(); system("cls"); return;
    }
    int foundCount = 0;
    cout << "\n\tMatching Playlists:" << endl;
    for (int i = 0; i < playlistCount; i++) { // Iterate through all playlists.
        if (custom_strstr(playlists[i].name, searchName) != NULL) { // Use custom_strstr for searching.
            cout << "\t- " << playlists[i].name << " (" << playlists[i].songCount << " songs)" << endl;
            foundCount++;
        }
    }
    if (foundCount == 0) {
        cout << "\tNo playlists found matching '" << searchName << "'." << endl;
    }
    _getch(); system("cls");
}

// Allows the user to delete an existing playlist.
void User::deletePlaylist() {
    system("cls");
    cout << "\n\t|-------------------------------|" << endl;
    cout << "\t|        DELETE PLAYLIST        |" << endl;
    cout << "\t|-------------------------------|" << endl;
    if (playlistCount == 0) { // Check if any playlists exist.
        cout << "\n\tNo playlists to delete. " << endl;
        _getch(); system("cls"); return;
    }
    cout << "\n\tAvailable playlists:" << endl;
    for (int i = 0; i < playlistCount; i++) { // Display available playlists.
        cout << "\t" << (i + 1) << ". " << playlists[i].name << " (" << playlists[i].songCount << " songs)" << endl;
    }
    cout << "\n\t0. Go back" << endl;
    int choice;
    while (true) { // Loop for valid playlist choice.
        cout << "\n\tEnter the playlist number to delete (1-" << playlistCount << ", or 0 to go back): ";
        cin >> choice;
        if (cin.fail()) { cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); cout << "\tInvalid input!" << endl; continue; }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        if (choice == 0) { system("cls"); return; }
        if (choice < 1 || choice > playlistCount) { cout << "\tInvalid choice!" << endl; } else break;
    }
    int playlistIndex = choice - 1; // Get array index.
    char confirm[10];
    cout << "\n\tAre you sure you want to delete playlist '" << playlists[playlistIndex].name << "'? (yes/no): ";
    cin.getline(confirm, 10); // Get confirmation from user.
    for (int i = 0; confirm[i] != '\0'; i++) confirm[i] = static_cast<char>(tolower(confirm[i])); // Convert to lowercase.
    if (strcmp(confirm, "yes") == 0) { // If confirmed.
        cout << "\n\tDeleting playlist: " << playlists[playlistIndex].name << endl;
        // Shift elements to the left to remove the playlist from the array.
        for (int i = playlistIndex; i < playlistCount - 1; i++) playlists[i] = playlists[i + 1];
        playlistCount--; // Decrement playlist count.
        if (playlistCount < MAX_PLAYLISTS) playlists[playlistCount] = Playlist(); // Clear last element if array shrunk.
        cout << "\n\tPlaylist deleted successfully!" << endl;
        savePlaylists(); // Save changes.
    } else {
        cout << "\n\tPlaylist deletion canceled." << endl;
    }
    _getch(); system("cls");
}

// Allows the user to delete a specific song from an existing playlist.
void User::deleteSong() {
    system("cls");
    cout << "\n\t|-------------------------------|" << endl;
    cout << "\t|          DELETE SONG          |" << endl;
    cout << "\t|-------------------------------|" << endl;
    if (playlistCount == 0) { // Check if any playlists exist.
        cout << "\n\tNo playlists available to delete songs from." << endl;
        _getch(); system("cls"); return;
    }
    cout << "\n\tAvailable playlists:" << endl;
    for (int i = 0; i < playlistCount; i++) { // Display available playlists.
        cout << "\t" << (i + 1) << ". " << playlists[i].name << " (" << playlists[i].songCount << " songs)" << endl;
    }
    cout << "\n\t0. Go back" << endl;
    int playlistChoice;
    while (true) { // Loop for valid playlist choice.
        cout << "\n\tEnter the playlist number to delete a song from (1-" << playlistCount << ", or 0 to go back): ";
        cin >> playlistChoice;
        if (cin.fail()) { cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); cout << "\tInvalid input!" << endl; continue; }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        if (playlistChoice == 0) { system("cls"); return; }
        if (playlistChoice < 1 || playlistChoice > playlistCount) { cout << "\tInvalid choice!" << endl; } else break;
    }
    Playlist* selectedPlaylist = &playlists[playlistChoice - 1]; // Get selected playlist.
    if (selectedPlaylist->songCount == 0) { // Check if playlist is empty.
        cout << "\tPlaylist \"" << selectedPlaylist->name << "\" is empty. No songs to delete." << endl;
        _getch(); system("cls"); return;
    }
    cout << "\n\t--- Songs in playlist \"" << selectedPlaylist->name << "\" ---" << endl;
    for (int i = 0; i < selectedPlaylist->songCount; i++) { // Display songs in selected playlist.
        cout << "\t" << (i + 1) << ". " << selectedPlaylist->songs[i].songName << " by " << selectedPlaylist->songs[i].artist << endl;
    }
    cout << "\n\t0. Go back to playlist selection" << endl;
    int songChoice;
    while (true) { // Loop for valid song choice.
        cout << "\n\tEnter the song number to delete (1-" << selectedPlaylist->songCount << ", or 0 to go back): ";
        cin >> songChoice;
        if (cin.fail()) { cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); cout << "\tInvalid input!" << endl; continue; }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        if (songChoice == 0) { system("cls"); deleteSong(); return; } // Go back to playlist selection.
        if (songChoice < 1 || songChoice > selectedPlaylist->songCount) { cout << "\tInvalid choice!" << endl; } else break;
    }
    int songIndex = songChoice - 1; // Get array index.
    char confirm[10];
    cout << "\n\tAre you sure you want to delete song '" << selectedPlaylist->songs[songIndex].songName << "'? (yes/no): ";
    cin.getline(confirm, 10); // Get confirmation.
    for (int i = 0; confirm[i] != '\0'; i++) confirm[i] = static_cast<char>(tolower(confirm[i]));
    if (strcmp(confirm, "yes") == 0) { // If confirmed.
        // Shift songs to the left to remove the selected song.
        for (int i = songIndex; i < selectedPlaylist->songCount - 1; i++) selectedPlaylist->songs[i] = selectedPlaylist->songs[i + 1];
        selectedPlaylist->songCount--; // Decrement song count.
        if (selectedPlaylist->songCount < MAX_SONGS) selectedPlaylist->songs[selectedPlaylist->songCount] = Song(); // Clear last element.
        cout << "\n\tSong deleted successfully from playlist \"" << selectedPlaylist->name << "\"!" << endl;
        savePlaylists(); // Save changes.
    } else {
        cout << "\n\tSong deletion canceled." << endl;
    }
    _getch(); system("cls");
}

// Imports songs from a text file into a new or existing playlist.
void User::importPlaylist() {
    system("cls");
    cout << "\n\t|-------------------------------|" << endl;
    cout << "\t|         IMPORT PLAYLIST       |" << endl;
    cout << "\t|-------------------------------|" << endl;
    cout << "\tFormat: song_name,artist,genre,language (one song per line)" << endl;
    cout << "\tNote: Genre and Language must match existing system options." << endl;
    char filename[MAX_NAME_LEN];
    cout << "\n\tEnter the filename to import songs from (e.g., import_songs.txt): ";
    cin.getline(filename, MAX_NAME_LEN); // Get filename from user.
    if(strlen(filename) == 0) { cout << "\tFilename cannot be empty. Aborted." << endl; _getch(); system("cls"); return; }
    FILE* file = fopen(filename, "r");
    if (!file) { // Handle file open error.
        cout << "\tError: Could not open file '" << filename << "'." << endl;
        _getch(); system("cls"); return;
    }
    Playlist tempImportPlaylist; // Temporary playlist to hold imported songs.
    custom_strncpy(tempImportPlaylist.name, filename, MAX_NAME_LEN); // Set playlist name from filename.
    char* dot = strrchr(tempImportPlaylist.name, '.'); // Remove file extension.
    if (dot) *dot = '\0';
    strncat(tempImportPlaylist.name, " (Imported)", MAX_NAME_LEN - strlen(tempImportPlaylist.name) -1); // Append "(Imported)".
    char line[256];
    int songsParsedFromFile = 0;
    while (fgets(line, sizeof(line), file) != NULL && tempImportPlaylist.songCount < MAX_SONGS) { // Read line by line.
        line[strcspn(line, "\n")] = 0; // Remove newline.
        if (strlen(line) == 0) continue; // Skip empty lines.
        char* line_copy = new char[strlen(line) + 1]; // Create mutable copy.
        if(!line_copy) { fprintf(stderr, "Memory allocation failed for import parsing\n"); continue;}
        strcpy(line_copy, line);
        Song importedSong; char* token;
        // Parse song details from CSV line.
        token = strtok(line_copy, ","); if (token) custom_strncpy(importedSong.songName, token, MAX_NAME_LEN); else { delete[] line_copy; continue; }
        token = strtok(NULL, ","); if (token) custom_strncpy(importedSong.artist, token, MAX_NAME_LEN); else { delete[] line_copy; continue; }
        token = strtok(NULL, ","); if (token) custom_strncpy(importedSong.genre, token, MAX_NAME_LEN); else { delete[] line_copy; continue; }
        token = strtok(NULL, ","); if (token) custom_strncpy(importedSong.language, token, MAX_NAME_LEN); else { delete[] line_copy; continue; }
        tempImportPlaylist.songs[tempImportPlaylist.songCount++] = importedSong; // Add song to temporary playlist.
        songsParsedFromFile++;
        delete[] line_copy;
    }
    fclose(file);
    if (songsParsedFromFile == 0) { // If no songs were parsed.
        cout << "\n\tNo valid songs found in '" << filename << "' or temporary import playlist is full." << endl;
        _getch(); system("cls"); return;
    }
    cout << "\n\tParsed " << songsParsedFromFile << " songs from file." << endl;
    cout << "\tDo you want to:" << endl;
    cout << "\t1. Create a new playlist named '" << tempImportPlaylist.name << "' with these songs?" << endl;
    cout << "\t2. Add these songs to an existing playlist?" << endl;
    cout << "\t0. Cancel import" << endl;
    cout << "\tEnter your choice: ";
    int importChoice; cin >> importChoice; // Get user's import action choice.
    if (cin.fail()) { cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); cout << "\tInvalid input." << endl; _getch(); system("cls"); return;}
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    if (importChoice == 0) { cout << "\tImport canceled." << endl; _getch(); system("cls"); return;}
    if (importChoice == 1) { // Create new playlist.
        if (playlistCount >= MAX_PLAYLISTS) { cout << "\tCannot create new playlist: Maximum limit reached." << endl; _getch(); system("cls"); return; }
        for(int i=0; i < playlistCount; ++i) { // Check for duplicate playlist name.
            if (strcmp(playlists[i].name, tempImportPlaylist.name) == 0) {
                cout << "\tA playlist named '" << tempImportPlaylist.name << "' already exists." << endl; _getch(); system("cls"); return;
            }
        }
        playlists[playlistCount++] = tempImportPlaylist; // Add new playlist.
        cout << "\n\tNew playlist '" << tempImportPlaylist.name << "' created with " << tempImportPlaylist.songCount << " songs." << endl;
        savePlaylists(); // Save changes.
    } else if (importChoice == 2) { // Add to existing playlist.
        if (playlistCount == 0) { cout << "\tNo existing playlists to add to." << endl; _getch(); system("cls"); return; }
        cout << "\n\tSelect a playlist to add songs to:" << endl;
        for (int i = 0; i < playlistCount; i++) cout << "\t" << (i + 1) << ". " << playlists[i].name << endl; // Display existing playlists.
        cout << "\t0. Cancel" << endl;
        cout << "\tEnter choice: ";
        int targetPlaylistChoice; cin >> targetPlaylistChoice; // Get target playlist.
        if (cin.fail()) { cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); cout << "\tInvalid input." << endl; _getch(); system("cls"); return;}
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        if (targetPlaylistChoice == 0) { cout << "\tImport to existing playlist canceled." << endl; _getch(); system("cls"); return;}
        if (targetPlaylistChoice < 1 || targetPlaylistChoice > playlistCount) { cout << "\tInvalid playlist selection." << endl; _getch(); system("cls"); return; }
        Playlist* targetPlaylist = &playlists[targetPlaylistChoice - 1]; // Get target playlist pointer.
        int songsAdded = 0, songsSkippedDuplicate = 0, songsSkippedFull = 0;
        for (int i = 0; i < tempImportPlaylist.songCount; ++i) { // Iterate through imported songs.
            if (targetPlaylist->songCount >= MAX_SONGS) { songsSkippedFull++; continue; } // Check if target playlist is full.
            bool isDuplicateInTarget = false;
            for (int j = 0; j < targetPlaylist->songCount; ++j) { // Check for duplicate song in target playlist.
                if (strcmp(targetPlaylist->songs[j].songName, tempImportPlaylist.songs[i].songName) == 0 &&
                    strcmp(targetPlaylist->songs[j].artist, tempImportPlaylist.songs[i].artist) == 0) {
                    isDuplicateInTarget = true; break;
                }
            }
            if (!isDuplicateInTarget) { targetPlaylist->songs[targetPlaylist->songCount++] = tempImportPlaylist.songs[i]; songsAdded++; } // Add if not duplicate.
            else songsSkippedDuplicate++;
        }
        cout << "\n\tImport to '" << targetPlaylist->name << "' complete." << endl;
        cout << "\tSongs added: " << songsAdded << endl;
        if (songsSkippedDuplicate > 0) cout << "\tSongs skipped (already exist in target): " << songsSkippedDuplicate << endl;
        if (songsSkippedFull > 0) cout << "\tSongs skipped (target playlist full): " << songsSkippedFull << endl;
        if (songsAdded > 0) savePlaylists(); // Save changes if songs were added.
    } else {
        cout << "\tInvalid choice for import." << endl;
    }
    _getch(); system("cls");
}

// Exports a selected playlist's songs to a text file in CSV format.
void User::downloadPlaylist() {
    system("cls");
    cout << "\n\t|-------------------------------|" << endl;
    cout << "\t|       DOWNLOAD PLAYLIST       |" << endl;
    cout << "\t|-------------------------------|" << endl;
    cout << "\t(Exports to a comma-separated file: name,artist,genre,language)" << endl;
    if (playlistCount == 0) { cout << "\n\tNo playlists available to download." << endl; _getch(); system("cls"); return; }
    cout << "\n\tAvailable playlists:" << endl;
    for (int i = 0; i < playlistCount; i++) cout << "\t" << (i + 1) << ". " << playlists[i].name << " (" << playlists[i].songCount << " songs)" << endl; // Display playlists.
    cout << "\n\t0. Go back" << endl;
    int playlistChoice;
    while (true) { // Loop for valid playlist choice.
        cout << "\n\tEnter the playlist number to download (1-" << playlistCount << ", or 0 to go back): ";
        cin >> playlistChoice;
        if (cin.fail()) { cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); cout << "\tInvalid input!" << endl; continue; }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        if (playlistChoice == 0) { system("cls"); return; }
        if (playlistChoice < 1 || playlistChoice > playlistCount) { cout << "\tInvalid choice!" << endl; } else break;
    }
    Playlist* selectedPlaylist = &playlists[playlistChoice - 1]; // Get selected playlist.
    if (selectedPlaylist->songCount == 0) { cout << "\tPlaylist \"" << selectedPlaylist->name << "\" is empty." << endl; _getch(); system("cls"); return; }
    char outputFilename[MAX_NAME_LEN + 20];
    custom_strncpy(outputFilename, selectedPlaylist->name, MAX_NAME_LEN); // Base filename on playlist name.
    for (int i = 0; outputFilename[i] != '\0'; ++i) { // Sanitize filename for common invalid characters.
        if (strchr(" /\\:*?\"<>|", outputFilename[i])) outputFilename[i] = '_';
    }
    strncat(outputFilename, "_download.txt", sizeof(outputFilename) - strlen(outputFilename) - 1); // Append "_download.txt".
    FILE* outFile = fopen(outputFilename, "w"); // Open file in write mode.
    if (!outFile) { fprintf(stderr, "\tError creating '%s'!\n", outputFilename); _getch(); system("cls"); return; }
    for (int i = 0; i < selectedPlaylist->songCount; ++i) { // Write each song to the file in CSV format.
        fprintf(outFile, "%s,%s,%s,%s\n",
                selectedPlaylist->songs[i].songName, selectedPlaylist->songs[i].artist,
                selectedPlaylist->songs[i].genre, selectedPlaylist->songs[i].language);
    }
    fclose(outFile);
    cout << "\n\tPlaylist '" << selectedPlaylist->name << "' downloaded to '" << outputFilename << "'." << endl;
    _getch(); system("cls");
}

// Plays songs from a selected playlist, handling subscription limits and utilizing the custom LinkedQueue.
void User::playPlaylist(SubscriptionManager* subManager, AppData* appData) {
    system("cls");
    cout << "\n\t|-------------------------------|" << endl;
    cout << "\t|         PLAY PLAYLIST         |" << endl;
    cout << "\t|-------------------------------|" << endl;
    if (playlistCount == 0) { cout << "\n\tNo playlists available to play." << endl; _getch(); system("cls"); return; }
    cout << "\n\tAvailable playlists:" << endl;
    for (int i = 0; i < playlistCount; i++) cout << "\t" << (i + 1) << ". " << playlists[i].name << " (" << playlists[i].songCount << " songs)" << endl; // Display playlists.
    cout << "\n\t0. Go back" << endl;
    int playlistChoice;
    while (true) { // Loop for valid playlist choice.
        cout << "\n\tEnter the playlist number to play (1-" << playlistCount << ", or 0 to go back): ";
        cin >> playlistChoice;
        if (cin.fail()) { cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); cout << "\tInvalid input!" << endl; continue; }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        if (playlistChoice == 0) { system("cls"); return; }
        if (playlistChoice < 1 || playlistChoice > playlistCount) { cout << "\tInvalid choice!" << endl; } else break;
    }
    Playlist* selectedPlaylist = &playlists[playlistChoice - 1]; // Get selected playlist.
    if (selectedPlaylist->songCount == 0) { cout << "\tPlaylist \"" << selectedPlaylist->name << "\" is empty." << endl; _getch(); system("cls"); return; }

    // Logic for Free Tier monthly song limit reset.
    AccountNode* userAccountNode = hashTableSearch(subManager->userHashTable, HASH_TABLE_SIZE, currentUsername);
    if (userAccountNode && userAccountNode->subscription.type == FREE_TIER) {
        long long currentTime = getCurrentUnixTime();
        time_t now_t = static_cast<time_t>(currentTime); struct tm* now_tm = localtime(&now_t);
        time_t last_reset_t = static_cast<time_t>(userAccountNode->subscription.lastResetUnixTime); struct tm* last_reset_tm = localtime(&last_reset_t);
        bool needsReset = false;
        if (userAccountNode->subscription.lastResetUnixTime == 0) needsReset = true; // First time playing.
        else if (now_tm && last_reset_tm && (now_tm->tm_year > last_reset_tm->tm_year || (now_tm->tm_year == last_reset_tm->tm_year && now_tm->tm_mon > last_reset_tm->tm_mon))) needsReset = true; // New month.
        if (needsReset) {
            cout << "\n\tResetting monthly free song limit for user '" << currentUsername << "'." << endl;
            userAccountNode->subscription.songsListened = 0; userAccountNode->subscription.lastResetUnixTime = currentTime; // Reset count and timestamp.
            subManager->updateUserSubscription(currentUsername, userAccountNode->subscription.type, userAccountNode->subscription.expiryUnixTime, userAccountNode->subscription.songsListened, userAccountNode->subscription.lastResetUnixTime);
            Sleep(1500);
        }
    }
    Subscription currentSub = subManager->getUserSubscription(currentUsername); // Get updated subscription info.
    int maxFreeSongsPerSession = 5, maxFreeSongsPerMonth = 100, songsPlayedThisSession = 0;
    const int MAX_PLAYED_SONGS = 1000;
    Song* playedSongs = new (nothrow) Song[MAX_PLAYED_SONGS]; // Dynamically allocated array to track played songs.
    if (!playedSongs) { fprintf(stderr, "\tMemory allocation failed for played songs tracking!\n"); _getch(); system("cls"); return; }
    int playedSongsCount = 0;

    playNextQueue.clear(); // Clear any previous songs in the queue.
    for (int i = 0; i < selectedPlaylist->songCount; ++i) playNextQueue.enqueue(selectedPlaylist->songs[i]); // Enqueue all songs from selected playlist.

    cout << "\n\tStarting playback for playlist: " << selectedPlaylist->name << endl;
    cout << "\t------------------------------------------------" << endl;
    char controlInput = '\0', animationChars[] = "|/-\\"; int animIndex = 0;
    const int songPlayDurationMs = 5000, inputCheckIntervalMs = 200; // Define song duration and check interval.

    while (!playNextQueue.isEmpty()) { // Loop while there are songs in the queue.
        Song currentSongToPlay = *playNextQueue.peek(); // Get the song at the front of the queue without removing it.
        currentSub = subManager->getUserSubscription(currentUsername); // Re-fetch subscription for real-time check.
        bool isFreeTier = (currentSub.type == FREE_TIER);
        bool isSubscriptionExpired = (currentSub.type != PERMANENT && currentSub.type != FREE_TIER && currentSub.expiryUnixTime > 0 && getCurrentUnixTime() > currentSub.expiryUnixTime);
        
        // Check free tier limits or subscription expiry.
        if (isFreeTier && (songsPlayedThisSession >= maxFreeSongsPerSession || currentSub.songsListened >= maxFreeSongsPerMonth)) {
            cout << "\n\t------------------------------------------------" << endl; cout << "\tFREE TIER LIMIT REACHED!" << endl;
            if (songsPlayedThisSession >= maxFreeSongsPerSession) cout << "\tMax " << maxFreeSongsPerSession << " songs this session." << endl;
            if (currentSub.songsListened >= maxFreeSongsPerMonth) cout << "\tMax " << currentSub.songsListened << " songs this month." << endl;
            cout << "\tPlease purchase a subscription." << endl; cout << "\t------------------------------------------------" << endl; break; // Stop playback.
        }
        if (isSubscriptionExpired) {
             cout << "\n\t------------------------------------------------" << endl; cout << "\tYOUR SUBSCRIPTION HAS EXPIRED!" << endl;
             cout << "\tPlease renew your subscription." << endl; cout << "\t------------------------------------------------" << endl; break; // Stop playback.
        }

        cout << "\n"; controlInput = '\0'; int elapsedTimeMs = 0; bool songInterruptedByUser = false, songPaused = false;
        while(elapsedTimeMs < songPlayDurationMs) { // Simulate song playing duration.
            if (songPaused) { // Handle pause state.
                 cout << "\r\tPAUSED: " << currentSongToPlay.songName << ". Press any key to resume or 'N'/'Q'."; fflush(stdout);
                 if (_kbhit()) { // Check for user input.
                    char resumeKey = static_cast<char>(tolower(_getch()));
                    if (resumeKey == 'n' || resumeKey == 'q') { controlInput = resumeKey; songInterruptedByUser = true; cout << endl; break; }
                    songPaused = false; cout << "\r                                                                                         \r"; // Clear pause message.
                 }
                 Sleep(inputCheckIntervalMs); continue;
            }
            // Display current playing song with animation and controls.
            cout << "\r\tNow Playing: " << currentSongToPlay.songName << " by " << currentSongToPlay.artist << " [" << currentSongToPlay.genre << ", " << currentSongToPlay.language << "] [" << animationChars[animIndex] << "] (N:Next, P:Pause, Q:Quit) ";
            fflush(stdout); animIndex = (animIndex + 1) % 4; // Update animation character.
            if (_kbhit()) { // Check for user input for controls.
                controlInput = static_cast<char>(tolower(_getch()));
                if (controlInput == 'p') songPaused = true;
                else if (controlInput == 'n' || controlInput == 'q') { songInterruptedByUser = true; cout << endl; break; }
                else controlInput = '\0'; // Clear other inputs.
            }
            Sleep(inputCheckIntervalMs); // Pause for interval.
            if (!songPaused) elapsedTimeMs += inputCheckIntervalMs; // Increment elapsed time if not paused.
        }

        // Display message after song finishes or is interrupted.
        if (!songInterruptedByUser && !songPaused) cout << "\r\tFinished:    " << currentSongToPlay.songName << " by " << currentSongToPlay.artist << "                                                               \n";
        else if (songPaused && !songInterruptedByUser) cout << "\r\tSong " << currentSongToPlay.songName << " was paused. Moving to next.   \n";
        
        // Add played song to tracking array.
        if (playedSongsCount < MAX_PLAYED_SONGS) {
            playedSongs[playedSongsCount++] = currentSongToPlay;
        } else {
            fprintf(stderr, "\tWarning: Exceeded played songs capacity.\n");
        }

        playNextQueue.dequeue(); // Remove the played song from the queue.
        appData->incrementFavoriteCount(currentSongToPlay.songName, currentSongToPlay.artist); // Increment favorite count.

        // Update free tier usage.
        if (isFreeTier) {
            currentSub.songsListened++; songsPlayedThisSession++;
            subManager->updateUserSubscription(currentUsername, currentSub.type, currentSub.expiryUnixTime, currentSub.songsListened, currentSub.lastResetUnixTime);
            cout << "\t(Free Tier: Session [" << songsPlayedThisSession << "/" << maxFreeSongsPerSession << "], Monthly [" << currentSub.songsListened << "/" << maxFreeSongsPerMonth << "])" << endl;
        }

        if (controlInput == 'q') { cout << "\n\tPlayback stopped." << endl; break; } // Quit playback if 'q' pressed.
        else if (controlInput == 'n') cout << "\n\tSkipping to next..." << endl; // Skip to next song if 'n' pressed.
    }
    if (playNextQueue.isEmpty() && controlInput != 'q') cout << "\n\tEnd of playlist '" << selectedPlaylist->name << "'." << endl;

    // Save played songs to 'sorted_information.txt' (part of Team B output requirement).
    if (playedSongsCount > 0) {
        FILE* sortedInfoFile = fopen("sorted_information.txt", "w");
        if (sortedInfoFile) {
            fprintf(sortedInfoFile, "# Played Songs from Playlist: %s - User: %s\n", selectedPlaylist->name, currentUsername);
            fprintf(sortedInfoFile, "# Title,Artist,Genre,Language\n");
            for (int i = 0; i < playedSongsCount; ++i) {
                fprintf(sortedInfoFile, "%s,%s,%s,%s\n",
                    playedSongs[i].songName,
                    playedSongs[i].artist,
                    playedSongs[i].genre,
                    playedSongs[i].language);
            }
            fclose(sortedInfoFile);
            cout << "\n\tPlayed songs (" << playedSongsCount << ") saved to 'sorted_information.txt'." << endl;
        } else {
            fprintf(stderr, "\tError saving played songs to 'sorted_information.txt'!\n");
        }
    }
    delete[] playedSongs; // Free dynamically allocated memory.
    _getch(); system("cls");
}

// Helper struct to hold wrapped text lines.
struct WrappedLines { char lines[10][MAX_NAME_LEN]; int lineCount; };

// Wraps a given text string into multiple lines based on a specified width.
WrappedLines wrapText(const char* text, int width) {
    WrappedLines result; result.lineCount = 0;
    int textLen = 0; while (text[textLen] != '\0') textLen++; // Get text length manually.
    if (width <= 0) { if (textLen > 0 && result.lineCount < 10) custom_strncpy(result.lines[result.lineCount++], text, MAX_NAME_LEN); return result; }
    int currentPos = 0;
    while (currentPos < textLen && result.lineCount < 10) { // Loop while text remains and line limit not reached.
        int remainingLen = textLen - currentPos; int lenToCopy = (remainingLen < width) ? remainingLen : width; int actualBreakPoint = currentPos + lenToCopy;
        if (lenToCopy == width && actualBreakPoint < textLen) { // If breaking exactly at width, try to break at a space.
            int potentialBreak = -1;
            for (int i = actualBreakPoint -1; i > currentPos; --i) if (text[i] == ' ') { potentialBreak = i; break; }
            if (potentialBreak != -1) { lenToCopy = potentialBreak - currentPos; actualBreakPoint = potentialBreak; }
        }
        for (int i = 0; i < lenToCopy; ++i) result.lines[result.lineCount][i] = text[currentPos + i]; // Copy characters.
        result.lines[result.lineCount][lenToCopy] = '\0'; result.lineCount++; // Null-terminate and increment line count.
        currentPos = actualBreakPoint;
        if (currentPos < textLen && text[currentPos] == ' ') currentPos++; // Skip leading space on next line.
    }
    return result;
}

// Prints a separator line for table formatting.
void printSeparatorLine(int totalWidth) {
    cout << "\t"; for (int i = 0; i < totalWidth; i++) cout << "-"; cout << endl;
}

// Displays a song's details across multiple lines if needed for table formatting.
void displayMultiLineRow(int rowNum, const Song& song, int colNo, int colTitle, int colArtist, int colGenre, int colLanguage) {
    WrappedLines titleLines = wrapText(song.songName, colTitle); WrappedLines artistLines = wrapText(song.artist, colArtist);
    WrappedLines genreLines = wrapText(song.genre, colGenre); WrappedLines languageLines = wrapText(song.language, colLanguage);
    int maxLines = titleLines.lineCount; // Determine max lines needed across all columns.
    if (artistLines.lineCount > maxLines) maxLines = artistLines.lineCount; if (genreLines.lineCount > maxLines) maxLines = genreLines.lineCount;
    if (languageLines.lineCount > maxLines) maxLines = languageLines.lineCount; if (maxLines == 0 && (strlen(song.songName) > 0 || strlen(song.artist) > 0)) maxLines = 1; // Ensure at least one line if content exists.
    for (int line = 0; line < maxLines; line++) { // Iterate for each line of the multi-line row.
        cout << "\t";
        if (line == 0) cout << right << setw(colNo) << rowNum; else for (int i = 0; i < colNo; i++) cout << " "; cout << " | "; // Print row number only on first line.
        // Print wrapped text for each column.
        if (line < titleLines.lineCount) cout << left << setw(colTitle) << titleLines.lines[line]; else for (int i = 0; i < colTitle; i++) cout << " "; cout << " | ";
        if (line < artistLines.lineCount) cout << left << setw(colArtist) << artistLines.lines[line]; else for (int i = 0; i < colArtist; i++) cout << " "; cout << " | ";
        if (line < genreLines.lineCount) cout << left << setw(colGenre) << genreLines.lines[line]; else for (int i = 0; i < colGenre; i++) cout << " "; cout << " | ";
        if (line < languageLines.lineCount) cout << left << setw(colLanguage) << languageLines.lines[line]; else for (int i = 0; i < colLanguage; i++) cout << " ";
        cout << endl;
    }
}

// Displays all songs from all playlists, with an option to sort them.
// This fulfills the Team B - Display Sorted Data requirement.
void displayAllSongsInPlaylists(const User* user, AppData* appData) {
    system("cls");
    cout << "\n\t|-------------------------------|" << endl;
    cout << "\t|      ALL SONGS IN PLAYLISTS   |" << endl;
    cout << "\t|-------------------------------|" << endl;
    if (user->playlistCount == 0) { cout << "\n\tNo playlists available." << endl; _getch(); system("cls"); return; }
    const int MAX_TOTAL_SONGS = MAX_PLAYLISTS * MAX_SONGS;
    Song* allSongs = new (nothrow) Song[MAX_TOTAL_SONGS]; // Dynamically allocated array to collect all songs.
    if (!allSongs) { fprintf(stderr, "\tMemory allocation failed!\n"); _getch(); system("cls"); return; }
    int totalSongCount = 0;
    // Collect all songs from all playlists into a single array.
    for (int i = 0; i < user->playlistCount; ++i) {
        for (int j = 0; j < user->playlists[i].songCount; ++j) {
            if (totalSongCount < MAX_TOTAL_SONGS) allSongs[totalSongCount++] = user->playlists[i].songs[j];
            else { fprintf(stderr, "\tWarning: Exceeded song capacity.\n"); goto end_collection_loop; } // Exit if total song capacity reached.
        }
    }
end_collection_loop:; // Label for goto.
    if (totalSongCount == 0) { cout << "\n\tNo songs in any playlists." << endl; delete[] allSongs; _getch(); system("cls"); return; }

    // Prompt user for sorting choice.
    cout << "\n\tSort by: 1. Name, 2. Genre, 3. Language, 0. Unsorted: "; int sortChoice;
    cin >> sortChoice;
    if (cin.fail()) { cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); cout << "\tInvalid. Displaying unsorted." << endl; sortChoice = 0; }
    else { cin.ignore(numeric_limits<streamsize>::max(), '\n'); if (sortChoice < 0 || sortChoice > 3) { cout << "\tInvalid. Unsorted." << endl; sortChoice = 0; }}
    
    const char* sortByStr = "Unsorted";
    if (sortChoice >= 1 && sortChoice <= 3) { // If a valid sort choice is made.
        appData->binggoSortSongs(allSongs, totalSongCount, sortChoice - 1); // Apply the custom Bingo Sort.
        if (sortChoice == 1) sortByStr = "Name"; else if (sortChoice == 2) sortByStr = "Genre"; else if (sortChoice == 3) sortByStr = "Language";
        
        // Save sorted data to 'sorted_information.txt' (Team B output requirement).
        FILE* sortedInfoFile = fopen("sorted_information.txt", "w");
        if (sortedInfoFile) {
            fprintf(sortedInfoFile, "# All Songs (Sorted by: %s) - User: %s\n# Title,Artist,Genre,Language\n", sortByStr, user->currentUsername);
            for (int i = 0; i < totalSongCount; ++i) fprintf(sortedInfoFile, "%s,%s,%s,%s\n", allSongs[i].songName, allSongs[i].artist, allSongs[i].genre, allSongs[i].language);
            fclose(sortedInfoFile); cout << "\n\tSorted data saved to 'sorted_information.txt'." << endl;
        } else fprintf(stderr, "\tError saving 'sorted_information.txt'!\n");
    }

    // Define column widths for display table.
    const int COL_NO = 4, COL_TITLE = 25, COL_ARTIST = 20, COL_GENRE = 12, COL_LANGUAGE = 10;
    const int SEPARATORS_WIDTH = 4 * 3, TOTAL_WIDTH = COL_NO + COL_TITLE + COL_ARTIST + COL_GENRE + COL_LANGUAGE + SEPARATORS_WIDTH;
    // Print table header.
    printSeparatorLine(TOTAL_WIDTH); cout << "\tAll Songs (" << totalSongCount << " total, Sorted by: " << sortByStr << "):" << endl; printSeparatorLine(TOTAL_WIDTH);
    cout << "\t" << left << setw(COL_NO) << "No." << " | " << setw(COL_TITLE) << "Title" << " | " << setw(COL_ARTIST) << "Artist" << " | " << setw(COL_GENRE) << "Genre" << " | " << setw(COL_LANGUAGE) << "Language" << endl;
    cout << "\t"; for (int i=0;i<COL_NO;i++)cout<<"-";cout<<"-+-";for(int i=0;i<COL_TITLE;i++)cout<<"-";cout<<"-+-";for(int i=0;i<COL_ARTIST;i++)cout<<"-";cout<<"-+-";for(int i=0;i<COL_GENRE;i++)cout<<"-";cout<<"-+-";for(int i=0;i<COL_LANGUAGE;i++)cout<<"-";cout<<endl;
    
    char lastGroup[MAX_NAME_LEN] = ""; // Keep track of last displayed group for grouped sorting.
    for (int i = 0; i < totalSongCount; ++i) { // Display each song.
        // Add group headers if sorting by genre or language and a new group starts.
        if (sortChoice == 2 && strcmp(allSongs[i].genre, lastGroup) != 0) { if (i>0) printSeparatorLine(TOTAL_WIDTH); cout << "\t      Genre: " << allSongs[i].genre << endl; printSeparatorLine(TOTAL_WIDTH); custom_strncpy(lastGroup, allSongs[i].genre, MAX_NAME_LEN); }
        else if (sortChoice == 3 && strcmp(allSongs[i].language, lastGroup) != 0) { if (i>0) printSeparatorLine(TOTAL_WIDTH); cout << "\t      Language: " << allSongs[i].language << endl; printSeparatorLine(TOTAL_WIDTH); custom_strncpy(lastGroup, allSongs[i].language, MAX_NAME_LEN); }
        displayMultiLineRow(i + 1, allSongs[i], COL_NO, COL_TITLE, COL_ARTIST, COL_GENRE, COL_LANGUAGE);
        
        bool nextIsNewGroup = false; // Check if next song starts a new group.
        if (i < totalSongCount - 1) { if (sortChoice == 2 && strcmp(allSongs[i].genre, allSongs[i+1].genre) != 0) nextIsNewGroup = true; if (sortChoice == 3 && strcmp(allSongs[i].language, allSongs[i+1].language) != 0) nextIsNewGroup = true; }
        if (i < totalSongCount - 1 && !nextIsNewGroup) { cout << "\t"; for (int j=0; j<TOTAL_WIDTH; j++) cout<<"."; cout<<endl; } // Print separator dots.
    }
    printSeparatorLine(TOTAL_WIDTH);

    // Option to save the displayed list to a custom file.
    char saveChoice[10]; cout << "\n\tSave list to custom file? (yes/no): "; cin.getline(saveChoice, 10);
    for (int k = 0; saveChoice[k] != '\0'; k++) if (saveChoice[k] >= 'A' && saveChoice[k] <= 'Z') saveChoice[k] += ('a' - 'A');
    if (strcmp(saveChoice, "yes") == 0) {
        char outputFilename[MAX_NAME_LEN]; cout << "\tEnter custom filename: "; cin.getline(outputFilename, MAX_NAME_LEN);
        if(strlen(outputFilename) == 0) strcpy(outputFilename, "all_songs_custom.txt"); // Default filename.
        FILE* outFile = fopen(outputFilename, "w");
        if (!outFile) fprintf(stderr, "\tError creating '%s'!\n", outputFilename);
        else {
            fprintf(outFile, "# All Songs (Sorted: %s) - User: %s\n# Title,Artist,Genre,Language\n", sortByStr, user->currentUsername);
            for (int k=0; k<totalSongCount; ++k) fprintf(outFile, "%s,%s,%s,%s\n", allSongs[k].songName, allSongs[k].artist, allSongs[k].genre, allSongs[k].language);
            fclose(outFile); cout << "\n\tSongs saved to " << outputFilename << endl;
        }
    } else cout << "\tCustom saving canceled." << endl;
    delete[] allSongs; // Free dynamically allocated memory.
    _getch(); system("cls");
}

// Merges songs from existing playlists into a new playlist based on genre or language.
// This can be seen as a complex search/filter operation combined with playlist creation.
void mergePlaylistsByGenreOrLanguage(User* user, AppData* appData) {
    system("cls");
    cout << "\n\t|-------------------------------|" << endl;
    cout << "\t|       MERGE PLAYLISTS         |" << endl;
    cout << "\t|-------------------------------|" << endl;
    if (user->playlistCount < 1) { cout << "\n\tNeed at least one playlist." << endl; _getch(); system("cls"); return; }

    // Choose merge criterion: Genre or Language.
    cout << "\n\tMerge by: 1. Genre, 2. Language, 0. Cancel: "; int mergeTypeChoice;
    cin >> mergeTypeChoice; if (cin.fail()) { cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); cout << "\tInvalid." << endl; _getch(); system("cls"); return; }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    if (mergeTypeChoice == 0) { system("cls"); return;} if (mergeTypeChoice != 1 && mergeTypeChoice != 2) { cout << "\tInvalid." << endl; _getch(); system("cls"); return; }

    char mergeCriterionValue[MAX_NAME_LEN]; const char* criterionTypeStr = (mergeTypeChoice == 1) ? "Genre" : "Language";
    int maxSystemItems = (mergeTypeChoice == 1) ? appData->genreCount : appData->languageCount;
    char (*systemItems)[MAX_NAME_LEN] = (mergeTypeChoice == 1) ? appData->genres : appData->languages;

    // Display available system genres/languages or allow custom input.
    cout << "\n\tAvailable " << criterionTypeStr << "s (System Defaults):" << endl;
    for(int i=0; i < maxSystemItems; ++i) cout << "\t" << (i+1) << ". " << systemItems[i] << endl;
    cout << "\t" << (maxSystemItems + 1) << ". Enter custom " << criterionTypeStr << " string" << endl; cout << "\t0. Cancel" << endl;
    cout << "\tSelect " << criterionTypeStr << " number or enter custom: "; int critChoice; cin >> critChoice;
    if (cin.fail()) { cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); cout << "\tInvalid." << endl; _getch(); system("cls"); return;} cin.ignore(numeric_limits<streamsize>::max(), '\n');
    if (critChoice == 0) { system("cls"); return;}
    if (critChoice > 0 && critChoice <= maxSystemItems) custom_strncpy(mergeCriterionValue, systemItems[critChoice-1], MAX_NAME_LEN);
    else if (critChoice == maxSystemItems + 1) { // User chose to enter custom string.
        cout << "\tEnter custom " << criterionTypeStr << ": "; cin.getline(mergeCriterionValue, MAX_NAME_LEN);
        if(strlen(mergeCriterionValue) == 0) { cout << "\tCannot be empty." << endl; _getch(); system("cls"); return;}
    } else { cout << "\tInvalid." << endl; _getch(); system("cls"); return; }

    if (user->playlistCount >= MAX_PLAYLISTS) { cout << "\n\tMax playlists reached." << endl; _getch(); system("cls"); return; }
    
    Playlist mergedPlaylist; // New playlist to store merged songs.
    char mergedPlaylistName[MAX_NAME_LEN + 30];
    sprintf(mergedPlaylistName, "Merged by %s - %s", criterionTypeStr, mergeCriterionValue); // Generate name for merged playlist.
    if (strlen(mergedPlaylistName) >= MAX_NAME_LEN) mergedPlaylistName[MAX_NAME_LEN - 1] = '\0';
    custom_strncpy(mergedPlaylist.name, mergedPlaylistName, MAX_NAME_LEN);
    
    int songsAddedToMerged = 0;
    // Iterate through all existing playlists and their songs to find matches.
    for (int i = 0; i < user->playlistCount; ++i) {
        for (int j = 0; j < user->playlists[i].songCount; ++j) {
            const Song& currentSong = user->playlists[i].songs[j];
            // Check if the song matches the selected criterion (genre or language), case-insensitively.
            bool matchesCriterion = (mergeTypeChoice == 1) ? (custom_strcmpi(currentSong.genre, mergeCriterionValue) == 0) : (custom_strcmpi(currentSong.language, mergeCriterionValue) == 0);
            if (matchesCriterion) {
                bool isDuplicateInMerged = false;
                // Check if the song is already in the merged playlist to avoid duplicates.
                for (int k=0; k<mergedPlaylist.songCount; ++k) if (strcmp(mergedPlaylist.songs[k].songName, currentSong.songName)==0 && strcmp(mergedPlaylist.songs[k].artist, currentSong.artist)==0) { isDuplicateInMerged=true; break; }
                if (!isDuplicateInMerged) {
                    if (mergedPlaylist.songCount < MAX_SONGS) { mergedPlaylist.songs[mergedPlaylist.songCount++] = currentSong; songsAddedToMerged++; } // Add unique song.
                    else { cout << "\n\tWarning: Merged playlist full." << endl; goto end_merge_loops; } // Stop if merged playlist is full.
                }
            }
        }
    }
end_merge_loops:; // Label for goto.

    if (songsAddedToMerged > 0) {
        int existingPlaylistIndex = -1;
        // Check if a playlist with the merged name already exists.
        for (int i=0; i<user->playlistCount; ++i) if (strcmp(user->playlists[i].name, mergedPlaylist.name)==0) { existingPlaylistIndex=i; break; }
        
        if (existingPlaylistIndex != -1) { // If playlist exists, offer to add unique songs to it.
            cout << "\n\tPlaylist '" << mergedPlaylist.name << "' already exists. Add " << songsAddedToMerged << " new unique songs? (yes/no): ";
            char choice[10]; cin.getline(choice, 10); for(int k=0;choice[k]!='\0';k++) choice[k]=static_cast<char>(tolower(choice[k]));
            if (strcmp(choice, "yes") == 0) {
                int actuallyAdded=0; Playlist* target = &user->playlists[existingPlaylistIndex];
                for(int i=0; i<mergedPlaylist.songCount; ++i) { // Iterate through newly merged songs.
                    bool exists=false;
                    for(int j=0; j<target->songCount; ++j) if(strcmp(target->songs[j].songName, mergedPlaylist.songs[i].songName)==0 && strcmp(target->songs[j].artist, mergedPlaylist.songs[i].artist)==0) {exists=true; break;} // Check for duplicates in target playlist.
                    if (!exists && target->songCount < MAX_SONGS) { target->songs[target->songCount++] = mergedPlaylist.songs[i]; actuallyAdded++; } // Add if unique and space available.
                    else if (!exists) cout << "\tSkipped '" << mergedPlaylist.songs[i].songName << "' (playlist full)." << endl;
                }
                cout << "\n\tAdded " << actuallyAdded << " new songs." << endl; if(actuallyAdded>0) user->savePlaylists(); // Save if any songs were added.
            } else cout << "\n\tMerging canceled." << endl;
        } else { // If merged playlist does not exist, create it.
             if (user->playlistCount < MAX_PLAYLISTS) { user->playlists[user->playlistCount++] = mergedPlaylist; cout << "\n\tCreated '" << mergedPlaylist.name << "' with " << mergedPlaylist.songCount << " songs." << endl; user->savePlaylists(); }
             else cout << "\n\tCould not create. Max playlists." << endl; // Cannot create if max playlists reached.
        }
    } else cout << "\n\tNo songs found matching '" << mergeCriterionValue << "'." << endl; // No songs matched the criterion.
    _getch(); system("cls");
}

// Allows the user to purchase a subscription package.
void User::purchasePackage(SubscriptionManager* subManager, AppData* appData) {
    system("cls");
    cout << "\n\t|-------------------------------|" << endl;
    cout << "\t|        PURCHASE PACKAGE       |" << endl;
    cout << "\t|-------------------------------|" << endl;
    Subscription currentSub = subManager->getUserSubscription(currentUsername); // Get current subscription status.
    cout << "\n\tCurrent Package: ";
    long long currentExpiry = currentSub.expiryUnixTime, lastResetTime = currentSub.lastResetUnixTime;
    // Display current package type.
    if (currentSub.type == FREE_TIER) cout << "Free Tier"; else if (currentSub.type == ONE_MONTH) cout << "1-Month"; else if (currentSub.type == THREE_MONTH) cout << "3-Month"; else if (currentSub.type == PERMANENT) cout << "Permanent";
    cout << endl;
    // Display expiry or free tier limits.
    if (currentSub.type != PERMANENT && currentSub.type != FREE_TIER && currentSub.expiryUnixTime > 0) {
        long long now = getCurrentUnixTime(); if (currentSub.expiryUnixTime < now) cout << "\t(Expired!)" << endl;
        else { long long rem = currentSub.expiryUnixTime - now; cout << "\t(Expires in " << rem/(86400) << "d " << (rem%(86400))/3600 << "h)" << endl; }
    } else if (currentSub.type == FREE_TIER) cout << "\t(Listened: " << currentSub.songsListened << ". Remaining: " << (100-currentSub.songsListened > 0 ? 100-currentSub.songsListened : 0) << ")" << endl;
    
    // Display available packages.
    cout << "\n\tAvailable Packages:" << endl;
    cout << "\t1. 1-Month: RM10.00" << endl; cout << "\t2. 3-Month: RM25.00" << endl; cout << "\t3. Permanent: RM100.00" << endl; cout << "\t0. Back" << endl;
    int choice; double price=0.0; PackageType selectedType=FREE_TIER; char pkgName[MAX_NAME_LEN]="";
    cout << "\n\tEnter choice: "; cin >> choice; // Get user choice.
    if (cin.fail()) { cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); cout << "\tInvalid." << endl; _getch(); system("cls"); return; }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    if (choice == 0) { system("cls"); return; }
    // Set package details based on choice.
    else if (choice == 1) { selectedType = ONE_MONTH; price = 10.00; custom_strncpy(pkgName, "1-Month Package", MAX_NAME_LEN); }
    else if (choice == 2) { selectedType = THREE_MONTH; price = 25.00; custom_strncpy(pkgName, "3-Month Package", MAX_NAME_LEN); }
    else if (choice == 3) { selectedType = PERMANENT; price = 100.00; custom_strncpy(pkgName, "Permanent Package", MAX_NAME_LEN); }
    else { cout << "\tInvalid choice." << endl; _getch(); system("cls"); return; }

    if (currentSub.type == PERMANENT) { cout << "\tAlready permanent." << endl; _getch(); system("cls"); return; } // Cannot upgrade from permanent.
    
    long long newExpiry = getCurrentUnixTime();
    long long newLastReset = (selectedType == FREE_TIER) ? lastResetTime : getCurrentUnixTime(); // Reset last reset time for new subscription.
    if (selectedType == PERMANENT) newExpiry = -1; // -1 indicates permanent.
    else { int days = 0; if (selectedType == ONE_MONTH) days=30; else if (selectedType == THREE_MONTH) days=90;
           // If current subscription is not free/permanent and not expired, extend from current expiry.
           if (currentSub.type != FREE_TIER && currentSub.type != PERMANENT && currentExpiry > newExpiry) newExpiry = currentExpiry;
           newExpiry += (long long)days * 86400; // Add days in seconds.
    }
    
    // Process payment via payment gateway.
    if (processPayment(currentUsername, pkgName, "Subscription", price, newExpiry)) {
        int newSongsListened = (selectedType == FREE_TIER) ? currentSub.songsListened : 0; // Reset songs listened if not free tier.
        long long newEffLastReset = (selectedType == FREE_TIER) ? currentSub.lastResetUnixTime : getCurrentUnixTime();
        // Update user subscription in hash table.
        subManager->updateUserSubscription(currentUsername, selectedType, newExpiry, newSongsListened, newEffLastReset);
        cout << "\n\tPackage '" << pkgName << "' purchased!" << endl;
        if (selectedType == PERMANENT) cout << "\tPermanent subscription active." << endl;
        else if (selectedType != FREE_TIER) { // Display new expiry date.
             time_t expiry_t = static_cast<time_t>(newExpiry); struct tm* dt = localtime(&expiry_t); char expStr[30];
             if (dt) strftime(expStr, sizeof(expStr), "%Y-%m-%d %H:%M", dt); else strcpy(expStr, "unknown");
             cout << "\tExpires on: " << expStr << endl;
        }
    } else cout << "\n\tPayment failed." << endl;
    _getch(); system("cls");
}

// Allows the user to purchase a digital music album.
void User::purchaseDigitalAlbum(SubscriptionManager* subManager, AppData* appData) {
    system("cls");
    cout << "\n\t|-------------------------------|" << endl;
    cout << "\t|     PURCHASE DIGITAL ALBUM    |" << endl;
    cout << "\t|-------------------------------|" << endl;
    if (appData->purchasableAlbumCount == 0) { cout << "\n\tNo albums available." << endl; _getch(); system("cls"); return; }
    cout << "\n\tAvailable Albums:" << endl;
    for (int i = 0; i < appData->purchasableAlbumCount; i++) { // Display available albums.
        cout << "\t" << (i + 1) << ". " << appData->purchasableAlbums[i].albumName << " by " << appData->purchasableAlbums[i].artist
             << " - RM" << fixed << setprecision(2) << appData->purchasableAlbums[i].price
             << " (" << appData->purchasableAlbums[i].songCount << " songs)" << endl;
    }
    cout << "\t0. Back" << endl; int choice; cout << "\n\tEnter choice: "; cin >> choice; // Get user choice.
    if (cin.fail()) { cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); cout << "\tInvalid." << endl; _getch(); system("cls"); return; }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    if (choice == 0) { system("cls"); return; }
    if (choice < 1 || choice > appData->purchasableAlbumCount) { cout << "\tInvalid choice." << endl; _getch(); system("cls"); return; }
    Album selectedAlbum = appData->purchasableAlbums[choice - 1]; // Get selected album.
    
    char purchasedPlaylistName[MAX_NAME_LEN + 20]; sprintf(purchasedPlaylistName, "Purchased - %s", selectedAlbum.albumName); // Generate playlist name for purchased album.
    if (strlen(purchasedPlaylistName) >= MAX_NAME_LEN) purchasedPlaylistName[MAX_NAME_LEN-1] = '\0';
    // Check if album has already been purchased (playlist with same name exists).
    for (int i = 0; i < playlistCount; ++i) if (strcmp(playlists[i].name, purchasedPlaylistName) == 0) { cout << "\tAlready own this album." << endl; _getch(); system("cls"); return; }
    
    // Process payment.
    if (processPayment(currentUsername, selectedAlbum.albumName, "Album", selectedAlbum.price, -1)) { // -1 for item expiry means permanent ownership.
        if (playlistCount >= MAX_PLAYLISTS) { // Check for playlist limit.
            cout << "\tAlbum purchased, but max playlists reached! Cannot add album to playlists." << endl; appData->incrementAlbumSoldCount(selectedAlbum.albumName, selectedAlbum.artist); _getch(); system("cls"); return; }
        
        Playlist newAlbumPlaylist; custom_strncpy(newAlbumPlaylist.name, purchasedPlaylistName, MAX_NAME_LEN); newAlbumPlaylist.songCount = 0;
        for (int i = 0; i < selectedAlbum.songCount; ++i) { // Add all songs from the purchased album to the new playlist.
            if (newAlbumPlaylist.songCount < MAX_SONGS) newAlbumPlaylist.songs[newAlbumPlaylist.songCount++] = selectedAlbum.songs[i];
            else { cout << "\tWarning: Playlist full for album." << endl; break; }
        }
        playlists[playlistCount++] = newAlbumPlaylist; // Add the new album playlist.
        appData->incrementAlbumSoldCount(selectedAlbum.albumName, selectedAlbum.artist); // Increment album's sold count.
        savePlaylists(); // Save changes.
        cout << "\n\tAlbum '" << selectedAlbum.albumName << "' purchased!" << endl;
        cout << "\tSongs added to playlist: '" << newAlbumPlaylist.name << "'." << endl;
    } else cout << "\n\tPayment failed." << endl;
    _getch(); system("cls");
}

// Simulates a payment gateway, records transaction details, and returns success/failure.
int User::processPayment(const char* username_payer, const char* itemName, const char* itemType, double amount, long long itemExpiry) {
    system("cls");
    cout << "\n\t|-------------------------------|" << endl;
    cout << "\t|         PAYMENT GATEWAY       |" << endl;
    cout << "\t|-------------------------------|" << endl;
    cout << "\n\tUser: " << username_payer << ", Item: " << itemName << " (" << itemType << "), Amount: RM" << fixed << setprecision(2) << amount << endl;
    cout << "\n\tPayment Method: 1. TNG eWallet, 2. VISA Card, 0. Cancel: "; int paymentChoice;
    cin >> paymentChoice; // Get payment method choice.
    if (cin.fail()) { cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); cout << "\tInvalid." << endl; Sleep(700); system("cls"); return 0; }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    if (paymentChoice == 0) { cout << "\tPayment canceled." << endl; Sleep(700); system("cls"); return 0; }
    
    PaymentRecord record; long long current_time = getCurrentUnixTime();
    sprintf(record.transactionId, "TX%lld-%s", current_time, username_payer); if(strlen(record.transactionId) >= MAX_NAME_LEN) record.transactionId[MAX_NAME_LEN-1] = '\0'; // Generate unique transaction ID.
    custom_strncpy(record.username, username_payer, MAX_NAME_LEN); custom_strncpy(record.itemName, itemName, MAX_NAME_LEN); custom_strncpy(record.itemType, itemType, sizeof(record.itemType)-1);
    record.amount = amount; record.timestamp = current_time; record.purchasedItemExpiry = itemExpiry; custom_strncpy(record.status, "PENDING", sizeof(record.status)-1); // Initialize record details.

    if (paymentChoice == 1) { // TNG eWallet.
        custom_strncpy(record.paymentMethod, "TNG_eWallet", MAX_NAME_LEN); char phoneNum[MAX_NAME_LEN]; cout << "\n\tTNG phone number: "; cin.getline(phoneNum, MAX_NAME_LEN);
        if(strlen(phoneNum) < 7) { cout << "\tInvalid phone. Failed." << endl; custom_strncpy(record.status, "FAILED", sizeof(record.status)-1); savePaymentRecord(&record); Sleep(700); system("cls"); return 0; }
        custom_strncpy(record.paymentDetails, phoneNum, sizeof(record.paymentDetails)-1); cout << "\n\tTNG processing..." << endl;
    } else if (paymentChoice == 2) { // VISA Card.
        custom_strncpy(record.paymentMethod, "VISA_Card", MAX_NAME_LEN); char cardNum[20], cvv[5]; cout << "\n\tVISA card (16 digits): "; cin.getline(cardNum, 20);
        if(strlen(cardNum) != 16) { cout << "\tInvalid card. Failed." << endl; custom_strncpy(record.status, "FAILED", sizeof(record.status)-1); savePaymentRecord(&record); Sleep(700); system("cls"); return 0; }
        cout << "\tCVV (3 digits): "; cin.getline(cvv, 5); if(strlen(cvv) != 3) { cout << "\tInvalid CVV. Failed." << endl; custom_strncpy(record.status, "FAILED", sizeof(record.status)-1); savePaymentRecord(&record); Sleep(700); system("cls"); return 0; }
        char maskedCard[MAX_NAME_LEN]; sprintf(maskedCard, "XXXX-XXXX-XXXX-%.4s", cardNum + 12); custom_strncpy(record.paymentDetails, maskedCard, sizeof(record.paymentDetails)-1); cout << "\n\tVISA processing..." << endl;
    } else { cout << "\tInvalid method. Failed." << endl; custom_strncpy(record.status, "FAILED", sizeof(record.status)-1); savePaymentRecord(&record); Sleep(700); system("cls"); return 0; }
    
    Sleep(1500); bool payment_successful = true; // Simulate processing time and success.
    if (payment_successful) {
        cout << "\tPayment successful!" << endl; custom_strncpy(record.status, "SUCCESS", sizeof(record.status)-1); savePaymentRecord(&record); return 1; } // Record success.
    else {
        cout << "\tPayment failed (simulated)." << endl; custom_strncpy(record.status, "FAILED", sizeof(record.status)-1); savePaymentRecord(&record); return 0; } // Record failure.
}

// Allows the user to create a new playlist and optionally add songs to it immediately.
void User::createPlaylist(AppData* appData) {
    system("cls");
    cout << "\n\t|-------------------------------|" << endl;
    cout << "\t|        CREATE PLAYLIST        |" << endl;
    cout << "\t|-------------------------------|" << endl;
    if (playlistCount >= MAX_PLAYLISTS) { cout << "\n\tMax playlists (" << MAX_PLAYLISTS << ") reached!" << endl; _getch(); system("cls"); return; }
    char playlistName[MAX_NAME_LEN];
    while (true) { // Loop until a valid, unique playlist name is entered.
        cout << "\n\tEnter new playlist name (or '0' to cancel): "; cin.getline(playlistName, MAX_NAME_LEN);
        if (strcmp(playlistName, "0") == 0) { cout << "\tCreation canceled." << endl; _getch(); system("cls"); return; }
        if (strlen(playlistName) == 0) { cout << "\tName cannot be empty." << endl; continue;}
        bool isDuplicate = false; for (int i=0; i<playlistCount; i++) if (strcmp(playlists[i].name, playlistName)==0) { isDuplicate=true; break; }
        if (isDuplicate) cout << "\tPlaylist '" << playlistName << "' already exists!" << endl; else break;
    }
    Playlist newPlaylist; custom_strncpy(newPlaylist.name, playlistName, MAX_NAME_LEN); newPlaylist.songCount = 0; // Initialize new playlist.
    char addSongsNowInput[10]; cout << "\n\tPlaylist '" << newPlaylist.name << "' created. Add songs now? (yes/no): ";
    while (true) { // Get user decision to add songs now.
        cin.getline(addSongsNowInput, 10); for(int i=0;addSongsNowInput[i]!='\0';i++) addSongsNowInput[i]=static_cast<char>(tolower(addSongsNowInput[i]));
        if (strcmp(addSongsNowInput, "yes")==0 || strcmp(addSongsNowInput, "no")==0) break; else cout << "\tInvalid. 'yes' or 'no': ";
    }
    if (strcmp(addSongsNowInput, "yes") == 0) { // If user wants to add songs.
        while (true) { // Loop to add multiple songs.
            if (newPlaylist.songCount >= MAX_SONGS) { cout << "\tPlaylist full." << endl; break; } // Check playlist song capacity.
            Song songToAdd; cout << "\n\t--- Adding Song " << newPlaylist.songCount + 1 << " to '" << newPlaylist.name << "' ---" << endl;
            // Get song details, with input validation.
            while(true) { cout << "\tSong name ('0' to finish): "; cin.getline(songToAdd.songName, MAX_NAME_LEN); if(strcmp(songToAdd.songName,"0")==0) goto end_song_addition; if(strlen(songToAdd.songName)==0)cout<<"\tEmpty."<<endl; else break; }
            while(true) { cout << "\tArtist name: "; cin.getline(songToAdd.artist, MAX_NAME_LEN); if(strlen(songToAdd.artist)==0)cout<<"\tEmpty."<<endl; else break; }
            int choice; bool valid;
            do { cout << "\tGenre ("; for(int i=0;i<appData->genreCount;i++)cout<<(i+1)<<"."<<appData->genres[i]<<(i==appData->genreCount-1?"":", "); cout<<"): "; cin>>choice; if(cin.fail()){cin.clear();cin.ignore(numeric_limits<streamsize>::max(),'\n');cout<<"\tInvalid."<<endl;valid=false;}else{cin.ignore(numeric_limits<streamsize>::max(),'\n');valid=(choice>=1&&choice<=appData->genreCount);if(!valid)cout<<"\tInvalid."<<endl;} }while(!valid); custom_strncpy(songToAdd.genre,appData->genres[choice-1],MAX_NAME_LEN);
            do { cout << "\tLanguage ("; for(int i=0;i<appData->languageCount;i++)cout<<(i+1)<<"."<<appData->languages[i]<<(i==appData->languageCount-1?"":", "); cout<<"): "; cin>>choice; if(cin.fail()){cin.clear();cin.ignore(numeric_limits<streamsize>::max(),'\n');cout<<"\tInvalid."<<endl;valid=false;}else{cin.ignore(numeric_limits<streamsize>::max(),'\n');valid=(choice>=1&&choice<=appData->languageCount);if(!valid)cout<<"\tInvalid."<<endl;} }while(!valid); custom_strncpy(songToAdd.language,appData->languages[choice-1],MAX_NAME_LEN); songToAdd.favoriteCount = 0;
            bool duplicateInNew=false; for(int k=0;k<newPlaylist.songCount;++k) if(strcmp(newPlaylist.songs[k].songName,songToAdd.songName)==0&&strcmp(newPlaylist.songs[k].artist,songToAdd.artist)==0){duplicateInNew=true;break;}
            if(duplicateInNew) cout << "\tSong already in new list." << endl; else { newPlaylist.songs[newPlaylist.songCount++]=songToAdd; cout << "\tSong added." << endl; } // Add song if not duplicate.
            char addAnother[10]; cout << "\n\tAdd another song? (yes/no): "; cin.getline(addAnother,10); for(int i=0;addAnother[i]!='\0';i++)addAnother[i]=static_cast<char>(tolower(addAnother[i]));
            while(strcmp(addAnother,"yes")!=0 && strcmp(addAnother,"no")!=0){cout<<"\tInvalid. 'yes' or 'no': "; cin.getline(addAnother,10); for(int i=0;addAnother[i]!='\0';i++)addAnother[i]=static_cast<char>(tolower(addAnother[i]));}
            if(strcmp(addAnother,"no")==0) break; // Exit song addition loop.
        }
    end_song_addition:; // Label for goto.
        if (newPlaylist.songCount > 0 && appData->recommendedSongCount > 0) { // Offer recommended songs if any were added.
            cout << "\n\t--- Recommended Songs ---" << endl; const Song& lastAdded = newPlaylist.songs[newPlaylist.songCount-1];
            int displayedRecsIndices[MAX_RECOMMENDED_SONGS], displayedRecCount = 0;
            // Display recommendations based on genre or language of the last added song.
            for (int i=0; i<appData->recommendedSongCount && displayedRecCount<5; i++) {
                bool inPlaylist=false; for(int k=0;k<newPlaylist.songCount;++k) if(strcmp(newPlaylist.songs[k].songName,appData->recommendedSongs[i].songName)==0 && strcmp(newPlaylist.songs[k].artist,appData->recommendedSongs[i].artist)==0){inPlaylist=true;break;}
                if (!inPlaylist && (strcmp(appData->recommendedSongs[i].genre,lastAdded.genre)==0 || strcmp(appData->recommendedSongs[i].language,lastAdded.language)==0)) {
                    cout << "\t" << (displayedRecCount + 1) << ". " << appData->recommendedSongs[i].songName << " by " << appData->recommendedSongs[i].artist << endl;
                    displayedRecsIndices[displayedRecCount++] = i;
                }
            }
            cout << "\t--------------------------" << endl;
            if (displayedRecCount > 0) {
                char addRecs[10]; cout << "\tAdd recommended? (yes/no): "; cin.getline(addRecs,10); for(int i=0;addRecs[i]!='\0';i++)addRecs[i]=static_cast<char>(tolower(addRecs[i]));
                if (strcmp(addRecs,"yes")==0) {
                    int recChoice; while(newPlaylist.songCount < MAX_SONGS) { // Loop to add chosen recommendations.
                        cout << "\tEnter rec # (1-"<<displayedRecCount<<", 0 to stop): "; cin>>recChoice; if(cin.fail()){cin.clear();cin.ignore(numeric_limits<streamsize>::max(),'\n');cout<<"\tInvalid."<<endl;continue;} cin.ignore(numeric_limits<streamsize>::max(),'\n');
                        if(recChoice==0)break; if(recChoice>=1 && recChoice<=displayedRecCount) {
                            Song& chosenRec = appData->recommendedSongs[displayedRecsIndices[recChoice-1]]; bool duplicate=false;
                            for(int k=0;k<newPlaylist.songCount;++k) if(strcmp(newPlaylist.songs[k].songName,chosenRec.songName)==0&&strcmp(newPlaylist.songs[k].artist,chosenRec.artist)==0){duplicate=true;break;}
                            if(duplicate)cout<<"\tAlready in list."<<endl; else {newPlaylist.songs[newPlaylist.songCount++]=chosenRec; appData->incrementFavoriteCount(chosenRec.songName,chosenRec.artist); cout<<"\tAdded."<<endl;}
                        } else cout << "\tInvalid rec #." << endl;
                        if(newPlaylist.songCount>=MAX_SONGS){cout<<"\tPlaylist full."<<endl;break;}
                    }
                }
            } else cout << "\tNo suitable recommendations." << endl;
        }
    }
    playlists[playlistCount++] = newPlaylist; // Add the newly created playlist to the user's list.
    cout << "\n\tPlaylist '" << newPlaylist.name << "' created with " << newPlaylist.songCount << " songs!" << endl;
    savePlaylists(); _getch(); system("cls");
}

// Displays a loading animation.
void laodingPage(int timer) {
    int code = 177, code2 = 219; // ASCII characters for progress bar.
    cout << "\t\t\tLoading..." << endl; cout << "\n\t\t\t";
    for(int i = 0; i < 25; i++) cout << static_cast<char>(code); // Background of progress bar.
    cout << "\r\t\t\t"; // Return cursor to start of line.
    for(int i = 0; i < 25; i++) { cout << static_cast<char>(code2); Sleep(timer); } // Fill progress bar.
    cout << " Done!" << endl; Sleep(200);
}

// Main function: Entry point of the Riffmix music system.
int main() {
    // Initial welcome message.
    cout << "\n\n\n\n\n\n\n";
    cout << "\t\t+-----------------------------------------+" << endl;
    cout << "\t\t|                                         |" << endl;
    cout << "\t\t|   Welcome to Riffmix Playlist system    |" << endl;
    cout << "\t\t|                                         |" << endl;
    cout << "\t\t+-----------------------------------------+" << endl;
    cout << "\t\t  \t\t----Enjoy High-Quality Music Experience----" << endl;
    Sleep(1500);
    system("cls");
    laodingPage(20); // Show loading animation.
    system("cls");

    AppData appData; // Initializes application data (genres, languages, songs, albums).
    SubscriptionManager subManager; // Manages user subscriptions using custom hash table.
    User user; // Manages current user's playlists and actions.

    char enteredUsername[MAX_NAME_LEN];
    cout << "\n\n\t+-----------------------------------------+" << endl;
    cout << "\t|           RIFFMIX MUSIC SYSTEM          |" << endl;
    cout << "\t+-----------------------------------------+" << endl;
    cout << "\tPlease enter your username: ";
    cin.getline(enteredUsername, MAX_NAME_LEN); // Get username.
    while (strlen(enteredUsername) == 0) { // Validate username input.
        cout << "\tUsername cannot be empty. Please enter your username: ";
        cin.getline(enteredUsername, MAX_NAME_LEN);
    }
    user.setCurrentUsername(enteredUsername); // Set current user.
    bool newUserCreated = subManager.ensureUserExists(enteredUsername, true); // Check/create user account in hash table.
    if (!newUserCreated) {
        cout << "\n\tWelcome back, " << user.currentUsername << "!" << endl;
        Sleep(1000);
    }
    user.loadPlaylists(); // Load playlists for the current user.
    system("cls");

    int choice;
    char decision_input[10];
    while (true) { // Main application loop (User Menu).
        // Display user menu options.
        cout << "\t+++++++++++++++++++++++++++++++++++++++++" << endl;
        cout << "\t|         RIFFMIX - USER MENU           |" << endl;
        cout << "\t|---------------------------------------|" << endl;
        cout << "\t|    1. Create Playlist                 |" << endl;
        cout << "\t|    2. Search Playlist                 | (Team B: String Search)" << endl;
        cout << "\t|    3. Update Playlist                 |" << endl;
        cout << "\t|    4. Add Song to Playlist            |" << endl;
        cout << "\t|    5. Delete Song from Playlist       |" << endl;
        cout << "\t|    6. Delete Playlist                 |" << endl;
        cout << "\t|    7. Display All Songs               | (Team B: Binggo Sort)" << endl;
        cout << "\t|    8. Merge Playlists                 |" << endl;
        cout << "\t|    9. Import Playlist                 |" << endl;
        cout << "\t|   10. Download Playlist               |" << endl;
        cout << "\t|   11. Play Playlist                   | (Team A: Linked Queue)" << endl;
        cout << "\t|   12. Purchase Subscription           | (Team A: Division method-separate chaining)" << endl;
        cout << "\t|   13. Purchase Digital Album          | (Team A: Division method-separate chaining)" << endl;
        cout << "\t|   14. View Purchase History           |" << endl;
        cout << "\t|   15. Switch User                     |" << endl;
        cout << "\t|   16. Exit Riffmix                    |" << endl;
        cout << "\t+++++++++++++++++++++++++++++++++++++++++" << endl;
        cout << "\n\tCurrent User: " << user.currentUsername << endl;
        cout << "\n\tEnter your choice (1-16): ";
        cin >> choice; // Get user's menu choice.
        if (cin.fail()) { // Handle invalid input.
            cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "\n\tInvalid input! Please enter a number (1-16)." << endl;
            Sleep(1000); system("cls"); continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (choice == 16) { // Handle exit choice.
            cout << "\n\tAre you sure you want to Exit Riffmix? (yes/no): ";
            cin.getline(decision_input, 10);
            for (int i = 0; decision_input[i] != '\0'; i++) decision_input[i] = static_cast<char>(tolower(decision_input[i]));
            if (strcmp(decision_input, "yes") == 0) {
                user.savePlaylists(); // Save current user's playlists before exiting.
                cout << "\n\tSaving data... Exiting Riffmix. Goodbye!" << endl;
                Sleep(1000);
                return 0; // Exit the program.
            } else {
                cout << "\n\tReturning to main menu." << endl;
                Sleep(700); system("cls"); continue; // Return to menu.
            }
        }
        
        if (choice < 1 || choice > 15) { // Validate menu choice range.
            cout << "\n\tInvalid choice! Please select a valid option (1-16)." << endl;
            Sleep(1000); system("cls"); continue;
        }

        // Switch case to handle different menu options.
        switch (choice) {
            case 1: user.createPlaylist(&appData); break;
            case 2: user.searchPlaylist(); break; // Implemented with custom string search.
            case 3: user.updatePlaylist(); break;
            case 4: user.addSong(&appData); break;
            case 5: user.deleteSong(); break;
            case 6: user.deletePlaylist(); break;
            case 7: displayAllSongsInPlaylists(&user, &appData); break; // Displays with sorting options (Team B).
            case 8: mergePlaylistsByGenreOrLanguage(&user, &appData); break;
            case 9: user.importPlaylist(); break;
            case 10: user.downloadPlaylist(); break;
            case 11: user.playPlaylist(&subManager, &appData); break; // Uses custom queue for playback (Team B).
            case 12: user.purchasePackage(&subManager, &appData); break;
            case 13: user.purchaseDigitalAlbum(&subManager, &appData); break;
            case 14: user.viewPurchaseHistory(); break;
            case 15: // Switch user functionality.
                user.savePlaylists(); // Save current user's data.
                user.playNextQueue.clear(); // Clear playback queue for new user.
                cout << "\n\tCurrent user '" << user.currentUsername << "' data saved." << endl;
                cout << "\tEnter new username to switch to: ";
                cin.getline(enteredUsername, MAX_NAME_LEN);
                while (strlen(enteredUsername) == 0) {
                    cout << "\tUsername cannot be empty. Please enter new username: ";
                    cin.getline(enteredUsername, MAX_NAME_LEN);
                }
                user.setCurrentUsername(enteredUsername); // Set new current user.
                newUserCreated = subManager.ensureUserExists(enteredUsername, true); // Check/create new user.
                if (!newUserCreated) {
                    cout << "\n\tSwitched to user: " << user.currentUsername << ". Welcome back!" << endl;
                } else {
                    cout << "\n\tSwitched to new user: " << user.currentUsername << "." << endl;
                }
                user.loadPlaylists(); // Load playlists for the new user.
                Sleep(1500);
                system("cls");
                break;
            default:
                cout << "\n\tError: Unexpected choice. Returning to menu." << endl;
                Sleep(700); system("cls"); break;
        }
    }
    return 0;
}
