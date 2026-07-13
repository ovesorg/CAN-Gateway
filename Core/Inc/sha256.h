
#ifndef SHA256_H
#define SHA256_H


#include <stdint.h>



#define Oves_crypto_HASH_ALG_INVALID     0x00        //!< Invalid hashing algorithm.
#define Oves_crypto_HASH_ALG_MD5         0x01        //!< MD5.
#define Oves_crypto_HASH_ALG_SHA0        0x02        //!< SHA-0.
#define Oves_crypto_HASH_ALG_SHA1        0x03        //!< SHA-1.
#define Oves_crypto_HASH_ALG_SHA224      0x04        //!< SHA-224 (SHA-2).
#define Oves_crypto_HASH_ALG_SHA256      0x05        //!< SHA-256 (SHA-2).
#define Oves_crypto_HASH_ALG_SHA384      0x06        //!< SHA-384 (SHA-2).
#define Oves_crypto_HASH_ALG_SHA512      0x07        //!< SHA-512 (SHA-2).


#define HASH_ERROR_BASE_NUM      (0x0)       ///< Global error base
#define HASH_ERROR_SDM_BASE_NUM  (0x1000)    ///< SDM error base
#define HASH_ERROR_SOC_BASE_NUM  (0x2000)    ///< SoC error base
#define HASH_ERROR_STK_BASE_NUM  (0x3000)    ///< STK error base
/** @} */

#define HASH_SUCCESS                           (HASH_ERROR_BASE_NUM + 0)  ///< Successful command
#define HASH_ERROR_SVC_HANDLER_MISSING         (HASH_ERROR_BASE_NUM + 1)  ///< SVC handler is missing
#define HASH_ERROR_SOFTDEVICE_NOT_ENABLED      (HASH_ERROR_BASE_NUM + 2)  ///< SoftDevice has not been enabled
#define HASH_ERROR_INTERNAL                    (HASH_ERROR_BASE_NUM + 3)  ///< Internal Error
#define HASH_ERROR_NO_MEM                      (HASH_ERROR_BASE_NUM + 4)  ///< No Memory for operation
#define HASH_ERROR_NOT_FOUND                   (HASH_ERROR_BASE_NUM + 5)  ///< Not found
#define HASH_ERROR_NOT_SUPPORTED               (HASH_ERROR_BASE_NUM + 6)  ///< Not supported
#define HASH_ERROR_INVALID_PARAM               (HASH_ERROR_BASE_NUM + 7)  ///< Invalid Parameter
#define HASH_ERROR_INVALID_STATE               (HASH_ERROR_BASE_NUM + 8)  ///< Invalid state, operation disallowed in this state
#define HASH_ERROR_INVALID_LENGTH              (HASH_ERROR_BASE_NUM + 9)  ///< Invalid Length
#define HASH_ERROR_INVALID_FLAGS               (HASH_ERROR_BASE_NUM + 10) ///< Invalid Flags
#define HASH_ERROR_INVALID_DATA                (HASH_ERROR_BASE_NUM + 11) ///< Invalid Data
#define HASH_ERROR_DATA_SIZE                   (HASH_ERROR_BASE_NUM + 12) ///< Invalid Data size
#define HASH_ERROR_TIMEOUT                     (HASH_ERROR_BASE_NUM + 13) ///< Operation timed out
#define HASH_ERROR_NULL                        (HASH_ERROR_BASE_NUM + 14) ///< Null Pointer
#define HASH_ERROR_FORBIDDEN                   (HASH_ERROR_BASE_NUM + 15) ///< Forbidden Operation
#define HASH_ERROR_INVALID_ADDR                (HASH_ERROR_BASE_NUM + 16) ///< Bad Memory Address
#define HASH_ERROR_BUSY                        (HASH_ERROR_BASE_NUM + 17) ///< Busy
#define HASH_ERROR_CONN_COUNT                  (HASH_ERROR_BASE_NUM + 18) ///< Maximum connection count exceeded.
#define HASH_ERROR_RESOURCES                   (HASH_ERROR_BASE_NUM + 19) ///< Not enough resources for operation


typedef struct
{
    uint8_t    p_le_data[128];       /**< Pointer to the key data in little-endian format. */
    uint32_t   len;              /**< Length of the key data in bytes. */
} Oves_crypto_key_t;


#define VERIFY_FALSE(statement, err_code)   \
do                                          \
{                                           \
    if ((statement))                        \
    {                                       \
        return err_code;                    \
    }                                       \
} while (0)


/**@brief Macro for verifying statement to be false. It will cause the exterior function to return
 *        if the statement is not false.
 *
 * @param[in]   statement    Statement to test.
 */
#define VERIFY_FALSE_VOID(statement) VERIFY_FALSE((statement), )

#define DISABLE_PARAM_CHECK

#ifdef DISABLE_PARAM_CHECK
#define VERIFY_PARAM_NOT_NULL()
#else
#define VERIFY_PARAM_NOT_NULL(param) VERIFY_FALSE(((param) == NULL), HASH_ERROR_NULL)
#endif /* DISABLE_PARAM_CHECK */



#ifdef DISABLE_PARAM_CHECK
#define VERIFY_PARAM_NOT_NULL_VOID()
#else
#define VERIFY_PARAM_NOT_NULL_VOID(param) VERIFY_FALSE_VOID(((param) == NULL))
#endif /* DISABLE_PARAM_CHECK */


typedef uint32_t ret_code_t;

/**@brief Current state of a hash operation.
 */
typedef struct {
    uint8_t data[64];
    uint32_t datalen;
    uint64_t bitlen;
    uint32_t state[8];
} sha256_context_t;


/**@brief Function for initializing a @ref sha256_context_t instance.
 *
 * @param[out] ctx  Context instance to be initialized.
 *
 * @retval HASH_SUCCESS     If the instance was successfully initialized.
 * @retval HASH_ERROR_NULL  If the parameter was NULL.
 */
ret_code_t sha256_init(sha256_context_t *ctx);

/**@brief Function for calculating the hash of an array of uint8_t data.
 *
 * @details This function can be called multiple times in sequence. This is equivalent to calling
 *          the function once on a concatenation of the data from the different calls.
 *
 * @param[in,out] ctx   Hash instance.
 * @param[in]     data  Data to be hashed.
 * @param[in]     len   Length of the data to be hashed.
 *
 * @retval HASH_SUCCESS     If the data was successfully hashed.
 * @retval HASH_ERROR_NULL  If the ctx parameter was NULL or the data parameter was NULL,  while the len parameter was not zero.
 */
ret_code_t sha256_update(sha256_context_t *ctx, const uint8_t * data, const size_t len);

/**@brief Function for extracting the hash value from a hash instance.
 *
 * @details This function should be called after all data to be hashed has been passed to the hash
 *          instance (by one or more calls to @ref sha256_update).
 *
 * Do not call @ref sha256_update again after @ref sha256_final has been called.
 *
 * @param[in,out] ctx   Hash instance.
 * @param[out]    hash  Array to hold the extracted hash value (assumed to be 32 bytes long).
 * @param[in]     le   Store the hash in little-endian.
 *
 * @retval HASH_SUCCESS     If the has value was successfully extracted.
 * @retval HASH_ERROR_NULL  If a parameter was NULL.
 */
ret_code_t sha256_final(sha256_context_t *ctx, uint8_t * hash, uint8_t le);

uint32_t Oves_crypto_hash_compute(uint32_t hash_alg,uint8_t const *p_data,uint32_t len,Oves_crypto_key_t *p_hash);
							
							
							



#endif   // SHA256_H


