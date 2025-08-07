// Does the code need to be improved?

#include "License.h"
#include "EEPROM.h"
#include "aes.h"
#include "main.h"
#include "md5.h"
#include "sha256.h"

#include <string.h>

#define UNIQUE_ID_LENGTH 12

// Global variables for license management
int8_t LicenseStatus = FAKE; // License status (-1: fake, 1: valid, etc.)
uint8_t LicenseTry = 0;      // Number of license verification attempts
uint32_t LicenseTimer = 0;   // Timer for license verification

// extern FSM_STATE_t FSM_Load_Config;  // External state machine configuration

//***************************************************************

// Encryption keys and IVs
const uint8_t private_passwords[8][16] = {
    {0x1A, 0x82, 0x6D, 0xFC, 0xBC, 0x65, 0x9D, 0x0D, 0x35, 0x97, 0x56, 0x39, 0x47, 0xAB, 0xD5, 0x20},
    {0xF5, 0x64, 0xD1, 0x96, 0xBB, 0xD7, 0x85, 0xDD, 0xD1, 0x8C, 0xBC, 0x3E, 0xEF, 0xFC, 0xDB, 0xEF},
    {0xA7, 0x9F, 0x7D, 0xD6, 0x25, 0x9F, 0xBB, 0x84, 0x68, 0x4E, 0xC3, 0x34, 0x1B, 0x5C, 0x9D, 0xA1},
    {0x34, 0xCE, 0x6A, 0x2E, 0xA6, 0x43, 0x3C, 0x2D, 0xB5, 0xAF, 0x9D, 0x4D, 0x16, 0xC1, 0xFB, 0x43},
    {0x65, 0x3F, 0x5C, 0xC8, 0xAF, 0x10, 0x7C, 0x97, 0x32, 0xAA, 0x1E, 0x02, 0xCD, 0x08, 0xA6, 0x47},
    {0xDA, 0x1D, 0x75, 0x9B, 0xB4, 0xD0, 0x51, 0x1B, 0x80, 0x38, 0x94, 0x00, 0xE4, 0x40, 0x0F, 0xBF},
    {0xAB, 0x20, 0x35, 0x02, 0xB6, 0x22, 0x5C, 0x44, 0x7A, 0x99, 0x45, 0x16, 0xE2, 0x1E, 0xB4, 0xE5},
    {0xE9, 0xD6, 0x72, 0xB1, 0x98, 0x37, 0xA8, 0x21, 0xEB, 0xCC, 0x70, 0x29, 0x7C, 0x50, 0xD0, 0xAE},
};

const uint8_t public_passwords[8][16] = {
    {0xBA, 0x61, 0xD6, 0x65, 0xC1, 0x29, 0xF2, 0x4E, 0x39, 0xEE, 0x27, 0x57, 0x0F, 0x1D, 0xBC, 0xCA},
    {0xE4, 0xA1, 0x5C, 0x76, 0x8F, 0x27, 0xD2, 0xC5, 0x7F, 0x9B, 0x59, 0x4D, 0x2C, 0x2F, 0xCD, 0xB0},
    {0x8C, 0xB9, 0x3D, 0x35, 0xD6, 0xBA, 0x8B, 0xAD, 0x0B, 0x1D, 0xF7, 0xEA, 0x0A, 0x6B, 0x04, 0x2C},
    {0x47, 0x43, 0xA6, 0xB4, 0x1C, 0xAC, 0x63, 0x26, 0xDA, 0x26, 0xFD, 0xE3, 0xCC, 0x4A, 0xDA, 0x7B},
    {0xE9, 0xA0, 0x53, 0xBF, 0x59, 0x85, 0xAF, 0x84, 0xD6, 0xF8, 0x70, 0x43, 0x1E, 0x2A, 0x4E, 0x01},
    {0xD5, 0x51, 0xEB, 0xE6, 0xD2, 0x0A, 0x64, 0xCD, 0xF8, 0x9D, 0x3E, 0x21, 0x10, 0x49, 0x96, 0x16},
    {0x4B, 0x63, 0x0A, 0xAB, 0x06, 0x3E, 0x93, 0xB6, 0x56, 0x2D, 0x0A, 0x4A, 0x6B, 0x32, 0x43, 0x7F},
    {0x75, 0xB2, 0x95, 0x0B, 0xBC, 0xDA, 0xD2, 0x92, 0x76, 0x96, 0xE0, 0x83, 0x98, 0x35, 0xBA, 0xC1},
};

const uint8_t IVs[8][12] = {
    {0xA3, 0x8A, 0xF6, 0xA3, 0x90, 0x52, 0x7B, 0x11, 0x08, 0xC5, 0x21, 0xB1},
    {0x47, 0xBF, 0xE5, 0xAE, 0xCA, 0xB0, 0x2C, 0x7B, 0x01, 0x16, 0x7F, 0xC1},
    {0xA6, 0x0F, 0xB4, 0xD0, 0xBC, 0xCD, 0x04, 0xDA, 0x4E, 0x96, 0x43, 0xBA},
    {0xD4, 0x1D, 0x8C, 0xD9, 0x8F, 0xA7, 0xB2, 0x04, 0xE9, 0x80, 0x09, 0x98},
    {0xDB, 0x9D, 0xFB, 0xD5, 0x08, 0x40, 0x1E, 0x50, 0x1D, 0x2F, 0x6B, 0x4A},
    {0x2A, 0x49, 0x5C, 0x75, 0x35, 0x72, 0xD6, 0x5C, 0x3D, 0xA6, 0x53, 0xEB},
    {0x0A, 0x95, 0xD1, 0x7D, 0x6A, 0x6D, 0x3F, 0x1D, 0x20, 0xD6, 0x61, 0xA1},
    {0xB9, 0x43, 0xA0, 0x51, 0x63, 0xB2, 0x64, 0x9F, 0xCE, 0x82, 0x56, 0x84},
};

//***************************************************************

// Public key and IV for AES encryption and decryption
uint8_t publicKey[32];
uint8_t publicIv[16];

//***************************************************************

// Function prototypes
void create_key_iv(const uint8_t *key, const uint8_t *iv);
int read_and_decrypt_unique_id(uint8_t *decrypted_id, size_t len);
void shiftArrayRight(uint8_t arr[], int length);

//***************************************************************

// Initialize EEPROM and create encryption key and IV
// void Enter_Check_License(void) {

//  LicenseStatus = FAKE;  // Set license status to Fake
//}

//***************************************************************

// Check if the unique ID stored in EEPROM matches the device's unique ID
uint8_t Check_License(void)
{
    uint8_t unique[UNIQUE_ID_LENGTH];            // Device's unique ID
    uint8_t EEPROM_Unique[UNIQUE_ID_LENGTH + 2]; // Unique ID read from EEPROM

    LicenseStatus = FAKE;

    // Read and decrypt the unique ID from EEPROM
    if (read_and_decrypt_unique_id(EEPROM_Unique, UNIQUE_ID_LENGTH + 2) == 0)
    {
        // Copy the device's unique ID
        memcpy(unique, (void *)(UID_BASE), UNIQUE_ID_LENGTH);

        // Check if the IDs match
        if (memcmp(unique, &EEPROM_Unique[2], UNIQUE_ID_LENGTH) == 0)
        {
            LicenseStatus = VALID; // Set license status to valid
            // FSMTransitionTo(&FSM_Load_Config);  // Transition to the next state
        }
    }

    return LicenseStatus;
}

//***************************************************************

// Read and decrypt the unique ID from EEPROM
int read_and_decrypt_unique_id(uint8_t *decrypted_id, size_t len)
{
    uint8_t eeprom_encrypted_id[ENCRYPTED_ID_LENGTH + ENCRYPTED_ALGORITHM_LENGTH]; // Encrypted unique ID from EEPROM
    struct AES_ctx ctx;                                                            // AES context

    // Read the encrypted unique ID from EEPROM
    if (EEPROM_Read(EEPROM_START_ADDR, eeprom_encrypted_id, ENCRYPTED_ID_LENGTH + ENCRYPTED_ALGORITHM_LENGTH) != HAL_OK)
    {
        return -1; // Return -1 if read failed
    }

    uint8_t password[16], iv[12];

    // Sanitize the level and shift parameters
    uint8_t level = eeprom_encrypted_id[ENCRYPTED_ID_LENGTH] >> 4;
    uint8_t shift = eeprom_encrypted_id[ENCRYPTED_ID_LENGTH] & 0x0F;

    level = (level < 8) ? level : 0;

    // Copy the password and IV corresponding to the given level
    memcpy(password, private_passwords[level], 16);
    memcpy(iv, IVs[level], 12);

    // If a shift value is provided, rotate the password and IV arrays
    if (shift)
    {
        for (uint8_t n = 0; n < shift; n++)
        {
            shiftArrayRight(password, 16);
            shiftArrayRight(iv, 12);
        }
    }

    // Create the key and IV for AES
    create_key_iv(password, iv);

    // Initialize AES context with the public key and IV
    AES_init_ctx_iv(&ctx, publicKey, publicIv);

    // Decrypt the unique ID
    AES_CBC_decrypt_buffer(&ctx, eeprom_encrypted_id, ENCRYPTED_ID_LENGTH);

    // Copy the decrypted unique ID to the output variable
    memcpy(decrypted_id, eeprom_encrypted_id, len);

    memset(publicKey, 0x00, sizeof(publicKey));
    memset(publicIv, 0x00, sizeof(publicIv));

    return 0; // Return 0 for success
}

//***************************************************************

// Sets the primary license by storing the encrypted unique ID in EEPROM
void License_SetPrimary(uint8_t level, uint8_t shift)
{
    uint8_t EEPROM_Unique[ENCRYPTED_ID_LENGTH +
                          ENCRYPTED_ALGORITHM_LENGTH]; // Array to hold the encrypted unique ID for EEPROM

    License_UniqueEncryption(EEPROM_Unique, level, shift, false);

    // Write the encrypted unique ID to EEPROM starting at address 0
    EEPROM_Write(EEPROM_START_ADDR, EEPROM_Unique, ENCRYPTED_ID_LENGTH + ENCRYPTED_ALGORITHM_LENGTH);
}

//***************************************************************
void License_UniqueEncryption(uint8_t *data, uint8_t level, uint8_t shift, bool isPublic)
{
    struct AES_ctx ctx; // AES encryption context

    uint8_t password[16];
    uint8_t iv[12];

    // Sanitize the level and shift parameters
    level = (level < 8) ? level : 0;
    shift = shift & 0x0F;

    // Copy the password and IV corresponding to the given index
    if (isPublic)
    {
        memcpy(password, public_passwords[level], 16);
    }
    else
    {
        memcpy(password, private_passwords[level], 16);
    }
    memcpy(iv, IVs[level], 12);

    // If a shift value is provided, rotate the password and IV arrays
    if (shift)
    {
        for (uint8_t n = 0; n < shift; n++)
        {
            shiftArrayRight(password, 16);
            shiftArrayRight(iv, 12);
        }
    }

    // Create a password setting byte with the index and shift value
    uint8_t passwordSetting = (level << 4) | shift;

    // Create the key and initialization vector (IV) for AES encryption
    create_key_iv(password, iv);

    memcpy((void *)&data[0], (const void *)MANUFACTURER_CODE, 2); // TR or CN factory

    // Copy the device's unique ID to the EEPROM_Unique array
    memcpy((void *)&data[2], (const void *)((uint32_t *)(UID_BASE)), 12);

    // Append the password setting byte to the EEPROM_Unique array
    data[16] = passwordSetting;

    // Initialize the AES context with the public key and IV
    AES_init_ctx_iv(&ctx, publicKey, publicIv);

    // Encrypt the unique ID using AES in CBC mode
    AES_CBC_encrypt_buffer(&ctx, data, 16);

    // Clear the public key and IV for security reasons
    memset(publicKey, 0x00, sizeof(publicKey));
    memset(publicIv, 0x00, sizeof(publicIv));
}
//***************************************************************

bool License_Check_Unique_HASH(uint8_t *data)
{
    bool result = false;
    uint8_t Unique[12];
    uint8_t buf[SHA256_BLOCK_SIZE];
    SHA256_CTX sha;

    // Copy the device's unique ID to the Unique array
    memcpy((void *)Unique, (const void *)((uint32_t *)(UID_BASE)), 12);

    sha256_init(&sha);
    sha256_update(&sha, Unique, 12);
    sha256_final(&sha, buf);

    if (memcmp(data, buf, SHA256_BLOCK_SIZE) == 0)
        result = true;

    return result;
}

//***************************************************************

// Create the public key and IV by hashing the input key and IV
void create_key_iv(const uint8_t *key, const uint8_t *iv)
{
    SHA256_CTX sha;
    MD5_CTX md5;

    sha256_init(&sha);
    sha256_update(&sha, key, 16);
    sha256_final(&sha, publicKey);

    md5_init(&md5);
    md5_update(&md5, iv, 12);
    md5_final(&md5, publicIv);
}

//***************************************************************

void shiftArrayRight(uint8_t arr[], int length)
{
    uint8_t last = arr[length - 1];

    for (int i = length - 1; i > 0; i--)
    {
        arr[i] = arr[i - 1];
    }

    arr[0] = last;
}

//***************************************************************
