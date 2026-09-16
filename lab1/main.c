#include <stdio.h>
#include <string.h>

#include "include/cipher.h"


#define BUFFER_SIZE 1024


int main(void)
{
    char text[BUFFER_SIZE];
    char encrypted[BUFFER_SIZE];
    char decrypted[BUFFER_SIZE];

    int caesar_key;
    char vigenere_key[BUFFER_SIZE];


    printf("=== Caesar and Vigenere Cipher ===\n\n");

    printf("Enter text for Caesar cipher: ");
    fgets(text, BUFFER_SIZE, stdin);

    /* Убираем символ переноса строки */
    text[strcspn(text, "\n")] = '\0';


    printf("Enter Caesar key: ");
    scanf("%d", &caesar_key);

    /*
     * После scanf в stdin остается '\n'.
     * Убираем его перед следующим fgets.
     */
    getchar();


    if (caesar_encrypt(text, encrypted, caesar_key) != 0)
    {
        printf("Caesar encryption error.\n");
        return 1;
    }

    if (caesar_decrypt(encrypted, decrypted, caesar_key) != 0)
    {
        printf("Caesar decryption error.\n");
        return 1;
    }


    printf("\n--- Caesar ---\n");
    printf("Original:  %s\n", text);
    printf("Encrypted: %s\n", encrypted);
    printf("Decrypted: %s\n", decrypted);


    /*
     * =========================
     * ВИЖЕНЕР
     * =========================
     */

    printf("\nEnter text for Vigenere cipher: ");
    fgets(text, BUFFER_SIZE, stdin);

    text[strcspn(text, "\n")] = '\0';


    printf("Enter Vigenere key: ");
    fgets(vigenere_key, BUFFER_SIZE, stdin);

    vigenere_key[strcspn(vigenere_key, "\n")] = '\0';


    if (vigenere_encrypt(text, encrypted, vigenere_key) != 0)
    {
        printf("Vigenere encryption error.\n");
        return 1;
    }

    if (vigenere_decrypt(encrypted, decrypted, vigenere_key) != 0)
    {
        printf("Vigenere decryption error.\n");
        return 1;
    }


    printf("\n--- Vigenere ---\n");
    printf("Original:  %s\n", text);
    printf("Encrypted: %s\n", encrypted);
    printf("Decrypted: %s\n", decrypted);


    return 0;
}